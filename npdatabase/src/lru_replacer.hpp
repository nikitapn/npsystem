#pragma once

#include <list>
#include <unordered_map>
#include <cstdint>

// Simple LRU replacement policy
class LRUReplacer {
private:
    std::list<size_t> lru_list_;  // Most recent at front
    std::unordered_map<size_t, std::list<size_t>::iterator> frame_to_iter_;
    size_t capacity_;

public:
    explicit LRUReplacer(size_t capacity) : capacity_(capacity) {}
    
    // Remove frame from consideration for eviction
    void Pin(size_t frame_id) {
        auto it = frame_to_iter_.find(frame_id);
        if (it != frame_to_iter_.end()) {
            lru_list_.erase(it->second);
            frame_to_iter_.erase(it);
        }
    }
    
    // Add frame back to consideration for eviction
    void Unpin(size_t frame_id) {
        auto it = frame_to_iter_.find(frame_id);
        if (it == frame_to_iter_.end()) {
            // Add to front (most recent)
            lru_list_.push_front(frame_id);
            frame_to_iter_[frame_id] = lru_list_.begin();
        }
    }
    
    // Get victim frame for eviction (least recently used)
    bool Victim(size_t& frame_id) {
        if (lru_list_.empty()) {
            return false;
        }
        
        frame_id = lru_list_.back();
        lru_list_.pop_back();
        frame_to_iter_.erase(frame_id);
        return true;
    }
    
    size_t Size() const {
        return lru_list_.size();
    }
};
