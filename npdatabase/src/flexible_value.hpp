#pragma once

#include <variant>
#include <vector>

// Value can be either inline data or a reference to external storage
template<typename T>
struct FlexibleValue {
    static constexpr size_t INLINE_THRESHOLD = 100; // bytes
    
    // Either store data inline or as external reference
    std::variant<T, uint32_t> data;  // T for small data, uint32_t for blob_id
    
    bool is_inline() const {
        return std::holds_alternative<T>(data);
    }
    
    bool is_external() const {
        return std::holds_alternative<uint32_t>(data);
    }
    
    T get_inline() const {
        return std::get<T>(data);
    }
    
    uint32_t get_blob_id() const {
        return std::get<uint32_t>(data);
    }
};

// Example: String that can be inline or external
struct FlexibleString {
    static constexpr size_t INLINE_THRESHOLD = 200; // bytes
    
    bool is_inline;
    union {
        char inline_data[INLINE_THRESHOLD];
        uint32_t blob_id;  // Reference to external storage
    };
    size_t size;
    
    FlexibleString(const std::string& str) : size(str.size()) {
        if (str.size() < INLINE_THRESHOLD) {
            is_inline = true;
            std::memcpy(inline_data, str.data(), str.size());
            inline_data[str.size()] = '\0';
        } else {
            is_inline = false;
            // blob_id would be set by BlobManager
        }
    }
};
