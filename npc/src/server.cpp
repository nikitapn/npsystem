#include "server.hpp"
#include "server_m.hpp"
#include <nprpc/nprpc_impl.hpp>

void SERVER__throw_exception(boost::beast::flat_buffer& buf);

namespace cbt1 { 
} // namespace cbt1

namespace nps { 
void nps::Pingable::Ping() {
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
  if (std_reply != 0) {
    std::cerr << "received an unusual reply for function with no output arguments\n";
    assert(false);
  }
}

void nps::IPingable_Servant::dispatch(nprpc::Buffers& bufs, nprpc::EndPoint remote_endpoint, bool from_parent, nprpc::ReferenceList& ref_list) {
  nprpc::detail::flat::CallHeader_Direct __ch(bufs(), 4 + 4);
  switch(__ch.function_idx()) {
    case 0: {
      Ping();
      nprpc::impl::make_simple_message(bufs(), nprpc::impl::MessageId::Success);
      break;
    }
    default:
      nprpc::impl::make_simple_message(bufs(), nprpc::impl::MessageId::Error_UnknownFunctionIdx);
  }
}

void nps::DataCallBack::OnDataChanged(/*in*/::flat::Span<const nps::server_value> a) {
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
  __ch.function_idx() = 0;
  ::flat::SERVER__M1_Direct _(buf,24);
  _._1(a.size());
  memcpy(_._1().data(), a.data(), a.size() * 24);
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

void nps::IDataCallBack_Servant::dispatch(nprpc::Buffers& bufs, nprpc::EndPoint remote_endpoint, bool from_parent, nprpc::ReferenceList& ref_list) {
  nprpc::detail::flat::CallHeader_Direct __ch(bufs(), 4 + 4);
  if (from_parent == false) {
    switch(__ch.interface_idx()) {
      case 0:
        break;
      case 1:
        IPingable_Servant::dispatch(bufs, remote_endpoint, true, ref_list);
        return;
      default:
        assert(false);
        throw "unknown interface";
    }
  }
  switch(__ch.function_idx()) {
    case 0: {
      ::flat::SERVER__M1_Direct ia(bufs(), 24);
      OnDataChanged(ia._1());
      nprpc::impl::make_simple_message(bufs(), nprpc::impl::MessageId::Success);
      break;
    }
    default:
      nprpc::impl::make_simple_message(bufs(), nprpc::impl::MessageId::Error_UnknownFunctionIdx);
  }
}

void nps::ItemManager::Activate(/*in*/const ObjectId& client) {
  boost::beast::flat_buffer buf;
  {
    auto mb = buf.prepare(184);
    buf.commit(56);
    *((uint32_t*)mb.data() + 1) = ::nprpc::impl::MessageId::FunctionCall;
  }
  ::nprpc::detail::flat::CallHeader_Direct __ch(buf, 4 + 4);
  __ch.object_id() = this->_data().object_id;
  __ch.poa_idx() = this->_data().poa_idx;
  __ch.interface_idx() = interface_idx_;
  __ch.function_idx() = 0;
  ::flat::SERVER__M2_Direct _(buf,24);
  memcpy(&_._1().ip4(), &client._data().ip4, 20);
  _._1().class_id(client._data().class_id);
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

void nps::ItemManager::Advise(/*in*/::flat::Span<const nps::DataDef> a, /*out*/std::vector<var_handle>& h) {
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
  __ch.function_idx() = 1;
  ::flat::SERVER__M3_Direct _(buf,24);
  _._1(a.size());
  memcpy(_._1().data(), a.data(), a.size() * 8);
  *(uint32_t*)buf.data().data() = static_cast<uint32_t>(buf.size() - 4);
  ::nprpc::impl::g_orb->call(
    nprpc::EndPoint(this->_data().ip4, this->_data().port), buf, this->get_timeout()
  );
  auto std_reply = nprpc::impl::handle_standart_reply(buf);
  if (std_reply == 1) {
    SERVER__throw_exception(buf);
  }
  if (std_reply != -1) {
    std::cerr << "received an unusual reply for function with output arguments\n";
    assert(false);
    throw nprpc::Exception("Unknown Error");
  }
  ::flat::SERVER__M4_Direct out(buf, 8);
  {
    auto span = out._1();
    h.resize(span.size());
    memcpy(h.data(), span.data(), 8 * span.size());
  }
}

void nps::ItemManager::UnAdvise(/*in*/::flat::Span<const var_handle> a) {
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
  __ch.function_idx() = 2;
  ::flat::SERVER__M4_Direct _(buf,24);
  _._1(a.size());
  memcpy(_._1().data(), a.data(), a.size() * 8);
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

void nps::IItemManager_Servant::dispatch(nprpc::Buffers& bufs, nprpc::EndPoint remote_endpoint, bool from_parent, nprpc::ReferenceList& ref_list) {
  nprpc::detail::flat::CallHeader_Direct __ch(bufs(), 4 + 4);
  if (from_parent == false) {
    switch(__ch.interface_idx()) {
      case 0:
        break;
      case 1:
        IPingable_Servant::dispatch(bufs, remote_endpoint, true, ref_list);
        return;
      default:
        assert(false);
        throw "unknown interface";
    }
  }
  switch(__ch.function_idx()) {
    case 0: {
      ::flat::SERVER__M2_Direct ia(bufs(), 24);
      Activate(nprpc::impl::g_orb->create_object_from_flat(ia._1(), remote_endpoint));
      nprpc::impl::make_simple_message(bufs(), nprpc::impl::MessageId::Success);
      break;
    }
    case 1: {
      ::flat::SERVER__M3_Direct ia(bufs(), 24);
      auto& obuf = bufs.flip();
      obuf.consume(obuf.size());
      obuf.prepare(144);
      obuf.commit(16);
      ::flat::SERVER__M4_Direct oa(obuf,8);
      try {
      Advise(ia._1(), oa._1_vd());
      }
      catch(nps::NPNetCommunicationError& e) {
        auto& obuf = bufs();
        obuf.consume(obuf.size());
        obuf.prepare(16);
        obuf.commit(16);
        nps::flat::NPNetCommunicationError_Direct oa(obuf,8);
        oa.__ex_id() = 0;
  oa.code() = e.code;
        *((uint32_t*)obuf.data().data()) = static_cast<uint32_t>(obuf.size() - 4);
        *((uint32_t*)obuf.data().data() + 1) = ::nprpc::impl::MessageId::Exception;
        return;
      }
      *((uint32_t*)obuf.data().data()) = static_cast<uint32_t>(obuf.size() - 4);
      *((uint32_t*)obuf.data().data() + 1) = ::nprpc::impl::MessageId::BlockResponse;
      break;
    }
    case 2: {
      ::flat::SERVER__M4_Direct ia(bufs(), 24);
      UnAdvise(ia._1());
      nprpc::impl::make_simple_message(bufs(), nprpc::impl::MessageId::Success);
      break;
    }
    default:
      nprpc::impl::make_simple_message(bufs(), nprpc::impl::MessageId::Error_UnknownFunctionIdx);
  }
}

void nps::Server::GetNetworkStatus(/*out*/std::vector<uint8_t>& network_status) {
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
  ::flat::SERVER__M5_Direct out(buf, 8);
  {
    auto span = out._1();
    network_status.resize(span.size());
    memcpy(network_status.data(), span.data(), 1 * span.size());
  }
}

void nps::Server::CreateItemManager(/*out*/Object*& im) {
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
  if (std_reply != -1) {
    std::cerr << "received an unusual reply for function with output arguments\n";
    assert(false);
    throw nprpc::Exception("Unknown Error");
  }
  ::flat::SERVER__M2_Direct out(buf, 8);
  im = this->create_from_object_id(out._1());
}

void nps::Server::SendRawData(/*in*/::flat::Span<const uint8_t> data) {
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
  __ch.function_idx() = 2;
  ::flat::SERVER__M5_Direct _(buf,24);
  _._1(data.size());
  memcpy(_._1().data(), data.data(), data.size() * 1);
  *(uint32_t*)buf.data().data() = static_cast<uint32_t>(buf.size() - 4);
  ::nprpc::impl::g_orb->call(
    nprpc::EndPoint(this->_data().ip4, this->_data().port), buf, this->get_timeout()
  );
  auto std_reply = nprpc::impl::handle_standart_reply(buf);
  if (std_reply == 1) {
    SERVER__throw_exception(buf);
  }
  if (std_reply != 0) {
    std::cerr << "received an unusual reply for function with no output arguments\n";
    assert(false);
  }
}

void nps::Server::Write_1(/*in*/uint8_t dev_addr, /*in*/uint16_t mem_addr, /*in*/uint8_t bit, /*in*/uint8_t value) {
  boost::beast::flat_buffer buf;
  {
    auto mb = buf.prepare(30);
    buf.commit(30);
    *((uint32_t*)mb.data() + 1) = ::nprpc::impl::MessageId::FunctionCall;
  }
  ::nprpc::detail::flat::CallHeader_Direct __ch(buf, 4 + 4);
  __ch.object_id() = this->_data().object_id;
  __ch.poa_idx() = this->_data().poa_idx;
  __ch.interface_idx() = interface_idx_;
  __ch.function_idx() = 3;
  ::flat::SERVER__M6_Direct _(buf,24);
  _._1() = dev_addr;
  _._2() = mem_addr;
  _._3() = bit;
  _._4() = value;
  *(uint32_t*)buf.data().data() = static_cast<uint32_t>(buf.size() - 4);
  ::nprpc::impl::g_orb->call(
    nprpc::EndPoint(this->_data().ip4, this->_data().port), buf, this->get_timeout()
  );
  auto std_reply = nprpc::impl::handle_standart_reply(buf);
  if (std_reply == 1) {
    SERVER__throw_exception(buf);
  }
  if (std_reply != 0) {
    std::cerr << "received an unusual reply for function with no output arguments\n";
    assert(false);
  }
}

void nps::Server::Write_q1(/*in*/uint8_t dev_addr, /*in*/uint16_t mem_addr, /*in*/uint8_t bit, /*in*/uint8_t value, /*in*/uint8_t qvalue) {
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
  __ch.function_idx() = 4;
  ::flat::SERVER__M7_Direct _(buf,24);
  _._1() = dev_addr;
  _._2() = mem_addr;
  _._3() = bit;
  _._4() = value;
  _._5() = qvalue;
  *(uint32_t*)buf.data().data() = static_cast<uint32_t>(buf.size() - 4);
  ::nprpc::impl::g_orb->call(
    nprpc::EndPoint(this->_data().ip4, this->_data().port), buf, this->get_timeout()
  );
  auto std_reply = nprpc::impl::handle_standart_reply(buf);
  if (std_reply == 1) {
    SERVER__throw_exception(buf);
  }
  if (std_reply != 0) {
    std::cerr << "received an unusual reply for function with no output arguments\n";
    assert(false);
  }
}

void nps::Server::Write_8(/*in*/uint8_t dev_addr, /*in*/uint16_t mem_addr, /*in*/uint8_t value) {
  boost::beast::flat_buffer buf;
  {
    auto mb = buf.prepare(30);
    buf.commit(30);
    *((uint32_t*)mb.data() + 1) = ::nprpc::impl::MessageId::FunctionCall;
  }
  ::nprpc::detail::flat::CallHeader_Direct __ch(buf, 4 + 4);
  __ch.object_id() = this->_data().object_id;
  __ch.poa_idx() = this->_data().poa_idx;
  __ch.interface_idx() = interface_idx_;
  __ch.function_idx() = 5;
  ::flat::SERVER__M8_Direct _(buf,24);
  _._1() = dev_addr;
  _._2() = mem_addr;
  _._3() = value;
  *(uint32_t*)buf.data().data() = static_cast<uint32_t>(buf.size() - 4);
  ::nprpc::impl::g_orb->call(
    nprpc::EndPoint(this->_data().ip4, this->_data().port), buf, this->get_timeout()
  );
  auto std_reply = nprpc::impl::handle_standart_reply(buf);
  if (std_reply == 1) {
    SERVER__throw_exception(buf);
  }
  if (std_reply != 0) {
    std::cerr << "received an unusual reply for function with no output arguments\n";
    assert(false);
  }
}

void nps::Server::Write_q8(/*in*/uint8_t dev_addr, /*in*/uint16_t mem_addr, /*in*/uint8_t value, /*in*/uint8_t qvalue) {
  boost::beast::flat_buffer buf;
  {
    auto mb = buf.prepare(30);
    buf.commit(30);
    *((uint32_t*)mb.data() + 1) = ::nprpc::impl::MessageId::FunctionCall;
  }
  ::nprpc::detail::flat::CallHeader_Direct __ch(buf, 4 + 4);
  __ch.object_id() = this->_data().object_id;
  __ch.poa_idx() = this->_data().poa_idx;
  __ch.interface_idx() = interface_idx_;
  __ch.function_idx() = 6;
  ::flat::SERVER__M6_Direct _(buf,24);
  _._1() = dev_addr;
  _._2() = mem_addr;
  _._3() = value;
  _._4() = qvalue;
  *(uint32_t*)buf.data().data() = static_cast<uint32_t>(buf.size() - 4);
  ::nprpc::impl::g_orb->call(
    nprpc::EndPoint(this->_data().ip4, this->_data().port), buf, this->get_timeout()
  );
  auto std_reply = nprpc::impl::handle_standart_reply(buf);
  if (std_reply == 1) {
    SERVER__throw_exception(buf);
  }
  if (std_reply != 0) {
    std::cerr << "received an unusual reply for function with no output arguments\n";
    assert(false);
  }
}

void nps::Server::Write_16(/*in*/uint8_t dev_addr, /*in*/uint16_t mem_addr, /*in*/uint16_t value) {
  boost::beast::flat_buffer buf;
  {
    auto mb = buf.prepare(30);
    buf.commit(30);
    *((uint32_t*)mb.data() + 1) = ::nprpc::impl::MessageId::FunctionCall;
  }
  ::nprpc::detail::flat::CallHeader_Direct __ch(buf, 4 + 4);
  __ch.object_id() = this->_data().object_id;
  __ch.poa_idx() = this->_data().poa_idx;
  __ch.interface_idx() = interface_idx_;
  __ch.function_idx() = 7;
  ::flat::SERVER__M9_Direct _(buf,24);
  _._1() = dev_addr;
  _._2() = mem_addr;
  _._3() = value;
  *(uint32_t*)buf.data().data() = static_cast<uint32_t>(buf.size() - 4);
  ::nprpc::impl::g_orb->call(
    nprpc::EndPoint(this->_data().ip4, this->_data().port), buf, this->get_timeout()
  );
  auto std_reply = nprpc::impl::handle_standart_reply(buf);
  if (std_reply == 1) {
    SERVER__throw_exception(buf);
  }
  if (std_reply != 0) {
    std::cerr << "received an unusual reply for function with no output arguments\n";
    assert(false);
  }
}

void nps::Server::Write_q16(/*in*/uint8_t dev_addr, /*in*/uint16_t mem_addr, /*in*/uint16_t value, /*in*/uint8_t qvalue) {
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
  __ch.function_idx() = 8;
  ::flat::SERVER__M10_Direct _(buf,24);
  _._1() = dev_addr;
  _._2() = mem_addr;
  _._3() = value;
  _._4() = qvalue;
  *(uint32_t*)buf.data().data() = static_cast<uint32_t>(buf.size() - 4);
  ::nprpc::impl::g_orb->call(
    nprpc::EndPoint(this->_data().ip4, this->_data().port), buf, this->get_timeout()
  );
  auto std_reply = nprpc::impl::handle_standart_reply(buf);
  if (std_reply == 1) {
    SERVER__throw_exception(buf);
  }
  if (std_reply != 0) {
    std::cerr << "received an unusual reply for function with no output arguments\n";
    assert(false);
  }
}

void nps::Server::Write_32(/*in*/uint8_t dev_addr, /*in*/uint16_t mem_addr, /*in*/uint32_t value) {
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
  ::flat::SERVER__M11_Direct _(buf,24);
  _._1() = dev_addr;
  _._2() = mem_addr;
  _._3() = value;
  *(uint32_t*)buf.data().data() = static_cast<uint32_t>(buf.size() - 4);
  ::nprpc::impl::g_orb->call(
    nprpc::EndPoint(this->_data().ip4, this->_data().port), buf, this->get_timeout()
  );
  auto std_reply = nprpc::impl::handle_standart_reply(buf);
  if (std_reply == 1) {
    SERVER__throw_exception(buf);
  }
  if (std_reply != 0) {
    std::cerr << "received an unusual reply for function with no output arguments\n";
    assert(false);
  }
}

void nps::Server::Write_q32(/*in*/uint8_t dev_addr, /*in*/uint16_t mem_addr, /*in*/uint32_t value, /*in*/uint8_t qvalue) {
  boost::beast::flat_buffer buf;
  {
    auto mb = buf.prepare(36);
    buf.commit(36);
    *((uint32_t*)mb.data() + 1) = ::nprpc::impl::MessageId::FunctionCall;
  }
  ::nprpc::detail::flat::CallHeader_Direct __ch(buf, 4 + 4);
  __ch.object_id() = this->_data().object_id;
  __ch.poa_idx() = this->_data().poa_idx;
  __ch.interface_idx() = interface_idx_;
  __ch.function_idx() = 10;
  ::flat::SERVER__M12_Direct _(buf,24);
  _._1() = dev_addr;
  _._2() = mem_addr;
  _._3() = value;
  _._4() = qvalue;
  *(uint32_t*)buf.data().data() = static_cast<uint32_t>(buf.size() - 4);
  ::nprpc::impl::g_orb->call(
    nprpc::EndPoint(this->_data().ip4, this->_data().port), buf, this->get_timeout()
  );
  auto std_reply = nprpc::impl::handle_standart_reply(buf);
  if (std_reply == 1) {
    SERVER__throw_exception(buf);
  }
  if (std_reply != 0) {
    std::cerr << "received an unusual reply for function with no output arguments\n";
    assert(false);
  }
}

void nps::Server::WriteBlock(/*in*/uint8_t dev_addr, /*in*/uint16_t mem_addr, /*in*/::flat::Span<const uint8_t> data) {
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
  __ch.function_idx() = 11;
  ::flat::SERVER__M13_Direct _(buf,24);
  _._1() = dev_addr;
  _._2() = mem_addr;
  _._3(data.size());
  memcpy(_._3().data(), data.data(), data.size() * 1);
  *(uint32_t*)buf.data().data() = static_cast<uint32_t>(buf.size() - 4);
  ::nprpc::impl::g_orb->call(
    nprpc::EndPoint(this->_data().ip4, this->_data().port), buf, this->get_timeout()
  );
  auto std_reply = nprpc::impl::handle_standart_reply(buf);
  if (std_reply == 1) {
    SERVER__throw_exception(buf);
  }
  if (std_reply != 0) {
    std::cerr << "received an unusual reply for function with no output arguments\n";
    assert(false);
  }
}

void nps::Server::ReadByte(/*in*/uint8_t dev_addr, /*in*/uint16_t addr, /*out*/uint8_t& value) {
  boost::beast::flat_buffer buf;
  {
    auto mb = buf.prepare(28);
    buf.commit(28);
    *((uint32_t*)mb.data() + 1) = ::nprpc::impl::MessageId::FunctionCall;
  }
  ::nprpc::detail::flat::CallHeader_Direct __ch(buf, 4 + 4);
  __ch.object_id() = this->_data().object_id;
  __ch.poa_idx() = this->_data().poa_idx;
  __ch.interface_idx() = interface_idx_;
  __ch.function_idx() = 12;
  ::flat::SERVER__M14_Direct _(buf,24);
  _._1() = dev_addr;
  _._2() = addr;
  *(uint32_t*)buf.data().data() = static_cast<uint32_t>(buf.size() - 4);
  ::nprpc::impl::g_orb->call(
    nprpc::EndPoint(this->_data().ip4, this->_data().port), buf, this->get_timeout()
  );
  auto std_reply = nprpc::impl::handle_standart_reply(buf);
  if (std_reply == 1) {
    SERVER__throw_exception(buf);
  }
  if (std_reply != -1) {
    std::cerr << "received an unusual reply for function with output arguments\n";
    assert(false);
    throw nprpc::Exception("Unknown Error");
  }
  ::flat::SERVER__M15_Direct out(buf, 8);
  value = out._1();
}

void nps::Server::ReadBlock(/*in*/uint8_t dev_addr, /*in*/uint16_t addr, /*in*/uint8_t len, /*out*/std::vector<uint8_t>& data) {
  boost::beast::flat_buffer buf;
  {
    auto mb = buf.prepare(30);
    buf.commit(30);
    *((uint32_t*)mb.data() + 1) = ::nprpc::impl::MessageId::FunctionCall;
  }
  ::nprpc::detail::flat::CallHeader_Direct __ch(buf, 4 + 4);
  __ch.object_id() = this->_data().object_id;
  __ch.poa_idx() = this->_data().poa_idx;
  __ch.interface_idx() = interface_idx_;
  __ch.function_idx() = 13;
  ::flat::SERVER__M8_Direct _(buf,24);
  _._1() = dev_addr;
  _._2() = addr;
  _._3() = len;
  *(uint32_t*)buf.data().data() = static_cast<uint32_t>(buf.size() - 4);
  ::nprpc::impl::g_orb->call(
    nprpc::EndPoint(this->_data().ip4, this->_data().port), buf, this->get_timeout()
  );
  auto std_reply = nprpc::impl::handle_standart_reply(buf);
  if (std_reply == 1) {
    SERVER__throw_exception(buf);
  }
  if (std_reply != -1) {
    std::cerr << "received an unusual reply for function with output arguments\n";
    assert(false);
    throw nprpc::Exception("Unknown Error");
  }
  ::flat::SERVER__M5_Direct out(buf, 8);
  {
    auto span = out._1();
    data.resize(span.size());
    memcpy(data.data(), span.data(), 1 * span.size());
  }
}

bool nps::Server::AVR_StopAlgorithm(/*in*/uint8_t dev_addr, /*in*/uint16_t alg_addr) {
  boost::beast::flat_buffer buf;
  {
    auto mb = buf.prepare(28);
    buf.commit(28);
    *((uint32_t*)mb.data() + 1) = ::nprpc::impl::MessageId::FunctionCall;
  }
  ::nprpc::detail::flat::CallHeader_Direct __ch(buf, 4 + 4);
  __ch.object_id() = this->_data().object_id;
  __ch.poa_idx() = this->_data().poa_idx;
  __ch.interface_idx() = interface_idx_;
  __ch.function_idx() = 14;
  ::flat::SERVER__M14_Direct _(buf,24);
  _._1() = dev_addr;
  _._2() = alg_addr;
  *(uint32_t*)buf.data().data() = static_cast<uint32_t>(buf.size() - 4);
  ::nprpc::impl::g_orb->call(
    nprpc::EndPoint(this->_data().ip4, this->_data().port), buf, this->get_timeout()
  );
  auto std_reply = nprpc::impl::handle_standart_reply(buf);
  if (std_reply == 1) {
    SERVER__throw_exception(buf);
  }
  if (std_reply != -1) {
    std::cerr << "received an unusual reply for function with output arguments\n";
    assert(false);
    throw nprpc::Exception("Unknown Error");
  }
  ::flat::SERVER__M15_Direct out(buf, 8);
  bool __ret_value;
  __ret_value = out._1();
  return __ret_value;
}

void nps::Server::AVR_ReinitIO(/*in*/uint8_t dev_addr) {
  boost::beast::flat_buffer buf;
  {
    auto mb = buf.prepare(25);
    buf.commit(25);
    *((uint32_t*)mb.data() + 1) = ::nprpc::impl::MessageId::FunctionCall;
  }
  ::nprpc::detail::flat::CallHeader_Direct __ch(buf, 4 + 4);
  __ch.object_id() = this->_data().object_id;
  __ch.poa_idx() = this->_data().poa_idx;
  __ch.interface_idx() = interface_idx_;
  __ch.function_idx() = 15;
  ::flat::SERVER__M15_Direct _(buf,24);
  _._1() = dev_addr;
  *(uint32_t*)buf.data().data() = static_cast<uint32_t>(buf.size() - 4);
  ::nprpc::impl::g_orb->call(
    nprpc::EndPoint(this->_data().ip4, this->_data().port), buf, this->get_timeout()
  );
  auto std_reply = nprpc::impl::handle_standart_reply(buf);
  if (std_reply == 1) {
    SERVER__throw_exception(buf);
  }
  if (std_reply != 0) {
    std::cerr << "received an unusual reply for function with no output arguments\n";
    assert(false);
  }
}

void nps::Server::AVR_SendRemoteData(/*in*/uint8_t dev_addr, /*in*/uint16_t page_num, /*in*/::flat::Span<const uint8_t> data) {
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
  __ch.function_idx() = 16;
  ::flat::SERVER__M13_Direct _(buf,24);
  _._1() = dev_addr;
  _._2() = page_num;
  _._3(data.size());
  memcpy(_._3().data(), data.data(), data.size() * 1);
  *(uint32_t*)buf.data().data() = static_cast<uint32_t>(buf.size() - 4);
  ::nprpc::impl::g_orb->call(
    nprpc::EndPoint(this->_data().ip4, this->_data().port), buf, this->get_timeout()
  );
  auto std_reply = nprpc::impl::handle_standart_reply(buf);
  if (std_reply == 1) {
    SERVER__throw_exception(buf);
  }
  if (std_reply != 0) {
    std::cerr << "received an unusual reply for function with no output arguments\n";
    assert(false);
  }
}

void nps::Server::AVR_SendPage(/*in*/uint8_t dev_addr, /*in*/uint8_t page_num, /*in*/::flat::Span<const uint8_t> data) {
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
  __ch.function_idx() = 17;
  ::flat::SERVER__M16_Direct _(buf,24);
  _._1() = dev_addr;
  _._2() = page_num;
  _._3(data.size());
  memcpy(_._3().data(), data.data(), data.size() * 1);
  *(uint32_t*)buf.data().data() = static_cast<uint32_t>(buf.size() - 4);
  ::nprpc::impl::g_orb->call(
    nprpc::EndPoint(this->_data().ip4, this->_data().port), buf, this->get_timeout()
  );
  auto std_reply = nprpc::impl::handle_standart_reply(buf);
  if (std_reply == 1) {
    SERVER__throw_exception(buf);
  }
  if (std_reply != 0) {
    std::cerr << "received an unusual reply for function with no output arguments\n";
    assert(false);
  }
}

void nps::Server::AVR_RemoveAlgorithm(/*in*/uint8_t dev_addr, /*in*/uint16_t alg_addr) {
  boost::beast::flat_buffer buf;
  {
    auto mb = buf.prepare(28);
    buf.commit(28);
    *((uint32_t*)mb.data() + 1) = ::nprpc::impl::MessageId::FunctionCall;
  }
  ::nprpc::detail::flat::CallHeader_Direct __ch(buf, 4 + 4);
  __ch.object_id() = this->_data().object_id;
  __ch.poa_idx() = this->_data().poa_idx;
  __ch.interface_idx() = interface_idx_;
  __ch.function_idx() = 18;
  ::flat::SERVER__M14_Direct _(buf,24);
  _._1() = dev_addr;
  _._2() = alg_addr;
  *(uint32_t*)buf.data().data() = static_cast<uint32_t>(buf.size() - 4);
  ::nprpc::impl::g_orb->call(
    nprpc::EndPoint(this->_data().ip4, this->_data().port), buf, this->get_timeout()
  );
  auto std_reply = nprpc::impl::handle_standart_reply(buf);
  if (std_reply == 1) {
    SERVER__throw_exception(buf);
  }
  if (std_reply != 0) {
    std::cerr << "received an unusual reply for function with no output arguments\n";
    assert(false);
  }
}

void nps::Server::AVR_ReplaceAlgorithm(/*in*/uint8_t dev_addr, /*in*/uint16_t old_addr, /*in*/uint16_t new_addr) {
  boost::beast::flat_buffer buf;
  {
    auto mb = buf.prepare(30);
    buf.commit(30);
    *((uint32_t*)mb.data() + 1) = ::nprpc::impl::MessageId::FunctionCall;
  }
  ::nprpc::detail::flat::CallHeader_Direct __ch(buf, 4 + 4);
  __ch.object_id() = this->_data().object_id;
  __ch.poa_idx() = this->_data().poa_idx;
  __ch.interface_idx() = interface_idx_;
  __ch.function_idx() = 19;
  ::flat::SERVER__M9_Direct _(buf,24);
  _._1() = dev_addr;
  _._2() = old_addr;
  _._3() = new_addr;
  *(uint32_t*)buf.data().data() = static_cast<uint32_t>(buf.size() - 4);
  ::nprpc::impl::g_orb->call(
    nprpc::EndPoint(this->_data().ip4, this->_data().port), buf, this->get_timeout()
  );
  auto std_reply = nprpc::impl::handle_standart_reply(buf);
  if (std_reply == 1) {
    SERVER__throw_exception(buf);
  }
  if (std_reply != 0) {
    std::cerr << "received an unusual reply for function with no output arguments\n";
    assert(false);
  }
}

void nps::Server::AVR_WriteEeprom(/*in*/uint8_t dev_addr, /*in*/uint16_t mem_addr, /*in*/::flat::Span<const uint8_t> data) {
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
  __ch.function_idx() = 20;
  ::flat::SERVER__M13_Direct _(buf,24);
  _._1() = dev_addr;
  _._2() = mem_addr;
  _._3(data.size());
  memcpy(_._3().data(), data.data(), data.size() * 1);
  *(uint32_t*)buf.data().data() = static_cast<uint32_t>(buf.size() - 4);
  ::nprpc::impl::g_orb->call(
    nprpc::EndPoint(this->_data().ip4, this->_data().port), buf, this->get_timeout()
  );
  auto std_reply = nprpc::impl::handle_standart_reply(buf);
  if (std_reply == 1) {
    SERVER__throw_exception(buf);
  }
  if (std_reply != 0) {
    std::cerr << "received an unusual reply for function with no output arguments\n";
    assert(false);
  }
}

void nps::Server::AVR_WriteTwiTable(/*in*/uint8_t dev_addr, /*in*/uint8_t page_num, /*in*/::flat::Span<const uint8_t> data) {
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
  __ch.function_idx() = 21;
  ::flat::SERVER__M16_Direct _(buf,24);
  _._1() = dev_addr;
  _._2() = page_num;
  _._3(data.size());
  memcpy(_._3().data(), data.data(), data.size() * 1);
  *(uint32_t*)buf.data().data() = static_cast<uint32_t>(buf.size() - 4);
  ::nprpc::impl::g_orb->call(
    nprpc::EndPoint(this->_data().ip4, this->_data().port), buf, this->get_timeout()
  );
  auto std_reply = nprpc::impl::handle_standart_reply(buf);
  if (std_reply == 1) {
    SERVER__throw_exception(buf);
  }
  if (std_reply != 0) {
    std::cerr << "received an unusual reply for function with no output arguments\n";
    assert(false);
  }
}

void nps::Server::AVR_V_GetFlash(/*in*/const cbt1::oid_t& device_id, /*out*/std::vector<uint16_t>& data) {
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
  __ch.function_idx() = 22;
  ::flat::SERVER__M17_Direct _(buf,24);
  _._1() = device_id;
  *(uint32_t*)buf.data().data() = static_cast<uint32_t>(buf.size() - 4);
  ::nprpc::impl::g_orb->call(
    nprpc::EndPoint(this->_data().ip4, this->_data().port), buf, this->get_timeout()
  );
  auto std_reply = nprpc::impl::handle_standart_reply(buf);
  if (std_reply == 1) {
    SERVER__throw_exception(buf);
  }
  if (std_reply != -1) {
    std::cerr << "received an unusual reply for function with output arguments\n";
    assert(false);
    throw nprpc::Exception("Unknown Error");
  }
  ::flat::SERVER__M18_Direct out(buf, 8);
  {
    auto span = out._1();
    data.resize(span.size());
    memcpy(data.data(), span.data(), 2 * span.size());
  }
}

bool nps::Server::AVR_V_StoreFlash(/*in*/const cbt1::oid_t& device_id) {
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
  __ch.function_idx() = 23;
  ::flat::SERVER__M17_Direct _(buf,24);
  _._1() = device_id;
  *(uint32_t*)buf.data().data() = static_cast<uint32_t>(buf.size() - 4);
  ::nprpc::impl::g_orb->call(
    nprpc::EndPoint(this->_data().ip4, this->_data().port), buf, this->get_timeout()
  );
  auto std_reply = nprpc::impl::handle_standart_reply(buf);
  if (std_reply == 1) {
    SERVER__throw_exception(buf);
  }
  if (std_reply != -1) {
    std::cerr << "received an unusual reply for function with output arguments\n";
    assert(false);
    throw nprpc::Exception("Unknown Error");
  }
  ::flat::SERVER__M15_Direct out(buf, 8);
  bool __ret_value;
  __ret_value = out._1();
  return __ret_value;
}

bool nps::Server::Notify_DeviceActivated(/*in*/const cbt1::oid_t& device_id) {
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
  __ch.function_idx() = 24;
  ::flat::SERVER__M17_Direct _(buf,24);
  _._1() = device_id;
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
  ::flat::SERVER__M15_Direct out(buf, 8);
  bool __ret_value;
  __ret_value = out._1();
  return __ret_value;
}

bool nps::Server::Notify_DeviceDeactivated(/*in*/const cbt1::oid_t& device_id) {
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
  __ch.function_idx() = 25;
  ::flat::SERVER__M17_Direct _(buf,24);
  _._1() = device_id;
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
  ::flat::SERVER__M15_Direct out(buf, 8);
  bool __ret_value;
  __ret_value = out._1();
  return __ret_value;
}

void nps::Server::Notify_ParameterRemoved(/*in*/const cbt1::oid_t& param_id) {
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
  __ch.function_idx() = 26;
  ::flat::SERVER__M17_Direct _(buf,24);
  _._1() = param_id;
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

void nps::Server::Notify_TypeOrVariableChanged(/*in*/const cbt1::oid_t& param_id) {
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
  __ch.function_idx() = 27;
  ::flat::SERVER__M17_Direct _(buf,24);
  _._1() = param_id;
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

void nps::Server::History_AddParameter(/*in*/const cbt1::oid_t& param_id) {
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
  __ch.function_idx() = 28;
  ::flat::SERVER__M17_Direct _(buf,24);
  _._1() = param_id;
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

void nps::Server::History_RemoveParameter(/*in*/const cbt1::oid_t& param_id) {
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
  __ch.function_idx() = 29;
  ::flat::SERVER__M17_Direct _(buf,24);
  _._1() = param_id;
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

void nps::IServer_Servant::dispatch(nprpc::Buffers& bufs, nprpc::EndPoint remote_endpoint, bool from_parent, nprpc::ReferenceList& ref_list) {
  nprpc::detail::flat::CallHeader_Direct __ch(bufs(), 4 + 4);
  if (from_parent == false) {
    switch(__ch.interface_idx()) {
      case 0:
        break;
      case 1:
        IPingable_Servant::dispatch(bufs, remote_endpoint, true, ref_list);
        return;
      default:
        assert(false);
        throw "unknown interface";
    }
  }
  switch(__ch.function_idx()) {
    case 0: {
      auto& obuf = bufs.flip();
      obuf.consume(obuf.size());
      obuf.prepare(144);
      obuf.commit(16);
      ::flat::SERVER__M5_Direct oa(obuf,8);
      GetNetworkStatus(oa._1_vd());
      *((uint32_t*)obuf.data().data()) = static_cast<uint32_t>(obuf.size() - 4);
      *((uint32_t*)obuf.data().data() + 1) = ::nprpc::impl::MessageId::BlockResponse;
      break;
    }
    case 1: {
      auto& obuf = bufs.flip();
      obuf.consume(obuf.size());
      obuf.prepare(168);
      obuf.commit(40);
      ::flat::SERVER__M2_Direct oa(obuf,8);
      CreateItemManager(oa._1());
      *((uint32_t*)obuf.data().data()) = static_cast<uint32_t>(obuf.size() - 4);
      *((uint32_t*)obuf.data().data() + 1) = ::nprpc::impl::MessageId::BlockResponse;
      break;
    }
    case 2: {
      ::flat::SERVER__M5_Direct ia(bufs(), 24);
      try {
      SendRawData(ia._1());
      }
      catch(nps::NPNetCommunicationError& e) {
        auto& obuf = bufs();
        obuf.consume(obuf.size());
        obuf.prepare(16);
        obuf.commit(16);
        nps::flat::NPNetCommunicationError_Direct oa(obuf,8);
        oa.__ex_id() = 0;
  oa.code() = e.code;
        *((uint32_t*)obuf.data().data()) = static_cast<uint32_t>(obuf.size() - 4);
        *((uint32_t*)obuf.data().data() + 1) = ::nprpc::impl::MessageId::Exception;
        return;
      }
      nprpc::impl::make_simple_message(bufs(), nprpc::impl::MessageId::Success);
      break;
    }
    case 3: {
      ::flat::SERVER__M6_Direct ia(bufs(), 24);
      try {
      Write_1(ia._1(), ia._2(), ia._3(), ia._4());
      }
      catch(nps::NPNetCommunicationError& e) {
        auto& obuf = bufs();
        obuf.consume(obuf.size());
        obuf.prepare(16);
        obuf.commit(16);
        nps::flat::NPNetCommunicationError_Direct oa(obuf,8);
        oa.__ex_id() = 0;
  oa.code() = e.code;
        *((uint32_t*)obuf.data().data()) = static_cast<uint32_t>(obuf.size() - 4);
        *((uint32_t*)obuf.data().data() + 1) = ::nprpc::impl::MessageId::Exception;
        return;
      }
      nprpc::impl::make_simple_message(bufs(), nprpc::impl::MessageId::Success);
      break;
    }
    case 4: {
      ::flat::SERVER__M7_Direct ia(bufs(), 24);
      try {
      Write_q1(ia._1(), ia._2(), ia._3(), ia._4(), ia._5());
      }
      catch(nps::NPNetCommunicationError& e) {
        auto& obuf = bufs();
        obuf.consume(obuf.size());
        obuf.prepare(16);
        obuf.commit(16);
        nps::flat::NPNetCommunicationError_Direct oa(obuf,8);
        oa.__ex_id() = 0;
  oa.code() = e.code;
        *((uint32_t*)obuf.data().data()) = static_cast<uint32_t>(obuf.size() - 4);
        *((uint32_t*)obuf.data().data() + 1) = ::nprpc::impl::MessageId::Exception;
        return;
      }
      nprpc::impl::make_simple_message(bufs(), nprpc::impl::MessageId::Success);
      break;
    }
    case 5: {
      ::flat::SERVER__M8_Direct ia(bufs(), 24);
      try {
      Write_8(ia._1(), ia._2(), ia._3());
      }
      catch(nps::NPNetCommunicationError& e) {
        auto& obuf = bufs();
        obuf.consume(obuf.size());
        obuf.prepare(16);
        obuf.commit(16);
        nps::flat::NPNetCommunicationError_Direct oa(obuf,8);
        oa.__ex_id() = 0;
  oa.code() = e.code;
        *((uint32_t*)obuf.data().data()) = static_cast<uint32_t>(obuf.size() - 4);
        *((uint32_t*)obuf.data().data() + 1) = ::nprpc::impl::MessageId::Exception;
        return;
      }
      nprpc::impl::make_simple_message(bufs(), nprpc::impl::MessageId::Success);
      break;
    }
    case 6: {
      ::flat::SERVER__M6_Direct ia(bufs(), 24);
      try {
      Write_q8(ia._1(), ia._2(), ia._3(), ia._4());
      }
      catch(nps::NPNetCommunicationError& e) {
        auto& obuf = bufs();
        obuf.consume(obuf.size());
        obuf.prepare(16);
        obuf.commit(16);
        nps::flat::NPNetCommunicationError_Direct oa(obuf,8);
        oa.__ex_id() = 0;
  oa.code() = e.code;
        *((uint32_t*)obuf.data().data()) = static_cast<uint32_t>(obuf.size() - 4);
        *((uint32_t*)obuf.data().data() + 1) = ::nprpc::impl::MessageId::Exception;
        return;
      }
      nprpc::impl::make_simple_message(bufs(), nprpc::impl::MessageId::Success);
      break;
    }
    case 7: {
      ::flat::SERVER__M9_Direct ia(bufs(), 24);
      try {
      Write_16(ia._1(), ia._2(), ia._3());
      }
      catch(nps::NPNetCommunicationError& e) {
        auto& obuf = bufs();
        obuf.consume(obuf.size());
        obuf.prepare(16);
        obuf.commit(16);
        nps::flat::NPNetCommunicationError_Direct oa(obuf,8);
        oa.__ex_id() = 0;
  oa.code() = e.code;
        *((uint32_t*)obuf.data().data()) = static_cast<uint32_t>(obuf.size() - 4);
        *((uint32_t*)obuf.data().data() + 1) = ::nprpc::impl::MessageId::Exception;
        return;
      }
      nprpc::impl::make_simple_message(bufs(), nprpc::impl::MessageId::Success);
      break;
    }
    case 8: {
      ::flat::SERVER__M10_Direct ia(bufs(), 24);
      try {
      Write_q16(ia._1(), ia._2(), ia._3(), ia._4());
      }
      catch(nps::NPNetCommunicationError& e) {
        auto& obuf = bufs();
        obuf.consume(obuf.size());
        obuf.prepare(16);
        obuf.commit(16);
        nps::flat::NPNetCommunicationError_Direct oa(obuf,8);
        oa.__ex_id() = 0;
  oa.code() = e.code;
        *((uint32_t*)obuf.data().data()) = static_cast<uint32_t>(obuf.size() - 4);
        *((uint32_t*)obuf.data().data() + 1) = ::nprpc::impl::MessageId::Exception;
        return;
      }
      nprpc::impl::make_simple_message(bufs(), nprpc::impl::MessageId::Success);
      break;
    }
    case 9: {
      ::flat::SERVER__M11_Direct ia(bufs(), 24);
      try {
      Write_32(ia._1(), ia._2(), ia._3());
      }
      catch(nps::NPNetCommunicationError& e) {
        auto& obuf = bufs();
        obuf.consume(obuf.size());
        obuf.prepare(16);
        obuf.commit(16);
        nps::flat::NPNetCommunicationError_Direct oa(obuf,8);
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
      ::flat::SERVER__M12_Direct ia(bufs(), 24);
      try {
      Write_q32(ia._1(), ia._2(), ia._3(), ia._4());
      }
      catch(nps::NPNetCommunicationError& e) {
        auto& obuf = bufs();
        obuf.consume(obuf.size());
        obuf.prepare(16);
        obuf.commit(16);
        nps::flat::NPNetCommunicationError_Direct oa(obuf,8);
        oa.__ex_id() = 0;
  oa.code() = e.code;
        *((uint32_t*)obuf.data().data()) = static_cast<uint32_t>(obuf.size() - 4);
        *((uint32_t*)obuf.data().data() + 1) = ::nprpc::impl::MessageId::Exception;
        return;
      }
      nprpc::impl::make_simple_message(bufs(), nprpc::impl::MessageId::Success);
      break;
    }
    case 11: {
      ::flat::SERVER__M13_Direct ia(bufs(), 24);
      try {
      WriteBlock(ia._1(), ia._2(), ia._3());
      }
      catch(nps::NPNetCommunicationError& e) {
        auto& obuf = bufs();
        obuf.consume(obuf.size());
        obuf.prepare(16);
        obuf.commit(16);
        nps::flat::NPNetCommunicationError_Direct oa(obuf,8);
        oa.__ex_id() = 0;
  oa.code() = e.code;
        *((uint32_t*)obuf.data().data()) = static_cast<uint32_t>(obuf.size() - 4);
        *((uint32_t*)obuf.data().data() + 1) = ::nprpc::impl::MessageId::Exception;
        return;
      }
      nprpc::impl::make_simple_message(bufs(), nprpc::impl::MessageId::Success);
      break;
    }
    case 12: {
      uint8_t _out_1;
      ::flat::SERVER__M14_Direct ia(bufs(), 24);
      try {
      ReadByte(ia._1(), ia._2(), _out_1);
      }
      catch(nps::NPNetCommunicationError& e) {
        auto& obuf = bufs();
        obuf.consume(obuf.size());
        obuf.prepare(16);
        obuf.commit(16);
        nps::flat::NPNetCommunicationError_Direct oa(obuf,8);
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
      ::flat::SERVER__M15_Direct oa(obuf,8);
  oa._1() = _out_1;
      *((uint32_t*)obuf.data().data()) = static_cast<uint32_t>(obuf.size() - 4);
      *((uint32_t*)obuf.data().data() + 1) = ::nprpc::impl::MessageId::BlockResponse;
      break;
    }
    case 13: {
      ::flat::SERVER__M8_Direct ia(bufs(), 24);
      auto& obuf = bufs.flip();
      obuf.consume(obuf.size());
      obuf.prepare(144);
      obuf.commit(16);
      ::flat::SERVER__M5_Direct oa(obuf,8);
      try {
      ReadBlock(ia._1(), ia._2(), ia._3(), oa._1_vd());
      }
      catch(nps::NPNetCommunicationError& e) {
        auto& obuf = bufs();
        obuf.consume(obuf.size());
        obuf.prepare(16);
        obuf.commit(16);
        nps::flat::NPNetCommunicationError_Direct oa(obuf,8);
        oa.__ex_id() = 0;
  oa.code() = e.code;
        *((uint32_t*)obuf.data().data()) = static_cast<uint32_t>(obuf.size() - 4);
        *((uint32_t*)obuf.data().data() + 1) = ::nprpc::impl::MessageId::Exception;
        return;
      }
      *((uint32_t*)obuf.data().data()) = static_cast<uint32_t>(obuf.size() - 4);
      *((uint32_t*)obuf.data().data() + 1) = ::nprpc::impl::MessageId::BlockResponse;
      break;
    }
    case 14: {
      ::flat::SERVER__M14_Direct ia(bufs(), 24);
bool __ret_val;
      try {
      __ret_val = AVR_StopAlgorithm(ia._1(), ia._2());
      }
      catch(nps::NPNetCommunicationError& e) {
        auto& obuf = bufs();
        obuf.consume(obuf.size());
        obuf.prepare(16);
        obuf.commit(16);
        nps::flat::NPNetCommunicationError_Direct oa(obuf,8);
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
      ::flat::SERVER__M15_Direct oa(obuf,8);
  oa._1() = __ret_val;
      *((uint32_t*)obuf.data().data()) = static_cast<uint32_t>(obuf.size() - 4);
      *((uint32_t*)obuf.data().data() + 1) = ::nprpc::impl::MessageId::BlockResponse;
      break;
    }
    case 15: {
      ::flat::SERVER__M15_Direct ia(bufs(), 24);
      try {
      AVR_ReinitIO(ia._1());
      }
      catch(nps::NPNetCommunicationError& e) {
        auto& obuf = bufs();
        obuf.consume(obuf.size());
        obuf.prepare(16);
        obuf.commit(16);
        nps::flat::NPNetCommunicationError_Direct oa(obuf,8);
        oa.__ex_id() = 0;
  oa.code() = e.code;
        *((uint32_t*)obuf.data().data()) = static_cast<uint32_t>(obuf.size() - 4);
        *((uint32_t*)obuf.data().data() + 1) = ::nprpc::impl::MessageId::Exception;
        return;
      }
      nprpc::impl::make_simple_message(bufs(), nprpc::impl::MessageId::Success);
      break;
    }
    case 16: {
      ::flat::SERVER__M13_Direct ia(bufs(), 24);
      try {
      AVR_SendRemoteData(ia._1(), ia._2(), ia._3());
      }
      catch(nps::NPNetCommunicationError& e) {
        auto& obuf = bufs();
        obuf.consume(obuf.size());
        obuf.prepare(16);
        obuf.commit(16);
        nps::flat::NPNetCommunicationError_Direct oa(obuf,8);
        oa.__ex_id() = 0;
  oa.code() = e.code;
        *((uint32_t*)obuf.data().data()) = static_cast<uint32_t>(obuf.size() - 4);
        *((uint32_t*)obuf.data().data() + 1) = ::nprpc::impl::MessageId::Exception;
        return;
      }
      nprpc::impl::make_simple_message(bufs(), nprpc::impl::MessageId::Success);
      break;
    }
    case 17: {
      ::flat::SERVER__M16_Direct ia(bufs(), 24);
      try {
      AVR_SendPage(ia._1(), ia._2(), ia._3());
      }
      catch(nps::NPNetCommunicationError& e) {
        auto& obuf = bufs();
        obuf.consume(obuf.size());
        obuf.prepare(16);
        obuf.commit(16);
        nps::flat::NPNetCommunicationError_Direct oa(obuf,8);
        oa.__ex_id() = 0;
  oa.code() = e.code;
        *((uint32_t*)obuf.data().data()) = static_cast<uint32_t>(obuf.size() - 4);
        *((uint32_t*)obuf.data().data() + 1) = ::nprpc::impl::MessageId::Exception;
        return;
      }
      nprpc::impl::make_simple_message(bufs(), nprpc::impl::MessageId::Success);
      break;
    }
    case 18: {
      ::flat::SERVER__M14_Direct ia(bufs(), 24);
      try {
      AVR_RemoveAlgorithm(ia._1(), ia._2());
      }
      catch(nps::NPNetCommunicationError& e) {
        auto& obuf = bufs();
        obuf.consume(obuf.size());
        obuf.prepare(16);
        obuf.commit(16);
        nps::flat::NPNetCommunicationError_Direct oa(obuf,8);
        oa.__ex_id() = 0;
  oa.code() = e.code;
        *((uint32_t*)obuf.data().data()) = static_cast<uint32_t>(obuf.size() - 4);
        *((uint32_t*)obuf.data().data() + 1) = ::nprpc::impl::MessageId::Exception;
        return;
      }
      nprpc::impl::make_simple_message(bufs(), nprpc::impl::MessageId::Success);
      break;
    }
    case 19: {
      ::flat::SERVER__M9_Direct ia(bufs(), 24);
      try {
      AVR_ReplaceAlgorithm(ia._1(), ia._2(), ia._3());
      }
      catch(nps::NPNetCommunicationError& e) {
        auto& obuf = bufs();
        obuf.consume(obuf.size());
        obuf.prepare(16);
        obuf.commit(16);
        nps::flat::NPNetCommunicationError_Direct oa(obuf,8);
        oa.__ex_id() = 0;
  oa.code() = e.code;
        *((uint32_t*)obuf.data().data()) = static_cast<uint32_t>(obuf.size() - 4);
        *((uint32_t*)obuf.data().data() + 1) = ::nprpc::impl::MessageId::Exception;
        return;
      }
      nprpc::impl::make_simple_message(bufs(), nprpc::impl::MessageId::Success);
      break;
    }
    case 20: {
      ::flat::SERVER__M13_Direct ia(bufs(), 24);
      try {
      AVR_WriteEeprom(ia._1(), ia._2(), ia._3());
      }
      catch(nps::NPNetCommunicationError& e) {
        auto& obuf = bufs();
        obuf.consume(obuf.size());
        obuf.prepare(16);
        obuf.commit(16);
        nps::flat::NPNetCommunicationError_Direct oa(obuf,8);
        oa.__ex_id() = 0;
  oa.code() = e.code;
        *((uint32_t*)obuf.data().data()) = static_cast<uint32_t>(obuf.size() - 4);
        *((uint32_t*)obuf.data().data() + 1) = ::nprpc::impl::MessageId::Exception;
        return;
      }
      nprpc::impl::make_simple_message(bufs(), nprpc::impl::MessageId::Success);
      break;
    }
    case 21: {
      ::flat::SERVER__M16_Direct ia(bufs(), 24);
      try {
      AVR_WriteTwiTable(ia._1(), ia._2(), ia._3());
      }
      catch(nps::NPNetCommunicationError& e) {
        auto& obuf = bufs();
        obuf.consume(obuf.size());
        obuf.prepare(16);
        obuf.commit(16);
        nps::flat::NPNetCommunicationError_Direct oa(obuf,8);
        oa.__ex_id() = 0;
  oa.code() = e.code;
        *((uint32_t*)obuf.data().data()) = static_cast<uint32_t>(obuf.size() - 4);
        *((uint32_t*)obuf.data().data() + 1) = ::nprpc::impl::MessageId::Exception;
        return;
      }
      nprpc::impl::make_simple_message(bufs(), nprpc::impl::MessageId::Success);
      break;
    }
    case 22: {
      ::flat::SERVER__M17_Direct ia(bufs(), 24);
      auto& obuf = bufs.flip();
      obuf.consume(obuf.size());
      obuf.prepare(144);
      obuf.commit(16);
      ::flat::SERVER__M18_Direct oa(obuf,8);
      try {
      AVR_V_GetFlash(ia._1(), oa._1_vd());
      }
      catch(nps::NPNetCommunicationError& e) {
        auto& obuf = bufs();
        obuf.consume(obuf.size());
        obuf.prepare(16);
        obuf.commit(16);
        nps::flat::NPNetCommunicationError_Direct oa(obuf,8);
        oa.__ex_id() = 0;
  oa.code() = e.code;
        *((uint32_t*)obuf.data().data()) = static_cast<uint32_t>(obuf.size() - 4);
        *((uint32_t*)obuf.data().data() + 1) = ::nprpc::impl::MessageId::Exception;
        return;
      }
      *((uint32_t*)obuf.data().data()) = static_cast<uint32_t>(obuf.size() - 4);
      *((uint32_t*)obuf.data().data() + 1) = ::nprpc::impl::MessageId::BlockResponse;
      break;
    }
    case 23: {
      ::flat::SERVER__M17_Direct ia(bufs(), 24);
bool __ret_val;
      try {
      __ret_val = AVR_V_StoreFlash(ia._1());
      }
      catch(nps::NPNetCommunicationError& e) {
        auto& obuf = bufs();
        obuf.consume(obuf.size());
        obuf.prepare(16);
        obuf.commit(16);
        nps::flat::NPNetCommunicationError_Direct oa(obuf,8);
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
      ::flat::SERVER__M15_Direct oa(obuf,8);
  oa._1() = __ret_val;
      *((uint32_t*)obuf.data().data()) = static_cast<uint32_t>(obuf.size() - 4);
      *((uint32_t*)obuf.data().data() + 1) = ::nprpc::impl::MessageId::BlockResponse;
      break;
    }
    case 24: {
      ::flat::SERVER__M17_Direct ia(bufs(), 24);
bool __ret_val;
      __ret_val = Notify_DeviceActivated(ia._1());
      auto& obuf = bufs();
      obuf.consume(obuf.size());
      obuf.prepare(9);
      obuf.commit(9);
      ::flat::SERVER__M15_Direct oa(obuf,8);
  oa._1() = __ret_val;
      *((uint32_t*)obuf.data().data()) = static_cast<uint32_t>(obuf.size() - 4);
      *((uint32_t*)obuf.data().data() + 1) = ::nprpc::impl::MessageId::BlockResponse;
      break;
    }
    case 25: {
      ::flat::SERVER__M17_Direct ia(bufs(), 24);
bool __ret_val;
      __ret_val = Notify_DeviceDeactivated(ia._1());
      auto& obuf = bufs();
      obuf.consume(obuf.size());
      obuf.prepare(9);
      obuf.commit(9);
      ::flat::SERVER__M15_Direct oa(obuf,8);
  oa._1() = __ret_val;
      *((uint32_t*)obuf.data().data()) = static_cast<uint32_t>(obuf.size() - 4);
      *((uint32_t*)obuf.data().data() + 1) = ::nprpc::impl::MessageId::BlockResponse;
      break;
    }
    case 26: {
      ::flat::SERVER__M17_Direct ia(bufs(), 24);
      Notify_ParameterRemoved(ia._1());
      nprpc::impl::make_simple_message(bufs(), nprpc::impl::MessageId::Success);
      break;
    }
    case 27: {
      ::flat::SERVER__M17_Direct ia(bufs(), 24);
      Notify_TypeOrVariableChanged(ia._1());
      nprpc::impl::make_simple_message(bufs(), nprpc::impl::MessageId::Success);
      break;
    }
    case 28: {
      ::flat::SERVER__M17_Direct ia(bufs(), 24);
      History_AddParameter(ia._1());
      nprpc::impl::make_simple_message(bufs(), nprpc::impl::MessageId::Success);
      break;
    }
    case 29: {
      ::flat::SERVER__M17_Direct ia(bufs(), 24);
      History_RemoveParameter(ia._1());
      nprpc::impl::make_simple_message(bufs(), nprpc::impl::MessageId::Success);
      break;
    }
    default:
      nprpc::impl::make_simple_message(bufs(), nprpc::impl::MessageId::Error_UnknownFunctionIdx);
  }
}

} // namespace nps


void SERVER__throw_exception(boost::beast::flat_buffer& buf) { 
  switch(*((uint32_t*)buf.data().data() + 2)) {
  case 0:
  {
    nps::flat::NPNetCommunicationError_Direct ex_flat(buf, 4 + 4);
    nps::NPNetCommunicationError ex;
  ex.code = ex_flat.code();
    throw ex;
  }
  default:
    throw std::runtime_error("unknown rpc exception");
  }
}
