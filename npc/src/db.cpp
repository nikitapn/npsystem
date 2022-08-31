#include "db.hpp"
#include <nprpc/impl/nprpc_impl.hpp>

void db_throw_exception(::nprpc::flat_buffer& buf);

namespace {
struct db_M1 {
  uint64_t _1;
  ::nprpc::flat::Vector<uint8_t> _2;
};

class db_M1_Direct {
  ::nprpc::flat_buffer& buffer_;
  const std::uint32_t offset_;

  auto& base() noexcept { return *reinterpret_cast<db_M1*>(reinterpret_cast<std::byte*>(buffer_.data().data()) + offset_); }
  auto const& base() const noexcept { return *reinterpret_cast<const db_M1*>(reinterpret_cast<const std::byte*>(buffer_.data().data()) + offset_); }
public:
  uint32_t offset() const noexcept { return offset_; }
  void* __data() noexcept { return (void*)&base(); }
  db_M1_Direct(::nprpc::flat_buffer& buffer, std::uint32_t offset)
    : buffer_(buffer)
    , offset_(offset)
  {
  }
  const uint64_t& _1() const noexcept { return base()._1;}
  uint64_t& _1() noexcept { return base()._1;}
  void _2(std::uint32_t elements_size) { new (&base()._2) ::nprpc::flat::Vector<uint8_t>(buffer_, elements_size); }
  auto _2_d() noexcept { return ::nprpc::flat::Vector_Direct1<uint8_t>(buffer_, offset_ + offsetof(db_M1, _2)); }
  auto _2() noexcept { return (::nprpc::flat::Span<uint8_t>)base()._2; }
};

struct db_M2 {
  uint64_t _1;
};

class db_M2_Direct {
  ::nprpc::flat_buffer& buffer_;
  const std::uint32_t offset_;

  auto& base() noexcept { return *reinterpret_cast<db_M2*>(reinterpret_cast<std::byte*>(buffer_.data().data()) + offset_); }
  auto const& base() const noexcept { return *reinterpret_cast<const db_M2*>(reinterpret_cast<const std::byte*>(buffer_.data().data()) + offset_); }
public:
  uint32_t offset() const noexcept { return offset_; }
  void* __data() noexcept { return (void*)&base(); }
  db_M2_Direct(::nprpc::flat_buffer& buffer, std::uint32_t offset)
    : buffer_(buffer)
    , offset_(offset)
  {
  }
  const uint64_t& _1() const noexcept { return base()._1;}
  uint64_t& _1() noexcept { return base()._1;}
};

struct db_M3 {
  ::nprpc::flat::String _1;
};

class db_M3_Direct {
  ::nprpc::flat_buffer& buffer_;
  const std::uint32_t offset_;

  auto& base() noexcept { return *reinterpret_cast<db_M3*>(reinterpret_cast<std::byte*>(buffer_.data().data()) + offset_); }
  auto const& base() const noexcept { return *reinterpret_cast<const db_M3*>(reinterpret_cast<const std::byte*>(buffer_.data().data()) + offset_); }
public:
  uint32_t offset() const noexcept { return offset_; }
  void* __data() noexcept { return (void*)&base(); }
  db_M3_Direct(::nprpc::flat_buffer& buffer, std::uint32_t offset)
    : buffer_(buffer)
    , offset_(offset)
  {
  }
  void _1(const char* str) { new (&base()._1) ::nprpc::flat::String(buffer_, str); }
  void _1(const std::string& str) { new (&base()._1) ::nprpc::flat::String(buffer_, str); }
  auto _1() noexcept { return (::nprpc::flat::Span<char>)base()._1; }
  auto _1() const noexcept { return (::nprpc::flat::Span<const char>)base()._1; }
  auto _1_d() noexcept {     return ::nprpc::flat::String_Direct1(buffer_, offset_ + offsetof(db_M3, _1));  }
};

struct db_M4 {
  ::nprpc::flat::Array<uint8_t,16> _1;
};

class db_M4_Direct {
  ::nprpc::flat_buffer& buffer_;
  const std::uint32_t offset_;

  auto& base() noexcept { return *reinterpret_cast<db_M4*>(reinterpret_cast<std::byte*>(buffer_.data().data()) + offset_); }
  auto const& base() const noexcept { return *reinterpret_cast<const db_M4*>(reinterpret_cast<const std::byte*>(buffer_.data().data()) + offset_); }
public:
  uint32_t offset() const noexcept { return offset_; }
  void* __data() noexcept { return (void*)&base(); }
  db_M4_Direct(::nprpc::flat_buffer& buffer, std::uint32_t offset)
    : buffer_(buffer)
    , offset_(offset)
  {
  }
  auto _1() noexcept { return (::nprpc::flat::Span<uint8_t>)base()._1; }
};

struct db_M5 {
  ::nprpc::flat::Vector<uint8_t> _1;
  ::nprpc::flat::Boolean _2;
};

class db_M5_Direct {
  ::nprpc::flat_buffer& buffer_;
  const std::uint32_t offset_;

  auto& base() noexcept { return *reinterpret_cast<db_M5*>(reinterpret_cast<std::byte*>(buffer_.data().data()) + offset_); }
  auto const& base() const noexcept { return *reinterpret_cast<const db_M5*>(reinterpret_cast<const std::byte*>(buffer_.data().data()) + offset_); }
public:
  uint32_t offset() const noexcept { return offset_; }
  void* __data() noexcept { return (void*)&base(); }
  db_M5_Direct(::nprpc::flat_buffer& buffer, std::uint32_t offset)
    : buffer_(buffer)
    , offset_(offset)
  {
  }
  void _1(std::uint32_t elements_size) { new (&base()._1) ::nprpc::flat::Vector<uint8_t>(buffer_, elements_size); }
  auto _1_d() noexcept { return ::nprpc::flat::Vector_Direct1<uint8_t>(buffer_, offset_ + offsetof(db_M5, _1)); }
  auto _1() noexcept { return (::nprpc::flat::Span<uint8_t>)base()._1; }
  const ::nprpc::flat::Boolean& _2() const noexcept { return base()._2;}
  ::nprpc::flat::Boolean& _2() noexcept { return base()._2;}
};

struct db_M6 {
  uint64_t _1;
  ::nprpc::flat::Vector<uint8_t> _2;
  ::nprpc::flat::Boolean _3;
};

class db_M6_Direct {
  ::nprpc::flat_buffer& buffer_;
  const std::uint32_t offset_;

  auto& base() noexcept { return *reinterpret_cast<db_M6*>(reinterpret_cast<std::byte*>(buffer_.data().data()) + offset_); }
  auto const& base() const noexcept { return *reinterpret_cast<const db_M6*>(reinterpret_cast<const std::byte*>(buffer_.data().data()) + offset_); }
public:
  uint32_t offset() const noexcept { return offset_; }
  void* __data() noexcept { return (void*)&base(); }
  db_M6_Direct(::nprpc::flat_buffer& buffer, std::uint32_t offset)
    : buffer_(buffer)
    , offset_(offset)
  {
  }
  const uint64_t& _1() const noexcept { return base()._1;}
  uint64_t& _1() noexcept { return base()._1;}
  void _2(std::uint32_t elements_size) { new (&base()._2) ::nprpc::flat::Vector<uint8_t>(buffer_, elements_size); }
  auto _2_d() noexcept { return ::nprpc::flat::Vector_Direct1<uint8_t>(buffer_, offset_ + offsetof(db_M6, _2)); }
  auto _2() noexcept { return (::nprpc::flat::Span<uint8_t>)base()._2; }
  const ::nprpc::flat::Boolean& _3() const noexcept { return base()._3;}
  ::nprpc::flat::Boolean& _3() noexcept { return base()._3;}
};

struct db_M7 {
  ::nprpc::flat::Vector<npd::flat::BatchOperation> _1;
};

class db_M7_Direct {
  ::nprpc::flat_buffer& buffer_;
  const std::uint32_t offset_;

  auto& base() noexcept { return *reinterpret_cast<db_M7*>(reinterpret_cast<std::byte*>(buffer_.data().data()) + offset_); }
  auto const& base() const noexcept { return *reinterpret_cast<const db_M7*>(reinterpret_cast<const std::byte*>(buffer_.data().data()) + offset_); }
public:
  uint32_t offset() const noexcept { return offset_; }
  void* __data() noexcept { return (void*)&base(); }
  db_M7_Direct(::nprpc::flat_buffer& buffer, std::uint32_t offset)
    : buffer_(buffer)
    , offset_(offset)
  {
  }
  void _1(std::uint32_t elements_size) { new (&base()._1) ::nprpc::flat::Vector<npd::flat::BatchOperation>(buffer_, elements_size); }
  auto _1_d() noexcept { return ::nprpc::flat::Vector_Direct2<npd::flat::BatchOperation,npd::flat::BatchOperation_Direct>(buffer_, offset_ + offsetof(db_M7, _1)); }
  auto _1() noexcept { return ::nprpc::flat::Span_ref<npd::flat::BatchOperation, npd::flat::BatchOperation_Direct>(buffer_, base()._1.range(buffer_.data().data())); }
};

struct db_M8 {
  ::nprpc::flat::Boolean _1;
};

class db_M8_Direct {
  ::nprpc::flat_buffer& buffer_;
  const std::uint32_t offset_;

  auto& base() noexcept { return *reinterpret_cast<db_M8*>(reinterpret_cast<std::byte*>(buffer_.data().data()) + offset_); }
  auto const& base() const noexcept { return *reinterpret_cast<const db_M8*>(reinterpret_cast<const std::byte*>(buffer_.data().data()) + offset_); }
public:
  uint32_t offset() const noexcept { return offset_; }
  void* __data() noexcept { return (void*)&base(); }
  db_M8_Direct(::nprpc::flat_buffer& buffer, std::uint32_t offset)
    : buffer_(buffer)
    , offset_(offset)
  {
  }
  const ::nprpc::flat::Boolean& _1() const noexcept { return base()._1;}
  ::nprpc::flat::Boolean& _1() noexcept { return base()._1;}
};

struct db_M9 {
  ::nprpc::flat::Boolean _1;
  ::nprpc::flat::Vector<uint8_t> _2;
};

class db_M9_Direct {
  ::nprpc::flat_buffer& buffer_;
  const std::uint32_t offset_;

  auto& base() noexcept { return *reinterpret_cast<db_M9*>(reinterpret_cast<std::byte*>(buffer_.data().data()) + offset_); }
  auto const& base() const noexcept { return *reinterpret_cast<const db_M9*>(reinterpret_cast<const std::byte*>(buffer_.data().data()) + offset_); }
public:
  uint32_t offset() const noexcept { return offset_; }
  void* __data() noexcept { return (void*)&base(); }
  db_M9_Direct(::nprpc::flat_buffer& buffer, std::uint32_t offset)
    : buffer_(buffer)
    , offset_(offset)
  {
  }
  const ::nprpc::flat::Boolean& _1() const noexcept { return base()._1;}
  ::nprpc::flat::Boolean& _1() noexcept { return base()._1;}
  void _2(std::uint32_t elements_size) { new (&base()._2) ::nprpc::flat::Vector<uint8_t>(buffer_, elements_size); }
  auto _2_d() noexcept { return ::nprpc::flat::Vector_Direct1<uint8_t>(buffer_, offset_ + offsetof(db_M9, _2)); }
  auto _2() noexcept { return (::nprpc::flat::Span<uint8_t>)base()._2; }
};

struct db_M10 {
  ::nprpc::flat::Vector<uint64_t> _1;
};

class db_M10_Direct {
  ::nprpc::flat_buffer& buffer_;
  const std::uint32_t offset_;

  auto& base() noexcept { return *reinterpret_cast<db_M10*>(reinterpret_cast<std::byte*>(buffer_.data().data()) + offset_); }
  auto const& base() const noexcept { return *reinterpret_cast<const db_M10*>(reinterpret_cast<const std::byte*>(buffer_.data().data()) + offset_); }
public:
  uint32_t offset() const noexcept { return offset_; }
  void* __data() noexcept { return (void*)&base(); }
  db_M10_Direct(::nprpc::flat_buffer& buffer, std::uint32_t offset)
    : buffer_(buffer)
    , offset_(offset)
  {
  }
  void _1(std::uint32_t elements_size) { new (&base()._1) ::nprpc::flat::Vector<uint64_t>(buffer_, elements_size); }
  auto _1_d() noexcept { return ::nprpc::flat::Vector_Direct1<uint64_t>(buffer_, offset_ + offsetof(db_M10, _1)); }
  auto _1() noexcept { return (::nprpc::flat::Span<uint64_t>)base()._1; }
};

struct db_M11 {
  uint64_t _1;
  ::nprpc::flat::Vector<uint8_t> _2;
};

class db_M11_Direct {
  ::nprpc::flat_buffer& buffer_;
  const std::uint32_t offset_;

  auto& base() noexcept { return *reinterpret_cast<db_M11*>(reinterpret_cast<std::byte*>(buffer_.data().data()) + offset_); }
  auto const& base() const noexcept { return *reinterpret_cast<const db_M11*>(reinterpret_cast<const std::byte*>(buffer_.data().data()) + offset_); }
public:
  uint32_t offset() const noexcept { return offset_; }
  void* __data() noexcept { return (void*)&base(); }
  db_M11_Direct(::nprpc::flat_buffer& buffer, std::uint32_t offset)
    : buffer_(buffer)
    , offset_(offset)
  {
  }
  const uint64_t& _1() const noexcept { return base()._1;}
  uint64_t& _1() noexcept { return base()._1;}
  void _2(std::uint32_t elements_size) { new (&base()._2) ::nprpc::flat::Vector<uint8_t>(buffer_, elements_size); }
  auto _2_d() noexcept { return ::nprpc::flat::Vector_Direct1<uint8_t>(buffer_, offset_ + offsetof(db_M11, _2)); }
  auto _2() noexcept { return (::nprpc::flat::Span<uint8_t>)base()._2; }
};

struct db_M12 {
  uint64_t _1;
  nprpc::detail::flat::ObjectId _2;
};

class db_M12_Direct {
  ::nprpc::flat_buffer& buffer_;
  const std::uint32_t offset_;

  auto& base() noexcept { return *reinterpret_cast<db_M12*>(reinterpret_cast<std::byte*>(buffer_.data().data()) + offset_); }
  auto const& base() const noexcept { return *reinterpret_cast<const db_M12*>(reinterpret_cast<const std::byte*>(buffer_.data().data()) + offset_); }
public:
  uint32_t offset() const noexcept { return offset_; }
  void* __data() noexcept { return (void*)&base(); }
  db_M12_Direct(::nprpc::flat_buffer& buffer, std::uint32_t offset)
    : buffer_(buffer)
    , offset_(offset)
  {
  }
  const uint64_t& _1() const noexcept { return base()._1;}
  uint64_t& _1() noexcept { return base()._1;}
  auto _2() noexcept { return nprpc::detail::flat::ObjectId_Direct(buffer_, offset_ + offsetof(db_M12, _2)); }
};


} // 

namespace cbt1 { 
} // namespace cbt1

namespace npd { 
void npd::NodeCallback::OnNodeChanged(/*in*/const cbt1::oid_t& id, /*in*/::nprpc::flat::Span<const uint8_t> data) {
  ::nprpc::flat_buffer buf;
  {
    auto mb = buf.prepare(176);
    buf.commit(48);
    static_cast<::nprpc::impl::Header*>(mb.data())->msg_id = ::nprpc::impl::MessageId::FunctionCall;
    static_cast<::nprpc::impl::Header*>(mb.data())->msg_type = ::nprpc::impl::MessageType::Request;
  }
  ::nprpc::impl::flat::CallHeader_Direct __ch(buf, sizeof(::nprpc::impl::Header));
  __ch.object_id() = this->_data().object_id;
  __ch.poa_idx() = this->_data().poa_idx;
  __ch.interface_idx() = interface_idx_;
  __ch.function_idx() = 0;
  db_M1_Direct _(buf,32);
  _._1() = id;
  _._2(static_cast<uint32_t>(data.size()));
  memcpy(_._2().data(), data.data(), data.size() * 1);
  static_cast<::nprpc::impl::Header*>(buf.data().data())->size = static_cast<uint32_t>(buf.size() - 4);
  ::nprpc::impl::g_orb->call(this->get_endpoint(), buf, this->get_timeout());
  auto std_reply = nprpc::impl::handle_standart_reply(buf);
  if (std_reply != 0) {
    std::cerr << "received an unusual reply for function with no output arguments\n";
  }
}

void npd::NodeCallback::OnNodeDeleted(/*in*/const cbt1::oid_t& id) {
  ::nprpc::flat_buffer buf;
  {
    auto mb = buf.prepare(40);
    buf.commit(40);
    static_cast<::nprpc::impl::Header*>(mb.data())->msg_id = ::nprpc::impl::MessageId::FunctionCall;
    static_cast<::nprpc::impl::Header*>(mb.data())->msg_type = ::nprpc::impl::MessageType::Request;
  }
  ::nprpc::impl::flat::CallHeader_Direct __ch(buf, sizeof(::nprpc::impl::Header));
  __ch.object_id() = this->_data().object_id;
  __ch.poa_idx() = this->_data().poa_idx;
  __ch.interface_idx() = interface_idx_;
  __ch.function_idx() = 1;
  db_M2_Direct _(buf,32);
  _._1() = id;
  static_cast<::nprpc::impl::Header*>(buf.data().data())->size = static_cast<uint32_t>(buf.size() - 4);
  ::nprpc::impl::g_orb->call(this->get_endpoint(), buf, this->get_timeout());
  auto std_reply = nprpc::impl::handle_standart_reply(buf);
  if (std_reply != 0) {
    std::cerr << "received an unusual reply for function with no output arguments\n";
  }
}

void npd::INodeCallback_Servant::dispatch(nprpc::Buffers& bufs, nprpc::EndPoint remote_endpoint, bool from_parent, nprpc::ReferenceList& ref_list) {
  nprpc::impl::flat::CallHeader_Direct __ch(bufs(), sizeof(::nprpc::impl::Header));
  switch(__ch.function_idx()) {
    case 0: {
      db_M1_Direct ia(bufs(), 32);
      OnNodeChanged(ia._1(), ia._2());
      nprpc::impl::make_simple_answer(bufs(), nprpc::impl::MessageId::Success);
      break;
    }
    case 1: {
      db_M2_Direct ia(bufs(), 32);
      OnNodeDeleted(ia._1());
      nprpc::impl::make_simple_answer(bufs(), nprpc::impl::MessageId::Success);
      break;
    }
    default:
      nprpc::impl::make_simple_answer(bufs(), nprpc::impl::MessageId::Error_UnknownFunctionIdx);
  }
}

std::string npd::Database::get_database_name() {
  ::nprpc::flat_buffer buf;
  {
    auto mb = buf.prepare(32);
    buf.commit(32);
    static_cast<::nprpc::impl::Header*>(mb.data())->msg_id = ::nprpc::impl::MessageId::FunctionCall;
    static_cast<::nprpc::impl::Header*>(mb.data())->msg_type = ::nprpc::impl::MessageType::Request;
  }
  ::nprpc::impl::flat::CallHeader_Direct __ch(buf, sizeof(::nprpc::impl::Header));
  __ch.object_id() = this->_data().object_id;
  __ch.poa_idx() = this->_data().poa_idx;
  __ch.interface_idx() = interface_idx_;
  __ch.function_idx() = 0;
  static_cast<::nprpc::impl::Header*>(buf.data().data())->size = static_cast<uint32_t>(buf.size() - 4);
  ::nprpc::impl::g_orb->call(this->get_endpoint(), buf, this->get_timeout());
  auto std_reply = nprpc::impl::handle_standart_reply(buf);
  if (std_reply != -1) {
    std::cerr << "received an unusual reply for function with output arguments\n";
    throw nprpc::Exception("Unknown Error");
  }
  db_M3_Direct out(buf, sizeof(::nprpc::impl::Header));
    std::string __ret_value;
    __ret_value = (std::string_view)out._1();
  return __ret_value;
}

cbt1::uuid npd::Database::get_database_uuid() {
  ::nprpc::flat_buffer buf;
  {
    auto mb = buf.prepare(32);
    buf.commit(32);
    static_cast<::nprpc::impl::Header*>(mb.data())->msg_id = ::nprpc::impl::MessageId::FunctionCall;
    static_cast<::nprpc::impl::Header*>(mb.data())->msg_type = ::nprpc::impl::MessageType::Request;
  }
  ::nprpc::impl::flat::CallHeader_Direct __ch(buf, sizeof(::nprpc::impl::Header));
  __ch.object_id() = this->_data().object_id;
  __ch.poa_idx() = this->_data().poa_idx;
  __ch.interface_idx() = interface_idx_;
  __ch.function_idx() = 1;
  static_cast<::nprpc::impl::Header*>(buf.data().data())->size = static_cast<uint32_t>(buf.size() - 4);
  ::nprpc::impl::g_orb->call(this->get_endpoint(), buf, this->get_timeout());
  auto std_reply = nprpc::impl::handle_standart_reply(buf);
  if (std_reply == 1) {
    db_throw_exception(buf);
  }
  if (std_reply != -1) {
    std::cerr << "received an unusual reply for function with output arguments\n";
    throw nprpc::Exception("Unknown Error");
  }
  db_M4_Direct out(buf, sizeof(::nprpc::impl::Header));
    cbt1::uuid __ret_value;
    {
      auto span = out._1();
      memcpy(__ret_value.data(), span.data(), 1 * span.size());
    }
  return __ret_value;
}

cbt1::oid_t npd::Database::next_oid(/*in*/const cbt1::oid_t& amount) {
  ::nprpc::flat_buffer buf;
  {
    auto mb = buf.prepare(40);
    buf.commit(40);
    static_cast<::nprpc::impl::Header*>(mb.data())->msg_id = ::nprpc::impl::MessageId::FunctionCall;
    static_cast<::nprpc::impl::Header*>(mb.data())->msg_type = ::nprpc::impl::MessageType::Request;
  }
  ::nprpc::impl::flat::CallHeader_Direct __ch(buf, sizeof(::nprpc::impl::Header));
  __ch.object_id() = this->_data().object_id;
  __ch.poa_idx() = this->_data().poa_idx;
  __ch.interface_idx() = interface_idx_;
  __ch.function_idx() = 2;
  db_M2_Direct _(buf,32);
  _._1() = amount;
  static_cast<::nprpc::impl::Header*>(buf.data().data())->size = static_cast<uint32_t>(buf.size() - 4);
  ::nprpc::impl::g_orb->call(this->get_endpoint(), buf, this->get_timeout());
  auto std_reply = nprpc::impl::handle_standart_reply(buf);
  if (std_reply == 1) {
    db_throw_exception(buf);
  }
  if (std_reply != -1) {
    std::cerr << "received an unusual reply for function with output arguments\n";
    throw nprpc::Exception("Unknown Error");
  }
  db_M2_Direct out(buf, sizeof(::nprpc::impl::Header));
    cbt1::oid_t __ret_value;
    __ret_value = out._1();
  return __ret_value;
}

cbt1::oid_t npd::Database::last_oid() {
  ::nprpc::flat_buffer buf;
  {
    auto mb = buf.prepare(32);
    buf.commit(32);
    static_cast<::nprpc::impl::Header*>(mb.data())->msg_id = ::nprpc::impl::MessageId::FunctionCall;
    static_cast<::nprpc::impl::Header*>(mb.data())->msg_type = ::nprpc::impl::MessageType::Request;
  }
  ::nprpc::impl::flat::CallHeader_Direct __ch(buf, sizeof(::nprpc::impl::Header));
  __ch.object_id() = this->_data().object_id;
  __ch.poa_idx() = this->_data().poa_idx;
  __ch.interface_idx() = interface_idx_;
  __ch.function_idx() = 3;
  static_cast<::nprpc::impl::Header*>(buf.data().data())->size = static_cast<uint32_t>(buf.size() - 4);
  ::nprpc::impl::g_orb->call(this->get_endpoint(), buf, this->get_timeout());
  auto std_reply = nprpc::impl::handle_standart_reply(buf);
  if (std_reply == 1) {
    db_throw_exception(buf);
  }
  if (std_reply != -1) {
    std::cerr << "received an unusual reply for function with output arguments\n";
    throw nprpc::Exception("Unknown Error");
  }
  db_M2_Direct out(buf, sizeof(::nprpc::impl::Header));
    cbt1::oid_t __ret_value;
    __ret_value = out._1();
  return __ret_value;
}

cbt1::oid_t npd::Database::create(/*in*/::nprpc::flat::Span<const uint8_t> data, /*in*/bool sync) {
  ::nprpc::flat_buffer buf;
  {
    auto mb = buf.prepare(172);
    buf.commit(44);
    static_cast<::nprpc::impl::Header*>(mb.data())->msg_id = ::nprpc::impl::MessageId::FunctionCall;
    static_cast<::nprpc::impl::Header*>(mb.data())->msg_type = ::nprpc::impl::MessageType::Request;
  }
  ::nprpc::impl::flat::CallHeader_Direct __ch(buf, sizeof(::nprpc::impl::Header));
  __ch.object_id() = this->_data().object_id;
  __ch.poa_idx() = this->_data().poa_idx;
  __ch.interface_idx() = interface_idx_;
  __ch.function_idx() = 4;
  db_M5_Direct _(buf,32);
  _._1(static_cast<uint32_t>(data.size()));
  memcpy(_._1().data(), data.data(), data.size() * 1);
  _._2() = sync;
  static_cast<::nprpc::impl::Header*>(buf.data().data())->size = static_cast<uint32_t>(buf.size() - 4);
  ::nprpc::impl::g_orb->call(this->get_endpoint(), buf, this->get_timeout());
  auto std_reply = nprpc::impl::handle_standart_reply(buf);
  if (std_reply == 1) {
    db_throw_exception(buf);
  }
  if (std_reply != -1) {
    std::cerr << "received an unusual reply for function with output arguments\n";
    throw nprpc::Exception("Unknown Error");
  }
  db_M2_Direct out(buf, sizeof(::nprpc::impl::Header));
    cbt1::oid_t __ret_value;
    __ret_value = out._1();
  return __ret_value;
}

cbt1::oid_t npd::Database::put(/*in*/const cbt1::oid_t& id, /*in*/::nprpc::flat::Span<const uint8_t> data, /*in*/bool sync) {
  ::nprpc::flat_buffer buf;
  {
    auto mb = buf.prepare(184);
    buf.commit(56);
    static_cast<::nprpc::impl::Header*>(mb.data())->msg_id = ::nprpc::impl::MessageId::FunctionCall;
    static_cast<::nprpc::impl::Header*>(mb.data())->msg_type = ::nprpc::impl::MessageType::Request;
  }
  ::nprpc::impl::flat::CallHeader_Direct __ch(buf, sizeof(::nprpc::impl::Header));
  __ch.object_id() = this->_data().object_id;
  __ch.poa_idx() = this->_data().poa_idx;
  __ch.interface_idx() = interface_idx_;
  __ch.function_idx() = 5;
  db_M6_Direct _(buf,32);
  _._1() = id;
  _._2(static_cast<uint32_t>(data.size()));
  memcpy(_._2().data(), data.data(), data.size() * 1);
  _._3() = sync;
  static_cast<::nprpc::impl::Header*>(buf.data().data())->size = static_cast<uint32_t>(buf.size() - 4);
  ::nprpc::impl::g_orb->call(this->get_endpoint(), buf, this->get_timeout());
  auto std_reply = nprpc::impl::handle_standart_reply(buf);
  if (std_reply == 1) {
    db_throw_exception(buf);
  }
  if (std_reply != -1) {
    std::cerr << "received an unusual reply for function with output arguments\n";
    throw nprpc::Exception("Unknown Error");
  }
  db_M2_Direct out(buf, sizeof(::nprpc::impl::Header));
    cbt1::oid_t __ret_value;
    __ret_value = out._1();
  return __ret_value;
}

bool npd::Database::exec_batch(/*in*/::nprpc::flat::Span<const npd::BatchOperation> data) {
  ::nprpc::flat_buffer buf;
  {
    auto mb = buf.prepare(168);
    buf.commit(40);
    static_cast<::nprpc::impl::Header*>(mb.data())->msg_id = ::nprpc::impl::MessageId::FunctionCall;
    static_cast<::nprpc::impl::Header*>(mb.data())->msg_type = ::nprpc::impl::MessageType::Request;
  }
  ::nprpc::impl::flat::CallHeader_Direct __ch(buf, sizeof(::nprpc::impl::Header));
  __ch.object_id() = this->_data().object_id;
  __ch.poa_idx() = this->_data().poa_idx;
  __ch.interface_idx() = interface_idx_;
  __ch.function_idx() = 6;
  db_M7_Direct _(buf,32);
  _._1(static_cast<uint32_t>(data.size()));
  {
    auto span = _._1();
    auto it = data.begin();
    for (auto e : span) {
      auto __ptr = ::nprpc::make_wrapper1(*it);
        e.create_or_update() = __ptr->create_or_update;
        e.id() = __ptr->id;
        e.data(__ptr->data);
      ++it;
    }
  }
  static_cast<::nprpc::impl::Header*>(buf.data().data())->size = static_cast<uint32_t>(buf.size() - 4);
  ::nprpc::impl::g_orb->call(this->get_endpoint(), buf, this->get_timeout());
  auto std_reply = nprpc::impl::handle_standart_reply(buf);
  if (std_reply == 1) {
    db_throw_exception(buf);
  }
  if (std_reply != -1) {
    std::cerr << "received an unusual reply for function with output arguments\n";
    throw nprpc::Exception("Unknown Error");
  }
  db_M8_Direct out(buf, sizeof(::nprpc::impl::Header));
    bool __ret_value;
    __ret_value = (bool)out._1();
  return __ret_value;
}

bool npd::Database::get(/*in*/const cbt1::oid_t& id, /*out*/std::vector<uint8_t>& data) {
  ::nprpc::flat_buffer buf;
  {
    auto mb = buf.prepare(40);
    buf.commit(40);
    static_cast<::nprpc::impl::Header*>(mb.data())->msg_id = ::nprpc::impl::MessageId::FunctionCall;
    static_cast<::nprpc::impl::Header*>(mb.data())->msg_type = ::nprpc::impl::MessageType::Request;
  }
  ::nprpc::impl::flat::CallHeader_Direct __ch(buf, sizeof(::nprpc::impl::Header));
  __ch.object_id() = this->_data().object_id;
  __ch.poa_idx() = this->_data().poa_idx;
  __ch.interface_idx() = interface_idx_;
  __ch.function_idx() = 7;
  db_M2_Direct _(buf,32);
  _._1() = id;
  static_cast<::nprpc::impl::Header*>(buf.data().data())->size = static_cast<uint32_t>(buf.size() - 4);
  ::nprpc::impl::g_orb->call(this->get_endpoint(), buf, this->get_timeout());
  auto std_reply = nprpc::impl::handle_standart_reply(buf);
  if (std_reply == 1) {
    db_throw_exception(buf);
  }
  if (std_reply != -1) {
    std::cerr << "received an unusual reply for function with output arguments\n";
    throw nprpc::Exception("Unknown Error");
  }
  db_M9_Direct out(buf, sizeof(::nprpc::impl::Header));
    {
      auto span = out._2();
      data.resize(span.size());
      memcpy(data.data(), span.data(), 1 * span.size());
    }
    bool __ret_value;
    __ret_value = (bool)out._1();
  return __ret_value;
}

uint64_t npd::Database::get_n(/*in*/::nprpc::flat::Span<const cbt1::oid_t> ids, /*out*/std::vector<uint8_t>& data) {
  ::nprpc::flat_buffer buf;
  {
    auto mb = buf.prepare(168);
    buf.commit(40);
    static_cast<::nprpc::impl::Header*>(mb.data())->msg_id = ::nprpc::impl::MessageId::FunctionCall;
    static_cast<::nprpc::impl::Header*>(mb.data())->msg_type = ::nprpc::impl::MessageType::Request;
  }
  ::nprpc::impl::flat::CallHeader_Direct __ch(buf, sizeof(::nprpc::impl::Header));
  __ch.object_id() = this->_data().object_id;
  __ch.poa_idx() = this->_data().poa_idx;
  __ch.interface_idx() = interface_idx_;
  __ch.function_idx() = 8;
  db_M10_Direct _(buf,32);
  _._1(static_cast<uint32_t>(ids.size()));
  memcpy(_._1().data(), ids.data(), ids.size() * 8);
  static_cast<::nprpc::impl::Header*>(buf.data().data())->size = static_cast<uint32_t>(buf.size() - 4);
  ::nprpc::impl::g_orb->call(this->get_endpoint(), buf, this->get_timeout());
  auto std_reply = nprpc::impl::handle_standart_reply(buf);
  if (std_reply == 1) {
    db_throw_exception(buf);
  }
  if (std_reply != -1) {
    std::cerr << "received an unusual reply for function with output arguments\n";
    throw nprpc::Exception("Unknown Error");
  }
  db_M11_Direct out(buf, sizeof(::nprpc::impl::Header));
    {
      auto span = out._2();
      data.resize(span.size());
      memcpy(data.data(), span.data(), 1 * span.size());
    }
    uint64_t __ret_value;
    __ret_value = out._1();
  return __ret_value;
}

void npd::Database::remove(/*in*/const cbt1::oid_t& id) {
  ::nprpc::flat_buffer buf;
  {
    auto mb = buf.prepare(40);
    buf.commit(40);
    static_cast<::nprpc::impl::Header*>(mb.data())->msg_id = ::nprpc::impl::MessageId::FunctionCall;
    static_cast<::nprpc::impl::Header*>(mb.data())->msg_type = ::nprpc::impl::MessageType::Request;
  }
  ::nprpc::impl::flat::CallHeader_Direct __ch(buf, sizeof(::nprpc::impl::Header));
  __ch.object_id() = this->_data().object_id;
  __ch.poa_idx() = this->_data().poa_idx;
  __ch.interface_idx() = interface_idx_;
  __ch.function_idx() = 9;
  db_M2_Direct _(buf,32);
  _._1() = id;
  static_cast<::nprpc::impl::Header*>(buf.data().data())->size = static_cast<uint32_t>(buf.size() - 4);
  ::nprpc::impl::g_orb->call(this->get_endpoint(), buf, this->get_timeout());
  auto std_reply = nprpc::impl::handle_standart_reply(buf);
  if (std_reply == 1) {
    db_throw_exception(buf);
  }
  if (std_reply != 0) {
    std::cerr << "received an unusual reply for function with no output arguments\n";
  }
}

void npd::Database::advise(/*in*/const cbt1::oid_t& id, /*in*/const ObjectId& client) {
  ::nprpc::flat_buffer buf;
  {
    auto mb = buf.prepare(208);
    buf.commit(80);
    static_cast<::nprpc::impl::Header*>(mb.data())->msg_id = ::nprpc::impl::MessageId::FunctionCall;
    static_cast<::nprpc::impl::Header*>(mb.data())->msg_type = ::nprpc::impl::MessageType::Request;
  }
  ::nprpc::impl::flat::CallHeader_Direct __ch(buf, sizeof(::nprpc::impl::Header));
  __ch.object_id() = this->_data().object_id;
  __ch.poa_idx() = this->_data().poa_idx;
  __ch.interface_idx() = interface_idx_;
  __ch.function_idx() = 10;
  db_M12_Direct _(buf,32);
  _._1() = id;
  memcpy(_._2().__data(), &client._data(), 24);
  _._2().class_id(client._data().class_id);
  _._2().hostname(client._data().hostname);
  static_cast<::nprpc::impl::Header*>(buf.data().data())->size = static_cast<uint32_t>(buf.size() - 4);
  ::nprpc::impl::g_orb->call(this->get_endpoint(), buf, this->get_timeout());
  auto std_reply = nprpc::impl::handle_standart_reply(buf);
  if (std_reply == 1) {
    db_throw_exception(buf);
  }
  if (std_reply != 0) {
    std::cerr << "received an unusual reply for function with no output arguments\n";
  }
}

void npd::IDatabase_Servant::dispatch(nprpc::Buffers& bufs, nprpc::EndPoint remote_endpoint, bool from_parent, nprpc::ReferenceList& ref_list) {
  nprpc::impl::flat::CallHeader_Direct __ch(bufs(), sizeof(::nprpc::impl::Header));
  switch(__ch.function_idx()) {
    case 0: {
      auto& obuf = bufs.flip();
      obuf.consume(obuf.size());
      obuf.prepare(152);
      obuf.commit(24);
      db_M3_Direct oa(obuf,16);
      std::string __ret_val;
      __ret_val = get_database_name();
      oa._1(__ret_val);
      static_cast<::nprpc::impl::Header*>(obuf.data().data())->size = static_cast<uint32_t>(obuf.size() - 4);
      static_cast<::nprpc::impl::Header*>(obuf.data().data())->msg_id = ::nprpc::impl::MessageId::BlockResponse;
      static_cast<::nprpc::impl::Header*>(obuf.data().data())->msg_type = ::nprpc::impl::MessageType::Answer;
      break;
    }
    case 1: {
      cbt1::uuid __ret_val;
      try {
        __ret_val = get_database_uuid();
      }
      catch(npd::DatabaseException& e) {
        auto& obuf = bufs();
        obuf.consume(obuf.size());
        obuf.prepare(24);
        obuf.commit(24);
        npd::flat::DatabaseException_Direct oa(obuf,16);
        oa.__ex_id() = 0;
        oa.code() = e.code;
        static_cast<::nprpc::impl::Header*>(obuf.data().data())->size = static_cast<uint32_t>(obuf.size() - 4);
        static_cast<::nprpc::impl::Header*>(obuf.data().data())->msg_id = ::nprpc::impl::MessageId::Exception;
        static_cast<::nprpc::impl::Header*>(obuf.data().data())->msg_type = ::nprpc::impl::MessageType::Answer;
        return;
      }
      auto& obuf = bufs();
      obuf.consume(obuf.size());
      obuf.prepare(32);
      obuf.commit(32);
      db_M4_Direct oa(obuf,16);
        memcpy(oa._1().data(), __ret_val.data(), __ret_val.size() * 1);
      static_cast<::nprpc::impl::Header*>(obuf.data().data())->size = static_cast<uint32_t>(obuf.size() - 4);
      static_cast<::nprpc::impl::Header*>(obuf.data().data())->msg_id = ::nprpc::impl::MessageId::BlockResponse;
      static_cast<::nprpc::impl::Header*>(obuf.data().data())->msg_type = ::nprpc::impl::MessageType::Answer;
      break;
    }
    case 2: {
      db_M2_Direct ia(bufs(), 32);
      cbt1::oid_t __ret_val;
      try {
        __ret_val = next_oid(ia._1());
      }
      catch(npd::DatabaseException& e) {
        auto& obuf = bufs();
        obuf.consume(obuf.size());
        obuf.prepare(24);
        obuf.commit(24);
        npd::flat::DatabaseException_Direct oa(obuf,16);
        oa.__ex_id() = 0;
        oa.code() = e.code;
        static_cast<::nprpc::impl::Header*>(obuf.data().data())->size = static_cast<uint32_t>(obuf.size() - 4);
        static_cast<::nprpc::impl::Header*>(obuf.data().data())->msg_id = ::nprpc::impl::MessageId::Exception;
        static_cast<::nprpc::impl::Header*>(obuf.data().data())->msg_type = ::nprpc::impl::MessageType::Answer;
        return;
      }
      auto& obuf = bufs();
      obuf.consume(obuf.size());
      obuf.prepare(24);
      obuf.commit(24);
      db_M2_Direct oa(obuf,16);
        oa._1() = __ret_val;
      static_cast<::nprpc::impl::Header*>(obuf.data().data())->size = static_cast<uint32_t>(obuf.size() - 4);
      static_cast<::nprpc::impl::Header*>(obuf.data().data())->msg_id = ::nprpc::impl::MessageId::BlockResponse;
      static_cast<::nprpc::impl::Header*>(obuf.data().data())->msg_type = ::nprpc::impl::MessageType::Answer;
      break;
    }
    case 3: {
      cbt1::oid_t __ret_val;
      try {
        __ret_val = last_oid();
      }
      catch(npd::DatabaseException& e) {
        auto& obuf = bufs();
        obuf.consume(obuf.size());
        obuf.prepare(24);
        obuf.commit(24);
        npd::flat::DatabaseException_Direct oa(obuf,16);
        oa.__ex_id() = 0;
        oa.code() = e.code;
        static_cast<::nprpc::impl::Header*>(obuf.data().data())->size = static_cast<uint32_t>(obuf.size() - 4);
        static_cast<::nprpc::impl::Header*>(obuf.data().data())->msg_id = ::nprpc::impl::MessageId::Exception;
        static_cast<::nprpc::impl::Header*>(obuf.data().data())->msg_type = ::nprpc::impl::MessageType::Answer;
        return;
      }
      auto& obuf = bufs();
      obuf.consume(obuf.size());
      obuf.prepare(24);
      obuf.commit(24);
      db_M2_Direct oa(obuf,16);
        oa._1() = __ret_val;
      static_cast<::nprpc::impl::Header*>(obuf.data().data())->size = static_cast<uint32_t>(obuf.size() - 4);
      static_cast<::nprpc::impl::Header*>(obuf.data().data())->msg_id = ::nprpc::impl::MessageId::BlockResponse;
      static_cast<::nprpc::impl::Header*>(obuf.data().data())->msg_type = ::nprpc::impl::MessageType::Answer;
      break;
    }
    case 4: {
      db_M5_Direct ia(bufs(), 32);
      cbt1::oid_t __ret_val;
      try {
        __ret_val = create(ia._1(), ia._2());
      }
      catch(npd::DatabaseException& e) {
        auto& obuf = bufs();
        obuf.consume(obuf.size());
        obuf.prepare(24);
        obuf.commit(24);
        npd::flat::DatabaseException_Direct oa(obuf,16);
        oa.__ex_id() = 0;
        oa.code() = e.code;
        static_cast<::nprpc::impl::Header*>(obuf.data().data())->size = static_cast<uint32_t>(obuf.size() - 4);
        static_cast<::nprpc::impl::Header*>(obuf.data().data())->msg_id = ::nprpc::impl::MessageId::Exception;
        static_cast<::nprpc::impl::Header*>(obuf.data().data())->msg_type = ::nprpc::impl::MessageType::Answer;
        return;
      }
      auto& obuf = bufs();
      obuf.consume(obuf.size());
      obuf.prepare(24);
      obuf.commit(24);
      db_M2_Direct oa(obuf,16);
        oa._1() = __ret_val;
      static_cast<::nprpc::impl::Header*>(obuf.data().data())->size = static_cast<uint32_t>(obuf.size() - 4);
      static_cast<::nprpc::impl::Header*>(obuf.data().data())->msg_id = ::nprpc::impl::MessageId::BlockResponse;
      static_cast<::nprpc::impl::Header*>(obuf.data().data())->msg_type = ::nprpc::impl::MessageType::Answer;
      break;
    }
    case 5: {
      db_M6_Direct ia(bufs(), 32);
      cbt1::oid_t __ret_val;
      try {
        __ret_val = put(ia._1(), ia._2(), ia._3());
      }
      catch(npd::DatabaseException& e) {
        auto& obuf = bufs();
        obuf.consume(obuf.size());
        obuf.prepare(24);
        obuf.commit(24);
        npd::flat::DatabaseException_Direct oa(obuf,16);
        oa.__ex_id() = 0;
        oa.code() = e.code;
        static_cast<::nprpc::impl::Header*>(obuf.data().data())->size = static_cast<uint32_t>(obuf.size() - 4);
        static_cast<::nprpc::impl::Header*>(obuf.data().data())->msg_id = ::nprpc::impl::MessageId::Exception;
        static_cast<::nprpc::impl::Header*>(obuf.data().data())->msg_type = ::nprpc::impl::MessageType::Answer;
        return;
      }
      auto& obuf = bufs();
      obuf.consume(obuf.size());
      obuf.prepare(24);
      obuf.commit(24);
      db_M2_Direct oa(obuf,16);
        oa._1() = __ret_val;
      static_cast<::nprpc::impl::Header*>(obuf.data().data())->size = static_cast<uint32_t>(obuf.size() - 4);
      static_cast<::nprpc::impl::Header*>(obuf.data().data())->msg_id = ::nprpc::impl::MessageId::BlockResponse;
      static_cast<::nprpc::impl::Header*>(obuf.data().data())->msg_type = ::nprpc::impl::MessageType::Answer;
      break;
    }
    case 6: {
      db_M7_Direct ia(bufs(), 32);
      bool __ret_val;
      try {
        __ret_val = exec_batch(ia._1());
      }
      catch(npd::DatabaseException& e) {
        auto& obuf = bufs();
        obuf.consume(obuf.size());
        obuf.prepare(24);
        obuf.commit(24);
        npd::flat::DatabaseException_Direct oa(obuf,16);
        oa.__ex_id() = 0;
        oa.code() = e.code;
        static_cast<::nprpc::impl::Header*>(obuf.data().data())->size = static_cast<uint32_t>(obuf.size() - 4);
        static_cast<::nprpc::impl::Header*>(obuf.data().data())->msg_id = ::nprpc::impl::MessageId::Exception;
        static_cast<::nprpc::impl::Header*>(obuf.data().data())->msg_type = ::nprpc::impl::MessageType::Answer;
        return;
      }
      auto& obuf = bufs();
      obuf.consume(obuf.size());
      obuf.prepare(17);
      obuf.commit(17);
      db_M8_Direct oa(obuf,16);
        oa._1() = __ret_val;
      static_cast<::nprpc::impl::Header*>(obuf.data().data())->size = static_cast<uint32_t>(obuf.size() - 4);
      static_cast<::nprpc::impl::Header*>(obuf.data().data())->msg_id = ::nprpc::impl::MessageId::BlockResponse;
      static_cast<::nprpc::impl::Header*>(obuf.data().data())->msg_type = ::nprpc::impl::MessageType::Answer;
      break;
    }
    case 7: {
      db_M2_Direct ia(bufs(), 32);
      auto& obuf = bufs.flip();
      obuf.consume(obuf.size());
      obuf.prepare(156);
      obuf.commit(28);
      db_M9_Direct oa(obuf,16);
      bool __ret_val;
      try {
        __ret_val = get(ia._1(), oa._2_d());
      }
      catch(npd::DatabaseException& e) {
        auto& obuf = bufs();
        obuf.consume(obuf.size());
        obuf.prepare(24);
        obuf.commit(24);
        npd::flat::DatabaseException_Direct oa(obuf,16);
        oa.__ex_id() = 0;
        oa.code() = e.code;
        static_cast<::nprpc::impl::Header*>(obuf.data().data())->size = static_cast<uint32_t>(obuf.size() - 4);
        static_cast<::nprpc::impl::Header*>(obuf.data().data())->msg_id = ::nprpc::impl::MessageId::Exception;
        static_cast<::nprpc::impl::Header*>(obuf.data().data())->msg_type = ::nprpc::impl::MessageType::Answer;
        return;
      }
        oa._1() = __ret_val;
      static_cast<::nprpc::impl::Header*>(obuf.data().data())->size = static_cast<uint32_t>(obuf.size() - 4);
      static_cast<::nprpc::impl::Header*>(obuf.data().data())->msg_id = ::nprpc::impl::MessageId::BlockResponse;
      static_cast<::nprpc::impl::Header*>(obuf.data().data())->msg_type = ::nprpc::impl::MessageType::Answer;
      break;
    }
    case 8: {
      db_M10_Direct ia(bufs(), 32);
      auto& obuf = bufs.flip();
      obuf.consume(obuf.size());
      obuf.prepare(160);
      obuf.commit(32);
      db_M11_Direct oa(obuf,16);
      uint64_t __ret_val;
      try {
        __ret_val = get_n(ia._1(), oa._2_d());
      }
      catch(npd::DatabaseException& e) {
        auto& obuf = bufs();
        obuf.consume(obuf.size());
        obuf.prepare(24);
        obuf.commit(24);
        npd::flat::DatabaseException_Direct oa(obuf,16);
        oa.__ex_id() = 0;
        oa.code() = e.code;
        static_cast<::nprpc::impl::Header*>(obuf.data().data())->size = static_cast<uint32_t>(obuf.size() - 4);
        static_cast<::nprpc::impl::Header*>(obuf.data().data())->msg_id = ::nprpc::impl::MessageId::Exception;
        static_cast<::nprpc::impl::Header*>(obuf.data().data())->msg_type = ::nprpc::impl::MessageType::Answer;
        return;
      }
        oa._1() = __ret_val;
      static_cast<::nprpc::impl::Header*>(obuf.data().data())->size = static_cast<uint32_t>(obuf.size() - 4);
      static_cast<::nprpc::impl::Header*>(obuf.data().data())->msg_id = ::nprpc::impl::MessageId::BlockResponse;
      static_cast<::nprpc::impl::Header*>(obuf.data().data())->msg_type = ::nprpc::impl::MessageType::Answer;
      break;
    }
    case 9: {
      db_M2_Direct ia(bufs(), 32);
      try {
        remove(ia._1());
      }
      catch(npd::DatabaseException& e) {
        auto& obuf = bufs();
        obuf.consume(obuf.size());
        obuf.prepare(24);
        obuf.commit(24);
        npd::flat::DatabaseException_Direct oa(obuf,16);
        oa.__ex_id() = 0;
        oa.code() = e.code;
        static_cast<::nprpc::impl::Header*>(obuf.data().data())->size = static_cast<uint32_t>(obuf.size() - 4);
        static_cast<::nprpc::impl::Header*>(obuf.data().data())->msg_id = ::nprpc::impl::MessageId::Exception;
        static_cast<::nprpc::impl::Header*>(obuf.data().data())->msg_type = ::nprpc::impl::MessageType::Answer;
        return;
      }
      nprpc::impl::make_simple_answer(bufs(), nprpc::impl::MessageId::Success);
      break;
    }
    case 10: {
      db_M12_Direct ia(bufs(), 32);
      try {
        advise(ia._1(), nprpc::impl::create_object_from_flat(ia._2(), remote_endpoint));
      }
      catch(npd::DatabaseException& e) {
        auto& obuf = bufs();
        obuf.consume(obuf.size());
        obuf.prepare(24);
        obuf.commit(24);
        npd::flat::DatabaseException_Direct oa(obuf,16);
        oa.__ex_id() = 0;
        oa.code() = e.code;
        static_cast<::nprpc::impl::Header*>(obuf.data().data())->size = static_cast<uint32_t>(obuf.size() - 4);
        static_cast<::nprpc::impl::Header*>(obuf.data().data())->msg_id = ::nprpc::impl::MessageId::Exception;
        static_cast<::nprpc::impl::Header*>(obuf.data().data())->msg_type = ::nprpc::impl::MessageType::Answer;
        return;
      }
      nprpc::impl::make_simple_answer(bufs(), nprpc::impl::MessageId::Success);
      break;
    }
    default:
      nprpc::impl::make_simple_answer(bufs(), nprpc::impl::MessageId::Error_UnknownFunctionIdx);
  }
}

} // namespace npd


void db_throw_exception(::nprpc::flat_buffer& buf) { 
  switch(*(uint32_t*)( (char*)buf.data().data() + sizeof(::nprpc::impl::Header)) ) {
  case 0:
  {
    npd::flat::DatabaseException_Direct ex_flat(buf, sizeof(::nprpc::impl::Header));
    npd::DatabaseException ex;
  ex.code = ex_flat.code();
    throw ex;
  }
  default:
    throw std::runtime_error("unknown rpc exception");
  }
}
