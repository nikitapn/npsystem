#pragma once

#include <vector>
#include <unordered_map>
#include <list>
#include <memory>
#include <mutex>
#include <chrono>

#include "page.hpp"
#include "disk_manager.hpp"
#include "lru_replacer.hpp"

class BufferPoolManager {
public:
    static constexpr size_t DEFAULT_POOL_SIZE = 1000;  // Number of pages in pool
    
    // Configuration for different page sizes based on use case
    enum class PageSizeConfig {
        SMALL_4KB = 4096,      // Good for metadata, small records
        MEDIUM_16KB = 16384,   // Good for medium-sized documents
        LARGE_64KB = 65536,    // Good for images, large documents
        HUGE_1MB = 1048576     // Good for very large blobs
    };
    
private:
    // Pool of pages in memory
    std::vector<std::unique_ptr<Page>> pages_;
    
    // Maps page_id to buffer pool index
    std::unordered_map<uint32_t, size_t> page_table_;
    
    // List of free buffer pool slots
    std::list<size_t> free_list_;
    
    // Disk manager for I/O operations
    std::unique_ptr<DiskManager> disk_manager_;
    
    // LRU replacer for eviction policy
    std::unique_ptr<LRUReplacer> replacer_;
    
    // Mutex for thread safety
    mutable std::mutex latch_;
    
    // Pool size
    size_t pool_size_;
    
    // Helper methods
    size_t FindFreeFrame();
    bool EvictPage(size_t frame_id);
    void UpdateAccessTime(size_t frame_id);

public:
    explicit BufferPoolManager(size_t pool_size = DEFAULT_POOL_SIZE, 
                              const std::string& db_file = "database.db");
    ~BufferPoolManager();
    
    // Core operations
    Page* FetchPage(uint32_t page_id);
    bool UnpinPage(uint32_t page_id, bool is_dirty);
    Page* NewPage(uint32_t& page_id);
    bool DeletePage(uint32_t page_id);
    
    // Utility operations
    bool FlushPage(uint32_t page_id);
    void FlushAllPages();
    
    // Statistics
    size_t GetPoolSize() const { return pool_size_; }
    size_t GetFreeFrameCount() const;
    
    // Disable copy constructor and assignment
    BufferPoolManager(const BufferPoolManager&) = delete;
    BufferPoolManager& operator=(const BufferPoolManager&) = delete;
};