#include "db.hpp"
#include "db_m.hpp"
#include <nprpc/nprpc_impl.hpp>

void DB__throw_exception(boost::beast::flat_buffer& buf);

namespace cbt1 { 
} // namespace cbt1

namespace npd { 
void npd::NodeCallback::OnNodeChanged(/*in*/const cbt1::oid_t& id, /*in*/::flat::Span<const uint8_t> data) {
  boost::beast::flat_buffer buf;
  {
    auto mb = buf.prepare(168);
    buf.commit(40);
    *((uint32_t*)mb.data() + 1) = ::nprpc::impl::MessageId::FunctionCall;
  }
  ::nprpc::detail::flat::CallHeader_Direct __ch(buf, 4 + 4);
  __ch.object_id() = this->_data().object_id;
  __ch.poa_idx() = this->_data().poa_idx;
  __ch.interface_idx() = interface_idx_;
  __ch.function_idx() = 0;
  ::flat::DB__M1_Direct _(buf,24);
  _._1() = id;
  _._2(data.size());
  memcpy(_._2().data(), data.data(), data.size() * 1);
  *(uint32_t*)buf.data().data() = static_cast<uint32_t>(buf.size() - 4);
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
    auto mb = buf.prepare(32);
    buf.commit(32);
    *((uint32_t*)mb.data() + 1) = ::nprpc::impl::MessageId::FunctionCall;
  }
  ::nprpc::detail::flat::CallHeader_Direct __ch(buf, 4 + 4);
  __ch.object_id() = this->_data().object_id;
  __ch.poa_idx() = this->_data().poa_idx;
  __ch.interface_idx() = interface_idx_;
  __ch.function_idx() = 1;
  ::flat::DB__M2_Direct _(buf,24);
  _._1() = id;
  *(uint32_t*)buf.data().data() = static_cast<uint32_t>(buf.size() - 4);
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
  nprpc::detail::flat::CallHeader_Direct __ch(bufs(), 4 + 4);
  switch(__ch.function_idx()) {
    case 0: {
      ::flat::DB__M1_Direct ia(bufs(), 24);
      OnNodeChanged(ia._1(), ia._2());
      nprpc::impl::make_simple_message(bufs(), nprpc::impl::MessageId::Success);
      break;
    }
    case 1: {
      ::flat::DB__M2_Direct ia(bufs(), 24);
      OnNodeDeleted(ia._1());
      nprpc::impl::make_simple_message(bufs(), nprpc::impl::MessageId::Success);
      break;
    }
    default:
      nprpc::impl::make_simple_message(bufs(), nprpc::impl::MessageId::Error_UnknownFunctionIdx);
  }
}

std::string npd::Database::get_database_name() {
  boost::beast::flat_buffer buf;
  {
    auto mb = buf.prepare(24);
    buf.commit(24);
    *((uint32_t*)mb.data() + 1) = ::nprpc::impl::MessageId::FunctionCall;
  }
  ::nprpc::detail::flat::CallHeader_Direct __ch(buf, 4 + 4);
  __ch.object_id() = this->_data().object_id;
  __ch.poa_idx() = this->_data().poa_idx;
  __ch.interface_idx() = interface_idx_;
  __ch.function_idx() = 0;
  *(uint32_t*)buf.data().data() = static_cast<uint32_t>(buf.size() - 4);
  ::nprpc::impl::g_orb->call(
    nprpc::EndPoint(this->_data().ip4, this->_data().port), buf, this->get_timeout()
  );
  auto std_reply = nprpc::impl::handle_standart_reply(buf);
  if (std_reply != -1) {
    std::cerr << "received an unusual reply for function with output arguments\n";
    assert(false);
    throw nprpc::Exception("Unknown Error");
  }
  ::flat::DB__M3_Direct out(buf, 8);
  std::string __ret_value;
  __ret_value = (std::string_view)out._1();
  return __ret_value;
}

cbt1::uuid npd::Database::get_database_uuid() {
  boost::beast::flat_buffer buf;
  {
    auto mb = buf.prepare(24);
    buf.commit(24);
    *((uint32_t*)mb.data() + 1) = ::nprpc::impl::MessageId::FunctionCall;
  }
  ::nprpc::detail::flat::CallHeader_Direct __ch(buf, 4 + 4);
  __ch.object_id() = this->_data().object_id;
  __ch.poa_idx() = this->_data().poa_idx;
  __ch.interface_idx() = interface_idx_;
  __ch.function_idx() = 1;
  *(uint32_t*)buf.data().data() = static_cast<uint32_t>(buf.size() - 4);
  ::nprpc::impl::g_orb->call(
    nprpc::EndPoint(this->_data().ip4, this->_data().port), buf, this->get_timeout()
  );
  auto std_reply = nprpc::impl::handle_standart_reply(buf);
  if (std_reply == 1) {
    DB__throw_exception(buf);
  }
  if (std_reply != -1) {
    std::cerr << "received an unusual reply for function with output arguments\n";
    assert(false);
    throw nprpc::Exception("Unknown Error");
  }
  ::flat::DB__M4_Direct out(buf, 8);
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
    auto mb = buf.prepare(32);
    buf.commit(32);
    *((uint32_t*)mb.data() + 1) = ::nprpc::impl::MessageId::FunctionCall;
  }
  ::nprpc::detail::flat::CallHeader_Direct __ch(buf, 4 + 4);
  __ch.object_id() = this->_data().object_id;
  __ch.poa_idx() = this->_data().poa_idx;
  __ch.interface_idx() = interface_idx_;
  __ch.function_idx() = 2;
  ::flat::DB__M2_Direct _(buf,24);
  _._1() = amount;
  *(uint32_t*)buf.data().data() = static_cast<uint32_t>(buf.size() - 4);
  ::nprpc::impl::g_orb->call(
    nprpc::EndPoint(this->_data().ip4, this->_data().port), buf, this->get_timeout()
  );
  auto std_reply = nprpc::impl::handle_standart_reply(buf);
  if (std_reply == 1) {
    DB__throw_exception(buf);
  }
  if (std_reply != -1) {
    std::cerr << "received an unusual reply for function with output arguments\n";
    assert(false);
    throw nprpc::Exception("Unknown Error");
  }
  ::flat::DB__M2_Direct out(buf, 8);
  cbt1::oid_t __ret_value;
  __ret_value = out._1();
  return __ret_value;
}

cbt1::oid_t npd::Database::last_oid() {
  boost::beast::flat_buffer buf;
  {
    auto mb = buf.prepare(24);
    buf.commit(24);
    *((uint32_t*)mb.data() + 1) = ::nprpc::impl::MessageId::FunctionCall;
  }
  ::nprpc::detail::flat::CallHeader_Direct __ch(buf, 4 + 4);
  __ch.object_id() = this->_data().object_id;
  __ch.poa_idx() = this->_data().poa_idx;
  __ch.interface_idx() = interface_idx_;
  __ch.function_idx() = 3;
  *(uint32_t*)buf.data().data() = static_cast<uint32_t>(buf.size() - 4);
  ::nprpc::impl::g_orb->call(
    nprpc::EndPoint(this->_data().ip4, this->_data().port), buf, this->get_timeout()
  );
  auto std_reply = nprpc::impl::handle_standart_reply(buf);
  if (std_reply == 1) {
    DB__throw_exception(buf);
  }
  if (std_reply != -1) {
    std::cerr << "received an unusual reply for function with output arguments\n";
    assert(false);
    throw nprpc::Exception("Unknown Error");
  }
  ::flat::DB__M2_Direct out(buf, 8);
  cbt1::oid_t __ret_value;
  __ret_value = out._1();
  return __ret_value;
}

cbt1::oid_t npd::Database::create(/*in*/::flat::Span<const uint8_t> data, /*in*/bool sync) {
  boost::beast::flat_buffer buf;
  {
    auto mb = buf.prepare(164);
    buf.commit(36);
    *((uint32_t*)mb.data() + 1) = ::nprpc::impl::MessageId::FunctionCall;
  }
  ::nprpc::detail::flat::CallHeader_Direct __ch(buf, 4 + 4);
  __ch.object_id() = this->_data().object_id;
  __ch.poa_idx() = this->_data().poa_idx;
  __ch.interface_idx() = interface_idx_;
  __ch.function_idx() = 4;
  ::flat::DB__M5_Direct _(buf,24);
  _._1(data.size());
  memcpy(_._1().data(), data.data(), data.size() * 1);
  _._2() = sync;
  *(uint32_t*)buf.data().data() = static_cast<uint32_t>(buf.size() - 4);
  ::nprpc::impl::g_orb->call(
    nprpc::EndPoint(this->_data().ip4, this->_data().port), buf, this->get_timeout()
  );
  auto std_reply = nprpc::impl::handle_standart_reply(buf);
  if (std_reply == 1) {
    DB__throw_exception(buf);
  }
  if (std_reply != -1) {
    std::cerr << "received an unusual reply for function with output arguments\n";
    assert(false);
    throw nprpc::Exception("Unknown Error");
  }
  ::flat::DB__M2_Direct out(buf, 8);
  cbt1::oid_t __ret_value;
  __ret_value = out._1();
  return __ret_value;
}

cbt1::oid_t npd::Database::put(/*in*/const cbt1::oid_t& id, /*in*/::flat::Span<const uint8_t> data, /*in*/bool sync) {
  boost::beast::flat_buffer buf;
  {
    auto mb = buf.prepare(176);
    buf.commit(48);
    *((uint32_t*)mb.data() + 1) = ::nprpc::impl::MessageId::FunctionCall;
  }
  ::nprpc::detail::flat::CallHeader_Direct __ch(buf, 4 + 4);
  __ch.object_id() = this->_data().object_id;
  __ch.poa_idx() = this->_data().poa_idx;
  __ch.interface_idx() = interface_idx_;
  __ch.function_idx() = 5;
  ::flat::DB__M6_Direct _(buf,24);
  _._1() = id;
  _._2(data.size());
  memcpy(_._2().data(), data.data(), data.size() * 1);
  _._3() = sync;
  *(uint32_t*)buf.data().data() = static_cast<uint32_t>(buf.size() - 4);
  ::nprpc::impl::g_orb->call(
    nprpc::EndPoint(this->_data().ip4, this->_data().port), buf, this->get_timeout()
  );
  auto std_reply = nprpc::impl::handle_standart_reply(buf);
  if (std_reply == 1) {
    DB__throw_exception(buf);
  }
  if (std_reply != -1) {
    std::cerr << "received an unusual reply for function with output arguments\n";
    assert(false);
    throw nprpc::Exception("Unknown Error");
  }
  ::flat::DB__M2_Direct out(buf, 8);
  cbt1::oid_t __ret_value;
  __ret_value = out._1();
  return __ret_value;
}

bool npd::Database::exec_batch(/*in*/::flat::Span<const npd::BatchOperation> data) {
  boost::beast::flat_buffer buf;
  {
    auto mb = buf.prepare(160);
    buf.commit(32);
    *((uint32_t*)mb.data() + 1) = ::nprpc::impl::MessageId::FunctionCall;
  }
  ::nprpc::detail::flat::CallHeader_Direct __ch(buf, 4 + 4);
  __ch.object_id() = this->_data().object_id;
  __ch.poa_idx() = this->_data().poa_idx;
  __ch.interface_idx() = interface_idx_;
  __ch.function_idx() = 6;
  ::flat::DB__M7_Direct _(buf,24);
  _._1(data.size());
  auto span = _._1();
  auto it = data.begin();
  for (auto e : span) {
    e.create_or_update() = (*it).create_or_update;
    e.id() = (*it).id;
    e.data((*it).data);
    ++it;
  }
  *(uint32_t*)buf.data().data() = static_cast<uint32_t>(buf.size() - 4);
  ::nprpc::impl::g_orb->call(
    nprpc::EndPoint(this->_data().ip4, this->_data().port), buf, this->get_timeout()
  );
  auto std_reply = nprpc::impl::handle_standart_reply(buf);
  if (std_reply == 1) {
    DB__throw_exception(buf);
  }
  if (std_reply != -1) {
    std::cerr << "received an unusual reply for function with output arguments\n";
    assert(false);
    throw nprpc::Exception("Unknown Error");
  }
  ::flat::DB__M8_Direct out(buf, 8);
  bool __ret_value;
  __ret_value = out._1();
  return __ret_value;
}

bool npd::Database::get(/*in*/const cbt1::oid_t& id, /*out*/std::vector<uint8_t>& data) {
  boost::beast::flat_buffer buf;
  {
    auto mb = buf.prepare(32);
    buf.commit(32);
    *((uint32_t*)mb.data() + 1) = ::nprpc::impl::MessageId::FunctionCall;
  }
  ::nprpc::detail::flat::CallHeader_Direct __ch(buf, 4 + 4);
  __ch.object_id() = this->_data().object_id;
  __ch.poa_idx() = this->_data().poa_idx;
  __ch.interface_idx() = interface_idx_;
  __ch.function_idx() = 7;
  ::flat::DB__M2_Direct _(buf,24);
  _._1() = id;
  *(uint32_t*)buf.data().data() = static_cast<uint32_t>(buf.size() - 4);
  ::nprpc::impl::g_orb->call(
    nprpc::EndPoint(this->_data().ip4, this->_data().port), buf, this->get_timeout()
  );
  auto std_reply = nprpc::impl::handle_standart_reply(buf);
  if (std_reply == 1) {
    DB__throw_exception(buf);
  }
  if (std_reply != -1) {
    std::cerr << "received an unusual reply for function with output arguments\n";
    assert(false);
    throw nprpc::Exception("Unknown Error");
  }
  ::flat::DB__M9_Direct out(buf, 8);
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
    auto mb = buf.prepare(160);
    buf.commit(32);
    *((uint32_t*)mb.data() + 1) = ::nprpc::impl::MessageId::FunctionCall;
  }
  ::nprpc::detail::flat::CallHeader_Direct __ch(buf, 4 + 4);
  __ch.object_id() = this->_data().object_id;
  __ch.poa_idx() = this->_data().poa_idx;
  __ch.interface_idx() = interface_idx_;
  __ch.function_idx() = 8;
  ::flat::DB__M10_Direct _(buf,24);
  _._1(ids.size());
  memcpy(_._1().data(), ids.data(), ids.size() * 8);
  *(uint32_t*)buf.data().data() = static_cast<uint32_t>(buf.size() - 4);
  ::nprpc::impl::g_orb->call(
    nprpc::EndPoint(this->_data().ip4, this->_data().port), buf, this->get_timeout()
  );
  auto std_reply = nprpc::impl::handle_standart_reply(buf);
  if (std_reply == 1) {
    DB__throw_exception(buf);
  }
  if (std_reply != -1) {
    std::cerr << "received an unusual reply for function with output arguments\n";
    assert(false);
    throw nprpc::Exception("Unknown Error");
  }
  ::flat::DB__M11_Direct out(buf, 8);
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
    auto mb = buf.prepare(32);
    buf.commit(32);
    *((uint32_t*)mb.data() + 1) = ::nprpc::impl::MessageId::FunctionCall;
  }
  ::nprpc::detail::flat::CallHeader_Direct __ch(buf, 4 + 4);
  __ch.object_id() = this->_data().object_id;
  __ch.poa_idx() = this->_data().poa_idx;
  __ch.interface_idx() = interface_idx_;
  __ch.function_idx() = 9;
  ::flat::DB__M2_Direct _(buf,24);
  _._1() = id;
  *(uint32_t*)buf.data().data() = static_cast<uint32_t>(buf.size() - 4);
  ::nprpc::impl::g_orb->call(
    nprpc::EndPoint(this->_data().ip4, this->_data().port), buf, this->get_timeout()
  );
  auto std_reply = nprpc::impl::handle_standart_reply(buf);
  if (std_reply == 1) {
    DB__throw_exception(buf);
  }
  if (std_reply != 0) {
    std::cerr << "received an unusual reply for function with no output arguments\n";
    assert(false);
  }
}

void npd::Database::advise(/*in*/const cbt1::oid_t& id, /*in*/const ObjectId& client) {
  boost::beast::flat_buffer buf;
  {
    auto mb = buf.prepare(192);
    buf.commit(64);
    *((uint32_t*)mb.data() + 1) = ::nprpc::impl::MessageId::FunctionCall;
  }
  ::nprpc::detail::flat::CallHeader_Direct __ch(buf, 4 + 4);
  __ch.object_id() = this->_data().object_id;
  __ch.poa_idx() = this->_data().poa_idx;
  __ch.interface_idx() = interface_idx_;
  __ch.function_idx() = 10;
  ::flat::DB__M12_Direct _(buf,24);
  _._1() = id;
  memcpy(&_._2().ip4(), &client._data().ip4, 20);
  _._2().class_id(client._data().class_id);
  *(uint32_t*)buf.data().data() = static_cast<uint32_t>(buf.size() - 4);
  ::nprpc::impl::g_orb->call(
    nprpc::EndPoint(this->_data().ip4, this->_data().port), buf, this->get_timeout()
  );
  auto std_reply = nprpc::impl::handle_standart_reply(buf);
  if (std_reply == 1) {
    DB__throw_exception(buf);
  }
  if (std_reply != 0) {
    std::cerr << "received an unusual reply for function with no output arguments\n";
    assert(false);
  }
}

void npd::IDatabase_Servant::dispatch(nprpc::Buffers& bufs, nprpc::EndPoint remote_endpoint, bool from_parent, nprpc::ReferenceList& ref_list) {
  nprpc::detail::flat::CallHeader_Direct __ch(bufs(), 4 + 4);
  switch(__ch.function_idx()) {
    case 0: {
      auto& obuf = bufs.flip();
      obuf.consume(obuf.size());
      obuf.prepare(144);
      obuf.commit(16);
      ::flat::DB__M3_Direct oa(obuf,8);
std::string __ret_val;
      __ret_val = get_database_name();
  oa._1(__ret_val);
      *((uint32_t*)obuf.data().data()) = static_cast<uint32_t>(obuf.size() - 4);
      *((uint32_t*)obuf.data().data() + 1) = ::nprpc::impl::MessageId::BlockResponse;
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
        obuf.prepare(16);
        obuf.commit(16);
        npd::flat::DatabaseException_Direct oa(obuf,8);
        oa.__ex_id() = 0;
  oa.code() = e.code;
        *((uint32_t*)obuf.data().data()) = static_cast<uint32_t>(obuf.size() - 4);
        *((uint32_t*)obuf.data().data() + 1) = ::nprpc::impl::MessageId::Exception;
        return;
      }
      auto& obuf = bufs();
      obuf.consume(obuf.size());
      obuf.prepare(24);
      obuf.commit(24);
      ::flat::DB__M4_Direct oa(obuf,8);
  memcpy(oa._1().data(), __ret_val.data(), __ret_val.size() * 1);
      *((uint32_t*)obuf.data().data()) = static_cast<uint32_t>(obuf.size() - 4);
      *((uint32_t*)obuf.data().data() + 1) = ::nprpc::impl::MessageId::BlockResponse;
      break;
    }
    case 2: {
      ::flat::DB__M2_Direct ia(bufs(), 24);
cbt1::oid_t __ret_val;
      try {
      __ret_val = next_oid(ia._1());
      }
      catch(npd::DatabaseException& e) {
        auto& obuf = bufs();
        obuf.consume(obuf.size());
        obuf.prepare(16);
        obuf.commit(16);
        npd::flat::DatabaseException_Direct oa(obuf,8);
        oa.__ex_id() = 0;
  oa.code() = e.code;
        *((uint32_t*)obuf.data().data()) = static_cast<uint32_t>(obuf.size() - 4);
        *((uint32_t*)obuf.data().data() + 1) = ::nprpc::impl::MessageId::Exception;
        return;
      }
      auto& obuf = bufs();
      obuf.consume(obuf.size());
      obuf.prepare(16);
      obuf.commit(16);
      ::flat::DB__M2_Direct oa(obuf,8);
  oa._1() = __ret_val;
      *((uint32_t*)obuf.data().data()) = static_cast<uint32_t>(obuf.size() - 4);
      *((uint32_t*)obuf.data().data() + 1) = ::nprpc::impl::MessageId::BlockResponse;
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
        obuf.prepare(16);
        obuf.commit(16);
        npd::flat::DatabaseException_Direct oa(obuf,8);
        oa.__ex_id() = 0;
  oa.code() = e.code;
        *((uint32_t*)obuf.data().data()) = static_cast<uint32_t>(obuf.size() - 4);
        *((uint32_t*)obuf.data().data() + 1) = ::nprpc::impl::MessageId::Exception;
        return;
      }
      auto& obuf = bufs();
      obuf.consume(obuf.size());
      obuf.prepare(16);
      obuf.commit(16);
      ::flat::DB__M2_Direct oa(obuf,8);
  oa._1() = __ret_val;
      *((uint32_t*)obuf.data().data()) = static_cast<uint32_t>(obuf.size() - 4);
      *((uint32_t*)obuf.data().data() + 1) = ::nprpc::impl::MessageId::BlockResponse;
      break;
    }
    case 4: {
      ::flat::DB__M5_Direct ia(bufs(), 24);
cbt1::oid_t __ret_val;
      try {
      __ret_val = create(ia._1(), ia._2());
      }
      catch(npd::DatabaseException& e) {
        auto& obuf = bufs();
        obuf.consume(obuf.size());
        obuf.prepare(16);
        obuf.commit(16);
        npd::flat::DatabaseException_Direct oa(obuf,8);
        oa.__ex_id() = 0;
  oa.code() = e.code;
        *((uint32_t*)obuf.data().data()) = static_cast<uint32_t>(obuf.size() - 4);
        *((uint32_t*)obuf.data().data() + 1) = ::nprpc::impl::MessageId::Exception;
        return;
      }
      auto& obuf = bufs();
      obuf.consume(obuf.size());
      obuf.prepare(16);
      obuf.commit(16);
      ::flat::DB__M2_Direct oa(obuf,8);
  oa._1() = __ret_val;
      *((uint32_t*)obuf.data().data()) = static_cast<uint32_t>(obuf.size() - 4);
      *((uint32_t*)obuf.data().data() + 1) = ::nprpc::impl::MessageId::BlockResponse;
      break;
    }
    case 5: {
      ::flat::DB__M6_Direct ia(bufs(), 24);
cbt1::oid_t __ret_val;
      try {
      __ret_val = put(ia._1(), ia._2(), ia._3());
      }
      catch(npd::DatabaseException& e) {
        auto& obuf = bufs();
        obuf.consume(obuf.size());
        obuf.prepare(16);
        obuf.commit(16);
        npd::flat::DatabaseException_Direct oa(obuf,8);
        oa.__ex_id() = 0;
  oa.code() = e.code;
        *((uint32_t*)obuf.data().data()) = static_cast<uint32_t>(obuf.size() - 4);
        *((uint32_t*)obuf.data().data() + 1) = ::nprpc::impl::MessageId::Exception;
        return;
      }
      auto& obuf = bufs();
      obuf.consume(obuf.size());
      obuf.prepare(16);
      obuf.commit(16);
      ::flat::DB__M2_Direct oa(obuf,8);
  oa._1() = __ret_val;
      *((uint32_t*)obuf.data().data()) = static_cast<uint32_t>(obuf.size() - 4);
      *((uint32_t*)obuf.data().data() + 1) = ::nprpc::impl::MessageId::BlockResponse;
      break;
    }
    case 6: {
      ::flat::DB__M7_Direct ia(bufs(), 24);
bool __ret_val;
      try {
      __ret_val = exec_batch(ia._1());
      }
      catch(npd::DatabaseException& e) {
        auto& obuf = bufs();
        obuf.consume(obuf.size());
        obuf.prepare(16);
        obuf.commit(16);
        npd::flat::DatabaseException_Direct oa(obuf,8);
        oa.__ex_id() = 0;
  oa.code() = e.code;
        *((uint32_t*)obuf.data().data()) = static_cast<uint32_t>(obuf.size() - 4);
        *((uint32_t*)obuf.data().data() + 1) = ::nprpc::impl::MessageId::Exception;
        return;
      }
      auto& obuf = bufs();
      obuf.consume(obuf.size());
      obuf.prepare(9);
      obuf.commit(9);
      ::flat::DB__M8_Direct oa(obuf,8);
  oa._1() = __ret_val;
      *((uint32_t*)obuf.data().data()) = static_cast<uint32_t>(obuf.size() - 4);
      *((uint32_t*)obuf.data().data() + 1) = ::nprpc::impl::MessageId::BlockResponse;
      break;
    }
    case 7: {
      ::flat::DB__M2_Direct ia(bufs(), 24);
      auto& obuf = bufs.flip();
      obuf.consume(obuf.size());
      obuf.prepare(148);
      obuf.commit(20);
      ::flat::DB__M9_Direct oa(obuf,8);
bool __ret_val;
      try {
      __ret_val = get(ia._1(), oa._2_vd());
      }
      catch(npd::DatabaseException& e) {
        auto& obuf = bufs();
        obuf.consume(obuf.size());
        obuf.prepare(16);
        obuf.commit(16);
        npd::flat::DatabaseException_Direct oa(obuf,8);
        oa.__ex_id() = 0;
  oa.code() = e.code;
        *((uint32_t*)obuf.data().data()) = static_cast<uint32_t>(obuf.size() - 4);
        *((uint32_t*)obuf.data().data() + 1) = ::nprpc::impl::MessageId::Exception;
        return;
      }
  oa._1() = __ret_val;
      *((uint32_t*)obuf.data().data()) = static_cast<uint32_t>(obuf.size() - 4);
      *((uint32_t*)obuf.data().data() + 1) = ::nprpc::impl::MessageId::BlockResponse;
      break;
    }
    case 8: {
      ::flat::DB__M10_Direct ia(bufs(), 24);
      auto& obuf = bufs.flip();
      obuf.consume(obuf.size());
      obuf.prepare(152);
      obuf.commit(24);
      ::flat::DB__M11_Direct oa(obuf,8);
uint64_t __ret_val;
      try {
      __ret_val = get_n(ia._1(), oa._2_vd());
      }
      catch(npd::DatabaseException& e) {
        auto& obuf = bufs();
        obuf.consume(obuf.size());
        obuf.prepare(16);
        obuf.commit(16);
        npd::flat::DatabaseException_Direct oa(obuf,8);
        oa.__ex_id() = 0;
  oa.code() = e.code;
        *((uint32_t*)obuf.data().data()) = static_cast<uint32_t>(obuf.size() - 4);
        *((uint32_t*)obuf.data().data() + 1) = ::nprpc::impl::MessageId::Exception;
        return;
      }
  oa._1() = __ret_val;
      *((uint32_t*)obuf.data().data()) = static_cast<uint32_t>(obuf.size() - 4);
      *((uint32_t*)obuf.data().data() + 1) = ::nprpc::impl::MessageId::BlockResponse;
      break;
    }
    case 9: {
      ::flat::DB__M2_Direct ia(bufs(), 24);
      try {
      remove(ia._1());
      }
      catch(npd::DatabaseException& e) {
        auto& obuf = bufs();
        obuf.consume(obuf.size());
        obuf.prepare(16);
        obuf.commit(16);
        npd::flat::DatabaseException_Direct oa(obuf,8);
        oa.__ex_id() = 0;
  oa.code() = e.code;
        *((uint32_t*)obuf.data().data()) = static_cast<uint32_t>(obuf.size() - 4);
        *((uint32_t*)obuf.data().data() + 1) = ::nprpc::impl::MessageId::Exception;
        return;
      }
      nprpc::impl::make_simple_message(bufs(), nprpc::impl::MessageId::Success);
      break;
    }
    case 10: {
      ::flat::DB__M12_Direct ia(bufs(), 24);
      try {
      advise(ia._1(), nprpc::impl::g_orb->create_object_from_flat(ia._2(), remote_endpoint));
      }
      catch(npd::DatabaseException& e) {
        auto& obuf = bufs();
        obuf.consume(obuf.size());
        obuf.prepare(16);
        obuf.commit(16);
        npd::flat::DatabaseException_Direct oa(obuf,8);
        oa.__ex_id() = 0;
  oa.code() = e.code;
        *((uint32_t*)obuf.data().data()) = static_cast<uint32_t>(obuf.size() - 4);
        *((uint32_t*)obuf.data().data() + 1) = ::nprpc::impl::MessageId::Exception;
        return;
      }
      nprpc::impl::make_simple_message(bufs(), nprpc::impl::MessageId::Success);
      break;
    }
    default:
      nprpc::impl::make_simple_message(bufs(), nprpc::impl::MessageId::Error_UnknownFunctionIdx);
  }
}

} // namespace npd


void DB__throw_exception(boost::beast::flat_buffer& buf) { 
  switch(*((uint32_t*)buf.data().data() + 2)) {
  case 0:
  {
    npd::flat::DatabaseException_Direct ex_flat(buf, 4 + 4);
    npd::DatabaseException ex;
  ex.code = ex_flat.code();
    throw ex;
  }
  default:
    throw std::runtime_error("unknown rpc exception");
  }
}
