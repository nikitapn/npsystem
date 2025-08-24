#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>
#include <atomic>
#include <mutex>

// Forward declaration
class BufferPoolManager;
struct Page;

// Header for overflow pages that store large objects
struct OverflowPageHeader {
    uint32_t magic_number = 0xDEADBEEF;  // Identify as overflow page
    uint32_t blob_id;                    // Unique identifier for this blob
    uint32_t total_size;                 // Total size of the blob
    uint32_t chunk_index;                // Which chunk this page represents (0-based)
    uint32_t next_page_id;               // Next page in chain (0 if last)
    uint32_t data_size;                  // Size of data in this page
    uint8_t reserved[8];                 // Future use/alignment
    
    // Verify this is a valid overflow page
    bool IsValid() const {
        return magic_number == 0xDEADBEEF;
    }
};

// Metadata about a blob stored in memory for quick access
struct BlobMetadata {
    uint32_t blob_id;
    uint32_t total_size;
    uint32_t first_page_id;
    uint32_t num_pages;
    
    BlobMetadata() = default;
    BlobMetadata(uint32_t id, uint32_t size, uint32_t first_page, uint32_t pages)
        : blob_id(id), total_size(size), first_page_id(first_page), num_pages(pages) {}
};

class BlobManager {
private:
    BufferPoolManager* buffer_pool_;
    
    // Size of data we can store in each overflow page
    static constexpr size_t OVERFLOW_DATA_SIZE = 4096 - sizeof(OverflowPageHeader);
    
    // Blob ID generation
    std::atomic<uint32_t> next_blob_id_;
    
    // Cache of blob metadata for quick lookups
    std::unordered_map<uint32_t, BlobMetadata> blob_cache_;
    mutable std::mutex cache_mutex_;
    
    // Helper methods
    uint32_t CalculateRequiredPages(size_t blob_size) const;
    std::vector<uint32_t> AllocatePageChain(uint32_t num_pages);
    void WritePageChain(const std::vector<uint32_t>& page_ids, 
                       uint32_t blob_id, 
                       const void* data, 
                       size_t size);
    std::vector<uint32_t> ReadPageChain(uint32_t first_page_id);
    void FreePageChain(const std::vector<uint32_t>& page_ids);
    
public:
    explicit BlobManager(BufferPoolManager* buffer_pool);
    ~BlobManager() = default;
    
    // Store a large blob, returns blob_id for reference
    uint32_t StoreBlob(const void* data, size_t size);
    
    // Retrieve a blob by its ID
    std::vector<uint8_t> GetBlob(uint32_t blob_id);
    
    // Delete a blob and free its pages
    bool DeleteBlob(uint32_t blob_id);
    
    // Get the first page ID for a blob (to store in B-tree)
    uint32_t GetBlobFirstPage(uint32_t blob_id);
    
    // Get blob size without reading the entire blob
    size_t GetBlobSize(uint32_t blob_id);
    
    // Statistics
    size_t GetBlobCount() const;
    
    // Utility for debugging
    void PrintBlobInfo(uint32_t blob_id) const;
    
    // Disable copy constructor and assignment
    BlobManager(const BlobManager&) = delete;
    BlobManager& operator=(const BlobManager&) = delete;
};
