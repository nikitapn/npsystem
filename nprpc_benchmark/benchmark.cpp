#include "benchmark.hpp"
#include <nprpc/impl/nprpc_impl.hpp>

void benchmark_throw_exception(::nprpc::flat_buffer& buf);

namespace {
struct benchmark_M1 {
  benchmark::flat::Data _1;
};

class benchmark_M1_Direct {
  ::nprpc::flat_buffer& buffer_;
  const std::uint32_t offset_;

  auto& base() noexcept { return *reinterpret_cast<benchmark_M1*>(reinterpret_cast<std::byte*>(buffer_.data().data()) + offset_); }
  auto const& base() const noexcept { return *reinterpret_cast<const benchmark_M1*>(reinterpret_cast<const std::byte*>(buffer_.data().data()) + offset_); }
public:
  uint32_t offset() const noexcept { return offset_; }
  void* __data() noexcept { return (void*)&base(); }
  benchmark_M1_Direct(::nprpc::flat_buffer& buffer, std::uint32_t offset)
    : buffer_(buffer)
    , offset_(offset)
  {
  }
  auto _1() noexcept { return benchmark::flat::Data_Direct(buffer_, offset_ + offsetof(benchmark_M1, _1)); }
};

struct benchmark_M2 {
  ::nprpc::flat::String _1;
};

class benchmark_M2_Direct {
  ::nprpc::flat_buffer& buffer_;
  const std::uint32_t offset_;

  auto& base() noexcept { return *reinterpret_cast<benchmark_M2*>(reinterpret_cast<std::byte*>(buffer_.data().data()) + offset_); }
  auto const& base() const noexcept { return *reinterpret_cast<const benchmark_M2*>(reinterpret_cast<const std::byte*>(buffer_.data().data()) + offset_); }
public:
  uint32_t offset() const noexcept { return offset_; }
  void* __data() noexcept { return (void*)&base(); }
  benchmark_M2_Direct(::nprpc::flat_buffer& buffer, std::uint32_t offset)
    : buffer_(buffer)
    , offset_(offset)
  {
  }
  void _1(const char* str) { new (&base()._1) ::nprpc::flat::String(buffer_, str); }
  void _1(const std::string& str) { new (&base()._1) ::nprpc::flat::String(buffer_, str); }
  auto _1() noexcept { return (::nprpc::flat::Span<char>)base()._1; }
  auto _1() const noexcept { return (::nprpc::flat::Span<const char>)base()._1; }
  auto _1_d() noexcept {     return ::nprpc::flat::String_Direct1(buffer_, offset_ + offsetof(benchmark_M2, _1));  }
};


} // 

namespace benchmark { 
Data benchmark::Benchmark::rpc() {
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
  benchmark_M1_Direct out(buf, sizeof(::nprpc::impl::Header));
    Data __ret_value;
    __ret_value.f1 = (std::string_view)out._1().f1();
    __ret_value.f2 = (std::string_view)out._1().f2();
    __ret_value.f3 = out._1().f3();
    __ret_value.f4 = out._1().f4();
    __ret_value.f5 = out._1().f5();
    {
      auto span = out._1().f6();
      __ret_value.f6.resize(span.size());
      auto it3 = std::begin(__ret_value.f6);
      for (auto e : span) {
        (*it3).f1 = (std::string_view)e.f1();
        (*it3).f2 = e.f2();
        ++it3;
      }
    }
  return __ret_value;
}

std::string benchmark::Benchmark::json() {
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
  benchmark_M2_Direct out(buf, sizeof(::nprpc::impl::Header));
    std::string __ret_value;
    __ret_value = (std::string_view)out._1();
  return __ret_value;
}

void benchmark::IBenchmark_Servant::dispatch(nprpc::Buffers& bufs, nprpc::EndPoint remote_endpoint, bool from_parent, nprpc::ReferenceList& ref_list) {
  nprpc::impl::flat::CallHeader_Direct __ch(bufs(), sizeof(::nprpc::impl::Header));
  switch(__ch.function_idx()) {
    case 0: {
      auto& obuf = bufs.flip();
      obuf.consume(obuf.size());
      obuf.prepare(176);
      obuf.commit(48);
      benchmark_M1_Direct oa(obuf,16);
      Data __ret_val;
      __ret_val = rpc();
      oa._1().f1(__ret_val.f1);
      oa._1().f2(__ret_val.f2);
      oa._1().f3() = __ret_val.f3;
      oa._1().f4() = __ret_val.f4;
      oa._1().f5() = __ret_val.f5;
      oa._1().f6(static_cast<uint32_t>(__ret_val.f6.size()));
      {
        auto span = oa._1().f6();
        auto it = __ret_val.f6.begin();
        for (auto e : span) {
          auto __ptr = ::nprpc::make_wrapper1(*it);
            e.f1(__ptr->f1);
            e.f2() = __ptr->f2;
          ++it;
        }
      }
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
      benchmark_M2_Direct oa(obuf,16);
      std::string __ret_val;
      __ret_val = json();
      oa._1(__ret_val);
      static_cast<::nprpc::impl::Header*>(obuf.data().data())->size = static_cast<uint32_t>(obuf.size() - 4);
      static_cast<::nprpc::impl::Header*>(obuf.data().data())->msg_id = ::nprpc::impl::MessageId::BlockResponse;
      static_cast<::nprpc::impl::Header*>(obuf.data().data())->msg_type = ::nprpc::impl::MessageType::Answer;
      break;
    }
    default:
      nprpc::impl::make_simple_answer(bufs(), nprpc::impl::MessageId::Error_UnknownFunctionIdx);
  }
}

} // namespace benchmark

