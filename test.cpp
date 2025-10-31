#include "test/test.hpp"
#include <nprpc/impl/nprpc_impl.hpp>

void test_throw_exception(::nprpc::flat_buffer& buf);

namespace {
struct test_M1 {
  ::nprpc::flat::Boolean _1;
};

class test_M1_Direct {
  ::nprpc::flat_buffer& buffer_;
  const std::uint32_t offset_;

  auto& base() noexcept { return *reinterpret_cast<test_M1*>(reinterpret_cast<std::byte*>(buffer_.data().data()) + offset_); }
  auto const& base() const noexcept { return *reinterpret_cast<const test_M1*>(reinterpret_cast<const std::byte*>(buffer_.data().data()) + offset_); }
public:
  uint32_t offset() const noexcept { return offset_; }
  void* __data() noexcept { return (void*)&base(); }
  test_M1_Direct(::nprpc::flat_buffer& buffer, std::uint32_t offset)
    : buffer_(buffer)
    , offset_(offset)
  {
  }
  const ::nprpc::flat::Boolean& _1() const noexcept { return base()._1;}
  ::nprpc::flat::Boolean& _1() noexcept { return base()._1;}
};

struct test_M2 {
  ::nprpc::flat::Vector<uint32_t> _1;
};

class test_M2_Direct {
  ::nprpc::flat_buffer& buffer_;
  const std::uint32_t offset_;

  auto& base() noexcept { return *reinterpret_cast<test_M2*>(reinterpret_cast<std::byte*>(buffer_.data().data()) + offset_); }
  auto const& base() const noexcept { return *reinterpret_cast<const test_M2*>(reinterpret_cast<const std::byte*>(buffer_.data().data()) + offset_); }
public:
  uint32_t offset() const noexcept { return offset_; }
  void* __data() noexcept { return (void*)&base(); }
  test_M2_Direct(::nprpc::flat_buffer& buffer, std::uint32_t offset)
    : buffer_(buffer)
    , offset_(offset)
  {
  }
  void _1(std::uint32_t elements_size) { new (&base()._1) ::nprpc::flat::Vector<uint32_t>(buffer_, elements_size); }
  auto _1_d() noexcept { return ::nprpc::flat::Vector_Direct1<uint32_t>(buffer_, offset_ + offsetof(test_M2, _1)); }
  auto _1() noexcept { return (::nprpc::flat::Span<uint32_t>)base()._1; }
  const auto _1() const noexcept { return (::nprpc::flat::Span<const uint32_t>)base()._1; }
};

struct test_M3 {
  uint32_t _1;
};

class test_M3_Direct {
  ::nprpc::flat_buffer& buffer_;
  const std::uint32_t offset_;

  auto& base() noexcept { return *reinterpret_cast<test_M3*>(reinterpret_cast<std::byte*>(buffer_.data().data()) + offset_); }
  auto const& base() const noexcept { return *reinterpret_cast<const test_M3*>(reinterpret_cast<const std::byte*>(buffer_.data().data()) + offset_); }
public:
  uint32_t offset() const noexcept { return offset_; }
  void* __data() noexcept { return (void*)&base(); }
  test_M3_Direct(::nprpc::flat_buffer& buffer, std::uint32_t offset)
    : buffer_(buffer)
    , offset_(offset)
  {
  }
  const uint32_t& _1() const noexcept { return base()._1;}
  uint32_t& _1() noexcept { return base()._1;}
};

struct test_M4 {
  uint32_t _1;
  ::nprpc::flat::Boolean _2;
  ::nprpc::flat::Vector<uint8_t> _3;
};

class test_M4_Direct {
  ::nprpc::flat_buffer& buffer_;
  const std::uint32_t offset_;

  auto& base() noexcept { return *reinterpret_cast<test_M4*>(reinterpret_cast<std::byte*>(buffer_.data().data()) + offset_); }
  auto const& base() const noexcept { return *reinterpret_cast<const test_M4*>(reinterpret_cast<const std::byte*>(buffer_.data().data()) + offset_); }
public:
  uint32_t offset() const noexcept { return offset_; }
  void* __data() noexcept { return (void*)&base(); }
  test_M4_Direct(::nprpc::flat_buffer& buffer, std::uint32_t offset)
    : buffer_(buffer)
    , offset_(offset)
  {
  }
  const uint32_t& _1() const noexcept { return base()._1;}
  uint32_t& _1() noexcept { return base()._1;}
  const ::nprpc::flat::Boolean& _2() const noexcept { return base()._2;}
  ::nprpc::flat::Boolean& _2() noexcept { return base()._2;}
  void _3(std::uint32_t elements_size) { new (&base()._3) ::nprpc::flat::Vector<uint8_t>(buffer_, elements_size); }
  auto _3_d() noexcept { return ::nprpc::flat::Vector_Direct1<uint8_t>(buffer_, offset_ + offsetof(test_M4, _3)); }
  auto _3() noexcept { return (::nprpc::flat::Span<uint8_t>)base()._3; }
  const auto _3() const noexcept { return (::nprpc::flat::Span<const uint8_t>)base()._3; }
};

struct test_M5 {
  ::nprpc::flat::Optional<uint32_t> _1;
};

class test_M5_Direct {
  ::nprpc::flat_buffer& buffer_;
  const std::uint32_t offset_;

  auto& base() noexcept { return *reinterpret_cast<test_M5*>(reinterpret_cast<std::byte*>(buffer_.data().data()) + offset_); }
  auto const& base() const noexcept { return *reinterpret_cast<const test_M5*>(reinterpret_cast<const std::byte*>(buffer_.data().data()) + offset_); }
public:
  uint32_t offset() const noexcept { return offset_; }
  void* __data() noexcept { return (void*)&base(); }
  test_M5_Direct(::nprpc::flat_buffer& buffer, std::uint32_t offset)
    : buffer_(buffer)
    , offset_(offset)
  {
  }
  auto _1() noexcept { return ::nprpc::flat::Optional_Direct<uint32_t,void>(buffer_, offset_ + offsetof(test_M5, _1));  }
};

struct test_M6 {
  ::nprpc::flat::Optional<uint32_t> _1;
  ::nprpc::flat::Optional<test::flat::AAA> _2;
};

class test_M6_Direct {
  ::nprpc::flat_buffer& buffer_;
  const std::uint32_t offset_;

  auto& base() noexcept { return *reinterpret_cast<test_M6*>(reinterpret_cast<std::byte*>(buffer_.data().data()) + offset_); }
  auto const& base() const noexcept { return *reinterpret_cast<const test_M6*>(reinterpret_cast<const std::byte*>(buffer_.data().data()) + offset_); }
public:
  uint32_t offset() const noexcept { return offset_; }
  void* __data() noexcept { return (void*)&base(); }
  test_M6_Direct(::nprpc::flat_buffer& buffer, std::uint32_t offset)
    : buffer_(buffer)
    , offset_(offset)
  {
  }
  auto _1() noexcept { return ::nprpc::flat::Optional_Direct<uint32_t,void>(buffer_, offset_ + offsetof(test_M6, _1));  }
  auto _2() noexcept { return ::nprpc::flat::Optional_Direct<test::flat::AAA,test::flat::AAA_Direct>(buffer_, offset_ + offsetof(test_M6, _2));  }
};

struct test_M7 {
  test::flat::Opt1 _1;
};

class test_M7_Direct {
  ::nprpc::flat_buffer& buffer_;
  const std::uint32_t offset_;

  auto& base() noexcept { return *reinterpret_cast<test_M7*>(reinterpret_cast<std::byte*>(buffer_.data().data()) + offset_); }
  auto const& base() const noexcept { return *reinterpret_cast<const test_M7*>(reinterpret_cast<const std::byte*>(buffer_.data().data()) + offset_); }
public:
  uint32_t offset() const noexcept { return offset_; }
  void* __data() noexcept { return (void*)&base(); }
  test_M7_Direct(::nprpc::flat_buffer& buffer, std::uint32_t offset)
    : buffer_(buffer)
    , offset_(offset)
  {
  }
  auto _1() noexcept { return test::flat::Opt1_Direct(buffer_, offset_ + offsetof(test_M7, _1)); }
};

struct test_M8 {
  ::nprpc::flat::Optional<test::flat::BBB> _1;
};

class test_M8_Direct {
  ::nprpc::flat_buffer& buffer_;
  const std::uint32_t offset_;

  auto& base() noexcept { return *reinterpret_cast<test_M8*>(reinterpret_cast<std::byte*>(buffer_.data().data()) + offset_); }
  auto const& base() const noexcept { return *reinterpret_cast<const test_M8*>(reinterpret_cast<const std::byte*>(buffer_.data().data()) + offset_); }
public:
  uint32_t offset() const noexcept { return offset_; }
  void* __data() noexcept { return (void*)&base(); }
  test_M8_Direct(::nprpc::flat_buffer& buffer, std::uint32_t offset)
    : buffer_(buffer)
    , offset_(offset)
  {
  }
  auto _1() noexcept { return ::nprpc::flat::Optional_Direct<test::flat::BBB,test::flat::BBB_Direct>(buffer_, offset_ + offsetof(test_M8, _1));  }
};

struct test_M9 {
  test::flat::TopLevel _1;
};

class test_M9_Direct {
  ::nprpc::flat_buffer& buffer_;
  const std::uint32_t offset_;

  auto& base() noexcept { return *reinterpret_cast<test_M9*>(reinterpret_cast<std::byte*>(buffer_.data().data()) + offset_); }
  auto const& base() const noexcept { return *reinterpret_cast<const test_M9*>(reinterpret_cast<const std::byte*>(buffer_.data().data()) + offset_); }
public:
  uint32_t offset() const noexcept { return offset_; }
  void* __data() noexcept { return (void*)&base(); }
  test_M9_Direct(::nprpc::flat_buffer& buffer, std::uint32_t offset)
    : buffer_(buffer)
    , offset_(offset)
  {
  }
  auto _1() noexcept { return test::flat::TopLevel_Direct(buffer_, offset_ + offsetof(test_M9, _1)); }
};

struct test_M10 {
  ::nprpc::flat::Vector<uint8_t> _1;
};

class test_M10_Direct {
  ::nprpc::flat_buffer& buffer_;
  const std::uint32_t offset_;

  auto& base() noexcept { return *reinterpret_cast<test_M10*>(reinterpret_cast<std::byte*>(buffer_.data().data()) + offset_); }
  auto const& base() const noexcept { return *reinterpret_cast<const test_M10*>(reinterpret_cast<const std::byte*>(buffer_.data().data()) + offset_); }
public:
  uint32_t offset() const noexcept { return offset_; }
  void* __data() noexcept { return (void*)&base(); }
  test_M10_Direct(::nprpc::flat_buffer& buffer, std::uint32_t offset)
    : buffer_(buffer)
    , offset_(offset)
  {
  }
  void _1(std::uint32_t elements_size) { new (&base()._1) ::nprpc::flat::Vector<uint8_t>(buffer_, elements_size); }
  auto _1_d() noexcept { return ::nprpc::flat::Vector_Direct1<uint8_t>(buffer_, offset_ + offsetof(test_M10, _1)); }
  auto _1() noexcept { return (::nprpc::flat::Span<uint8_t>)base()._1; }
  const auto _1() const noexcept { return (::nprpc::flat::Span<const uint8_t>)base()._1; }
};


bool check_1VFu8(nprpc::flat_buffer& buf, test_M10_Direct& ia) {
  if (static_cast<std::uint32_t>(buf.size()) < ia.offset() + 8) goto check_failed;
  {
    if(!ia._1_d()._check_size_align(static_cast<std::uint32_t>(buf.size()))) goto check_failed;
  }
  return true;
check_failed:
  nprpc::impl::make_simple_answer(buf, nprpc::impl::MessageId::Error_BadInput);
  return false;
}
} // 

namespace test { 
void test::ServerControl::Shutdown() {
  ::nprpc::flat_buffer buf;
  {
    auto mb = buf.prepare(32);
    buf.commit(32);
    static_cast<::nprpc::impl::Header*>(mb.data())->msg_id = ::nprpc::impl::MessageId::FunctionCall;
    static_cast<::nprpc::impl::Header*>(mb.data())->msg_type = ::nprpc::impl::MessageType::Request;
  }
  ::nprpc::impl::flat::CallHeader_Direct __ch(buf, sizeof(::nprpc::impl::Header));
  __ch.object_id() = this->object_id();
  __ch.poa_idx() = this->poa_idx();
  __ch.interface_idx() = interface_idx_;
  __ch.function_idx() = 0;
  static_cast<::nprpc::impl::Header*>(buf.data().data())->size = static_cast<uint32_t>(buf.size() - 4);
  ::nprpc::impl::g_orb->call(this->get_endpoint(), buf, this->get_timeout());
  auto std_reply = nprpc::impl::handle_standart_reply(buf);
  if (std_reply != 0) {
    throw nprpc::Exception("Unknown Error");
  }
}

void test::IServerControl_Servant::dispatch(nprpc::Buffers& bufs, [[maybe_unused]] nprpc::SessionContext& ctx, [[maybe_unused]] bool from_parent) {
  nprpc::impl::flat::CallHeader_Direct __ch(bufs(), sizeof(::nprpc::impl::Header));
  switch(__ch.function_idx()) {
    case 0: {
      Shutdown();
      nprpc::impl::make_simple_answer(bufs(), nprpc::impl::MessageId::Success);
      break;
    }
    default:
      nprpc::impl::make_simple_answer(bufs(), nprpc::impl::MessageId::Error_UnknownFunctionIdx);
  }
}

bool test::TestBasic::ReturnBoolean() {
  ::nprpc::flat_buffer buf;
  {
    auto mb = buf.prepare(32);
    buf.commit(32);
    static_cast<::nprpc::impl::Header*>(mb.data())->msg_id = ::nprpc::impl::MessageId::FunctionCall;
    static_cast<::nprpc::impl::Header*>(mb.data())->msg_type = ::nprpc::impl::MessageType::Request;
  }
  ::nprpc::impl::flat::CallHeader_Direct __ch(buf, sizeof(::nprpc::impl::Header));
  __ch.object_id() = this->object_id();
  __ch.poa_idx() = this->poa_idx();
  __ch.interface_idx() = interface_idx_;
  __ch.function_idx() = 0;
  static_cast<::nprpc::impl::Header*>(buf.data().data())->size = static_cast<uint32_t>(buf.size() - 4);
  ::nprpc::impl::g_orb->call(this->get_endpoint(), buf, this->get_timeout());
  auto std_reply = nprpc::impl::handle_standart_reply(buf);
  if (std_reply != -1) {
    throw nprpc::Exception("Unknown Error");
  }
  test_M1_Direct out(buf, sizeof(::nprpc::impl::Header));
    bool __ret_value;
    __ret_value = (bool)out._1();
  return __ret_value;
}

IdArray test::TestBasic::ReturnIdArray() {
  ::nprpc::flat_buffer buf;
  {
    auto mb = buf.prepare(32);
    buf.commit(32);
    static_cast<::nprpc::impl::Header*>(mb.data())->msg_id = ::nprpc::impl::MessageId::FunctionCall;
    static_cast<::nprpc::impl::Header*>(mb.data())->msg_type = ::nprpc::impl::MessageType::Request;
  }
  ::nprpc::impl::flat::CallHeader_Direct __ch(buf, sizeof(::nprpc::impl::Header));
  __ch.object_id() = this->object_id();
  __ch.poa_idx() = this->poa_idx();
  __ch.interface_idx() = interface_idx_;
  __ch.function_idx() = 1;
  static_cast<::nprpc::impl::Header*>(buf.data().data())->size = static_cast<uint32_t>(buf.size() - 4);
  ::nprpc::impl::g_orb->call(this->get_endpoint(), buf, this->get_timeout());
  auto std_reply = nprpc::impl::handle_standart_reply(buf);
  if (std_reply != -1) {
    throw nprpc::Exception("Unknown Error");
  }
  test_M2_Direct out(buf, sizeof(::nprpc::impl::Header));
    IdArray __ret_value;
    {
      auto span = out._1();
      __ret_value.resize(span.size());
      memcpy(__ret_value.data(), span.data(), 4 * span.size());
    }
  return __ret_value;
}

uint32_t test::TestBasic::ReturnU32() {
  ::nprpc::flat_buffer buf;
  {
    auto mb = buf.prepare(32);
    buf.commit(32);
    static_cast<::nprpc::impl::Header*>(mb.data())->msg_id = ::nprpc::impl::MessageId::FunctionCall;
    static_cast<::nprpc::impl::Header*>(mb.data())->msg_type = ::nprpc::impl::MessageType::Request;
  }
  ::nprpc::impl::flat::CallHeader_Direct __ch(buf, sizeof(::nprpc::impl::Header));
  __ch.object_id() = this->object_id();
  __ch.poa_idx() = this->poa_idx();
  __ch.interface_idx() = interface_idx_;
  __ch.function_idx() = 2;
  static_cast<::nprpc::impl::Header*>(buf.data().data())->size = static_cast<uint32_t>(buf.size() - 4);
  ::nprpc::impl::g_orb->call(this->get_endpoint(), buf, this->get_timeout());
  auto std_reply = nprpc::impl::handle_standart_reply(buf);
  if (std_reply != -1) {
    throw nprpc::Exception("Unknown Error");
  }
  test_M3_Direct out(buf, sizeof(::nprpc::impl::Header));
    uint32_t __ret_value;
    __ret_value = out._1();
  return __ret_value;
}

bool test::TestBasic::In(uint32_t a, bool b, ::nprpc::flat::Span<const uint8_t> c) {
  ::nprpc::flat_buffer buf;
  {
    auto mb = buf.prepare(176);
    buf.commit(48);
    static_cast<::nprpc::impl::Header*>(mb.data())->msg_id = ::nprpc::impl::MessageId::FunctionCall;
    static_cast<::nprpc::impl::Header*>(mb.data())->msg_type = ::nprpc::impl::MessageType::Request;
  }
  ::nprpc::impl::flat::CallHeader_Direct __ch(buf, sizeof(::nprpc::impl::Header));
  __ch.object_id() = this->object_id();
  __ch.poa_idx() = this->poa_idx();
  __ch.interface_idx() = interface_idx_;
  __ch.function_idx() = 3;
  test_M4_Direct _(buf,32);
  _._1() = a;
  _._2() = b;
  _._3(static_cast<uint32_t>(c.size()));
  memcpy(_._3().data(), c.data(), c.size() * 1);
  static_cast<::nprpc::impl::Header*>(buf.data().data())->size = static_cast<uint32_t>(buf.size() - 4);
  ::nprpc::impl::g_orb->call(this->get_endpoint(), buf, this->get_timeout());
  auto std_reply = nprpc::impl::handle_standart_reply(buf);
  if (std_reply != -1) {
    throw nprpc::Exception("Unknown Error");
  }
  test_M1_Direct out(buf, sizeof(::nprpc::impl::Header));
    bool __ret_value;
    __ret_value = (bool)out._1();
  return __ret_value;
}

void test::TestBasic::Out(uint32_t& a, bool& b, std::vector<uint8_t>& c) {
  ::nprpc::flat_buffer buf;
  {
    auto mb = buf.prepare(32);
    buf.commit(32);
    static_cast<::nprpc::impl::Header*>(mb.data())->msg_id = ::nprpc::impl::MessageId::FunctionCall;
    static_cast<::nprpc::impl::Header*>(mb.data())->msg_type = ::nprpc::impl::MessageType::Request;
  }
  ::nprpc::impl::flat::CallHeader_Direct __ch(buf, sizeof(::nprpc::impl::Header));
  __ch.object_id() = this->object_id();
  __ch.poa_idx() = this->poa_idx();
  __ch.interface_idx() = interface_idx_;
  __ch.function_idx() = 4;
  static_cast<::nprpc::impl::Header*>(buf.data().data())->size = static_cast<uint32_t>(buf.size() - 4);
  ::nprpc::impl::g_orb->call(this->get_endpoint(), buf, this->get_timeout());
  auto std_reply = nprpc::impl::handle_standart_reply(buf);
  if (std_reply != -1) {
    throw nprpc::Exception("Unknown Error");
  }
  test_M4_Direct out(buf, sizeof(::nprpc::impl::Header));
    a = out._1();
    b = (bool)out._2();
    {
      auto span = out._3();
      c.resize(span.size());
      memcpy(c.data(), span.data(), 1 * span.size());
    }
}

void test::ITestBasic_Servant::dispatch(nprpc::Buffers& bufs, [[maybe_unused]] nprpc::SessionContext& ctx, [[maybe_unused]] bool from_parent) {
  nprpc::impl::flat::CallHeader_Direct __ch(bufs(), sizeof(::nprpc::impl::Header));
  switch(__ch.function_idx()) {
    case 0: {
      bool __ret_val;
      __ret_val = ReturnBoolean();
      auto& obuf = bufs();
      obuf.consume(obuf.size());
      obuf.prepare(17);
      obuf.commit(17);
      test_M1_Direct oa(obuf,16);
      oa._1() = __ret_val;
      static_cast<::nprpc::impl::Header*>(obuf.data().data())->size = static_cast<uint32_t>(obuf.size() - 4);
      static_cast<::nprpc::impl::Header*>(obuf.data().data())->msg_id = ::nprpc::impl::MessageId::BlockResponse;
      static_cast<::nprpc::impl::Header*>(obuf.data().data())->msg_type = ::nprpc::impl::MessageType::Answer;
      break;
    }
    case 1: {
      auto& obuf = bufs.flip();
      obuf.consume(obuf.size());
      obuf.prepare(152);
      obuf.commit(24);
      test_M2_Direct oa(obuf,16);
      IdArray __ret_val;
      __ret_val = ReturnIdArray();
      oa._1(static_cast<uint32_t>(__ret_val.size()));
      memcpy(oa._1().data(), __ret_val.data(), __ret_val.size() * 4);
      static_cast<::nprpc::impl::Header*>(obuf.data().data())->size = static_cast<uint32_t>(obuf.size() - 4);
      static_cast<::nprpc::impl::Header*>(obuf.data().data())->msg_id = ::nprpc::impl::MessageId::BlockResponse;
      static_cast<::nprpc::impl::Header*>(obuf.data().data())->msg_type = ::nprpc::impl::MessageType::Answer;
      break;
    }
    case 2: {
      uint32_t __ret_val;
      __ret_val = ReturnU32();
      auto& obuf = bufs();
      obuf.consume(obuf.size());
      obuf.prepare(20);
      obuf.commit(20);
      test_M3_Direct oa(obuf,16);
      oa._1() = __ret_val;
      static_cast<::nprpc::impl::Header*>(obuf.data().data())->size = static_cast<uint32_t>(obuf.size() - 4);
      static_cast<::nprpc::impl::Header*>(obuf.data().data())->msg_id = ::nprpc::impl::MessageId::BlockResponse;
      static_cast<::nprpc::impl::Header*>(obuf.data().data())->msg_type = ::nprpc::impl::MessageType::Answer;
      break;
    }
    case 3: {
      test_M4_Direct ia(bufs(), 32);
      bool __ret_val;
      __ret_val = In(ia._1(), ia._2(), ia._3());
      auto& obuf = bufs();
      obuf.consume(obuf.size());
      obuf.prepare(17);
      obuf.commit(17);
      test_M1_Direct oa(obuf,16);
      oa._1() = __ret_val;
      static_cast<::nprpc::impl::Header*>(obuf.data().data())->size = static_cast<uint32_t>(obuf.size() - 4);
      static_cast<::nprpc::impl::Header*>(obuf.data().data())->msg_id = ::nprpc::impl::MessageId::BlockResponse;
      static_cast<::nprpc::impl::Header*>(obuf.data().data())->msg_type = ::nprpc::impl::MessageType::Answer;
      break;
    }
    case 4: {
      auto& obuf = bufs.flip();
      obuf.consume(obuf.size());
      obuf.prepare(160);
      obuf.commit(32);
      test_M4_Direct oa(obuf,16);
      Out(oa._1(), oa._2(), oa._3_d());
      static_cast<::nprpc::impl::Header*>(obuf.data().data())->size = static_cast<uint32_t>(obuf.size() - 4);
      static_cast<::nprpc::impl::Header*>(obuf.data().data())->msg_id = ::nprpc::impl::MessageId::BlockResponse;
      static_cast<::nprpc::impl::Header*>(obuf.data().data())->msg_type = ::nprpc::impl::MessageType::Answer;
      break;
    }
    default:
      nprpc::impl::make_simple_answer(bufs(), nprpc::impl::MessageId::Error_UnknownFunctionIdx);
  }
}

bool test::TestLargeMessage::In(uint32_t a, bool b, ::nprpc::flat::Span<const uint8_t> c) {
  ::nprpc::flat_buffer buf;
  {
    auto mb = buf.prepare(176);
    buf.commit(48);
    static_cast<::nprpc::impl::Header*>(mb.data())->msg_id = ::nprpc::impl::MessageId::FunctionCall;
    static_cast<::nprpc::impl::Header*>(mb.data())->msg_type = ::nprpc::impl::MessageType::Request;
  }
  ::nprpc::impl::flat::CallHeader_Direct __ch(buf, sizeof(::nprpc::impl::Header));
  __ch.object_id() = this->object_id();
  __ch.poa_idx() = this->poa_idx();
  __ch.interface_idx() = interface_idx_;
  __ch.function_idx() = 0;
  test_M4_Direct _(buf,32);
  _._1() = a;
  _._2() = b;
  _._3(static_cast<uint32_t>(c.size()));
  memcpy(_._3().data(), c.data(), c.size() * 1);
  static_cast<::nprpc::impl::Header*>(buf.data().data())->size = static_cast<uint32_t>(buf.size() - 4);
  ::nprpc::impl::g_orb->call(this->get_endpoint(), buf, this->get_timeout());
  auto std_reply = nprpc::impl::handle_standart_reply(buf);
  if (std_reply != -1) {
    throw nprpc::Exception("Unknown Error");
  }
  test_M1_Direct out(buf, sizeof(::nprpc::impl::Header));
    bool __ret_value;
    __ret_value = (bool)out._1();
  return __ret_value;
}

void test::TestLargeMessage::Out(uint32_t& a, bool& b, std::vector<uint8_t>& c) {
  ::nprpc::flat_buffer buf;
  {
    auto mb = buf.prepare(32);
    buf.commit(32);
    static_cast<::nprpc::impl::Header*>(mb.data())->msg_id = ::nprpc::impl::MessageId::FunctionCall;
    static_cast<::nprpc::impl::Header*>(mb.data())->msg_type = ::nprpc::impl::MessageType::Request;
  }
  ::nprpc::impl::flat::CallHeader_Direct __ch(buf, sizeof(::nprpc::impl::Header));
  __ch.object_id() = this->object_id();
  __ch.poa_idx() = this->poa_idx();
  __ch.interface_idx() = interface_idx_;
  __ch.function_idx() = 1;
  static_cast<::nprpc::impl::Header*>(buf.data().data())->size = static_cast<uint32_t>(buf.size() - 4);
  ::nprpc::impl::g_orb->call(this->get_endpoint(), buf, this->get_timeout());
  auto std_reply = nprpc::impl::handle_standart_reply(buf);
  if (std_reply != -1) {
    throw nprpc::Exception("Unknown Error");
  }
  test_M4_Direct out(buf, sizeof(::nprpc::impl::Header));
    a = out._1();
    b = (bool)out._2();
    {
      auto span = out._3();
      c.resize(span.size());
      memcpy(c.data(), span.data(), 1 * span.size());
    }
}

void test::ITestLargeMessage_Servant::dispatch(nprpc::Buffers& bufs, [[maybe_unused]] nprpc::SessionContext& ctx, [[maybe_unused]] bool from_parent) {
  nprpc::impl::flat::CallHeader_Direct __ch(bufs(), sizeof(::nprpc::impl::Header));
  switch(__ch.function_idx()) {
    case 0: {
      test_M4_Direct ia(bufs(), 32);
      bool __ret_val;
      __ret_val = In(ia._1(), ia._2(), ia._3());
      auto& obuf = bufs();
      obuf.consume(obuf.size());
      obuf.prepare(17);
      obuf.commit(17);
      test_M1_Direct oa(obuf,16);
      oa._1() = __ret_val;
      static_cast<::nprpc::impl::Header*>(obuf.data().data())->size = static_cast<uint32_t>(obuf.size() - 4);
      static_cast<::nprpc::impl::Header*>(obuf.data().data())->msg_id = ::nprpc::impl::MessageId::BlockResponse;
      static_cast<::nprpc::impl::Header*>(obuf.data().data())->msg_type = ::nprpc::impl::MessageType::Answer;
      break;
    }
    case 1: {
      auto& obuf = bufs.flip();
      obuf.consume(obuf.size());
      obuf.prepare(160);
      obuf.commit(32);
      test_M4_Direct oa(obuf,16);
      Out(oa._1(), oa._2(), oa._3_d());
      static_cast<::nprpc::impl::Header*>(obuf.data().data())->size = static_cast<uint32_t>(obuf.size() - 4);
      static_cast<::nprpc::impl::Header*>(obuf.data().data())->msg_id = ::nprpc::impl::MessageId::BlockResponse;
      static_cast<::nprpc::impl::Header*>(obuf.data().data())->msg_type = ::nprpc::impl::MessageType::Answer;
      break;
    }
    default:
      nprpc::impl::make_simple_answer(bufs(), nprpc::impl::MessageId::Error_UnknownFunctionIdx);
  }
}

bool test::TestOptional::InEmpty(const std::optional<uint32_t>& a) {
  ::nprpc::flat_buffer buf;
  {
    auto mb = buf.prepare(164);
    buf.commit(36);
    static_cast<::nprpc::impl::Header*>(mb.data())->msg_id = ::nprpc::impl::MessageId::FunctionCall;
    static_cast<::nprpc::impl::Header*>(mb.data())->msg_type = ::nprpc::impl::MessageType::Request;
  }
  ::nprpc::impl::flat::CallHeader_Direct __ch(buf, sizeof(::nprpc::impl::Header));
  __ch.object_id() = this->object_id();
  __ch.poa_idx() = this->poa_idx();
  __ch.interface_idx() = interface_idx_;
  __ch.function_idx() = 0;
  test_M5_Direct _(buf,32);
  if (a) {
    _._1().alloc();
    _._1().value() = a.value();
  } else { 
    _._1().set_nullopt();
  }
  static_cast<::nprpc::impl::Header*>(buf.data().data())->size = static_cast<uint32_t>(buf.size() - 4);
  ::nprpc::impl::g_orb->call(this->get_endpoint(), buf, this->get_timeout());
  auto std_reply = nprpc::impl::handle_standart_reply(buf);
  if (std_reply != -1) {
    throw nprpc::Exception("Unknown Error");
  }
  test_M1_Direct out(buf, sizeof(::nprpc::impl::Header));
    bool __ret_value;
    __ret_value = (bool)out._1();
  return __ret_value;
}

bool test::TestOptional::In(const std::optional<uint32_t>& a, const std::optional<test::AAA>& b) {
  ::nprpc::flat_buffer buf;
  {
    auto mb = buf.prepare(168);
    buf.commit(40);
    static_cast<::nprpc::impl::Header*>(mb.data())->msg_id = ::nprpc::impl::MessageId::FunctionCall;
    static_cast<::nprpc::impl::Header*>(mb.data())->msg_type = ::nprpc::impl::MessageType::Request;
  }
  ::nprpc::impl::flat::CallHeader_Direct __ch(buf, sizeof(::nprpc::impl::Header));
  __ch.object_id() = this->object_id();
  __ch.poa_idx() = this->poa_idx();
  __ch.interface_idx() = interface_idx_;
  __ch.function_idx() = 1;
  test_M6_Direct _(buf,32);
  if (a) {
    _._1().alloc();
    _._1().value() = a.value();
  } else { 
    _._1().set_nullopt();
  }
  if (b) {
    _._2().alloc();
    auto value = _._2().value();
    value.a() = b.value().a;
    value.b(b.value().b);
    value.c(b.value().c);
  } else { 
    _._2().set_nullopt();
  }
  static_cast<::nprpc::impl::Header*>(buf.data().data())->size = static_cast<uint32_t>(buf.size() - 4);
  ::nprpc::impl::g_orb->call(this->get_endpoint(), buf, this->get_timeout());
  auto std_reply = nprpc::impl::handle_standart_reply(buf);
  if (std_reply != -1) {
    throw nprpc::Exception("Unknown Error");
  }
  test_M1_Direct out(buf, sizeof(::nprpc::impl::Header));
    bool __ret_value;
    __ret_value = (bool)out._1();
  return __ret_value;
}

void test::TestOptional::OutEmpty(std::optional<uint32_t>& a) {
  ::nprpc::flat_buffer buf;
  {
    auto mb = buf.prepare(32);
    buf.commit(32);
    static_cast<::nprpc::impl::Header*>(mb.data())->msg_id = ::nprpc::impl::MessageId::FunctionCall;
    static_cast<::nprpc::impl::Header*>(mb.data())->msg_type = ::nprpc::impl::MessageType::Request;
  }
  ::nprpc::impl::flat::CallHeader_Direct __ch(buf, sizeof(::nprpc::impl::Header));
  __ch.object_id() = this->object_id();
  __ch.poa_idx() = this->poa_idx();
  __ch.interface_idx() = interface_idx_;
  __ch.function_idx() = 2;
  static_cast<::nprpc::impl::Header*>(buf.data().data())->size = static_cast<uint32_t>(buf.size() - 4);
  ::nprpc::impl::g_orb->call(this->get_endpoint(), buf, this->get_timeout());
  auto std_reply = nprpc::impl::handle_standart_reply(buf);
  if (std_reply != -1) {
    throw nprpc::Exception("Unknown Error");
  }
  test_M5_Direct out(buf, sizeof(::nprpc::impl::Header));
    {
      auto opt = out._1();
      if (opt.has_value()) {
        a = std::decay<decltype(a)>::type::value_type{};
        auto& value_to = a.value();
        value_to = opt.value();
      } else { 
        a = std::nullopt;
      }
    }
}

void test::TestOptional::Out(std::optional<uint32_t>& a) {
  ::nprpc::flat_buffer buf;
  {
    auto mb = buf.prepare(32);
    buf.commit(32);
    static_cast<::nprpc::impl::Header*>(mb.data())->msg_id = ::nprpc::impl::MessageId::FunctionCall;
    static_cast<::nprpc::impl::Header*>(mb.data())->msg_type = ::nprpc::impl::MessageType::Request;
  }
  ::nprpc::impl::flat::CallHeader_Direct __ch(buf, sizeof(::nprpc::impl::Header));
  __ch.object_id() = this->object_id();
  __ch.poa_idx() = this->poa_idx();
  __ch.interface_idx() = interface_idx_;
  __ch.function_idx() = 3;
  static_cast<::nprpc::impl::Header*>(buf.data().data())->size = static_cast<uint32_t>(buf.size() - 4);
  ::nprpc::impl::g_orb->call(this->get_endpoint(), buf, this->get_timeout());
  auto std_reply = nprpc::impl::handle_standart_reply(buf);
  if (std_reply != -1) {
    throw nprpc::Exception("Unknown Error");
  }
  test_M5_Direct out(buf, sizeof(::nprpc::impl::Header));
    {
      auto opt = out._1();
      if (opt.has_value()) {
        a = std::decay<decltype(a)>::type::value_type{};
        auto& value_to = a.value();
        value_to = opt.value();
      } else { 
        a = std::nullopt;
      }
    }
}

Opt1 test::TestOptional::ReturnOpt1() {
  ::nprpc::flat_buffer buf;
  {
    auto mb = buf.prepare(32);
    buf.commit(32);
    static_cast<::nprpc::impl::Header*>(mb.data())->msg_id = ::nprpc::impl::MessageId::FunctionCall;
    static_cast<::nprpc::impl::Header*>(mb.data())->msg_type = ::nprpc::impl::MessageType::Request;
  }
  ::nprpc::impl::flat::CallHeader_Direct __ch(buf, sizeof(::nprpc::impl::Header));
  __ch.object_id() = this->object_id();
  __ch.poa_idx() = this->poa_idx();
  __ch.interface_idx() = interface_idx_;
  __ch.function_idx() = 4;
  static_cast<::nprpc::impl::Header*>(buf.data().data())->size = static_cast<uint32_t>(buf.size() - 4);
  ::nprpc::impl::g_orb->call(this->get_endpoint(), buf, this->get_timeout());
  auto std_reply = nprpc::impl::handle_standart_reply(buf);
  if (std_reply != -1) {
    throw nprpc::Exception("Unknown Error");
  }
  test_M7_Direct out(buf, sizeof(::nprpc::impl::Header));
    Opt1 __ret_value;
    __ret_value.str = (std::string_view)out._1().str();
    {
      auto opt = out._1().stream();
      if (opt.has_value()) {
        __ret_value.stream = std::decay<decltype(__ret_value.stream)>::type::value_type{};
        auto& value_to = __ret_value.stream.value();
        auto value_from = opt.value();
        {
          auto span = value_from();
          value_to.resize(span.size());
          memcpy(value_to.data(), span.data(), 1 * span.size());
        }
      } else { 
        __ret_value.stream = std::nullopt;
      }
    }
  return __ret_value;
}

void test::ITestOptional_Servant::dispatch(nprpc::Buffers& bufs, [[maybe_unused]] nprpc::SessionContext& ctx, [[maybe_unused]] bool from_parent) {
  nprpc::impl::flat::CallHeader_Direct __ch(bufs(), sizeof(::nprpc::impl::Header));
  switch(__ch.function_idx()) {
    case 0: {
      test_M5_Direct ia(bufs(), 32);
      bool __ret_val;
      __ret_val = InEmpty(ia._1());
      auto& obuf = bufs();
      obuf.consume(obuf.size());
      obuf.prepare(17);
      obuf.commit(17);
      test_M1_Direct oa(obuf,16);
      oa._1() = __ret_val;
      static_cast<::nprpc::impl::Header*>(obuf.data().data())->size = static_cast<uint32_t>(obuf.size() - 4);
      static_cast<::nprpc::impl::Header*>(obuf.data().data())->msg_id = ::nprpc::impl::MessageId::BlockResponse;
      static_cast<::nprpc::impl::Header*>(obuf.data().data())->msg_type = ::nprpc::impl::MessageType::Answer;
      break;
    }
    case 1: {
      test_M6_Direct ia(bufs(), 32);
      bool __ret_val;
      __ret_val = In(ia._1(), ia._2());
      auto& obuf = bufs();
      obuf.consume(obuf.size());
      obuf.prepare(17);
      obuf.commit(17);
      test_M1_Direct oa(obuf,16);
      oa._1() = __ret_val;
      static_cast<::nprpc::impl::Header*>(obuf.data().data())->size = static_cast<uint32_t>(obuf.size() - 4);
      static_cast<::nprpc::impl::Header*>(obuf.data().data())->msg_id = ::nprpc::impl::MessageId::BlockResponse;
      static_cast<::nprpc::impl::Header*>(obuf.data().data())->msg_type = ::nprpc::impl::MessageType::Answer;
      break;
    }
    case 2: {
      auto& obuf = bufs.flip();
      obuf.consume(obuf.size());
      obuf.prepare(148);
      obuf.commit(20);
      test_M5_Direct oa(obuf,16);
      OutEmpty(oa._1());
      static_cast<::nprpc::impl::Header*>(obuf.data().data())->size = static_cast<uint32_t>(obuf.size() - 4);
      static_cast<::nprpc::impl::Header*>(obuf.data().data())->msg_id = ::nprpc::impl::MessageId::BlockResponse;
      static_cast<::nprpc::impl::Header*>(obuf.data().data())->msg_type = ::nprpc::impl::MessageType::Answer;
      break;
    }
    case 3: {
      auto& obuf = bufs.flip();
      obuf.consume(obuf.size());
      obuf.prepare(148);
      obuf.commit(20);
      test_M5_Direct oa(obuf,16);
      Out(oa._1());
      static_cast<::nprpc::impl::Header*>(obuf.data().data())->size = static_cast<uint32_t>(obuf.size() - 4);
      static_cast<::nprpc::impl::Header*>(obuf.data().data())->msg_id = ::nprpc::impl::MessageId::BlockResponse;
      static_cast<::nprpc::impl::Header*>(obuf.data().data())->msg_type = ::nprpc::impl::MessageType::Answer;
      break;
    }
    case 4: {
      auto& obuf = bufs.flip();
      obuf.consume(obuf.size());
      obuf.prepare(156);
      obuf.commit(28);
      test_M7_Direct oa(obuf,16);
      Opt1 __ret_val;
      __ret_val = ReturnOpt1();
      oa._1().str(__ret_val.str);
      if (__ret_val.stream) {
        oa._1().stream().alloc();
        auto value = oa._1().stream().value();
        value.length(static_cast<uint32_t>(__ret_val.stream.value().size()));
        memcpy(value().data(), __ret_val.stream.value().data(), __ret_val.stream.value().size() * 1);
      } else { 
        oa._1().stream().set_nullopt();
      }
      static_cast<::nprpc::impl::Header*>(obuf.data().data())->size = static_cast<uint32_t>(obuf.size() - 4);
      static_cast<::nprpc::impl::Header*>(obuf.data().data())->msg_id = ::nprpc::impl::MessageId::BlockResponse;
      static_cast<::nprpc::impl::Header*>(obuf.data().data())->msg_type = ::nprpc::impl::MessageType::Answer;
      break;
    }
    default:
      nprpc::impl::make_simple_answer(bufs(), nprpc::impl::MessageId::Error_UnknownFunctionIdx);
  }
}

void test::TestNested::Out(std::optional<test::BBB>& a) {
  ::nprpc::flat_buffer buf;
  {
    auto mb = buf.prepare(32);
    buf.commit(32);
    static_cast<::nprpc::impl::Header*>(mb.data())->msg_id = ::nprpc::impl::MessageId::FunctionCall;
    static_cast<::nprpc::impl::Header*>(mb.data())->msg_type = ::nprpc::impl::MessageType::Request;
  }
  ::nprpc::impl::flat::CallHeader_Direct __ch(buf, sizeof(::nprpc::impl::Header));
  __ch.object_id() = this->object_id();
  __ch.poa_idx() = this->poa_idx();
  __ch.interface_idx() = interface_idx_;
  __ch.function_idx() = 0;
  static_cast<::nprpc::impl::Header*>(buf.data().data())->size = static_cast<uint32_t>(buf.size() - 4);
  ::nprpc::impl::g_orb->call(this->get_endpoint(), buf, this->get_timeout());
  auto std_reply = nprpc::impl::handle_standart_reply(buf);
  if (std_reply != -1) {
    throw nprpc::Exception("Unknown Error");
  }
  test_M8_Direct out(buf, sizeof(::nprpc::impl::Header));
    {
      auto opt = out._1();
      if (opt.has_value()) {
        a = std::decay<decltype(a)>::type::value_type{};
        auto& value_to = a.value();
        auto value_from = opt.value();
        {
          auto span = value_from.a();
          value_to.a.resize(span.size());
          auto it5 = std::begin(value_to.a);
          for (auto e : span) {
            (*it5).a = e.a();
            (*it5).b = (std::string_view)e.b();
            (*it5).c = (std::string_view)e.c();
            ++it5;
          }
        }
        {
          auto span = value_from.b();
          value_to.b.resize(span.size());
          auto it5 = std::begin(value_to.b);
          for (auto e : span) {
            (*it5).a = (std::string_view)e.a();
            (*it5).b = (std::string_view)e.b();
            {
              auto opt = e.c();
              if (opt.has_value()) {
                (*it5).c = std::decay<decltype((*it5).c)>::type::value_type{};
                auto& value_to = (*it5).c.value();
                value_to = (bool)opt.value();
              } else { 
                (*it5).c = std::nullopt;
              }
            }
            ++it5;
          }
        }
      } else { 
        a = std::nullopt;
      }
    }
}

TopLevel test::TestNested::ReturnNested() {
  ::nprpc::flat_buffer buf;
  {
    auto mb = buf.prepare(32);
    buf.commit(32);
    static_cast<::nprpc::impl::Header*>(mb.data())->msg_id = ::nprpc::impl::MessageId::FunctionCall;
    static_cast<::nprpc::impl::Header*>(mb.data())->msg_type = ::nprpc::impl::MessageType::Request;
  }
  ::nprpc::impl::flat::CallHeader_Direct __ch(buf, sizeof(::nprpc::impl::Header));
  __ch.object_id() = this->object_id();
  __ch.poa_idx() = this->poa_idx();
  __ch.interface_idx() = interface_idx_;
  __ch.function_idx() = 1;
  static_cast<::nprpc::impl::Header*>(buf.data().data())->size = static_cast<uint32_t>(buf.size() - 4);
  ::nprpc::impl::g_orb->call(this->get_endpoint(), buf, this->get_timeout());
  auto std_reply = nprpc::impl::handle_standart_reply(buf);
  if (std_reply != -1) {
    throw nprpc::Exception("Unknown Error");
  }
  test_M9_Direct out(buf, sizeof(::nprpc::impl::Header));
    TopLevel __ret_value;
    __ret_value.x = (std::string_view)out._1().x();
    __ret_value.y.x = (std::string_view)out._1().y().x();
    __ret_value.y.y.x = (std::string_view)out._1().y().y().x();
    {
      auto span = out._1().y().y().y();
      __ret_value.y.y.y.resize(span.size());
      memcpy(__ret_value.y.y.y.data(), span.data(), 1 * span.size());
    }
    __ret_value.y.y.z = out._1().y().y().z();
    __ret_value.y.z = out._1().y().z();
    __ret_value.z = out._1().z();
  return __ret_value;
}

void test::ITestNested_Servant::dispatch(nprpc::Buffers& bufs, [[maybe_unused]] nprpc::SessionContext& ctx, [[maybe_unused]] bool from_parent) {
  nprpc::impl::flat::CallHeader_Direct __ch(bufs(), sizeof(::nprpc::impl::Header));
  switch(__ch.function_idx()) {
    case 0: {
      auto& obuf = bufs.flip();
      obuf.consume(obuf.size());
      obuf.prepare(148);
      obuf.commit(20);
      test_M8_Direct oa(obuf,16);
      Out(oa._1());
      static_cast<::nprpc::impl::Header*>(obuf.data().data())->size = static_cast<uint32_t>(obuf.size() - 4);
      static_cast<::nprpc::impl::Header*>(obuf.data().data())->msg_id = ::nprpc::impl::MessageId::BlockResponse;
      static_cast<::nprpc::impl::Header*>(obuf.data().data())->msg_type = ::nprpc::impl::MessageType::Answer;
      break;
    }
    case 1: {
      auto& obuf = bufs.flip();
      obuf.consume(obuf.size());
      obuf.prepare(200);
      obuf.commit(72);
      test_M9_Direct oa(obuf,16);
      TopLevel __ret_val;
      __ret_val = ReturnNested();
      oa._1().x(__ret_val.x);
      oa._1().y().x(__ret_val.y.x);
      oa._1().y().y().x(__ret_val.y.y.x);
      oa._1().y().y().y(static_cast<uint32_t>(__ret_val.y.y.y.size()));
      memcpy(oa._1().y().y().y().data(), __ret_val.y.y.y.data(), __ret_val.y.y.y.size() * 1);
      oa._1().y().y().z() = __ret_val.y.y.z;
      oa._1().y().z() = __ret_val.y.z;
      oa._1().z() = __ret_val.z;
      static_cast<::nprpc::impl::Header*>(obuf.data().data())->size = static_cast<uint32_t>(obuf.size() - 4);
      static_cast<::nprpc::impl::Header*>(obuf.data().data())->msg_id = ::nprpc::impl::MessageId::BlockResponse;
      static_cast<::nprpc::impl::Header*>(obuf.data().data())->msg_type = ::nprpc::impl::MessageType::Answer;
      break;
    }
    default:
      nprpc::impl::make_simple_answer(bufs(), nprpc::impl::MessageId::Error_UnknownFunctionIdx);
  }
}

void test::TestBadInput::In(::nprpc::flat::Span<const uint8_t> a) {
  ::nprpc::flat_buffer buf;
  {
    auto mb = buf.prepare(168);
    buf.commit(40);
    static_cast<::nprpc::impl::Header*>(mb.data())->msg_id = ::nprpc::impl::MessageId::FunctionCall;
    static_cast<::nprpc::impl::Header*>(mb.data())->msg_type = ::nprpc::impl::MessageType::Request;
  }
  ::nprpc::impl::flat::CallHeader_Direct __ch(buf, sizeof(::nprpc::impl::Header));
  __ch.object_id() = this->object_id();
  __ch.poa_idx() = this->poa_idx();
  __ch.interface_idx() = interface_idx_;
  __ch.function_idx() = 0;
  test_M10_Direct _(buf,32);
  _._1(static_cast<uint32_t>(a.size()));
  memcpy(_._1().data(), a.data(), a.size() * 1);
  static_cast<::nprpc::impl::Header*>(buf.data().data())->size = static_cast<uint32_t>(buf.size() - 4);
  ::nprpc::impl::g_orb->call(this->get_endpoint(), buf, this->get_timeout());
  auto std_reply = nprpc::impl::handle_standart_reply(buf);
  if (std_reply != 0) {
    throw nprpc::Exception("Unknown Error");
  }
}

void test::ITestBadInput_Servant::dispatch(nprpc::Buffers& bufs, [[maybe_unused]] nprpc::SessionContext& ctx, [[maybe_unused]] bool from_parent) {
  nprpc::impl::flat::CallHeader_Direct __ch(bufs(), sizeof(::nprpc::impl::Header));
  switch(__ch.function_idx()) {
    case 0: {
      test_M10_Direct ia(bufs(), 32);
      if ( !check_1VFu8(bufs(), ia) ) break;
      In(ia._1());
      nprpc::impl::make_simple_answer(bufs(), nprpc::impl::MessageId::Success);
      break;
    }
    default:
      nprpc::impl::make_simple_answer(bufs(), nprpc::impl::MessageId::Error_UnknownFunctionIdx);
  }
}

} // namespace test

