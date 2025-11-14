#pragma once

#include <spdlog/spdlog.h>
#include <streambuf>
#include <string>

namespace nplib {

// Redirects output from a standard stream (e.g., std::cout, std::cerr) to a spdlog logger.
template<class Element = char, class Trait = std::char_traits<Element>>
class SpdlogRedirect : public std::basic_streambuf<Element, Trait>
{
  std::basic_ostream<Element, Trait>& stream_;
  std::streambuf* buffer_;
  std::string string_buffer_;
  std::shared_ptr<spdlog::logger> logger_;
protected:
  std::streamsize xsputn(const Element* elements, const std::streamsize count) override {
    string_buffer_.append(elements, static_cast<std::string::size_type>(count));
    return count;
  }

  typename Trait::int_type overflow(typename Trait::int_type final_character) override {
    if (final_character != Trait::eof()) {
      string_buffer_ += static_cast<Element>(final_character);
    }
    return Trait::not_eof(final_character);
  }

  int sync() override {
    if (!string_buffer_.empty()) {
      // Remove trailing \n if present, as spdlog adds its own
      if (!string_buffer_.empty() && string_buffer_.back() == '\n')
        string_buffer_.pop_back();
      logger_->info(string_buffer_);
      string_buffer_.clear();
    }
    return 0;
  }
public:
  SpdlogRedirect(std::ostream& stream, std::shared_ptr<spdlog::logger> logger)
    : stream_{ stream }
    , logger_{ std::move(logger) }
  {
    buffer_ = stream_.rdbuf(this);
  };

  ~SpdlogRedirect()
  {
    try {
      stream_.rdbuf(buffer_);
    } catch(const std::ios_base::failure& exception) {
      logger_->error(exception.what());
    }
  }

  SpdlogRedirect(const SpdlogRedirect& other) = delete;
  SpdlogRedirect& operator=(const SpdlogRedirect& other) = delete;

  SpdlogRedirect(SpdlogRedirect&& other) noexcept = default;
  SpdlogRedirect& operator=(SpdlogRedirect&& other) noexcept = default;
};

} // namespace nplib
