#include "npdatabase/impl/disk_manager.hpp"
#include <iostream>
#include <stdexcept>
#include <cstring>

#ifdef _WIN32
#include <windows.h>
#include <io.h>
#else
#include <unistd.h>
#endif

DiskManager::DiskManager(const std::string& db_file_name) 
    : file_name_(db_file_name), next_page_id_(0) {

    // Open the database file in binary mode for read/write
    db_file_.open(file_name_, std::ios::binary | std::ios::in | std::ios::out);

    // If file doesn't exist, create it
    if (!db_file_.is_open()) {
        db_file_.clear();
        db_file_.open(file_name_, std::ios::binary | std::ios::trunc | std::ios::out);
        db_file_.close();
        db_file_.open(file_name_, std::ios::binary | std::ios::in | std::ios::out);
    }

    if (!db_file_.is_open()) {
        throw std::runtime_error("Cannot open database file: " + file_name_);
    }

    // Calculate next_page_id based on existing file size
    db_file_.seekg(0, std::ios::end);
    auto file_size = db_file_.tellg();
    next_page_id_ = static_cast<uint32_t>(file_size / PAGE_SIZE);
}

DiskManager::~DiskManager() {
    if (db_file_.is_open()) {
        db_file_.close();
    }
}

void DiskManager::WritePage(uint32_t page_id, const char* page_data) {
    std::lock_guard<std::mutex> lock(file_mutex_);

    if (!db_file_.is_open()) {
        throw std::runtime_error("Database file is not open");
    }

    // Calculate file offset
    auto offset = static_cast<std::streamoff>(page_id) * PAGE_SIZE;

    // Seek to the correct position
    db_file_.seekp(offset);
    if (db_file_.fail()) {
        throw std::runtime_error("Failed to seek to page " + std::to_string(page_id));
    }

    // Write the page data
    db_file_.write(page_data, PAGE_SIZE);
    if (db_file_.fail()) {
        throw std::runtime_error("Failed to write page " + std::to_string(page_id));
    }

    // Ensure data is written to disk - use flush() for performance,
    // but caller should use SyncToDisk() for durability guarantees
    db_file_.flush();
}

void DiskManager::ReadPage(uint32_t page_id, char* page_data) {
    std::lock_guard<std::mutex> lock(file_mutex_);

    if (!db_file_.is_open()) {
        throw std::runtime_error("Database file is not open");
    }

    // Calculate file offset
    auto offset = static_cast<std::streamoff>(page_id) * PAGE_SIZE;

    // Check if we're reading beyond the file
    db_file_.seekg(0, std::ios::end);
    auto file_size = db_file_.tellg();

    if (offset >= file_size) {
        // Reading beyond file size - return zeroed page
        std::memset(page_data, 0, PAGE_SIZE);
        return;
    }

    // Seek to the correct position
    db_file_.seekg(offset);
    if (db_file_.fail()) {
        throw std::runtime_error("Failed to seek to page " + std::to_string(page_id));
    }

    // Read the page data
    db_file_.read(page_data, PAGE_SIZE);
    if (db_file_.fail() && !db_file_.eof()) {
        throw std::runtime_error("Failed to read page " + std::to_string(page_id));
    }

    // If we read less than PAGE_SIZE (at end of file), zero-fill the rest
    auto bytes_read = db_file_.gcount();
    if (bytes_read < PAGE_SIZE) {
        std::memset(page_data + bytes_read, 0, PAGE_SIZE - bytes_read);
    }
}

uint32_t DiskManager::AllocatePage() {
    return next_page_id_.fetch_add(1);
}

void DiskManager::DeallocatePage(uint32_t page_id) {
    // For now, we don't reuse deallocated pages
    // In a production system, you'd maintain a free page list
    // and track deallocated pages for reuse

    // Zero out the page to mark it as deallocated
    std::array<char, PAGE_SIZE> zero_page{};
    WritePage(page_id, zero_page.data());
}

void DiskManager::FlushToDisk() {
    std::lock_guard<std::mutex> lock(file_mutex_);
    if (db_file_.is_open()) {
        db_file_.flush();
    }
}

void DiskManager::SyncToDisk() {
    std::lock_guard<std::mutex> lock(file_mutex_);
    if (!db_file_.is_open()) {
        return;
    }

    // First flush the C++ stream
    db_file_.flush();

    // Then force OS to write to disk
    int fd = GetFileDescriptor();
    if (fd != -1) {
#ifdef _WIN32
        HANDLE handle = reinterpret_cast<HANDLE>(_get_osfhandle(fd));
        if (handle != INVALID_HANDLE_VALUE) {
            FlushFileBuffers(handle);
        }
#else
        if (fsync(fd) == -1) {
            throw std::runtime_error("Failed to sync file to disk");
        }
#endif
    }
}

int DiskManager::GetFileDescriptor() const {
    // Getting file descriptor from std::fstream is not portable
    // For production code, consider using POSIX file operations directly
    // or storing the file descriptor separately during construction
    return -1;  // Not implemented - would require platform-specific code
}

size_t DiskManager::GetFileSize() const {
    // Remove const to allow file operations
    auto* this_non_const = const_cast<DiskManager*>(this);
    std::lock_guard<std::mutex> lock(this_non_const->file_mutex_);

    if (!this_non_const->db_file_.is_open()) {
        return 0;
    }

    auto current_pos = this_non_const->db_file_.tellg();
    this_non_const->db_file_.seekg(0, std::ios::end);
    auto size = this_non_const->db_file_.tellg();
    this_non_const->db_file_.seekg(current_pos);

    return static_cast<size_t>(size);
}

bool DiskManager::IsOpen() const {
    return db_file_.is_open();
}