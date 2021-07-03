#ifndef SERVER_M_
#define SERVER_M_

struct SERVER__M1 {
  std::vector<nps::server_value> _1;
};

namespace flat {
struct SERVER__M1 {
  ::flat::Vector<nps::flat::server_value> _1;
};

class SERVER__M1_Direct {
  boost::beast::flat_buffer& buffer_;
  const size_t offset_;

  auto& base() noexcept { return *reinterpret_cast<SERVER__M1*>(reinterpret_cast<std::byte*>(buffer_.data().data()) + offset_); }
  auto const& base() const noexcept { return *reinterpret_cast<const SERVER__M1*>(reinterpret_cast<const std::byte*>(buffer_.data().data()) + offset_); }
public:
  void* __data() noexcept { return (void*)&base(); }
  SERVER__M1_Direct(boost::beast::flat_buffer& buffer, size_t offset)
    : buffer_(buffer)
    , offset_(offset)
  {
  }
  void _1(size_t elements_size) { new (&base()._1) ::flat::Vector<nps::flat::server_value>(buffer_, elements_size); }
  auto _1_vd() noexcept { return ::flat::Vector_Direct2<nps::flat::server_value, nps::flat::server_value_Direct>(buffer_, offset_ + offsetof(SERVER__M1, _1)); }
  auto _1() noexcept { return ::flat::Span_ref<nps::flat::server_value, nps::flat::server_value_Direct>(buffer_, base()._1.range(buffer_.data().data())); }
};
} // namespace flat

struct SERVER__M2 {
  nprpc::ObjectId _1;
};

namespace flat {
struct SERVER__M2 {
  nprpc::detail::flat::ObjectId _1;
};

class SERVER__M2_Direct {
  boost::beast::flat_buffer& buffer_;
  const size_t offset_;

  auto& base() noexcept { return *reinterpret_cast<SERVER__M2*>(reinterpret_cast<std::byte*>(buffer_.data().data()) + offset_); }
  auto const& base() const noexcept { return *reinterpret_cast<const SERVER__M2*>(reinterpret_cast<const std::byte*>(buffer_.data().data()) + offset_); }
public:
  void* __data() noexcept { return (void*)&base(); }
  SERVER__M2_Direct(boost::beast::flat_buffer& buffer, size_t offset)
    : buffer_(buffer)
    , offset_(offset)
  {
  }
  auto _1() noexcept { return nprpc::detail::flat::ObjectId_Direct(buffer_, offset_ + offsetof(SERVER__M2, _1)); }
};
} // namespace flat

struct SERVER__M3 {
  std::vector<nps::DataDef> _1;
};

namespace flat {
struct SERVER__M3 {
  ::flat::Vector<nps::flat::DataDef> _1;
};

class SERVER__M3_Direct {
  boost::beast::flat_buffer& buffer_;
  const size_t offset_;

  auto& base() noexcept { return *reinterpret_cast<SERVER__M3*>(reinterpret_cast<std::byte*>(buffer_.data().data()) + offset_); }
  auto const& base() const noexcept { return *reinterpret_cast<const SERVER__M3*>(reinterpret_cast<const std::byte*>(buffer_.data().data()) + offset_); }
public:
  void* __data() noexcept { return (void*)&base(); }
  SERVER__M3_Direct(boost::beast::flat_buffer& buffer, size_t offset)
    : buffer_(buffer)
    , offset_(offset)
  {
  }
  void _1(size_t elements_size) { new (&base()._1) ::flat::Vector<nps::flat::DataDef>(buffer_, elements_size); }
  auto _1_vd() noexcept { return ::flat::Vector_Direct2<nps::flat::DataDef, nps::flat::DataDef_Direct>(buffer_, offset_ + offsetof(SERVER__M3, _1)); }
  auto _1() noexcept { return ::flat::Span_ref<nps::flat::DataDef, nps::flat::DataDef_Direct>(buffer_, base()._1.range(buffer_.data().data())); }
};
} // namespace flat

struct SERVER__M4 {
  std::vector<nps::var_handle> _1;
};

namespace flat {
struct SERVER__M4 {
  ::flat::Vector<uint64_t> _1;
};

class SERVER__M4_Direct {
  boost::beast::flat_buffer& buffer_;
  const size_t offset_;

  auto& base() noexcept { return *reinterpret_cast<SERVER__M4*>(reinterpret_cast<std::byte*>(buffer_.data().data()) + offset_); }
  auto const& base() const noexcept { return *reinterpret_cast<const SERVER__M4*>(reinterpret_cast<const std::byte*>(buffer_.data().data()) + offset_); }
public:
  void* __data() noexcept { return (void*)&base(); }
  SERVER__M4_Direct(boost::beast::flat_buffer& buffer, size_t offset)
    : buffer_(buffer)
    , offset_(offset)
  {
  }
  void _1(size_t elements_size) { new (&base()._1) ::flat::Vector<uint64_t>(buffer_, elements_size); }
  auto _1_vd() noexcept { return ::flat::Vector_Direct1<uint64_t>(buffer_, offset_ + offsetof(SERVER__M4, _1)); }
  auto _1() noexcept { return (::flat::Span<uint64_t>)base()._1; }
};
} // namespace flat

struct SERVER__M5 {
  std::vector<uint8_t> _1;
};

namespace flat {
struct SERVER__M5 {
  ::flat::Vector<uint8_t> _1;
};

class SERVER__M5_Direct {
  boost::beast::flat_buffer& buffer_;
  const size_t offset_;

  auto& base() noexcept { return *reinterpret_cast<SERVER__M5*>(reinterpret_cast<std::byte*>(buffer_.data().data()) + offset_); }
  auto const& base() const noexcept { return *reinterpret_cast<const SERVER__M5*>(reinterpret_cast<const std::byte*>(buffer_.data().data()) + offset_); }
public:
  void* __data() noexcept { return (void*)&base(); }
  SERVER__M5_Direct(boost::beast::flat_buffer& buffer, size_t offset)
    : buffer_(buffer)
    , offset_(offset)
  {
  }
  void _1(size_t elements_size) { new (&base()._1) ::flat::Vector<uint8_t>(buffer_, elements_size); }
  auto _1_vd() noexcept { return ::flat::Vector_Direct1<uint8_t>(buffer_, offset_ + offsetof(SERVER__M5, _1)); }
  auto _1() noexcept { return (::flat::Span<uint8_t>)base()._1; }
};
} // namespace flat

struct SERVER__M6 {
  uint8_t _1;
  uint16_t _2;
  uint8_t _3;
  uint8_t _4;
};

namespace flat {
struct SERVER__M6 {
  uint8_t _1;
  uint16_t _2;
  uint8_t _3;
  uint8_t _4;
};

class SERVER__M6_Direct {
  boost::beast::flat_buffer& buffer_;
  const size_t offset_;

  auto& base() noexcept { return *reinterpret_cast<SERVER__M6*>(reinterpret_cast<std::byte*>(buffer_.data().data()) + offset_); }
  auto const& base() const noexcept { return *reinterpret_cast<const SERVER__M6*>(reinterpret_cast<const std::byte*>(buffer_.data().data()) + offset_); }
public:
  void* __data() noexcept { return (void*)&base(); }
  SERVER__M6_Direct(boost::beast::flat_buffer& buffer, size_t offset)
    : buffer_(buffer)
    , offset_(offset)
  {
  }
  const uint8_t& _1() const noexcept { return base()._1;}
  uint8_t& _1() noexcept { return base()._1;}
  const uint16_t& _2() const noexcept { return base()._2;}
  uint16_t& _2() noexcept { return base()._2;}
  const uint8_t& _3() const noexcept { return base()._3;}
  uint8_t& _3() noexcept { return base()._3;}
  const uint8_t& _4() const noexcept { return base()._4;}
  uint8_t& _4() noexcept { return base()._4;}
};
} // namespace flat

struct SERVER__M7 {
  uint8_t _1;
  uint16_t _2;
  uint8_t _3;
  uint8_t _4;
  uint8_t _5;
};

namespace flat {
struct SERVER__M7 {
  uint8_t _1;
  uint16_t _2;
  uint8_t _3;
  uint8_t _4;
  uint8_t _5;
};

class SERVER__M7_Direct {
  boost::beast::flat_buffer& buffer_;
  const size_t offset_;

  auto& base() noexcept { return *reinterpret_cast<SERVER__M7*>(reinterpret_cast<std::byte*>(buffer_.data().data()) + offset_); }
  auto const& base() const noexcept { return *reinterpret_cast<const SERVER__M7*>(reinterpret_cast<const std::byte*>(buffer_.data().data()) + offset_); }
public:
  void* __data() noexcept { return (void*)&base(); }
  SERVER__M7_Direct(boost::beast::flat_buffer& buffer, size_t offset)
    : buffer_(buffer)
    , offset_(offset)
  {
  }
  const uint8_t& _1() const noexcept { return base()._1;}
  uint8_t& _1() noexcept { return base()._1;}
  const uint16_t& _2() const noexcept { return base()._2;}
  uint16_t& _2() noexcept { return base()._2;}
  const uint8_t& _3() const noexcept { return base()._3;}
  uint8_t& _3() noexcept { return base()._3;}
  const uint8_t& _4() const noexcept { return base()._4;}
  uint8_t& _4() noexcept { return base()._4;}
  const uint8_t& _5() const noexcept { return base()._5;}
  uint8_t& _5() noexcept { return base()._5;}
};
} // namespace flat

struct SERVER__M8 {
  uint8_t _1;
  uint16_t _2;
  uint8_t _3;
};

namespace flat {
struct SERVER__M8 {
  uint8_t _1;
  uint16_t _2;
  uint8_t _3;
};

class SERVER__M8_Direct {
  boost::beast::flat_buffer& buffer_;
  const size_t offset_;

  auto& base() noexcept { return *reinterpret_cast<SERVER__M8*>(reinterpret_cast<std::byte*>(buffer_.data().data()) + offset_); }
  auto const& base() const noexcept { return *reinterpret_cast<const SERVER__M8*>(reinterpret_cast<const std::byte*>(buffer_.data().data()) + offset_); }
public:
  void* __data() noexcept { return (void*)&base(); }
  SERVER__M8_Direct(boost::beast::flat_buffer& buffer, size_t offset)
    : buffer_(buffer)
    , offset_(offset)
  {
  }
  const uint8_t& _1() const noexcept { return base()._1;}
  uint8_t& _1() noexcept { return base()._1;}
  const uint16_t& _2() const noexcept { return base()._2;}
  uint16_t& _2() noexcept { return base()._2;}
  const uint8_t& _3() const noexcept { return base()._3;}
  uint8_t& _3() noexcept { return base()._3;}
};
} // namespace flat

struct SERVER__M9 {
  uint8_t _1;
  uint16_t _2;
  uint16_t _3;
};

namespace flat {
struct SERVER__M9 {
  uint8_t _1;
  uint16_t _2;
  uint16_t _3;
};

class SERVER__M9_Direct {
  boost::beast::flat_buffer& buffer_;
  const size_t offset_;

  auto& base() noexcept { return *reinterpret_cast<SERVER__M9*>(reinterpret_cast<std::byte*>(buffer_.data().data()) + offset_); }
  auto const& base() const noexcept { return *reinterpret_cast<const SERVER__M9*>(reinterpret_cast<const std::byte*>(buffer_.data().data()) + offset_); }
public:
  void* __data() noexcept { return (void*)&base(); }
  SERVER__M9_Direct(boost::beast::flat_buffer& buffer, size_t offset)
    : buffer_(buffer)
    , offset_(offset)
  {
  }
  const uint8_t& _1() const noexcept { return base()._1;}
  uint8_t& _1() noexcept { return base()._1;}
  const uint16_t& _2() const noexcept { return base()._2;}
  uint16_t& _2() noexcept { return base()._2;}
  const uint16_t& _3() const noexcept { return base()._3;}
  uint16_t& _3() noexcept { return base()._3;}
};
} // namespace flat

struct SERVER__M10 {
  uint8_t _1;
  uint16_t _2;
  uint16_t _3;
  uint8_t _4;
};

namespace flat {
struct SERVER__M10 {
  uint8_t _1;
  uint16_t _2;
  uint16_t _3;
  uint8_t _4;
};

class SERVER__M10_Direct {
  boost::beast::flat_buffer& buffer_;
  const size_t offset_;

  auto& base() noexcept { return *reinterpret_cast<SERVER__M10*>(reinterpret_cast<std::byte*>(buffer_.data().data()) + offset_); }
  auto const& base() const noexcept { return *reinterpret_cast<const SERVER__M10*>(reinterpret_cast<const std::byte*>(buffer_.data().data()) + offset_); }
public:
  void* __data() noexcept { return (void*)&base(); }
  SERVER__M10_Direct(boost::beast::flat_buffer& buffer, size_t offset)
    : buffer_(buffer)
    , offset_(offset)
  {
  }
  const uint8_t& _1() const noexcept { return base()._1;}
  uint8_t& _1() noexcept { return base()._1;}
  const uint16_t& _2() const noexcept { return base()._2;}
  uint16_t& _2() noexcept { return base()._2;}
  const uint16_t& _3() const noexcept { return base()._3;}
  uint16_t& _3() noexcept { return base()._3;}
  const uint8_t& _4() const noexcept { return base()._4;}
  uint8_t& _4() noexcept { return base()._4;}
};
} // namespace flat

struct SERVER__M11 {
  uint8_t _1;
  uint16_t _2;
  uint32_t _3;
};

namespace flat {
struct SERVER__M11 {
  uint8_t _1;
  uint16_t _2;
  uint32_t _3;
};

class SERVER__M11_Direct {
  boost::beast::flat_buffer& buffer_;
  const size_t offset_;

  auto& base() noexcept { return *reinterpret_cast<SERVER__M11*>(reinterpret_cast<std::byte*>(buffer_.data().data()) + offset_); }
  auto const& base() const noexcept { return *reinterpret_cast<const SERVER__M11*>(reinterpret_cast<const std::byte*>(buffer_.data().data()) + offset_); }
public:
  void* __data() noexcept { return (void*)&base(); }
  SERVER__M11_Direct(boost::beast::flat_buffer& buffer, size_t offset)
    : buffer_(buffer)
    , offset_(offset)
  {
  }
  const uint8_t& _1() const noexcept { return base()._1;}
  uint8_t& _1() noexcept { return base()._1;}
  const uint16_t& _2() const noexcept { return base()._2;}
  uint16_t& _2() noexcept { return base()._2;}
  const uint32_t& _3() const noexcept { return base()._3;}
  uint32_t& _3() noexcept { return base()._3;}
};
} // namespace flat

struct SERVER__M12 {
  uint8_t _1;
  uint16_t _2;
  uint32_t _3;
  uint8_t _4;
};

namespace flat {
struct SERVER__M12 {
  uint8_t _1;
  uint16_t _2;
  uint32_t _3;
  uint8_t _4;
};

class SERVER__M12_Direct {
  boost::beast::flat_buffer& buffer_;
  const size_t offset_;

  auto& base() noexcept { return *reinterpret_cast<SERVER__M12*>(reinterpret_cast<std::byte*>(buffer_.data().data()) + offset_); }
  auto const& base() const noexcept { return *reinterpret_cast<const SERVER__M12*>(reinterpret_cast<const std::byte*>(buffer_.data().data()) + offset_); }
public:
  void* __data() noexcept { return (void*)&base(); }
  SERVER__M12_Direct(boost::beast::flat_buffer& buffer, size_t offset)
    : buffer_(buffer)
    , offset_(offset)
  {
  }
  const uint8_t& _1() const noexcept { return base()._1;}
  uint8_t& _1() noexcept { return base()._1;}
  const uint16_t& _2() const noexcept { return base()._2;}
  uint16_t& _2() noexcept { return base()._2;}
  const uint32_t& _3() const noexcept { return base()._3;}
  uint32_t& _3() noexcept { return base()._3;}
  const uint8_t& _4() const noexcept { return base()._4;}
  uint8_t& _4() noexcept { return base()._4;}
};
} // namespace flat

struct SERVER__M13 {
  uint8_t _1;
  uint16_t _2;
  std::vector<uint8_t> _3;
};

namespace flat {
struct SERVER__M13 {
  uint8_t _1;
  uint16_t _2;
  ::flat::Vector<uint8_t> _3;
};

class SERVER__M13_Direct {
  boost::beast::flat_buffer& buffer_;
  const size_t offset_;

  auto& base() noexcept { return *reinterpret_cast<SERVER__M13*>(reinterpret_cast<std::byte*>(buffer_.data().data()) + offset_); }
  auto const& base() const noexcept { return *reinterpret_cast<const SERVER__M13*>(reinterpret_cast<const std::byte*>(buffer_.data().data()) + offset_); }
public:
  void* __data() noexcept { return (void*)&base(); }
  SERVER__M13_Direct(boost::beast::flat_buffer& buffer, size_t offset)
    : buffer_(buffer)
    , offset_(offset)
  {
  }
  const uint8_t& _1() const noexcept { return base()._1;}
  uint8_t& _1() noexcept { return base()._1;}
  const uint16_t& _2() const noexcept { return base()._2;}
  uint16_t& _2() noexcept { return base()._2;}
  void _3(size_t elements_size) { new (&base()._3) ::flat::Vector<uint8_t>(buffer_, elements_size); }
  auto _3_vd() noexcept { return ::flat::Vector_Direct1<uint8_t>(buffer_, offset_ + offsetof(SERVER__M13, _3)); }
  auto _3() noexcept { return (::flat::Span<uint8_t>)base()._3; }
};
} // namespace flat

struct SERVER__M14 {
  uint8_t _1;
  uint16_t _2;
};

namespace flat {
struct SERVER__M14 {
  uint8_t _1;
  uint16_t _2;
};

class SERVER__M14_Direct {
  boost::beast::flat_buffer& buffer_;
  const size_t offset_;

  auto& base() noexcept { return *reinterpret_cast<SERVER__M14*>(reinterpret_cast<std::byte*>(buffer_.data().data()) + offset_); }
  auto const& base() const noexcept { return *reinterpret_cast<const SERVER__M14*>(reinterpret_cast<const std::byte*>(buffer_.data().data()) + offset_); }
public:
  void* __data() noexcept { return (void*)&base(); }
  SERVER__M14_Direct(boost::beast::flat_buffer& buffer, size_t offset)
    : buffer_(buffer)
    , offset_(offset)
  {
  }
  const uint8_t& _1() const noexcept { return base()._1;}
  uint8_t& _1() noexcept { return base()._1;}
  const uint16_t& _2() const noexcept { return base()._2;}
  uint16_t& _2() noexcept { return base()._2;}
};
} // namespace flat

struct SERVER__M15 {
  uint8_t _1;
};

namespace flat {
struct SERVER__M15 {
  uint8_t _1;
};

class SERVER__M15_Direct {
  boost::beast::flat_buffer& buffer_;
  const size_t offset_;

  auto& base() noexcept { return *reinterpret_cast<SERVER__M15*>(reinterpret_cast<std::byte*>(buffer_.data().data()) + offset_); }
  auto const& base() const noexcept { return *reinterpret_cast<const SERVER__M15*>(reinterpret_cast<const std::byte*>(buffer_.data().data()) + offset_); }
public:
  void* __data() noexcept { return (void*)&base(); }
  SERVER__M15_Direct(boost::beast::flat_buffer& buffer, size_t offset)
    : buffer_(buffer)
    , offset_(offset)
  {
  }
  const uint8_t& _1() const noexcept { return base()._1;}
  uint8_t& _1() noexcept { return base()._1;}
};
} // namespace flat

struct SERVER__M16 {
  uint8_t _1;
  uint8_t _2;
  std::vector<uint8_t> _3;
};

namespace flat {
struct SERVER__M16 {
  uint8_t _1;
  uint8_t _2;
  ::flat::Vector<uint8_t> _3;
};

class SERVER__M16_Direct {
  boost::beast::flat_buffer& buffer_;
  const size_t offset_;

  auto& base() noexcept { return *reinterpret_cast<SERVER__M16*>(reinterpret_cast<std::byte*>(buffer_.data().data()) + offset_); }
  auto const& base() const noexcept { return *reinterpret_cast<const SERVER__M16*>(reinterpret_cast<const std::byte*>(buffer_.data().data()) + offset_); }
public:
  void* __data() noexcept { return (void*)&base(); }
  SERVER__M16_Direct(boost::beast::flat_buffer& buffer, size_t offset)
    : buffer_(buffer)
    , offset_(offset)
  {
  }
  const uint8_t& _1() const noexcept { return base()._1;}
  uint8_t& _1() noexcept { return base()._1;}
  const uint8_t& _2() const noexcept { return base()._2;}
  uint8_t& _2() noexcept { return base()._2;}
  void _3(size_t elements_size) { new (&base()._3) ::flat::Vector<uint8_t>(buffer_, elements_size); }
  auto _3_vd() noexcept { return ::flat::Vector_Direct1<uint8_t>(buffer_, offset_ + offsetof(SERVER__M16, _3)); }
  auto _3() noexcept { return (::flat::Span<uint8_t>)base()._3; }
};
} // namespace flat

struct SERVER__M17 {
  cbt1::oid_t _1;
};

namespace flat {
struct SERVER__M17 {
  uint64_t _1;
};

class SERVER__M17_Direct {
  boost::beast::flat_buffer& buffer_;
  const size_t offset_;

  auto& base() noexcept { return *reinterpret_cast<SERVER__M17*>(reinterpret_cast<std::byte*>(buffer_.data().data()) + offset_); }
  auto const& base() const noexcept { return *reinterpret_cast<const SERVER__M17*>(reinterpret_cast<const std::byte*>(buffer_.data().data()) + offset_); }
public:
  void* __data() noexcept { return (void*)&base(); }
  SERVER__M17_Direct(boost::beast::flat_buffer& buffer, size_t offset)
    : buffer_(buffer)
    , offset_(offset)
  {
  }
  const uint64_t& _1() const noexcept { return base()._1;}
  uint64_t& _1() noexcept { return base()._1;}
};
} // namespace flat

struct SERVER__M18 {
  std::vector<uint16_t> _1;
};

namespace flat {
struct SERVER__M18 {
  ::flat::Vector<uint16_t> _1;
};

class SERVER__M18_Direct {
  boost::beast::flat_buffer& buffer_;
  const size_t offset_;

  auto& base() noexcept { return *reinterpret_cast<SERVER__M18*>(reinterpret_cast<std::byte*>(buffer_.data().data()) + offset_); }
  auto const& base() const noexcept { return *reinterpret_cast<const SERVER__M18*>(reinterpret_cast<const std::byte*>(buffer_.data().data()) + offset_); }
public:
  void* __data() noexcept { return (void*)&base(); }
  SERVER__M18_Direct(boost::beast::flat_buffer& buffer, size_t offset)
    : buffer_(buffer)
    , offset_(offset)
  {
  }
  void _1(size_t elements_size) { new (&base()._1) ::flat::Vector<uint16_t>(buffer_, elements_size); }
  auto _1_vd() noexcept { return ::flat::Vector_Direct1<uint16_t>(buffer_, offset_ + offsetof(SERVER__M18, _1)); }
  auto _1() noexcept { return (::flat::Span<uint16_t>)base()._1; }
};
} // namespace flat


#endif