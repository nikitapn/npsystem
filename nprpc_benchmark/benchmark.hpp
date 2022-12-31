#ifndef BENCHMARK_
#define BENCHMARK_

#include <nprpc/flat.hpp>
#include <nprpc/nprpc.hpp>

namespace benchmark { 
struct Data1 {
  std::string f1;
  float f2;
};

namespace flat {
struct Data1 {
  ::nprpc::flat::String f1;
  float f2;
};

class Data1_Direct {
  ::nprpc::flat_buffer& buffer_;
  const std::uint32_t offset_;

  auto& base() noexcept { return *reinterpret_cast<Data1*>(reinterpret_cast<std::byte*>(buffer_.data().data()) + offset_); }
  auto const& base() const noexcept { return *reinterpret_cast<const Data1*>(reinterpret_cast<const std::byte*>(buffer_.data().data()) + offset_); }
public:
  uint32_t offset() const noexcept { return offset_; }
  void* __data() noexcept { return (void*)&base(); }
  Data1_Direct(::nprpc::flat_buffer& buffer, std::uint32_t offset)
    : buffer_(buffer)
    , offset_(offset)
  {
  }
  void f1(const char* str) { new (&base().f1) ::nprpc::flat::String(buffer_, str); }
  void f1(const std::string& str) { new (&base().f1) ::nprpc::flat::String(buffer_, str); }
  auto f1() noexcept { return (::nprpc::flat::Span<char>)base().f1; }
  auto f1() const noexcept { return (::nprpc::flat::Span<const char>)base().f1; }
  auto f1_d() noexcept {     return ::nprpc::flat::String_Direct1(buffer_, offset_ + offsetof(Data1, f1));  }
  const float& f2() const noexcept { return base().f2;}
  float& f2() noexcept { return base().f2;}
};
} // namespace flat

struct Data {
  std::string f1;
  std::string f2;
  uint8_t f3;
  uint16_t f4;
  uint32_t f5;
  std::vector<Data1> f6;
};

namespace flat {
struct Data {
  ::nprpc::flat::String f1;
  ::nprpc::flat::String f2;
  uint8_t f3;
  uint16_t f4;
  uint32_t f5;
  ::nprpc::flat::Vector<benchmark::flat::Data1> f6;
};

class Data_Direct {
  ::nprpc::flat_buffer& buffer_;
  const std::uint32_t offset_;

  auto& base() noexcept { return *reinterpret_cast<Data*>(reinterpret_cast<std::byte*>(buffer_.data().data()) + offset_); }
  auto const& base() const noexcept { return *reinterpret_cast<const Data*>(reinterpret_cast<const std::byte*>(buffer_.data().data()) + offset_); }
public:
  uint32_t offset() const noexcept { return offset_; }
  void* __data() noexcept { return (void*)&base(); }
  Data_Direct(::nprpc::flat_buffer& buffer, std::uint32_t offset)
    : buffer_(buffer)
    , offset_(offset)
  {
  }
  void f1(const char* str) { new (&base().f1) ::nprpc::flat::String(buffer_, str); }
  void f1(const std::string& str) { new (&base().f1) ::nprpc::flat::String(buffer_, str); }
  auto f1() noexcept { return (::nprpc::flat::Span<char>)base().f1; }
  auto f1() const noexcept { return (::nprpc::flat::Span<const char>)base().f1; }
  auto f1_d() noexcept {     return ::nprpc::flat::String_Direct1(buffer_, offset_ + offsetof(Data, f1));  }
  void f2(const char* str) { new (&base().f2) ::nprpc::flat::String(buffer_, str); }
  void f2(const std::string& str) { new (&base().f2) ::nprpc::flat::String(buffer_, str); }
  auto f2() noexcept { return (::nprpc::flat::Span<char>)base().f2; }
  auto f2() const noexcept { return (::nprpc::flat::Span<const char>)base().f2; }
  auto f2_d() noexcept {     return ::nprpc::flat::String_Direct1(buffer_, offset_ + offsetof(Data, f2));  }
  const uint8_t& f3() const noexcept { return base().f3;}
  uint8_t& f3() noexcept { return base().f3;}
  const uint16_t& f4() const noexcept { return base().f4;}
  uint16_t& f4() noexcept { return base().f4;}
  const uint32_t& f5() const noexcept { return base().f5;}
  uint32_t& f5() noexcept { return base().f5;}
  void f6(std::uint32_t elements_size) { new (&base().f6) ::nprpc::flat::Vector<benchmark::flat::Data1>(buffer_, elements_size); }
  auto f6_d() noexcept { return ::nprpc::flat::Vector_Direct2<benchmark::flat::Data1,benchmark::flat::Data1_Direct>(buffer_, offset_ + offsetof(Data, f6)); }
  auto f6() noexcept { return ::nprpc::flat::Span_ref<benchmark::flat::Data1, benchmark::flat::Data1_Direct>(buffer_, base().f6.range(buffer_.data().data())); }
};
} // namespace flat

class IBenchmark_Servant
  : public virtual nprpc::ObjectServant
{
public:
  static std::string_view _get_class() noexcept { return "benchmark/benchmark.Benchmark"; }
  std::string_view get_class() const noexcept override { return IBenchmark_Servant::_get_class(); }
  void dispatch(nprpc::Buffers& bufs, nprpc::EndPoint remote_endpoint, bool from_parent, nprpc::ReferenceList& ref_list) override;
  virtual Data rpc () = 0;
  virtual std::string json () = 0;
};

class Benchmark
  : public virtual nprpc::Object
{
  const uint8_t interface_idx_;
public:
  using servant_t = IBenchmark_Servant;

  Benchmark(uint8_t interface_idx) : interface_idx_(interface_idx) {}
  Data rpc ();
  std::string json ();
};

} // namespace benchmark

namespace benchmark::helper {
} // namespace benchmark::helper

#endif