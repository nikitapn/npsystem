#include "npdatabase/impl/buffer_pool_manager.hpp"
#include <cassert>
#include <stdexcept>

BufferPoolManager::BufferPoolManager(size_t pool_size, const std::string& db_file)
    : pool_size_(pool_size) {

    // Initialize the disk manager
    disk_manager_ = std::make_unique<DiskManager>(db_file);

    // Initialize the replacer
    replacer_ = std::make_unique<LRUReplacer>(pool_size);

    // Initialize the buffer pool
    pages_.reserve(pool_size);
    for (size_t i = 0; i < pool_size; ++i) {
        pages_.emplace_back(std::make_unique<Page>());
        free_list_.push_back(i);
    }
}

BufferPoolManager::~BufferPoolManager() {
    FlushAllPages();
}

Page* BufferPoolManager::FetchPage(uint32_t page_id) {
    std::lock_guard<std::mutex> lock(latch_);

    // Check if page is already in buffer pool
    auto it = page_table_.find(page_id);
    if (it != page_table_.end()) {
        size_t frame_id = it->second;
        Page* page = pages_[frame_id].get();

        // Increment pin count and remove from replacer
        page->pin_count_++;
        replacer_->Pin(frame_id);
        UpdateAccessTime(frame_id);

        return page;
    }

    // Page not in buffer pool - need to fetch from disk
    size_t frame_id = FindFreeFrame();
    if (frame_id == SIZE_MAX) {
        // No free frame available
        return nullptr;
    }

    // Get the page from the buffer pool
    Page* page = pages_[frame_id].get();

    // Read page from disk
    try {
        disk_manager_->ReadPage(page_id, reinterpret_cast<char*>(page->data_.data()));
    } catch (const std::exception& e) {
        // If read fails, put frame back in free list
        free_list_.push_front(frame_id);
        throw;
    }

    // Initialize page metadata
    page->page_id_ = page_id;
    page->pin_count_ = 1;
    page->is_dirty_ = false;
    UpdateAccessTime(frame_id);

    // Update page table
    page_table_[page_id] = frame_id;

    // Remove from replacer since it's pinned
    replacer_->Pin(frame_id);

    return page;
}

bool BufferPoolManager::UnpinPage(uint32_t page_id, bool is_dirty) {
    std::lock_guard<std::mutex> lock(latch_);

    auto it = page_table_.find(page_id);
    if (it == page_table_.end()) {
        // Page not in buffer pool
        return false;
    }

    size_t frame_id = it->second;
    Page* page = pages_[frame_id].get();

    if (page->pin_count_ == 0) {
        // Page is already unpinned
        return false;
    }

    // Decrement pin count
    page->pin_count_--;

    // Update dirty flag
    if (is_dirty) {
        page->is_dirty_ = true;
    }

    // If pin count reaches 0, add to replacer
    if (page->pin_count_ == 0) {
        replacer_->Unpin(frame_id);
    }

    return true;
}

Page* BufferPoolManager::NewPage(uint32_t& page_id) {
    std::lock_guard<std::mutex> lock(latch_);

    // Find a free frame
    size_t frame_id = FindFreeFrame();
    if (frame_id == SIZE_MAX) {
        // No free frame available
        return nullptr;
    }

    // Allocate new page ID from disk manager
    page_id = disk_manager_->AllocatePage();

    // Get the page from buffer pool
    Page* page = pages_[frame_id].get();

    // Initialize the page
    page->Reset();
    page->page_id_ = page_id;
    page->pin_count_ = 1;
    page->is_dirty_ = true;  // New page is dirty by default
    UpdateAccessTime(frame_id);

    // Update page table
    page_table_[page_id] = frame_id;

    // Remove from replacer since it's pinned
    replacer_->Pin(frame_id);

    return page;
}

bool BufferPoolManager::DeletePage(uint32_t page_id) {
    std::lock_guard<std::mutex> lock(latch_);

    auto it = page_table_.find(page_id);
    if (it == page_table_.end()) {
        // Page not in buffer pool - deallocate from disk
        disk_manager_->DeallocatePage(page_id);
        return true;
    }

    size_t frame_id = it->second;
    Page* page = pages_[frame_id].get();

    if (page->pin_count_ > 0) {
        // Cannot delete pinned page
        return false;
    }

    // Remove from page table and replacer
    page_table_.erase(it);
    replacer_->Pin(frame_id);  // Remove from replacer

    // Reset page and add frame to free list
    page->Reset();
    free_list_.push_front(frame_id);

    // Deallocate from disk
    disk_manager_->DeallocatePage(page_id);

    return true;
}

bool BufferPoolManager::FlushPage(uint32_t page_id) {
    std::lock_guard<std::mutex> lock(latch_);

    auto it = page_table_.find(page_id);
    if (it == page_table_.end()) {
        // Page not in buffer pool
        return false;
    }

    size_t frame_id = it->second;
    Page* page = pages_[frame_id].get();

    if (page->is_dirty_) {
        try {
            disk_manager_->WritePage(page_id, reinterpret_cast<const char*>(page->data_.data()));
            page->is_dirty_ = false;
        } catch (const std::exception& e) {
            return false;
        }
    }

    return true;
}

void BufferPoolManager::FlushAllPages() {
    std::lock_guard<std::mutex> lock(latch_);

    for (const auto& [page_id, frame_id] : page_table_) {
        Page* page = pages_[frame_id].get();
        if (page->is_dirty_) {
            try {
                disk_manager_->WritePage(page_id, reinterpret_cast<const char*>(page->data_.data()));
                page->is_dirty_ = false;
            } catch (const std::exception& e) {
                // Log error but continue flushing other pages
                // In a real implementation, you'd use proper logging
                continue;
            }
        }
    }
}

size_t BufferPoolManager::FindFreeFrame() {
    // First try to get a frame from free list
    if (!free_list_.empty()) {
        size_t frame_id = free_list_.front();
        free_list_.pop_front();
        return frame_id;
    }

    // No free frames - try to evict a page
    size_t victim_frame_id;
    if (replacer_->Victim(victim_frame_id)) {
        // Found a victim frame
        if (EvictPage(victim_frame_id)) {
            return victim_frame_id;
        }
    }

    // No frame available
    return SIZE_MAX;
}

bool BufferPoolManager::EvictPage(size_t frame_id) {
    Page* page = pages_[frame_id].get();

    // Page should not be pinned
    assert(page->pin_count_ == 0);

    // If page is dirty, write it to disk
    if (page->is_dirty_) {
        try {
            disk_manager_->WritePage(page->page_id_, reinterpret_cast<const char*>(page->data_.data()));
        } catch (const std::exception& e) {
            // Failed to write - put back in replacer
            replacer_->Unpin(frame_id);
            return false;
        }
    }

    // Remove from page table
    page_table_.erase(page->page_id_);

    // Reset the page
    page->Reset();

    return true;
}

void BufferPoolManager::UpdateAccessTime(size_t frame_id) {
    pages_[frame_id]->last_access_ = std::chrono::steady_clock::now();
}

size_t BufferPoolManager::GetFreeFrameCount() const {
    std::lock_guard<std::mutex> lock(latch_);
    return free_list_.size() + replacer_->Size();
}