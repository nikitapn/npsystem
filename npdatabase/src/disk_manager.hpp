#pragma once

#include <fstream>
#include <atomic>
#include <cstdint>
#include <array>
#include <string>
#include <mutex>

#ifdef _WIN32
#include <windows.h>
#include <io.h>
#else
#include <unistd.h>
#endif

class DiskManager {
public:
  static constexpr size_t PAGE_SIZE = 4096;  // 4KB pages
  static constexpr uint32_t INVALID_PAGE_ID = UINT32_MAX;

private:
  std::fstream db_file_;
  std::string file_name_;
  std::atomic<uint32_t> next_page_id_;
  mutable std::mutex file_mutex_;  // Protect file operations

public:
  explicit DiskManager(const std::string& db_file_name);
  ~DiskManager();

  void WritePage(uint32_t page_id, const char* page_data);
  void ReadPage(uint32_t page_id, char* page_data);
  uint32_t AllocatePage();
  void DeallocatePage(uint32_t page_id);
  
  // Utility methods
  void FlushToDisk();
  void SyncToDisk();  // Force sync to disk using fsync()
  size_t GetFileSize() const;
  bool IsOpen() const;

private:
  int GetFileDescriptor() const;
};
