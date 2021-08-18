#ifndef NPWEBSERVER_
#define NPWEBSERVER_

#include <nprpc/flat.hpp>
#include <nprpc/nprpc.hpp>

namespace npwebserver { 
struct WebItem {
  std::string name;
  std::string description;
  std::string path;
  uint8_t dev_addr;
  uint16_t mem_addr;
  uint32_t type;
};

namespace flat {
struct WebItem {
  ::flat::String name;
  ::flat::String description;
  ::flat::String path;
  uint8_t dev_addr;
  uint16_t mem_addr;
  uint32_t type;
};

class WebItem_Direct {
  boost::beast::flat_buffer& buffer_;
  const size_t offset_;

  auto& base() noexcept { return *reinterpret_cast<WebItem*>(reinterpret_cast<std::byte*>(buffer_.data().data()) + offset_); }
  auto const& base() const noexcept { return *reinterpret_cast<const WebItem*>(reinterpret_cast<const std::byte*>(buffer_.data().data()) + offset_); }
public:
  void* __data() noexcept { return (void*)&base(); }
  WebItem_Direct(boost::beast::flat_buffer& buffer, size_t offset)
    : buffer_(buffer)
    , offset_(offset)
  {
  }
  void name(const char* str) { new (&base().name) ::flat::String(buffer_, str); }
  void name(const std::string& str) { new (&base().name) ::flat::String(buffer_, str); }
  auto name() noexcept { return (::flat::Span<char>)base().name; }
  auto name() const noexcept { return (::flat::Span<const char>)base().name; }
  auto name_vd() noexcept {     return ::flat::String_Direct1(buffer_, offset_ + offsetof(WebItem, name));  }
  void description(const char* str) { new (&base().description) ::flat::String(buffer_, str); }
  void description(const std::string& str) { new (&base().description) ::flat::String(buffer_, str); }
  auto description() noexcept { return (::flat::Span<char>)base().description; }
  auto description() const noexcept { return (::flat::Span<const char>)base().description; }
  auto description_vd() noexcept {     return ::flat::String_Direct1(buffer_, offset_ + offsetof(WebItem, description));  }
  void path(const char* str) { new (&base().path) ::flat::String(buffer_, str); }
  void path(const std::string& str) { new (&base().path) ::flat::String(buffer_, str); }
  auto path() noexcept { return (::flat::Span<char>)base().path; }
  auto path() const noexcept { return (::flat::Span<const char>)base().path; }
  auto path_vd() noexcept {     return ::flat::String_Direct1(buffer_, offset_ + offsetof(WebItem, path));  }
  const uint8_t& dev_addr() const noexcept { return base().dev_addr;}
  uint8_t& dev_addr() noexcept { return base().dev_addr;}
  const uint16_t& mem_addr() const noexcept { return base().mem_addr;}
  uint16_t& mem_addr() noexcept { return base().mem_addr;}
  const uint32_t& type() const noexcept { return base().type;}
  uint32_t& type() noexcept { return base().type;}
};
} // namespace flat

struct WebCategory {
  std::string name;
  std::vector<WebItem> items;
};

namespace flat {
struct WebCategory {
  ::flat::String name;
  ::flat::Vector<npwebserver::flat::WebItem> items;
};

class WebCategory_Direct {
  boost::beast::flat_buffer& buffer_;
  const size_t offset_;

  auto& base() noexcept { return *reinterpret_cast<WebCategory*>(reinterpret_cast<std::byte*>(buffer_.data().data()) + offset_); }
  auto const& base() const noexcept { return *reinterpret_cast<const WebCategory*>(reinterpret_cast<const std::byte*>(buffer_.data().data()) + offset_); }
public:
  void* __data() noexcept { return (void*)&base(); }
  WebCategory_Direct(boost::beast::flat_buffer& buffer, size_t offset)
    : buffer_(buffer)
    , offset_(offset)
  {
  }
  void name(const char* str) { new (&base().name) ::flat::String(buffer_, str); }
  void name(const std::string& str) { new (&base().name) ::flat::String(buffer_, str); }
  auto name() noexcept { return (::flat::Span<char>)base().name; }
  auto name() const noexcept { return (::flat::Span<const char>)base().name; }
  auto name_vd() noexcept {     return ::flat::String_Direct1(buffer_, offset_ + offsetof(WebCategory, name));  }
  void items(size_t elements_size) { new (&base().items) ::flat::Vector<npwebserver::flat::WebItem>(buffer_, elements_size); }
  auto items_vd() noexcept { return ::flat::Vector_Direct2<npwebserver::flat::WebItem, npwebserver::flat::WebItem_Direct>(buffer_, offset_ + offsetof(WebCategory, items)); }
  auto items() noexcept { return ::flat::Span_ref<npwebserver::flat::WebItem, npwebserver::flat::WebItem_Direct>(buffer_, base().items.range(buffer_.data().data())); }
};
} // namespace flat

class IWebServer_Servant
  : public virtual nprpc::ObjectServant
{
public:
  static std::string_view _get_class() noexcept { return "npwebserver/npwebserver.WebServer"; }
  std::string_view get_class() const noexcept override { return IWebServer_Servant::_get_class(); }
  void dispatch(nprpc::Buffers& bufs, nprpc::EndPoint remote_endpoint, bool from_parent, nprpc::ReferenceList& ref_list) override;
  virtual void get_web_categories (/*out*/::flat::Vector_Direct2<npwebserver::flat::WebCategory, npwebserver::flat::WebCategory_Direct> cats) = 0;
};

class WebServer
  : public virtual nprpc::Object
{
  const uint8_t interface_idx_;
public:
  using servant_t = IWebServer_Servant;

  WebServer(uint8_t interface_idx) : interface_idx_(interface_idx) {}
  void get_web_categories (/*out*/std::vector<npwebserver::WebCategory>& cats);
};

} // namespace npwebserver

namespace npwebserver::helper {
template<::nprpc::IterableCollection T>
void assign_get_web_categories_cats(/*out*/::flat::Vector_Direct2<npwebserver::flat::WebCategory, npwebserver::flat::WebCategory_Direct>& dest, const T & src) {
  dest.length(src.size());
  auto span = dest();
  auto it = src.begin();
  for (auto e : span) {
    auto __ptr = ::nprpc::make_wrapper1(*it);
    e.name(__ptr->name);
    e.items(__ptr->items.size());
  auto span =   e.items();
  auto it = __ptr->items.begin();
  for (auto e : span) {
    auto __ptr = ::nprpc::make_wrapper1(*it);
    e.name(__ptr->name);
    e.description(__ptr->description);
    e.path(__ptr->path);
    e.dev_addr() = __ptr->dev_addr;
    e.mem_addr() = __ptr->mem_addr;
    e.type() = __ptr->type;
    ++it;
  }
    ++it;
  }
}
} // namespace npwebserver::helper

#endif