#ifndef __NPRPC_TEST_HPP__
#define __NPRPC_TEST_HPP__

#include <nprpc/flat.hpp>
#include <nprpc/nprpc.hpp>

namespace test { 
using Id = uint32_t;
using IdArray = std::vector<Id>;
using bytestream = std::vector<uint8_t>;
class IServerControl_Servant
  : public virtual nprpc::ObjectServant
{
public:
  static std::string_view _get_class() noexcept { return "test/test.ServerControl"; }
  std::string_view get_class() const noexcept override { return IServerControl_Servant::_get_class(); }
  void dispatch(nprpc::Buffers& bufs, [[maybe_unused]] nprpc::SessionContext& ctx, [[maybe_unused]] bool from_parent) override;
  virtual void Shutdown () = 0;
};

class ServerControl
  : public virtual nprpc::Object
{
  const uint8_t interface_idx_;
public:
  using servant_t = IServerControl_Servant;

  ServerControl(uint8_t interface_idx) : interface_idx_(interface_idx) {}
  void Shutdown ();
};

struct FlatStruct {
  int32_t a;
  uint32_t b;
  float c;
};

namespace flat {
struct FlatStruct {
  int32_t a;
  uint32_t b;
  float c;
};

class FlatStruct_Direct {
  ::nprpc::flat_buffer& buffer_;
  const std::uint32_t offset_;

  auto& base() noexcept { return *reinterpret_cast<FlatStruct*>(reinterpret_cast<std::byte*>(buffer_.data().data()) + offset_); }
  auto const& base() const noexcept { return *reinterpret_cast<const FlatStruct*>(reinterpret_cast<const std::byte*>(buffer_.data().data()) + offset_); }
public:
  uint32_t offset() const noexcept { return offset_; }
  void* __data() noexcept { return (void*)&base(); }
  FlatStruct_Direct(::nprpc::flat_buffer& buffer, std::uint32_t offset)
    : buffer_(buffer)
    , offset_(offset)
  {
  }
  const int32_t& a() const noexcept { return base().a;}
  int32_t& a() noexcept { return base().a;}
  const uint32_t& b() const noexcept { return base().b;}
  uint32_t& b() noexcept { return base().b;}
  const float& c() const noexcept { return base().c;}
  float& c() noexcept { return base().c;}
};
} // namespace flat

struct AAA {
  uint32_t a;
  std::string b;
  std::string c;
};

namespace flat {
struct AAA {
  uint32_t a;
  ::nprpc::flat::String b;
  ::nprpc::flat::String c;
};

class AAA_Direct {
  ::nprpc::flat_buffer& buffer_;
  const std::uint32_t offset_;

  auto& base() noexcept { return *reinterpret_cast<AAA*>(reinterpret_cast<std::byte*>(buffer_.data().data()) + offset_); }
  auto const& base() const noexcept { return *reinterpret_cast<const AAA*>(reinterpret_cast<const std::byte*>(buffer_.data().data()) + offset_); }
public:
  uint32_t offset() const noexcept { return offset_; }
  void* __data() noexcept { return (void*)&base(); }
  AAA_Direct(::nprpc::flat_buffer& buffer, std::uint32_t offset)
    : buffer_(buffer)
    , offset_(offset)
  {
  }
  const uint32_t& a() const noexcept { return base().a;}
  uint32_t& a() noexcept { return base().a;}
  void b(const char* str) { new (&base().b) ::nprpc::flat::String(buffer_, str); }
  void b(const std::string& str) { new (&base().b) ::nprpc::flat::String(buffer_, str); }
  auto b() noexcept { return (::nprpc::flat::Span<char>)base().b; }
  auto b() const noexcept { return (::nprpc::flat::Span<const char>)base().b; }
  auto b_d() noexcept {     return ::nprpc::flat::String_Direct1(buffer_, offset_ + offsetof(AAA, b));  }
  void c(const char* str) { new (&base().c) ::nprpc::flat::String(buffer_, str); }
  void c(const std::string& str) { new (&base().c) ::nprpc::flat::String(buffer_, str); }
  auto c() noexcept { return (::nprpc::flat::Span<char>)base().c; }
  auto c() const noexcept { return (::nprpc::flat::Span<const char>)base().c; }
  auto c_d() noexcept {     return ::nprpc::flat::String_Direct1(buffer_, offset_ + offsetof(AAA, c));  }
};
} // namespace flat

class ITestBasic_Servant
  : public virtual nprpc::ObjectServant
{
public:
  static std::string_view _get_class() noexcept { return "test/test.TestBasic"; }
  std::string_view get_class() const noexcept override { return ITestBasic_Servant::_get_class(); }
  void dispatch(nprpc::Buffers& bufs, [[maybe_unused]] nprpc::SessionContext& ctx, [[maybe_unused]] bool from_parent) override;
  virtual bool ReturnBoolean () = 0;
  virtual IdArray ReturnIdArray () = 0;
  virtual uint32_t ReturnU32 () = 0;
  virtual bool In (uint32_t a, ::nprpc::flat::Boolean b, ::nprpc::flat::Span<uint8_t> c) = 0;
  virtual void Out (uint32_t& a, ::nprpc::flat::Boolean& b, /*out*/::nprpc::flat::Vector_Direct1<uint8_t> c) = 0;
};

class TestBasic
  : public virtual nprpc::Object
{
  const uint8_t interface_idx_;
public:
  using servant_t = ITestBasic_Servant;

  TestBasic(uint8_t interface_idx) : interface_idx_(interface_idx) {}
  bool ReturnBoolean ();
  IdArray ReturnIdArray ();
  uint32_t ReturnU32 ();
  bool In (uint32_t a, bool b, ::nprpc::flat::Span<const uint8_t> c);
  void Out (uint32_t& a, bool& b, std::vector<uint8_t>& c);
};

class ITestLargeMessage_Servant
  : public virtual nprpc::ObjectServant
{
public:
  static std::string_view _get_class() noexcept { return "test/test.TestLargeMessage"; }
  std::string_view get_class() const noexcept override { return ITestLargeMessage_Servant::_get_class(); }
  void dispatch(nprpc::Buffers& bufs, [[maybe_unused]] nprpc::SessionContext& ctx, [[maybe_unused]] bool from_parent) override;
  virtual bool In (uint32_t a, ::nprpc::flat::Boolean b, ::nprpc::flat::Span<uint8_t> c) = 0;
  virtual void Out (uint32_t& a, ::nprpc::flat::Boolean& b, /*out*/::nprpc::flat::Vector_Direct1<uint8_t> c) = 0;
};

class TestLargeMessage
  : public virtual nprpc::Object
{
  const uint8_t interface_idx_;
public:
  using servant_t = ITestLargeMessage_Servant;

  TestLargeMessage(uint8_t interface_idx) : interface_idx_(interface_idx) {}
  bool In (uint32_t a, bool b, ::nprpc::flat::Span<const uint8_t> c);
  void Out (uint32_t& a, bool& b, std::vector<uint8_t>& c);
};

struct Opt1 {
  std::string str;
  std::optional<bytestream> stream;
};

namespace flat {
struct Opt1 {
  ::nprpc::flat::String str;
  ::nprpc::flat::Optional<::nprpc::flat::Vector<uint8_t>> stream;
};

class Opt1_Direct {
  ::nprpc::flat_buffer& buffer_;
  const std::uint32_t offset_;

  auto& base() noexcept { return *reinterpret_cast<Opt1*>(reinterpret_cast<std::byte*>(buffer_.data().data()) + offset_); }
  auto const& base() const noexcept { return *reinterpret_cast<const Opt1*>(reinterpret_cast<const std::byte*>(buffer_.data().data()) + offset_); }
public:
  uint32_t offset() const noexcept { return offset_; }
  void* __data() noexcept { return (void*)&base(); }
  Opt1_Direct(::nprpc::flat_buffer& buffer, std::uint32_t offset)
    : buffer_(buffer)
    , offset_(offset)
  {
  }
  void str(const char* str) { new (&base().str) ::nprpc::flat::String(buffer_, str); }
  void str(const std::string& str) { new (&base().str) ::nprpc::flat::String(buffer_, str); }
  auto str() noexcept { return (::nprpc::flat::Span<char>)base().str; }
  auto str() const noexcept { return (::nprpc::flat::Span<const char>)base().str; }
  auto str_d() noexcept {     return ::nprpc::flat::String_Direct1(buffer_, offset_ + offsetof(Opt1, str));  }
  auto stream() noexcept { return ::nprpc::flat::Optional_Direct<::nprpc::flat::Vector<uint8_t>,::nprpc::flat::Vector_Direct1<uint8_t>>(buffer_, offset_ + offsetof(Opt1, stream));  }
};
} // namespace flat

class ITestOptional_Servant
  : public virtual nprpc::ObjectServant
{
public:
  static std::string_view _get_class() noexcept { return "test/test.TestOptional"; }
  std::string_view get_class() const noexcept override { return ITestOptional_Servant::_get_class(); }
  void dispatch(nprpc::Buffers& bufs, [[maybe_unused]] nprpc::SessionContext& ctx, [[maybe_unused]] bool from_parent) override;
  virtual bool InEmpty (::nprpc::flat::Optional_Direct<uint32_t> a) = 0;
  virtual bool In (::nprpc::flat::Optional_Direct<uint32_t> a, ::nprpc::flat::Optional_Direct<test::flat::AAA, test::flat::AAA_Direct> b) = 0;
  virtual void OutEmpty (::nprpc::flat::Optional_Direct<uint32_t> a) = 0;
  virtual void Out (::nprpc::flat::Optional_Direct<uint32_t> a) = 0;
  virtual Opt1 ReturnOpt1 () = 0;
};

class TestOptional
  : public virtual nprpc::Object
{
  const uint8_t interface_idx_;
public:
  using servant_t = ITestOptional_Servant;

  TestOptional(uint8_t interface_idx) : interface_idx_(interface_idx) {}
  bool InEmpty (const std::optional<uint32_t>& a);
  bool In (const std::optional<uint32_t>& a, const std::optional<test::AAA>& b);
  void OutEmpty (std::optional<uint32_t>& a);
  void Out (std::optional<uint32_t>& a);
  Opt1 ReturnOpt1 ();
};

struct CCC {
  std::string a;
  std::string b;
  std::optional<bool> c;
};

namespace flat {
struct CCC {
  ::nprpc::flat::String a;
  ::nprpc::flat::String b;
  ::nprpc::flat::Optional<::nprpc::flat::Boolean> c;
};

class CCC_Direct {
  ::nprpc::flat_buffer& buffer_;
  const std::uint32_t offset_;

  auto& base() noexcept { return *reinterpret_cast<CCC*>(reinterpret_cast<std::byte*>(buffer_.data().data()) + offset_); }
  auto const& base() const noexcept { return *reinterpret_cast<const CCC*>(reinterpret_cast<const std::byte*>(buffer_.data().data()) + offset_); }
public:
  uint32_t offset() const noexcept { return offset_; }
  void* __data() noexcept { return (void*)&base(); }
  CCC_Direct(::nprpc::flat_buffer& buffer, std::uint32_t offset)
    : buffer_(buffer)
    , offset_(offset)
  {
  }
  void a(const char* str) { new (&base().a) ::nprpc::flat::String(buffer_, str); }
  void a(const std::string& str) { new (&base().a) ::nprpc::flat::String(buffer_, str); }
  auto a() noexcept { return (::nprpc::flat::Span<char>)base().a; }
  auto a() const noexcept { return (::nprpc::flat::Span<const char>)base().a; }
  auto a_d() noexcept {     return ::nprpc::flat::String_Direct1(buffer_, offset_ + offsetof(CCC, a));  }
  void b(const char* str) { new (&base().b) ::nprpc::flat::String(buffer_, str); }
  void b(const std::string& str) { new (&base().b) ::nprpc::flat::String(buffer_, str); }
  auto b() noexcept { return (::nprpc::flat::Span<char>)base().b; }
  auto b() const noexcept { return (::nprpc::flat::Span<const char>)base().b; }
  auto b_d() noexcept {     return ::nprpc::flat::String_Direct1(buffer_, offset_ + offsetof(CCC, b));  }
  auto c() noexcept { return ::nprpc::flat::Optional_Direct<::nprpc::flat::Boolean,void>(buffer_, offset_ + offsetof(CCC, c));  }
};
} // namespace flat

struct BBB {
  std::vector<AAA> a;
  std::vector<CCC> b;
};

namespace flat {
struct BBB {
  ::nprpc::flat::Vector<test::flat::AAA> a;
  ::nprpc::flat::Vector<test::flat::CCC> b;
};

class BBB_Direct {
  ::nprpc::flat_buffer& buffer_;
  const std::uint32_t offset_;

  auto& base() noexcept { return *reinterpret_cast<BBB*>(reinterpret_cast<std::byte*>(buffer_.data().data()) + offset_); }
  auto const& base() const noexcept { return *reinterpret_cast<const BBB*>(reinterpret_cast<const std::byte*>(buffer_.data().data()) + offset_); }
public:
  uint32_t offset() const noexcept { return offset_; }
  void* __data() noexcept { return (void*)&base(); }
  BBB_Direct(::nprpc::flat_buffer& buffer, std::uint32_t offset)
    : buffer_(buffer)
    , offset_(offset)
  {
  }
  void a(std::uint32_t elements_size) { new (&base().a) ::nprpc::flat::Vector<test::flat::AAA>(buffer_, elements_size); }
  auto a_d() noexcept { return ::nprpc::flat::Vector_Direct2<test::flat::AAA,test::flat::AAA_Direct>(buffer_, offset_ + offsetof(BBB, a)); }
  auto a() noexcept { return ::nprpc::flat::Span_ref<test::flat::AAA, test::flat::AAA_Direct>(buffer_, base().a.range(buffer_.data().data())); }
  void b(std::uint32_t elements_size) { new (&base().b) ::nprpc::flat::Vector<test::flat::CCC>(buffer_, elements_size); }
  auto b_d() noexcept { return ::nprpc::flat::Vector_Direct2<test::flat::CCC,test::flat::CCC_Direct>(buffer_, offset_ + offsetof(BBB, b)); }
  auto b() noexcept { return ::nprpc::flat::Span_ref<test::flat::CCC, test::flat::CCC_Direct>(buffer_, base().b.range(buffer_.data().data())); }
};
} // namespace flat

struct Level2 {
  std::string x;
  std::vector<uint8_t> y;
  uint64_t z;
};

namespace flat {
struct Level2 {
  ::nprpc::flat::String x;
  ::nprpc::flat::Vector<uint8_t> y;
  uint64_t z;
};

class Level2_Direct {
  ::nprpc::flat_buffer& buffer_;
  const std::uint32_t offset_;

  auto& base() noexcept { return *reinterpret_cast<Level2*>(reinterpret_cast<std::byte*>(buffer_.data().data()) + offset_); }
  auto const& base() const noexcept { return *reinterpret_cast<const Level2*>(reinterpret_cast<const std::byte*>(buffer_.data().data()) + offset_); }
public:
  uint32_t offset() const noexcept { return offset_; }
  void* __data() noexcept { return (void*)&base(); }
  Level2_Direct(::nprpc::flat_buffer& buffer, std::uint32_t offset)
    : buffer_(buffer)
    , offset_(offset)
  {
  }
  void x(const char* str) { new (&base().x) ::nprpc::flat::String(buffer_, str); }
  void x(const std::string& str) { new (&base().x) ::nprpc::flat::String(buffer_, str); }
  auto x() noexcept { return (::nprpc::flat::Span<char>)base().x; }
  auto x() const noexcept { return (::nprpc::flat::Span<const char>)base().x; }
  auto x_d() noexcept {     return ::nprpc::flat::String_Direct1(buffer_, offset_ + offsetof(Level2, x));  }
  void y(std::uint32_t elements_size) { new (&base().y) ::nprpc::flat::Vector<uint8_t>(buffer_, elements_size); }
  auto y_d() noexcept { return ::nprpc::flat::Vector_Direct1<uint8_t>(buffer_, offset_ + offsetof(Level2, y)); }
  auto y() noexcept { return (::nprpc::flat::Span<uint8_t>)base().y; }
  const auto y() const noexcept { return (::nprpc::flat::Span<const uint8_t>)base().y; }
  const uint64_t& z() const noexcept { return base().z;}
  uint64_t& z() noexcept { return base().z;}
};
} // namespace flat

struct Level1 {
  std::string x;
  Level2 y;
  uint64_t z;
};

namespace flat {
struct Level1 {
  ::nprpc::flat::String x;
  test::flat::Level2 y;
  uint64_t z;
};

class Level1_Direct {
  ::nprpc::flat_buffer& buffer_;
  const std::uint32_t offset_;

  auto& base() noexcept { return *reinterpret_cast<Level1*>(reinterpret_cast<std::byte*>(buffer_.data().data()) + offset_); }
  auto const& base() const noexcept { return *reinterpret_cast<const Level1*>(reinterpret_cast<const std::byte*>(buffer_.data().data()) + offset_); }
public:
  uint32_t offset() const noexcept { return offset_; }
  void* __data() noexcept { return (void*)&base(); }
  Level1_Direct(::nprpc::flat_buffer& buffer, std::uint32_t offset)
    : buffer_(buffer)
    , offset_(offset)
  {
  }
  void x(const char* str) { new (&base().x) ::nprpc::flat::String(buffer_, str); }
  void x(const std::string& str) { new (&base().x) ::nprpc::flat::String(buffer_, str); }
  auto x() noexcept { return (::nprpc::flat::Span<char>)base().x; }
  auto x() const noexcept { return (::nprpc::flat::Span<const char>)base().x; }
  auto x_d() noexcept {     return ::nprpc::flat::String_Direct1(buffer_, offset_ + offsetof(Level1, x));  }
  auto y() noexcept { return test::flat::Level2_Direct(buffer_, offset_ + offsetof(Level1, y)); }
  const uint64_t& z() const noexcept { return base().z;}
  uint64_t& z() noexcept { return base().z;}
};
} // namespace flat

struct TopLevel {
  std::string x;
  Level1 y;
  uint64_t z;
};

namespace flat {
struct TopLevel {
  ::nprpc::flat::String x;
  test::flat::Level1 y;
  uint64_t z;
};

class TopLevel_Direct {
  ::nprpc::flat_buffer& buffer_;
  const std::uint32_t offset_;

  auto& base() noexcept { return *reinterpret_cast<TopLevel*>(reinterpret_cast<std::byte*>(buffer_.data().data()) + offset_); }
  auto const& base() const noexcept { return *reinterpret_cast<const TopLevel*>(reinterpret_cast<const std::byte*>(buffer_.data().data()) + offset_); }
public:
  uint32_t offset() const noexcept { return offset_; }
  void* __data() noexcept { return (void*)&base(); }
  TopLevel_Direct(::nprpc::flat_buffer& buffer, std::uint32_t offset)
    : buffer_(buffer)
    , offset_(offset)
  {
  }
  void x(const char* str) { new (&base().x) ::nprpc::flat::String(buffer_, str); }
  void x(const std::string& str) { new (&base().x) ::nprpc::flat::String(buffer_, str); }
  auto x() noexcept { return (::nprpc::flat::Span<char>)base().x; }
  auto x() const noexcept { return (::nprpc::flat::Span<const char>)base().x; }
  auto x_d() noexcept {     return ::nprpc::flat::String_Direct1(buffer_, offset_ + offsetof(TopLevel, x));  }
  auto y() noexcept { return test::flat::Level1_Direct(buffer_, offset_ + offsetof(TopLevel, y)); }
  const uint64_t& z() const noexcept { return base().z;}
  uint64_t& z() noexcept { return base().z;}
};
} // namespace flat

class ITestNested_Servant
  : public virtual nprpc::ObjectServant
{
public:
  static std::string_view _get_class() noexcept { return "test/test.TestNested"; }
  std::string_view get_class() const noexcept override { return ITestNested_Servant::_get_class(); }
  void dispatch(nprpc::Buffers& bufs, [[maybe_unused]] nprpc::SessionContext& ctx, [[maybe_unused]] bool from_parent) override;
  virtual void Out (::nprpc::flat::Optional_Direct<test::flat::BBB, test::flat::BBB_Direct> a) = 0;
  virtual TopLevel ReturnNested () = 0;
};

class TestNested
  : public virtual nprpc::Object
{
  const uint8_t interface_idx_;
public:
  using servant_t = ITestNested_Servant;

  TestNested(uint8_t interface_idx) : interface_idx_(interface_idx) {}
  void Out (std::optional<test::BBB>& a);
  TopLevel ReturnNested ();
};

class ITestBadInput_Servant
  : public virtual nprpc::ObjectServant
{
public:
  static std::string_view _get_class() noexcept { return "test/test.TestBadInput"; }
  std::string_view get_class() const noexcept override { return ITestBadInput_Servant::_get_class(); }
  void dispatch(nprpc::Buffers& bufs, [[maybe_unused]] nprpc::SessionContext& ctx, [[maybe_unused]] bool from_parent) override;
  virtual void In (::nprpc::flat::Span<uint8_t> a) = 0;
};

class TestBadInput
  : public virtual nprpc::Object
{
  const uint8_t interface_idx_;
public:
  using servant_t = ITestBadInput_Servant;

  TestBadInput(uint8_t interface_idx) : interface_idx_(interface_idx) {}
  void In (::nprpc::flat::Span<const uint8_t> a);
};

} // namespace test

namespace test::helper {
template<::nprpc::IterableCollection T>
void assign_from_cpp_Out_a(::nprpc::flat::Optional_Direct<test::flat::BBB, test::flat::BBB_Direct>& dest, const T & src) {
  if (src) {
    dest.alloc();
    auto value = dest.value();
    value.a(static_cast<uint32_t>(src.value().a.size()));
    {
      auto span = value.a();
      auto it = src.value().a.begin();
      for (auto e : span) {
        auto __ptr = ::nprpc::make_wrapper1(*it);
          e.a() = __ptr->a;
          e.b(__ptr->b);
          e.c(__ptr->c);
        ++it;
      }
    }
    value.b(static_cast<uint32_t>(src.value().b.size()));
    {
      auto span = value.b();
      auto it = src.value().b.begin();
      for (auto e : span) {
        auto __ptr = ::nprpc::make_wrapper1(*it);
          e.a(__ptr->a);
          e.b(__ptr->b);
        if (__ptr->c) {
            e.c().alloc();
            e.c().value() = __ptr->c.value();
        } else { 
            e.c().set_nullopt();
        }
        ++it;
      }
    }
  } else { 
    dest.set_nullopt();
  }
}
} // namespace test::helper
namespace test::helpers {
inline void assign_from_flat_TopLevel(test::flat::TopLevel_Direct& src, test::TopLevel& dest) {
  dest.x = (std::string_view)src.x();
  dest.y.x = (std::string_view)src.y().x();
  dest.y.y.x = (std::string_view)src.y().y().x();
  {
    auto span = src.y().y().y();
    dest.y.y.y.resize(span.size());
    memcpy(dest.y.y.y.data(), span.data(), 1 * span.size());
  }
  dest.y.y.z = src.y().y().z();
  dest.y.z = src.y().z();
  dest.z = src.z();
}
inline void assign_from_cpp_TopLevel(test::flat::TopLevel_Direct& dest, const test::TopLevel& src) {
  dest.x(src.x);
  dest.y().x(src.y.x);
  dest.y().y().x(src.y.y.x);
  dest.y().y().y(static_cast<uint32_t>(src.y.y.y.size()));
  memcpy(dest.y().y().y().data(), src.y.y.y.data(), src.y.y.y.size() * 1);
  dest.y().y().z() = src.y.y.z;
  dest.y().z() = src.y.z;
  dest.z() = src.z;
}
} // namespace test::flat
namespace test::helpers {
inline void assign_from_flat_Level1(test::flat::Level1_Direct& src, test::Level1& dest) {
  dest.x = (std::string_view)src.x();
  dest.y.x = (std::string_view)src.y().x();
  {
    auto span = src.y().y();
    dest.y.y.resize(span.size());
    memcpy(dest.y.y.data(), span.data(), 1 * span.size());
  }
  dest.y.z = src.y().z();
  dest.z = src.z();
}
inline void assign_from_cpp_Level1(test::flat::Level1_Direct& dest, const test::Level1& src) {
  dest.x(src.x);
  dest.y().x(src.y.x);
  dest.y().y(static_cast<uint32_t>(src.y.y.size()));
  memcpy(dest.y().y().data(), src.y.y.data(), src.y.y.size() * 1);
  dest.y().z() = src.y.z;
  dest.z() = src.z;
}
} // namespace test::flat
namespace test::helpers {
inline void assign_from_flat_Level2(test::flat::Level2_Direct& src, test::Level2& dest) {
  dest.x = (std::string_view)src.x();
  {
    auto span = src.y();
    dest.y.resize(span.size());
    memcpy(dest.y.data(), span.data(), 1 * span.size());
  }
  dest.z = src.z();
}
inline void assign_from_cpp_Level2(test::flat::Level2_Direct& dest, const test::Level2& src) {
  dest.x(src.x);
  dest.y(static_cast<uint32_t>(src.y.size()));
  memcpy(dest.y().data(), src.y.data(), src.y.size() * 1);
  dest.z() = src.z;
}
} // namespace test::flat
namespace test::helpers {
inline void assign_from_flat_BBB(test::flat::BBB_Direct& src, test::BBB& dest) {
  {
    auto span = src.a();
    dest.a.resize(span.size());
    auto it2 = std::begin(dest.a);
    for (auto e : span) {
      (*it2).a = e.a();
      (*it2).b = (std::string_view)e.b();
      (*it2).c = (std::string_view)e.c();
      ++it2;
    }
  }
  {
    auto span = src.b();
    dest.b.resize(span.size());
    auto it2 = std::begin(dest.b);
    for (auto e : span) {
      (*it2).a = (std::string_view)e.a();
      (*it2).b = (std::string_view)e.b();
      {
        auto opt = e.c();
        if (opt.has_value()) {
          (*it2).c = std::decay<decltype((*it2).c)>::type::value_type{};
          auto& value_to = (*it2).c.value();
          value_to = (bool)opt.value();
        } else { 
          (*it2).c = std::nullopt;
        }
      }
      ++it2;
    }
  }
}
inline void assign_from_cpp_BBB(test::flat::BBB_Direct& dest, const test::BBB& src) {
  dest.a(static_cast<uint32_t>(src.a.size()));
  {
    auto span = dest.a();
    auto it = src.a.begin();
    for (auto e : span) {
      auto __ptr = ::nprpc::make_wrapper1(*it);
        e.a() = __ptr->a;
        e.b(__ptr->b);
        e.c(__ptr->c);
      ++it;
    }
  }
  dest.b(static_cast<uint32_t>(src.b.size()));
  {
    auto span = dest.b();
    auto it = src.b.begin();
    for (auto e : span) {
      auto __ptr = ::nprpc::make_wrapper1(*it);
        e.a(__ptr->a);
        e.b(__ptr->b);
      if (__ptr->c) {
          e.c().alloc();
          e.c().value() = __ptr->c.value();
      } else { 
          e.c().set_nullopt();
      }
      ++it;
    }
  }
}
} // namespace test::flat
namespace test::helpers {
inline void assign_from_flat_CCC(test::flat::CCC_Direct& src, test::CCC& dest) {
  dest.a = (std::string_view)src.a();
  dest.b = (std::string_view)src.b();
  {
    auto opt = src.c();
    if (opt.has_value()) {
      dest.c = std::decay<decltype(dest.c)>::type::value_type{};
      auto& value_to = dest.c.value();
      value_to = (bool)opt.value();
    } else { 
      dest.c = std::nullopt;
    }
  }
}
inline void assign_from_cpp_CCC(test::flat::CCC_Direct& dest, const test::CCC& src) {
  dest.a(src.a);
  dest.b(src.b);
  if (src.c) {
    dest.c().alloc();
    dest.c().value() = src.c.value();
  } else { 
    dest.c().set_nullopt();
  }
}
} // namespace test::flat
namespace test::helpers {
inline void assign_from_flat_Opt1(test::flat::Opt1_Direct& src, test::Opt1& dest) {
  dest.str = (std::string_view)src.str();
  {
    auto opt = src.stream();
    if (opt.has_value()) {
      dest.stream = std::decay<decltype(dest.stream)>::type::value_type{};
      auto& value_to = dest.stream.value();
      auto value_from = opt.value();
      {
        auto span = value_from();
        value_to.resize(span.size());
        memcpy(value_to.data(), span.data(), 1 * span.size());
      }
    } else { 
      dest.stream = std::nullopt;
    }
  }
}
inline void assign_from_cpp_Opt1(test::flat::Opt1_Direct& dest, const test::Opt1& src) {
  dest.str(src.str);
  if (src.stream) {
    dest.stream().alloc();
    auto value = dest.stream().value();
    value.length(static_cast<uint32_t>(src.stream.value().size()));
    memcpy(value().data(), src.stream.value().data(), src.stream.value().size() * 1);
  } else { 
    dest.stream().set_nullopt();
  }
}
} // namespace test::flat
namespace test::helpers {
inline void assign_from_flat_AAA(test::flat::AAA_Direct& src, test::AAA& dest) {
  dest.a = src.a();
  dest.b = (std::string_view)src.b();
  dest.c = (std::string_view)src.c();
}
inline void assign_from_cpp_AAA(test::flat::AAA_Direct& dest, const test::AAA& src) {
  dest.a() = src.a;
  dest.b(src.b);
  dest.c(src.c);
}
} // namespace test::flat
namespace test::helpers {
inline void assign_from_flat_FlatStruct(test::flat::FlatStruct_Direct& src, test::FlatStruct& dest) {
  memcpy(&dest, src.__data(), 12);
}
inline void assign_from_cpp_FlatStruct(test::flat::FlatStruct_Direct& dest, const test::FlatStruct& src) {
  memcpy(dest.__data(), &src, 12);
}
} // namespace test::flat

#endif