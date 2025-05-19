#pragma once

#include <array>
#include <cstddef>
#include <string>
#include <memory>

namespace nprpc::impl {

class SharedUUID {
public:
    using uuid_array = std::array<std::uint8_t, 16>;
    
    static SharedUUID& instance();
    
    // Get current UUID value
    const uuid_array& get() const noexcept;
    
    // Generate new UUID and store it
    void generate_new();
    
    ~SharedUUID();

private:
    SharedUUID();
    SharedUUID(const SharedUUID&) = delete;
    SharedUUID& operator=(const SharedUUID&) = delete;

    struct Impl;
    std::unique_ptr<Impl> pimpl_;
};

} // namespace nprpc::impl