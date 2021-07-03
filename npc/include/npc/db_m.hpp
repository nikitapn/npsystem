#ifndef DB_M_
#define DB_M_

struct DB__M1 {
  cbt1::oid_t _1;
  std::vector<uint8_t> _2;
};

namespace flat {
struct DB__M1 {
  uint64_t _1;
  ::flat::Vector<uint8_t> _2;
};

class DB__M1_Direct {
  boost::beast::flat_buffer& buffer_;
  const size_t offset_;

  auto& base() noexcept { return *reinterpret_cast<DB__M1*>(reinterpret_cast<std::byte*>(buffer_.data().data()) + offset_); }
  auto const& base() const noexcept { return *reinterpret_cast<const DB__M1*>(reinterpret_cast<const std::byte*>(buffer_.data().data()) + offset_); }
public:
  void* __data() noexcept { return (void*)&base(); }
  DB__M1_Direct(boost::beast::flat_buffer& buffer, size_t offset)
    : buffer_(buffer)
    , offset_(offset)
  {
  }
  const uint64_t& _1() const noexcept { return base()._1;}
  uint64_t& _1() noexcept { return base()._1;}
  void _2(size_t elements_size) { new (&base()._2) ::flat::Vector<uint8_t>(buffer_, elements_size); }
  auto _2_vd() noexcept { return ::flat::Vector_Direct1<uint8_t>(buffer_, offset_ + offsetof(DB__M1, _2)); }
  auto _2() noexcept { return (::flat::Span<uint8_t>)base()._2; }
};
} // namespace flat

struct DB__M2 {
  cbt1::oid_t _1;
};

namespace flat {
struct DB__M2 {
  uint64_t _1;
};

class DB__M2_Direct {
  boost::beast::flat_buffer& buffer_;
  const size_t offset_;

  auto& base() noexcept { return *reinterpret_cast<DB__M2*>(reinterpret_cast<std::byte*>(buffer_.data().data()) + offset_); }
  auto const& base() const noexcept { return *reinterpret_cast<const DB__M2*>(reinterpret_cast<const std::byte*>(buffer_.data().data()) + offset_); }
public:
  void* __data() noexcept { return (void*)&base(); }
  DB__M2_Direct(boost::beast::flat_buffer& buffer, size_t offset)
    : buffer_(buffer)
    , offset_(offset)
  {
  }
  const uint64_t& _1() const noexcept { return base()._1;}
  uint64_t& _1() noexcept { return base()._1;}
};
} // namespace flat

struct DB__M3 {
  std::string _1;
};

namespace flat {
struct DB__M3 {
  ::flat::String _1;
};

class DB__M3_Direct {
  boost::beast::flat_buffer& buffer_;
  const size_t offset_;

  auto& base() noexcept { return *reinterpret_cast<DB__M3*>(reinterpret_cast<std::byte*>(buffer_.data().data()) + offset_); }
  auto const& base() const noexcept { return *reinterpret_cast<const DB__M3*>(reinterpret_cast<const std::byte*>(buffer_.data().data()) + offset_); }
public:
  void* __data() noexcept { return (void*)&base(); }
  DB__M3_Direct(boost::beast::flat_buffer& buffer, size_t offset)
    : buffer_(buffer)
    , offset_(offset)
  {
  }
  void _1(const char* str) { new (&base()._1) ::flat::String(buffer_, str); }
  void _1(const std::string& str) { new (&base()._1) ::flat::String(buffer_, str); }
  auto _1() noexcept { return (::flat::Span<char>)base()._1; }
};
} // namespace flat

struct DB__M4 {
  cbt1::uuid _1;
};

namespace flat {
struct DB__M4 {
  ::flat::Array<uint8_t,16> _1;
};

class DB__M4_Direct {
  boost::beast::flat_buffer& buffer_;
  const size_t offset_;

  auto& base() noexcept { return *reinterpret_cast<DB__M4*>(reinterpret_cast<std::byte*>(buffer_.data().data()) + offset_); }
  auto const& base() const noexcept { return *reinterpret_cast<const DB__M4*>(reinterpret_cast<const std::byte*>(buffer_.data().data()) + offset_); }
public:
  void* __data() noexcept { return (void*)&base(); }
  DB__M4_Direct(boost::beast::flat_buffer& buffer, size_t offset)
    : buffer_(buffer)
    , offset_(offset)
  {
  }
  auto _1() noexcept { return (::flat::Span<uint8_t>)base()._1; }
};
} // namespace flat

struct DB__M5 {
  std::vector<uint8_t> _1;
  bool _2;
};

namespace flat {
struct DB__M5 {
  ::flat::Vector<uint8_t> _1;
  bool _2;
};

class DB__M5_Direct {
  boost::beast::flat_buffer& buffer_;
  const size_t offset_;

  auto& base() noexcept { return *reinterpret_cast<DB__M5*>(reinterpret_cast<std::byte*>(buffer_.data().data()) + offset_); }
  auto const& base() const noexcept { return *reinterpret_cast<const DB__M5*>(reinterpret_cast<const std::byte*>(buffer_.data().data()) + offset_); }
public:
  void* __data() noexcept { return (void*)&base(); }
  DB__M5_Direct(boost::beast::flat_buffer& buffer, size_t offset)
    : buffer_(buffer)
    , offset_(offset)
  {
  }
  void _1(size_t elements_size) { new (&base()._1) ::flat::Vector<uint8_t>(buffer_, elements_size); }
  auto _1_vd() noexcept { return ::flat::Vector_Direct1<uint8_t>(buffer_, offset_ + offsetof(DB__M5, _1)); }
  auto _1() noexcept { return (::flat::Span<uint8_t>)base()._1; }
  const bool& _2() const noexcept { return base()._2;}
  bool& _2() noexcept { return base()._2;}
};
} // namespace flat

struct DB__M6 {
  cbt1::oid_t _1;
  std::vector<uint8_t> _2;
  bool _3;
};

namespace flat {
struct DB__M6 {
  uint64_t _1;
  ::flat::Vector<uint8_t> _2;
  bool _3;
};

class DB__M6_Direct {
  boost::beast::flat_buffer& buffer_;
  const size_t offset_;

  auto& base() noexcept { return *reinterpret_cast<DB__M6*>(reinterpret_cast<std::byte*>(buffer_.data().data()) + offset_); }
  auto const& base() const noexcept { return *reinterpret_cast<const DB__M6*>(reinterpret_cast<const std::byte*>(buffer_.data().data()) + offset_); }
public:
  void* __data() noexcept { return (void*)&base(); }
  DB__M6_Direct(boost::beast::flat_buffer& buffer, size_t offset)
    : buffer_(buffer)
    , offset_(offset)
  {
  }
  const uint64_t& _1() const noexcept { return base()._1;}
  uint64_t& _1() noexcept { return base()._1;}
  void _2(size_t elements_size) { new (&base()._2) ::flat::Vector<uint8_t>(buffer_, elements_size); }
  auto _2_vd() noexcept { return ::flat::Vector_Direct1<uint8_t>(buffer_, offset_ + offsetof(DB__M6, _2)); }
  auto _2() noexcept { return (::flat::Span<uint8_t>)base()._2; }
  const bool& _3() const noexcept { return base()._3;}
  bool& _3() noexcept { return base()._3;}
};
} // namespace flat

struct DB__M7 {
  std::vector<npd::BatchOperation> _1;
};

namespace flat {
struct DB__M7 {
  ::flat::Vector<npd::flat::BatchOperation> _1;
};

class DB__M7_Direct {
  boost::beast::flat_buffer& buffer_;
  const size_t offset_;

  auto& base() noexcept { return *reinterpret_cast<DB__M7*>(reinterpret_cast<std::byte*>(buffer_.data().data()) + offset_); }
  auto const& base() const noexcept { return *reinterpret_cast<const DB__M7*>(reinterpret_cast<const std::byte*>(buffer_.data().data()) + offset_); }
public:
  void* __data() noexcept { return (void*)&base(); }
  DB__M7_Direct(boost::beast::flat_buffer& buffer, size_t offset)
    : buffer_(buffer)
    , offset_(offset)
  {
  }
  void _1(size_t elements_size) { new (&base()._1) ::flat::Vector<npd::flat::BatchOperation>(buffer_, elements_size); }
  auto _1_vd() noexcept { return ::flat::Vector_Direct2<npd::flat::BatchOperation, npd::flat::BatchOperation_Direct>(buffer_, offset_ + offsetof(DB__M7, _1)); }
  auto _1() noexcept { return ::flat::Span_ref<npd::flat::BatchOperation, npd::flat::BatchOperation_Direct>(buffer_, base()._1.range(buffer_.data().data())); }
};
} // namespace flat

struct DB__M8 {
  bool _1;
};

namespace flat {
struct DB__M8 {
  bool _1;
};

class DB__M8_Direct {
  boost::beast::flat_buffer& buffer_;
  const size_t offset_;

  auto& base() noexcept { return *reinterpret_cast<DB__M8*>(reinterpret_cast<std::byte*>(buffer_.data().data()) + offset_); }
  auto const& base() const noexcept { return *reinterpret_cast<const DB__M8*>(reinterpret_cast<const std::byte*>(buffer_.data().data()) + offset_); }
public:
  void* __data() noexcept { return (void*)&base(); }
  DB__M8_Direct(boost::beast::flat_buffer& buffer, size_t offset)
    : buffer_(buffer)
    , offset_(offset)
  {
  }
  const bool& _1() const noexcept { return base()._1;}
  bool& _1() noexcept { return base()._1;}
};
} // namespace flat

struct DB__M9 {
  bool _1;
  std::vector<uint8_t> _2;
};

namespace flat {
struct DB__M9 {
  bool _1;
  ::flat::Vector<uint8_t> _2;
};

class DB__M9_Direct {
  boost::beast::flat_buffer& buffer_;
  const size_t offset_;

  auto& base() noexcept { return *reinterpret_cast<DB__M9*>(reinterpret_cast<std::byte*>(buffer_.data().data()) + offset_); }
  auto const& base() const noexcept { return *reinterpret_cast<const DB__M9*>(reinterpret_cast<const std::byte*>(buffer_.data().data()) + offset_); }
public:
  void* __data() noexcept { return (void*)&base(); }
  DB__M9_Direct(boost::beast::flat_buffer& buffer, size_t offset)
    : buffer_(buffer)
    , offset_(offset)
  {
  }
  const bool& _1() const noexcept { return base()._1;}
  bool& _1() noexcept { return base()._1;}
  void _2(size_t elements_size) { new (&base()._2) ::flat::Vector<uint8_t>(buffer_, elements_size); }
  auto _2_vd() noexcept { return ::flat::Vector_Direct1<uint8_t>(buffer_, offset_ + offsetof(DB__M9, _2)); }
  auto _2() noexcept { return (::flat::Span<uint8_t>)base()._2; }
};
} // namespace flat

struct DB__M10 {
  std::vector<cbt1::oid_t> _1;
};

namespace flat {
struct DB__M10 {
  ::flat::Vector<uint64_t> _1;
};

class DB__M10_Direct {
  boost::beast::flat_buffer& buffer_;
  const size_t offset_;

  auto& base() noexcept { return *reinterpret_cast<DB__M10*>(reinterpret_cast<std::byte*>(buffer_.data().data()) + offset_); }
  auto const& base() const noexcept { return *reinterpret_cast<const DB__M10*>(reinterpret_cast<const std::byte*>(buffer_.data().data()) + offset_); }
public:
  void* __data() noexcept { return (void*)&base(); }
  DB__M10_Direct(boost::beast::flat_buffer& buffer, size_t offset)
    : buffer_(buffer)
    , offset_(offset)
  {
  }
  void _1(size_t elements_size) { new (&base()._1) ::flat::Vector<uint64_t>(buffer_, elements_size); }
  auto _1_vd() noexcept { return ::flat::Vector_Direct1<uint64_t>(buffer_, offset_ + offsetof(DB__M10, _1)); }
  auto _1() noexcept { return (::flat::Span<uint64_t>)base()._1; }
};
} // namespace flat

struct DB__M11 {
  uint64_t _1;
  std::vector<uint8_t> _2;
};

namespace flat {
struct DB__M11 {
  uint64_t _1;
  ::flat::Vector<uint8_t> _2;
};

class DB__M11_Direct {
  boost::beast::flat_buffer& buffer_;
  const size_t offset_;

  auto& base() noexcept { return *reinterpret_cast<DB__M11*>(reinterpret_cast<std::byte*>(buffer_.data().data()) + offset_); }
  auto const& base() const noexcept { return *reinterpret_cast<const DB__M11*>(reinterpret_cast<const std::byte*>(buffer_.data().data()) + offset_); }
public:
  void* __data() noexcept { return (void*)&base(); }
  DB__M11_Direct(boost::beast::flat_buffer& buffer, size_t offset)
    : buffer_(buffer)
    , offset_(offset)
  {
  }
  const uint64_t& _1() const noexcept { return base()._1;}
  uint64_t& _1() noexcept { return base()._1;}
  void _2(size_t elements_size) { new (&base()._2) ::flat::Vector<uint8_t>(buffer_, elements_size); }
  auto _2_vd() noexcept { return ::flat::Vector_Direct1<uint8_t>(buffer_, offset_ + offsetof(DB__M11, _2)); }
  auto _2() noexcept { return (::flat::Span<uint8_t>)base()._2; }
};
} // namespace flat

struct DB__M12 {
  cbt1::oid_t _1;
  nprpc::ObjectId _2;
};

namespace flat {
struct DB__M12 {
  uint64_t _1;
  nprpc::detail::flat::ObjectId _2;
};

class DB__M12_Direct {
  boost::beast::flat_buffer& buffer_;
  const size_t offset_;

  auto& base() noexcept { return *reinterpret_cast<DB__M12*>(reinterpret_cast<std::byte*>(buffer_.data().data()) + offset_); }
  auto const& base() const noexcept { return *reinterpret_cast<const DB__M12*>(reinterpret_cast<const std::byte*>(buffer_.data().data()) + offset_); }
public:
  void* __data() noexcept { return (void*)&base(); }
  DB__M12_Direct(boost::beast::flat_buffer& buffer, size_t offset)
    : buffer_(buffer)
    , offset_(offset)
  {
  }
  const uint64_t& _1() const noexcept { return base()._1;}
  uint64_t& _1() noexcept { return base()._1;}
  auto _2() noexcept { return nprpc::detail::flat::ObjectId_Direct(buffer_, offset_ + offsetof(DB__M12, _2)); }
};
} // namespace flat


#endif