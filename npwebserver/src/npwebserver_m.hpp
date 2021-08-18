#ifndef NPWEBSERVER_M_
#define NPWEBSERVER_M_

struct npwebserver_M1 {
  std::vector<npwebserver::WebCategory> _1;
};

namespace flat {
struct npwebserver_M1 {
  ::flat::Vector<npwebserver::flat::WebCategory> _1;
};

class npwebserver_M1_Direct {
  boost::beast::flat_buffer& buffer_;
  const size_t offset_;

  auto& base() noexcept { return *reinterpret_cast<npwebserver_M1*>(reinterpret_cast<std::byte*>(buffer_.data().data()) + offset_); }
  auto const& base() const noexcept { return *reinterpret_cast<const npwebserver_M1*>(reinterpret_cast<const std::byte*>(buffer_.data().data()) + offset_); }
public:
  void* __data() noexcept { return (void*)&base(); }
  npwebserver_M1_Direct(boost::beast::flat_buffer& buffer, size_t offset)
    : buffer_(buffer)
    , offset_(offset)
  {
  }
  void _1(size_t elements_size) { new (&base()._1) ::flat::Vector<npwebserver::flat::WebCategory>(buffer_, elements_size); }
  auto _1_vd() noexcept { return ::flat::Vector_Direct2<npwebserver::flat::WebCategory, npwebserver::flat::WebCategory_Direct>(buffer_, offset_ + offsetof(npwebserver_M1, _1)); }
  auto _1() noexcept { return ::flat::Span_ref<npwebserver::flat::WebCategory, npwebserver::flat::WebCategory_Direct>(buffer_, base()._1.range(buffer_.data().data())); }
};
} // namespace flat


#endif