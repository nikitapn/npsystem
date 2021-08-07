#include "db.hpp"
#include "db_m.hpp"
#include <nprpc/nprpc_impl.hpp>

void db_throw_exception(boost::beast::flat_buffer& buf);

namespace cbt1 { 
} // namespace cbt1

namespace npd { 
void npd::NodeCallback::OnNodeChanged(/*in*/const cbt1::oid_t& id, /*in*/::flat::Span<const uint8_t> data) {
  boost::beast::flat_buffer buf;
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
  ::flat::db_M1_Direct _(buf,32);
  _._1() = id;
  _._2(data.size());
  memcpy(_._2().data(), data.data(), data.size() * 1);
  static_cast<::nprpc::impl::Header*>(buf.data().data())->size = static_cast<uint32_t>(buf.size() - 4);
  ::nprpc::impl::g_orb->call(
    nprpc::EndPoint(this->_data().ip4, this->_data().port), buf, this->get_timeout()
  );
  auto std_reply = nprpc::impl::handle_standart_reply(buf);
  if (std_reply != 0) {
    std::cerr << "received an unusual reply for function with no output arguments\n";
    assert(false);
  }
}

void npd::NodeCallback::OnNodeDeleted(/*in*/const cbt1::oid_t& id) {
  boost::beast::flat_buffer buf;
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
  ::flat::db_M2_Direct _(buf,32);
  _._1() = id;
  static_cast<::nprpc::impl::Header*>(buf.data().data())->size = static_cast<uint32_t>(buf.size() - 4);
  ::nprpc::impl::g_orb->call(
    nprpc::EndPoint(this->_data().ip4, this->_data().port), buf, this->get_timeout()
  );
  auto std_reply = nprpc::impl::handle_standart_reply(buf);
  if (std_reply != 0) {
    std::cerr << "received an unusual reply for function with no output arguments\n";
    assert(false);
  }
}

void npd::INodeCallback_Servant::dispatch(nprpc::Buffers& bufs, nprpc::EndPoint remote_endpoint, bool from_parent, nprpc::ReferenceList& ref_list) {
  nprpc::impl::flat::CallHeader_Direct __ch(bufs(), sizeof(::nprpc::impl::Header));
  switch(__ch.function_idx()) {
    case 0: {
      ::flat::db_M1_Direct ia(bufs(), 32);
      OnNodeChanged(ia._1(), ia._2());
      nprpc::impl::make_simple_answer(bufs(), nprpc::impl::MessageId::Success);
      break;
    }
    case 1: {
      ::flat::db_M2_Direct ia(bufs(), 32);
      OnNodeDeleted(ia._1());
      nprpc::impl::make_simple_answer(bufs(), nprpc::impl::MessageId::Success);
      break;
    }
    default:
      nprpc::impl::make_simple_answer(bufs(), nprpc::impl::MessageId::Error_UnknownFunctionIdx);
  }
}

std::string npd::Database::get_database_name() {
  boost::beast::flat_buffer buf;
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
  ::nprpc::impl::g_orb->call(
    nprpc::EndPoint(this->_data().ip4, this->_data().port), buf, this->get_timeout()
  );
  auto std_reply = nprpc::impl::handle_standart_reply(buf);
  if (std_reply != -1) {
    std::cerr << "received an unusual reply for function with output arguments\n";
    assert(false);
    throw nprpc::Exception("Unknown Error");
  }
  ::flat::db_M3_Direct out(buf, sizeof(::nprpc::impl::Header));
  std::string __ret_value;
  __ret_value = (std::string_view)out._1();
  return __ret_value;
}

cbt1::uuid npd::Database::get_database_uuid() {
  boost::beast::flat_buffer buf;
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
  ::nprpc::impl::g_orb->call(
    nprpc::EndPoint(this->_data().ip4, this->_data().port), buf, this->get_timeout()
  );
  auto std_reply = nprpc::impl::handle_standart_reply(buf);
  if (std_reply == 1) {
    db_throw_exception(buf);
  }
  if (std_reply != -1) {
    std::cerr << "received an unusual reply for function with output arguments\n";
    assert(false);
    throw nprpc::Exception("Unknown Error");
  }
  ::flat::db_M4_Direct out(buf, sizeof(::nprpc::impl::Header));
  cbt1::uuid __ret_value;
  {
    auto span = out._1();
    memcpy(__ret_value.data(), span.data(), 1 * span.size());
  }
  return __ret_value;
}

cbt1::oid_t npd::Database::next_oid(/*in*/const cbt1::oid_t& amount) {
  boost::beast::flat_buffer buf;
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
  ::flat::db_M2_Direct _(buf,32);
  _._1() = amount;
  static_cast<::nprpc::impl::Header*>(buf.data().data())->size = static_cast<uint32_t>(buf.size() - 4);
  ::nprpc::impl::g_orb->call(
    nprpc::EndPoint(this->_data().ip4, this->_data().port), buf, this->get_timeout()
  );
  auto std_reply = nprpc::impl::handle_standart_reply(buf);
  if (std_reply == 1) {
    db_throw_exception(buf);
  }
  if (std_reply != -1) {
    std::cerr << "received an unusual reply for function with output arguments\n";
    assert(false);
    throw nprpc::Exception("Unknown Error");
  }
  ::flat::db_M2_Direct out(buf, sizeof(::nprpc::impl::Header));
  cbt1::oid_t __ret_value;
  __ret_value = out._1();
  return __ret_value;
}

cbt1::oid_t npd::Database::last_oid() {
  boost::beast::flat_buffer buf;
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
  ::nprpc::impl::g_orb->call(
    nprpc::EndPoint(this->_data().ip4, this->_data().port), buf, this->get_timeout()
  );
  auto std_reply = nprpc::impl::handle_standart_reply(buf);
  if (std_reply == 1) {
    db_throw_exception(buf);
  }
  if (std_reply != -1) {
    std::cerr << "received an unusual reply for function with output arguments\n";
    assert(false);
    throw nprpc::Exception("Unknown Error");
  }
  ::flat::db_M2_Direct out(buf, sizeof(::nprpc::impl::Header));
  cbt1::oid_t __ret_value;
  __ret_value = out._1();
  return __ret_value;
}

cbt1::oid_t npd::Database::create(/*in*/::flat::Span<const uint8_t> data, /*in*/bool sync) {
  boost::beast::flat_buffer buf;
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
  ::flat::db_M5_Direct _(buf,32);
  _._1(data.size());
  memcpy(_._1().data(), data.data(), data.size() * 1);
  _._2() = sync;
  static_cast<::nprpc::impl::Header*>(buf.data().data())->size = static_cast<uint32_t>(buf.size() - 4);
  ::nprpc::impl::g_orb->call(
    nprpc::EndPoint(this->_data().ip4, this->_data().port), buf, this->get_timeout()
  );
  auto std_reply = nprpc::impl::handle_standart_reply(buf);
  if (std_reply == 1) {
    db_throw_exception(buf);
  }
  if (std_reply != -1) {
    std::cerr << "received an unusual reply for function with output arguments\n";
    assert(false);
    throw nprpc::Exception("Unknown Error");
  }
  ::flat::db_M2_Direct out(buf, sizeof(::nprpc::impl::Header));
  cbt1::oid_t __ret_value;
  __ret_value = out._1();
  return __ret_value;
}

cbt1::oid_t npd::Database::put(/*in*/const cbt1::oid_t& id, /*in*/::flat::Span<const uint8_t> data, /*in*/bool sync) {
  boost::beast::flat_buffer buf;
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
  ::flat::db_M6_Direct _(buf,32);
  _._1() = id;
  _._2(data.size());
  memcpy(_._2().data(), data.data(), data.size() * 1);
  _._3() = sync;
  static_cast<::nprpc::impl::Header*>(buf.data().data())->size = static_cast<uint32_t>(buf.size() - 4);
  ::nprpc::impl::g_orb->call(
    nprpc::EndPoint(this->_data().ip4, this->_data().port), buf, this->get_timeout()
  );
  auto std_reply = nprpc::impl::handle_standart_reply(buf);
  if (std_reply == 1) {
    db_throw_exception(buf);
  }
  if (std_reply != -1) {
    std::cerr << "received an unusual reply for function with output arguments\n";
    assert(false);
    throw nprpc::Exception("Unknown Error");
  }
  ::flat::db_M2_Direct out(buf, sizeof(::nprpc::impl::Header));
  cbt1::oid_t __ret_value;
  __ret_value = out._1();
  return __ret_value;
}

bool npd::Database::exec_batch(/*in*/::flat::Span<const npd::BatchOperation> data) {
  boost::beast::flat_buffer buf;
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
  ::flat::db_M7_Direct _(buf,32);
  _._1(data.size());
  auto span = _._1();
  auto it = data.begin();
  for (auto e : span) {
    auto __ptr = ::nprpc::make_wrapper1(*it);
    e.create_or_update() = __ptr->create_or_update;
    e.id() = __ptr->id;
    e.data(__ptr->data);
    ++it;
  }
  static_cast<::nprpc::impl::Header*>(buf.data().data())->size = static_cast<uint32_t>(buf.size() - 4);
  ::nprpc::impl::g_orb->call(
    nprpc::EndPoint(this->_data().ip4, this->_data().port), buf, this->get_timeout()
  );
  auto std_reply = nprpc::impl::handle_standart_reply(buf);
  if (std_reply == 1) {
    db_throw_exception(buf);
  }
  if (std_reply != -1) {
    std::cerr << "received an unusual reply for function with output arguments\n";
    assert(false);
    throw nprpc::Exception("Unknown Error");
  }
  ::flat::db_M8_Direct out(buf, sizeof(::nprpc::impl::Header));
  bool __ret_value;
  __ret_value = out._1();
  return __ret_value;
}

bool npd::Database::get(/*in*/const cbt1::oid_t& id, /*out*/std::vector<uint8_t>& data) {
  boost::beast::flat_buffer buf;
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
  ::flat::db_M2_Direct _(buf,32);
  _._1() = id;
  static_cast<::nprpc::impl::Header*>(buf.data().data())->size = static_cast<uint32_t>(buf.size() - 4);
  ::nprpc::impl::g_orb->call(
    nprpc::EndPoint(this->_data().ip4, this->_data().port), buf, this->get_timeout()
  );
  auto std_reply = nprpc::impl::handle_standart_reply(buf);
  if (std_reply == 1) {
    db_throw_exception(buf);
  }
  if (std_reply != -1) {
    std::cerr << "received an unusual reply for function with output arguments\n";
    assert(false);
    throw nprpc::Exception("Unknown Error");
  }
  ::flat::db_M9_Direct out(buf, sizeof(::nprpc::impl::Header));
  {
    auto span = out._2();
    data.resize(span.size());
    memcpy(data.data(), span.data(), 1 * span.size());
  }
  bool __ret_value;
  __ret_value = out._1();
  return __ret_value;
}

uint64_t npd::Database::get_n(/*in*/::flat::Span<const cbt1::oid_t> ids, /*out*/std::vector<uint8_t>& data) {
  boost::beast::flat_buffer buf;
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
  ::flat::db_M10_Direct _(buf,32);
  _._1(ids.size());
  memcpy(_._1().data(), ids.data(), ids.size() * 8);
  static_cast<::nprpc::impl::Header*>(buf.data().data())->size = static_cast<uint32_t>(buf.size() - 4);
  ::nprpc::impl::g_orb->call(
    nprpc::EndPoint(this->_data().ip4, this->_data().port), buf, this->get_timeout()
  );
  auto std_reply = nprpc::impl::handle_standart_reply(buf);
  if (std_reply == 1) {
    db_throw_exception(buf);
  }
  if (std_reply != -1) {
    std::cerr << "received an unusual reply for function with output arguments\n";
    assert(false);
    throw nprpc::Exception("Unknown Error");
  }
  ::flat::db_M11_Direct out(buf, sizeof(::nprpc::impl::Header));
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
  boost::beast::flat_buffer buf;
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
  ::flat::db_M2_Direct _(buf,32);
  _._1() = id;
  static_cast<::nprpc::impl::Header*>(buf.data().data())->size = static_cast<uint32_t>(buf.size() - 4);
  ::nprpc::impl::g_orb->call(
    nprpc::EndPoint(this->_data().ip4, this->_data().port), buf, this->get_timeout()
  );
  auto std_reply = nprpc::impl::handle_standart_reply(buf);
  if (std_reply == 1) {
    db_throw_exception(buf);
  }
  if (std_reply != 0) {
    std::cerr << "received an unusual reply for function with no output arguments\n";
    assert(false);
  }
}

void npd::Database::advise(/*in*/const cbt1::oid_t& id, /*in*/const ObjectId& client) {
  boost::beast::flat_buffer buf;
  {
    auto mb = buf.prepare(200);
    buf.commit(72);
    static_cast<::nprpc::impl::Header*>(mb.data())->msg_id = ::nprpc::impl::MessageId::FunctionCall;
    static_cast<::nprpc::impl::Header*>(mb.data())->msg_type = ::nprpc::impl::MessageType::Request;
  }
  ::nprpc::impl::flat::CallHeader_Direct __ch(buf, sizeof(::nprpc::impl::Header));
  __ch.object_id() = this->_data().object_id;
  __ch.poa_idx() = this->_data().poa_idx;
  __ch.interface_idx() = interface_idx_;
  __ch.function_idx() = 10;
  ::flat::db_M12_Direct _(buf,32);
  _._1() = id;
  memcpy(&_._2().ip4(), &client._data().ip4, 20);
  _._2().class_id(client._data().class_id);
  static_cast<::nprpc::impl::Header*>(buf.data().data())->size = static_cast<uint32_t>(buf.size() - 4);
  ::nprpc::impl::g_orb->call(
    nprpc::EndPoint(this->_data().ip4, this->_data().port), buf, this->get_timeout()
  );
  auto std_reply = nprpc::impl::handle_standart_reply(buf);
  if (std_reply == 1) {
    db_throw_exception(buf);
  }
  if (std_reply != 0) {
    std::cerr << "received an unusual reply for function with no output arguments\n";
    assert(false);
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
      ::flat::db_M3_Direct oa(obuf,16);
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
      ::flat::db_M4_Direct oa(obuf,16);
  memcpy(oa._1().data(), __ret_val.data(), __ret_val.size() * 1);
      static_cast<::nprpc::impl::Header*>(obuf.data().data())->size = static_cast<uint32_t>(obuf.size() - 4);
      static_cast<::nprpc::impl::Header*>(obuf.data().data())->msg_id = ::nprpc::impl::MessageId::BlockResponse;
      static_cast<::nprpc::impl::Header*>(obuf.data().data())->msg_type = ::nprpc::impl::MessageType::Answer;
      break;
    }
    case 2: {
      ::flat::db_M2_Direct ia(bufs(), 32);
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
      ::flat::db_M2_Direct oa(obuf,16);
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
      ::flat::db_M2_Direct oa(obuf,16);
  oa._1() = __ret_val;
      static_cast<::nprpc::impl::Header*>(obuf.data().data())->size = static_cast<uint32_t>(obuf.size() - 4);
      static_cast<::nprpc::impl::Header*>(obuf.data().data())->msg_id = ::nprpc::impl::MessageId::BlockResponse;
      static_cast<::nprpc::impl::Header*>(obuf.data().data())->msg_type = ::nprpc::impl::MessageType::Answer;
      break;
    }
    case 4: {
      ::flat::db_M5_Direct ia(bufs(), 32);
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
      ::flat::db_M2_Direct oa(obuf,16);
  oa._1() = __ret_val;
      static_cast<::nprpc::impl::Header*>(obuf.data().data())->size = static_cast<uint32_t>(obuf.size() - 4);
      static_cast<::nprpc::impl::Header*>(obuf.data().data())->msg_id = ::nprpc::impl::MessageId::BlockResponse;
      static_cast<::nprpc::impl::Header*>(obuf.data().data())->msg_type = ::nprpc::impl::MessageType::Answer;
      break;
    }
    case 5: {
      ::flat::db_M6_Direct ia(bufs(), 32);
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
      ::flat::db_M2_Direct oa(obuf,16);
  oa._1() = __ret_val;
      static_cast<::nprpc::impl::Header*>(obuf.data().data())->size = static_cast<uint32_t>(obuf.size() - 4);
      static_cast<::nprpc::impl::Header*>(obuf.data().data())->msg_id = ::nprpc::impl::MessageId::BlockResponse;
      static_cast<::nprpc::impl::Header*>(obuf.data().data())->msg_type = ::nprpc::impl::MessageType::Answer;
      break;
    }
    case 6: {
      ::flat::db_M7_Direct ia(bufs(), 32);
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
      ::flat::db_M8_Direct oa(obuf,16);
  oa._1() = __ret_val;
      static_cast<::nprpc::impl::Header*>(obuf.data().data())->size = static_cast<uint32_t>(obuf.size() - 4);
      static_cast<::nprpc::impl::Header*>(obuf.data().data())->msg_id = ::nprpc::impl::MessageId::BlockResponse;
      static_cast<::nprpc::impl::Header*>(obuf.data().data())->msg_type = ::nprpc::impl::MessageType::Answer;
      break;
    }
    case 7: {
      ::flat::db_M2_Direct ia(bufs(), 32);
      auto& obuf = bufs.flip();
      obuf.consume(obuf.size());
      obuf.prepare(156);
      obuf.commit(28);
      ::flat::db_M9_Direct oa(obuf,16);
bool __ret_val;
      try {
      __ret_val = get(ia._1(), oa._2_vd());
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
      ::flat::db_M10_Direct ia(bufs(), 32);
      auto& obuf = bufs.flip();
      obuf.consume(obuf.size());
      obuf.prepare(160);
      obuf.commit(32);
      ::flat::db_M11_Direct oa(obuf,16);
uint64_t __ret_val;
      try {
      __ret_val = get_n(ia._1(), oa._2_vd());
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
      ::flat::db_M2_Direct ia(bufs(), 32);
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
      ::flat::db_M12_Direct ia(bufs(), 32);
      try {
      advise(ia._1(), nprpc::impl::g_orb->create_object_from_flat(ia._2(), remote_endpoint));
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


void db_throw_exception(boost::beast::flat_buffer& buf) { 
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
