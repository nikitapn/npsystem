#pragma once

#include <cstdint>
#include <array>
#include <chrono>

struct Page {
    static constexpr size_t PAGE_SIZE = 4096;
    
    uint32_t page_id_{0};
    uint32_t pin_count_{0};
    bool is_dirty_{false};
    std::chrono::steady_clock::time_point last_access_;
    
    // Actual page data - aligned for performance
    alignas(64) std::array<uint8_t, PAGE_SIZE> data_{};
    
    // Reset page to initial state
    void Reset() {
        page_id_ = 0;
        pin_count_ = 0;
        is_dirty_ = false;
        data_.fill(0);
    }
    
    // Check if page can be evicted
    bool CanEvict() const {
        return pin_count_ == 0;
    }
};
