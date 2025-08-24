#pragma once

#include <atomic>
#include <cstdint>
#include <array>
#include <string>
#include <mutex>

class PosixDiskManager {
public:
  static constexpr size_t PAGE_SIZE = 4096;
  static constexpr uint32_t INVALID_PAGE_ID = UINT32_MAX;

private:
  int fd_;  // File descriptor for direct POSIX operations
  std::string file_name_;
  std::atomic<uint32_t> next_page_id_;
  mutable std::mutex file_mutex_;

public:
  explicit PosixDiskManager(const std::string& db_file_name);
  ~PosixDiskManager();

  void      WritePage(uint32_t page_id, const char* page_data);
  void      WritePageSync(uint32_t page_id, const char* page_data);  // With fsync
  void      ReadPage(uint32_t page_id, char* page_data);
  uint32_t  AllocatePage();
  void      DeallocatePage(uint32_t page_id);
  
  void      SyncToDisk();
  size_t    GetFileSize() const;
  bool      IsOpen() const;
};
