#ifndef DB_
#define DB_

#include <nprpc/flat.hpp>
#include <nprpc/nprpc.hpp>

namespace cbt1 { 
using oid_t = uint64_t;
using uuid = std::array<uint8_t,16>;
} // namespace cbt1

namespace npd { 
class DatabaseException : public ::nprpc::Exception {
public:
  uint32_t code;

  DatabaseException() : ::nprpc::Exception("DatabaseException") {} 
  DatabaseException(uint32_t _code)
    : ::nprpc::Exception("DatabaseException")
    , code(_code)
  {
  }
};

namespace flat {
struct DatabaseException {
  uint32_t __ex_id;
  uint32_t code;
};

class DatabaseException_Direct {
  boost::beast::flat_buffer& buffer_;
  const size_t offset_;

  auto& base() noexcept { return *reinterpret_cast<DatabaseException*>(reinterpret_cast<std::byte*>(buffer_.data().data()) + offset_); }
  auto const& base() const noexcept { return *reinterpret_cast<const DatabaseException*>(reinterpret_cast<const std::byte*>(buffer_.data().data()) + offset_); }
public:
  void* __data() noexcept { return (void*)&base(); }
  DatabaseException_Direct(boost::beast::flat_buffer& buffer, size_t offset)
    : buffer_(buffer)
    , offset_(offset)
  {
  }
  const uint32_t& __ex_id() const noexcept { return base().__ex_id;}
  uint32_t& __ex_id() noexcept { return base().__ex_id;}
  const uint32_t& code() const noexcept { return base().code;}
  uint32_t& code() noexcept { return base().code;}
};
} // namespace flat

class INodeCallback_Servant
  : public virtual nprpc::ObjectServant
{
public:
  static std::string_view _get_class() noexcept { return "db/npd.NodeCallback"; }
  std::string_view get_class() const noexcept override { return INodeCallback_Servant::_get_class(); }
  void dispatch(nprpc::Buffers& bufs, nprpc::EndPoint remote_endpoint, bool from_parent, nprpc::ReferenceList& ref_list) override;
  virtual void OnNodeChanged (cbt1::oid_t id, ::flat::Span<uint8_t> data) = 0;
  virtual void OnNodeDeleted (cbt1::oid_t id) = 0;
};

class NodeCallback
  : public virtual nprpc::Object
{
  const uint8_t interface_idx_;
public:
  using servant_t = INodeCallback_Servant;

  NodeCallback(uint8_t interface_idx) : interface_idx_(interface_idx) {}
  void OnNodeChanged (/*in*/const cbt1::oid_t& id, /*in*/::flat::Span<const uint8_t> data);
  void OnNodeDeleted (/*in*/const cbt1::oid_t& id);
};

struct BatchOperation {
  bool create_or_update;
  cbt1::oid_t id;
  std::string data;
};

namespace flat {
struct BatchOperation {
  bool create_or_update;
  uint64_t id;
  ::flat::String data;
};

class BatchOperation_Direct {
  boost::beast::flat_buffer& buffer_;
  const size_t offset_;

  auto& base() noexcept { return *reinterpret_cast<BatchOperation*>(reinterpret_cast<std::byte*>(buffer_.data().data()) + offset_); }
  auto const& base() const noexcept { return *reinterpret_cast<const BatchOperation*>(reinterpret_cast<const std::byte*>(buffer_.data().data()) + offset_); }
public:
  void* __data() noexcept { return (void*)&base(); }
  BatchOperation_Direct(boost::beast::flat_buffer& buffer, size_t offset)
    : buffer_(buffer)
    , offset_(offset)
  {
  }
  const bool& create_or_update() const noexcept { return base().create_or_update;}
  bool& create_or_update() noexcept { return base().create_or_update;}
  const uint64_t& id() const noexcept { return base().id;}
  uint64_t& id() noexcept { return base().id;}
  void data(const char* str) { new (&base().data) ::flat::String(buffer_, str); }
  void data(const std::string& str) { new (&base().data) ::flat::String(buffer_, str); }
  auto data() noexcept { return (::flat::Span<char>)base().data; }
  auto data_vd() noexcept {     return ::flat::String_Direct1(buffer_, offset_ + offsetof(BatchOperation, data));  }
};
} // namespace flat

class IDatabase_Servant
  : public virtual nprpc::ObjectServant
{
public:
  static std::string_view _get_class() noexcept { return "db/npd.Database"; }
  std::string_view get_class() const noexcept override { return IDatabase_Servant::_get_class(); }
  void dispatch(nprpc::Buffers& bufs, nprpc::EndPoint remote_endpoint, bool from_parent, nprpc::ReferenceList& ref_list) override;
  virtual std::string get_database_name () = 0;
  virtual cbt1::uuid get_database_uuid () = 0;
  virtual cbt1::oid_t next_oid (cbt1::oid_t amount) = 0;
  virtual cbt1::oid_t last_oid () = 0;
  virtual cbt1::oid_t create (::flat::Span<uint8_t> data, bool sync) = 0;
  virtual cbt1::oid_t put (cbt1::oid_t id, ::flat::Span<uint8_t> data, bool sync) = 0;
  virtual bool exec_batch (::flat::Span_ref<npd::flat::BatchOperation, npd::flat::BatchOperation_Direct> data) = 0;
  virtual bool get (cbt1::oid_t id, /*out*/::flat::Vector_Direct1<uint8_t> data) = 0;
  virtual uint64_t get_n (::flat::Span<uint64_t> ids, /*out*/::flat::Vector_Direct1<uint8_t> data) = 0;
  virtual void remove (cbt1::oid_t id) = 0;
  virtual void advise (cbt1::oid_t id, nprpc::Object* client) = 0;
};

class Database
  : public virtual nprpc::Object
{
  const uint8_t interface_idx_;
public:
  using servant_t = IDatabase_Servant;

  Database(uint8_t interface_idx) : interface_idx_(interface_idx) {}
  std::string get_database_name ();
  cbt1::uuid get_database_uuid ();
  cbt1::oid_t next_oid (/*in*/const cbt1::oid_t& amount);
  cbt1::oid_t last_oid ();
  cbt1::oid_t create (/*in*/::flat::Span<const uint8_t> data, /*in*/bool sync);
  cbt1::oid_t put (/*in*/const cbt1::oid_t& id, /*in*/::flat::Span<const uint8_t> data, /*in*/bool sync);
  bool exec_batch (/*in*/::flat::Span<const npd::BatchOperation> data);
  bool get (/*in*/const cbt1::oid_t& id, /*out*/std::vector<uint8_t>& data);
  uint64_t get_n (/*in*/::flat::Span<const cbt1::oid_t> ids, /*out*/std::vector<uint8_t>& data);
  void remove (/*in*/const cbt1::oid_t& id);
  void advise (/*in*/const cbt1::oid_t& id, /*in*/const ObjectId& client);
};

} // namespace npd

namespace db::helper {
template<::nprpc::IterableCollection T>
void assign_get_data(/*out*/::flat::Vector_Direct1<uint8_t>& dest, const T & src) {
  dest.length(src.size());
  memcpy(dest().data(), src.data(), src.size() * 1);
}
template<::nprpc::IterableCollection T>
void assign_get_n_data(/*out*/::flat::Vector_Direct1<uint8_t>& dest, const T & src) {
  dest.length(src.size());
  memcpy(dest().data(), src.data(), src.size() * 1);
}
} // namespace db::helper

#endif