#include "npwebserver.hpp"
#include <nprpc/impl/nprpc_impl.hpp>

void npwebserver_throw_exception(::nprpc::flat_buffer& buf);

namespace {
struct npwebserver_M1 {
  ::nprpc::flat::Vector<npwebserver::flat::WebCategory> _1;
};

class npwebserver_M1_Direct {
  ::nprpc::flat_buffer& buffer_;
  const std::uint32_t offset_;

  auto& base() noexcept { return *reinterpret_cast<npwebserver_M1*>(reinterpret_cast<std::byte*>(buffer_.data().data()) + offset_); }
  auto const& base() const noexcept { return *reinterpret_cast<const npwebserver_M1*>(reinterpret_cast<const std::byte*>(buffer_.data().data()) + offset_); }
public:
  uint32_t offset() const noexcept { return offset_; }
  void* __data() noexcept { return (void*)&base(); }
  npwebserver_M1_Direct(::nprpc::flat_buffer& buffer, std::uint32_t offset)
    : buffer_(buffer)
    , offset_(offset)
  {
  }
  void _1(std::uint32_t elements_size) { new (&base()._1) ::nprpc::flat::Vector<npwebserver::flat::WebCategory>(buffer_, elements_size); }
  auto _1_d() noexcept { return ::nprpc::flat::Vector_Direct2<npwebserver::flat::WebCategory,npwebserver::flat::WebCategory_Direct>(buffer_, offset_ + offsetof(npwebserver_M1, _1)); }
  auto _1() noexcept { return ::nprpc::flat::Span_ref<npwebserver::flat::WebCategory, npwebserver::flat::WebCategory_Direct>(buffer_, base()._1.range(buffer_.data().data())); }
};


} // 

namespace npwebserver { 
void npwebserver::WebServer::get_web_categories(/*out*/std::vector<npwebserver::WebCategory>& cats) {
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
  npwebserver_M1_Direct out(buf, sizeof(::nprpc::impl::Header));
    {
      auto span = out._1();
      cats.resize(span.size());
      auto it3 = std::begin(cats);
      for (auto e : span) {
        (*it3).name = (std::string_view)e.name();
        {
          auto span = e.items();
          (*it3).items.resize(span.size());
          auto it5 = std::begin((*it3).items);
          for (auto e : span) {
            (*it5).name = (std::string_view)e.name();
            (*it5).description = (std::string_view)e.description();
            (*it5).path = (std::string_view)e.path();
            (*it5).dev_addr = e.dev_addr();
            (*it5).mem_addr = e.mem_addr();
            (*it5).type = e.type();
            ++it5;
          }
        }
        ++it3;
      }
    }
}

void npwebserver::IWebServer_Servant::dispatch(nprpc::Buffers& bufs, nprpc::EndPoint remote_endpoint, bool from_parent, nprpc::ReferenceList& ref_list) {
  nprpc::impl::flat::CallHeader_Direct __ch(bufs(), sizeof(::nprpc::impl::Header));
  switch(__ch.function_idx()) {
    case 0: {
      auto& obuf = bufs.flip();
      obuf.consume(obuf.size());
      obuf.prepare(152);
      obuf.commit(24);
      npwebserver_M1_Direct oa(obuf,16);
      get_web_categories(oa._1_d());
      static_cast<::nprpc::impl::Header*>(obuf.data().data())->size = static_cast<uint32_t>(obuf.size() - 4);
      static_cast<::nprpc::impl::Header*>(obuf.data().data())->msg_id = ::nprpc::impl::MessageId::BlockResponse;
      static_cast<::nprpc::impl::Header*>(obuf.data().data())->msg_type = ::nprpc::impl::MessageType::Answer;
      break;
    }
    default:
      nprpc::impl::make_simple_answer(bufs(), nprpc::impl::MessageId::Error_UnknownFunctionIdx);
  }
}

} // namespace npwebserver

