#include "npdatabase/impl/blob_manager.hpp"
#include "npdatabase/impl/buffer_pool_manager.hpp"
#include "npdatabase/impl/page.hpp"
#include <cstring>
#include <iostream>
#include <cassert>

BlobManager::BlobManager(BufferPoolManager* buffer_pool) 
    : buffer_pool_(buffer_pool), next_blob_id_(1) {
    if (!buffer_pool_) {
        throw std::invalid_argument("BufferPoolManager cannot be null");
    }
}

uint32_t BlobManager::StoreBlob(const void* data, size_t size) {
    if (!data || size == 0) {
        throw std::invalid_argument("Invalid blob data or size");
    }

    // Generate unique blob ID
    uint32_t blob_id = next_blob_id_.fetch_add(1);

    // Calculate how many pages we need
    uint32_t num_pages = CalculateRequiredPages(size);

    // Allocate a chain of pages
    std::vector<uint32_t> page_ids = AllocatePageChain(num_pages);

    try {
        // Write the blob data across the page chain
        WritePageChain(page_ids, blob_id, data, size);

        // Cache the metadata
        {
            std::lock_guard<std::mutex> lock(cache_mutex_);
            blob_cache_[blob_id] = BlobMetadata(blob_id, static_cast<uint32_t>(size), 
                                              page_ids[0], num_pages);
        }

        return blob_id;

    } catch (const std::exception& e) {
        // Clean up on failure
        FreePageChain(page_ids);
        throw;
    }
}

std::vector<uint8_t> BlobManager::GetBlob(uint32_t blob_id) {
    // First try to get metadata from cache
    BlobMetadata metadata;
    {
        std::lock_guard<std::mutex> lock(cache_mutex_);
        auto it = blob_cache_.find(blob_id);
        if (it == blob_cache_.end()) {
            throw std::runtime_error("Blob not found: " + std::to_string(blob_id));
        }
        metadata = it->second;
    }

    // Read the page chain
    std::vector<uint32_t> page_ids = ReadPageChain(metadata.first_page_id);

    // Prepare result vector
    std::vector<uint8_t> result;
    result.reserve(metadata.total_size);

    // Read data from each page in the chain
    for (size_t i = 0; i < page_ids.size(); ++i) {
        Page* page = buffer_pool_->FetchPage(page_ids[i]);
        if (!page) {
            throw std::runtime_error("Failed to fetch page: " + std::to_string(page_ids[i]));
        }

        // Read the overflow page header
        auto* header = reinterpret_cast<OverflowPageHeader*>(page->data_.data());

        // Verify this is a valid overflow page
        if (!header->IsValid() || header->blob_id != blob_id) {
            buffer_pool_->UnpinPage(page_ids[i], false);
            throw std::runtime_error("Invalid overflow page");
        }

        // Copy data from this page
        const uint8_t* page_data = page->data_.data() + sizeof(OverflowPageHeader);
        result.insert(result.end(), page_data, page_data + header->data_size);

        // Unpin the page
        buffer_pool_->UnpinPage(page_ids[i], false);
    }

    return result;
}

bool BlobManager::DeleteBlob(uint32_t blob_id) {
    // Get metadata from cache
    BlobMetadata metadata;
    {
        std::lock_guard<std::mutex> lock(cache_mutex_);
        auto it = blob_cache_.find(blob_id);
        if (it == blob_cache_.end()) {
            return false;  // Blob doesn't exist
        }
        metadata = it->second;
        blob_cache_.erase(it);
    }

    try {
        // Read the page chain
        std::vector<uint32_t> page_ids = ReadPageChain(metadata.first_page_id);

        // Free all pages
        FreePageChain(page_ids);

        return true;

    } catch (const std::exception& e) {
        // Re-add to cache if deletion failed
        {
            std::lock_guard<std::mutex> lock(cache_mutex_);
            blob_cache_[blob_id] = metadata;
        }
        return false;
    }
}

uint32_t BlobManager::GetBlobFirstPage(uint32_t blob_id) {
    std::lock_guard<std::mutex> lock(cache_mutex_);
    auto it = blob_cache_.find(blob_id);
    if (it == blob_cache_.end()) {
        throw std::runtime_error("Blob not found: " + std::to_string(blob_id));
    }
    return it->second.first_page_id;
}

size_t BlobManager::GetBlobSize(uint32_t blob_id) {
    std::lock_guard<std::mutex> lock(cache_mutex_);
    auto it = blob_cache_.find(blob_id);
    if (it == blob_cache_.end()) {
        throw std::runtime_error("Blob not found: " + std::to_string(blob_id));
    }
    return it->second.total_size;
}

size_t BlobManager::GetBlobCount() const {
    std::lock_guard<std::mutex> lock(cache_mutex_);
    return blob_cache_.size();
}

void BlobManager::PrintBlobInfo(uint32_t blob_id) const {
    std::lock_guard<std::mutex> lock(cache_mutex_);
    auto it = blob_cache_.find(blob_id);
    if (it == blob_cache_.end()) {
        std::cout << "Blob " << blob_id << " not found\n";
        return;
    }

    const auto& metadata = it->second;
    std::cout << "Blob ID: " << metadata.blob_id << "\n"
              << "Total Size: " << metadata.total_size << " bytes\n"
              << "First Page: " << metadata.first_page_id << "\n"
              << "Number of Pages: " << metadata.num_pages << "\n";
}

// Private helper methods

uint32_t BlobManager::CalculateRequiredPages(size_t blob_size) const {
    return static_cast<uint32_t>((blob_size + OVERFLOW_DATA_SIZE - 1) / OVERFLOW_DATA_SIZE);
}

std::vector<uint32_t> BlobManager::AllocatePageChain(uint32_t num_pages) {
    std::vector<uint32_t> page_ids;
    page_ids.reserve(num_pages);

    for (uint32_t i = 0; i < num_pages; ++i) {
        uint32_t page_id;
        Page* page = buffer_pool_->NewPage(page_id);
        if (!page) {
            // Clean up allocated pages on failure
            for (uint32_t allocated_id : page_ids) {
                buffer_pool_->DeletePage(allocated_id);
            }
            throw std::runtime_error("Failed to allocate page for blob");
        }

        page_ids.push_back(page_id);

        // Unpin the page for now
        buffer_pool_->UnpinPage(page_id, false);
    }

    return page_ids;
}

void BlobManager::WritePageChain(const std::vector<uint32_t>& page_ids, 
                                uint32_t blob_id, 
                                const void* data, 
                                size_t size) {
    const uint8_t* byte_data = static_cast<const uint8_t*>(data);
    size_t bytes_written = 0;

    for (size_t i = 0; i < page_ids.size(); ++i) {
        Page* page = buffer_pool_->FetchPage(page_ids[i]);
        if (!page) {
            throw std::runtime_error("Failed to fetch page for writing: " + std::to_string(page_ids[i]));
        }

        // Calculate how much data to write to this page
        size_t remaining = size - bytes_written;
        size_t to_write = std::min(remaining, OVERFLOW_DATA_SIZE);

        // Set up the header
        auto* header = reinterpret_cast<OverflowPageHeader*>(page->data_.data());
        header->magic_number = 0xDEADBEEF;
        header->blob_id = blob_id;
        header->total_size = static_cast<uint32_t>(size);
        header->chunk_index = static_cast<uint32_t>(i);
        header->next_page_id = (i + 1 < page_ids.size()) ? page_ids[i + 1] : 0;
        header->data_size = static_cast<uint32_t>(to_write);

        // Copy the data
        uint8_t* page_data = page->data_.data() + sizeof(OverflowPageHeader);
        std::memcpy(page_data, byte_data + bytes_written, to_write);

        bytes_written += to_write;

        // Unpin the page (mark as dirty)
        buffer_pool_->UnpinPage(page_ids[i], true);
    }

    assert(bytes_written == size);
}

std::vector<uint32_t> BlobManager::ReadPageChain(uint32_t first_page_id) {
    std::vector<uint32_t> page_ids;
    uint32_t current_page_id = first_page_id;

    while (current_page_id != 0) {
        page_ids.push_back(current_page_id);

        // Fetch the page to get the next page ID
        Page* page = buffer_pool_->FetchPage(current_page_id);
        if (!page) {
            throw std::runtime_error("Failed to fetch page: " + std::to_string(current_page_id));
        }

        auto* header = reinterpret_cast<OverflowPageHeader*>(page->data_.data());
        if (!header->IsValid()) {
            buffer_pool_->UnpinPage(current_page_id, false);
            throw std::runtime_error("Invalid overflow page: " + std::to_string(current_page_id));
        }

        current_page_id = header->next_page_id;

        // Unpin the page
        buffer_pool_->UnpinPage(current_page_id, false);
    }

    return page_ids;
}

void BlobManager::FreePageChain(const std::vector<uint32_t>& page_ids) {
    for (uint32_t page_id : page_ids) {
        buffer_pool_->DeletePage(page_id);
    }
}
