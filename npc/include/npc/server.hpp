#ifndef SERVER_
#define SERVER_

#include <nprpc/flat.hpp>
#include <nprpc/nprpc.hpp>

namespace cbt1 { 
using discrete = bool;
using oid_t = uint64_t;
using uuid = std::array<uint8_t,16>;
} // namespace cbt1

namespace nps { 
using var_handle = uint64_t;
using var_type = uint32_t;
enum class controller_type : uint32_t {
  CT_AVR5
};
enum class var_status : uint32_t {
  VST_DEVICE_RESPONDED,
  VST_DEVICE_NOT_RESPONDED,
  VST_UNKNOWN
};
class NPNetCommunicationError : public ::nprpc::Exception {
public:
  int32_t code;

  NPNetCommunicationError() : ::nprpc::Exception("NPNetCommunicationError") {} 
  NPNetCommunicationError(int32_t _code)
    : ::nprpc::Exception("NPNetCommunicationError")
    , code(_code)
  {
  }
};

namespace flat {
struct NPNetCommunicationError {
  uint32_t __ex_id;
  int32_t code;
};

class NPNetCommunicationError_Direct {
  boost::beast::flat_buffer& buffer_;
  const size_t offset_;

  auto& base() noexcept { return *reinterpret_cast<NPNetCommunicationError*>(reinterpret_cast<std::byte*>(buffer_.data().data()) + offset_); }
  auto const& base() const noexcept { return *reinterpret_cast<const NPNetCommunicationError*>(reinterpret_cast<const std::byte*>(buffer_.data().data()) + offset_); }
public:
  void* __data() noexcept { return (void*)&base(); }
  NPNetCommunicationError_Direct(boost::beast::flat_buffer& buffer, size_t offset)
    : buffer_(buffer)
    , offset_(offset)
  {
  }
  const uint32_t& __ex_id() const noexcept { return base().__ex_id;}
  uint32_t& __ex_id() noexcept { return base().__ex_id;}
  const int32_t& code() const noexcept { return base().code;}
  int32_t& code() noexcept { return base().code;}
};
} // namespace flat

struct server_value {
  var_handle h;
  var_status s;
  std::array<uint8_t,8> data;
};

namespace flat {
struct server_value {
  uint64_t h;
  var_status s;
  ::flat::Array<uint8_t,8> data;
};

class server_value_Direct {
  boost::beast::flat_buffer& buffer_;
  const size_t offset_;

  auto& base() noexcept { return *reinterpret_cast<server_value*>(reinterpret_cast<std::byte*>(buffer_.data().data()) + offset_); }
  auto const& base() const noexcept { return *reinterpret_cast<const server_value*>(reinterpret_cast<const std::byte*>(buffer_.data().data()) + offset_); }
public:
  void* __data() noexcept { return (void*)&base(); }
  server_value_Direct(boost::beast::flat_buffer& buffer, size_t offset)
    : buffer_(buffer)
    , offset_(offset)
  {
  }
  const uint64_t& h() const noexcept { return base().h;}
  uint64_t& h() noexcept { return base().h;}
  const var_status& s() const noexcept { return base().s;}
  var_status& s() noexcept { return base().s;}
  auto data() noexcept { return (::flat::Span<uint8_t>)base().data; }
};
} // namespace flat

struct RawDataDef {
  uint8_t dev_addr;
  uint16_t address;
  uint8_t size;
};

namespace flat {
struct RawDataDef {
  uint8_t dev_addr;
  uint16_t address;
  uint8_t size;
};

class RawDataDef_Direct {
  boost::beast::flat_buffer& buffer_;
  const size_t offset_;

  auto& base() noexcept { return *reinterpret_cast<RawDataDef*>(reinterpret_cast<std::byte*>(buffer_.data().data()) + offset_); }
  auto const& base() const noexcept { return *reinterpret_cast<const RawDataDef*>(reinterpret_cast<const std::byte*>(buffer_.data().data()) + offset_); }
public:
  void* __data() noexcept { return (void*)&base(); }
  RawDataDef_Direct(boost::beast::flat_buffer& buffer, size_t offset)
    : buffer_(buffer)
    , offset_(offset)
  {
  }
  const uint8_t& dev_addr() const noexcept { return base().dev_addr;}
  uint8_t& dev_addr() noexcept { return base().dev_addr;}
  const uint16_t& address() const noexcept { return base().address;}
  uint16_t& address() noexcept { return base().address;}
  const uint8_t& size() const noexcept { return base().size;}
  uint8_t& size() noexcept { return base().size;}
};
} // namespace flat

struct DataDef {
  uint8_t dev_addr;
  uint16_t mem_addr;
  var_type type;
};

namespace flat {
struct DataDef {
  uint8_t dev_addr;
  uint16_t mem_addr;
  uint32_t type;
};

class DataDef_Direct {
  boost::beast::flat_buffer& buffer_;
  const size_t offset_;

  auto& base() noexcept { return *reinterpret_cast<DataDef*>(reinterpret_cast<std::byte*>(buffer_.data().data()) + offset_); }
  auto const& base() const noexcept { return *reinterpret_cast<const DataDef*>(reinterpret_cast<const std::byte*>(buffer_.data().data()) + offset_); }
public:
  void* __data() noexcept { return (void*)&base(); }
  DataDef_Direct(boost::beast::flat_buffer& buffer, size_t offset)
    : buffer_(buffer)
    , offset_(offset)
  {
  }
  const uint8_t& dev_addr() const noexcept { return base().dev_addr;}
  uint8_t& dev_addr() noexcept { return base().dev_addr;}
  const uint16_t& mem_addr() const noexcept { return base().mem_addr;}
  uint16_t& mem_addr() noexcept { return base().mem_addr;}
  const uint32_t& type() const noexcept { return base().type;}
  uint32_t& type() noexcept { return base().type;}
};
} // namespace flat

class IPingable_Servant
  : public virtual nprpc::ObjectServant
{
public:
  static std::string_view _get_class() noexcept { return "server/nps.Pingable"; }
  std::string_view get_class() const noexcept override { return IPingable_Servant::_get_class(); }
  void dispatch(nprpc::Buffers& bufs, nprpc::EndPoint remote_endpoint, bool from_parent, nprpc::ReferenceList& ref_list) override;
  virtual void Ping () = 0;
};

class Pingable
  : public virtual nprpc::Object
{
  const uint8_t interface_idx_;
public:
  using servant_t = IPingable_Servant;

  Pingable(uint8_t interface_idx) : interface_idx_(interface_idx) {}
  void Ping ();
};

class IDataCallBack_Servant
  : public IPingable_Servant
{
public:
  static std::string_view _get_class() noexcept { return "server/nps.DataCallBack"; }
  std::string_view get_class() const noexcept override { return IDataCallBack_Servant::_get_class(); }
  void dispatch(nprpc::Buffers& bufs, nprpc::EndPoint remote_endpoint, bool from_parent, nprpc::ReferenceList& ref_list) override;
  virtual void OnDataChanged (::flat::Span_ref<nps::flat::server_value, nps::flat::server_value_Direct> a) = 0;
};

class DataCallBack
  : public Pingable
{
  const uint8_t interface_idx_;
public:
  using servant_t = IDataCallBack_Servant;

  DataCallBack(uint8_t interface_idx)
    : interface_idx_(interface_idx)
    , Pingable(interface_idx + 1)
  {
  }
  void OnDataChanged (/*in*/::flat::Span<const nps::server_value> a);
};

class IItemManager_Servant
  : public IPingable_Servant
{
public:
  static std::string_view _get_class() noexcept { return "server/nps.ItemManager"; }
  std::string_view get_class() const noexcept override { return IItemManager_Servant::_get_class(); }
  void dispatch(nprpc::Buffers& bufs, nprpc::EndPoint remote_endpoint, bool from_parent, nprpc::ReferenceList& ref_list) override;
  virtual void Activate (nprpc::Object* client) = 0;
  virtual void Advise (::flat::Span_ref<nps::flat::DataDef, nps::flat::DataDef_Direct> a, /*out*/::flat::Vector_Direct1<uint64_t> h) = 0;
  virtual void UnAdvise (::flat::Span<uint64_t> a) = 0;
};

class ItemManager
  : public Pingable
{
  const uint8_t interface_idx_;
public:
  using servant_t = IItemManager_Servant;

  ItemManager(uint8_t interface_idx)
    : interface_idx_(interface_idx)
    , Pingable(interface_idx + 1)
  {
  }
  void Activate (/*in*/const ObjectId& client);
  void Advise (/*in*/::flat::Span<const nps::DataDef> a, /*out*/std::vector<var_handle>& h);
  void UnAdvise (/*in*/::flat::Span<const var_handle> a);
};

class IServer_Servant
  : public IPingable_Servant
{
public:
  static std::string_view _get_class() noexcept { return "server/nps.Server"; }
  std::string_view get_class() const noexcept override { return IServer_Servant::_get_class(); }
  void dispatch(nprpc::Buffers& bufs, nprpc::EndPoint remote_endpoint, bool from_parent, nprpc::ReferenceList& ref_list) override;
  virtual void GetNetworkStatus (/*out*/::flat::Vector_Direct1<uint8_t> network_status) = 0;
  virtual void CreateItemManager (nprpc::detail::flat::ObjectId_Direct im) = 0;
  virtual void SendRawData (::flat::Span<uint8_t> data) = 0;
  virtual void Write_1 (uint8_t dev_addr, uint16_t mem_addr, uint8_t bit, uint8_t value) = 0;
  virtual void Write_q1 (uint8_t dev_addr, uint16_t mem_addr, uint8_t bit, uint8_t value, uint8_t qvalue) = 0;
  virtual void Write_8 (uint8_t dev_addr, uint16_t mem_addr, uint8_t value) = 0;
  virtual void Write_q8 (uint8_t dev_addr, uint16_t mem_addr, uint8_t value, uint8_t qvalue) = 0;
  virtual void Write_16 (uint8_t dev_addr, uint16_t mem_addr, uint16_t value) = 0;
  virtual void Write_q16 (uint8_t dev_addr, uint16_t mem_addr, uint16_t value, uint8_t qvalue) = 0;
  virtual void Write_32 (uint8_t dev_addr, uint16_t mem_addr, uint32_t value) = 0;
  virtual void Write_q32 (uint8_t dev_addr, uint16_t mem_addr, uint32_t value, uint8_t qvalue) = 0;
  virtual void WriteBlock (uint8_t dev_addr, uint16_t mem_addr, ::flat::Span<uint8_t> data) = 0;
  virtual void ReadByte (uint8_t dev_addr, uint16_t addr, uint8_t& value) = 0;
  virtual void ReadBlock (uint8_t dev_addr, uint16_t addr, uint8_t len, /*out*/::flat::Vector_Direct1<uint8_t> data) = 0;
  virtual bool AVR_StopAlgorithm (uint8_t dev_addr, uint16_t alg_addr) = 0;
  virtual void AVR_ReinitIO (uint8_t dev_addr) = 0;
  virtual void AVR_SendRemoteData (uint8_t dev_addr, uint16_t page_num, ::flat::Span<uint8_t> data) = 0;
  virtual void AVR_SendPage (uint8_t dev_addr, uint8_t page_num, ::flat::Span<uint8_t> data) = 0;
  virtual void AVR_RemoveAlgorithm (uint8_t dev_addr, uint16_t alg_addr) = 0;
  virtual void AVR_ReplaceAlgorithm (uint8_t dev_addr, uint16_t old_addr, uint16_t new_addr) = 0;
  virtual void AVR_WriteEeprom (uint8_t dev_addr, uint16_t mem_addr, ::flat::Span<uint8_t> data) = 0;
  virtual void AVR_WriteTwiTable (uint8_t dev_addr, uint8_t page_num, ::flat::Span<uint8_t> data) = 0;
  virtual void AVR_V_GetFlash (cbt1::oid_t device_id, /*out*/::flat::Vector_Direct1<uint16_t> data) = 0;
  virtual bool AVR_V_StoreFlash (cbt1::oid_t device_id) = 0;
  virtual bool Notify_DeviceActivated (cbt1::oid_t device_id) = 0;
  virtual bool Notify_DeviceDeactivated (cbt1::oid_t device_id) = 0;
  virtual void Notify_ParameterRemoved (cbt1::oid_t param_id) = 0;
  virtual void Notify_TypeOrVariableChanged (cbt1::oid_t param_id) = 0;
  virtual void History_AddParameter (cbt1::oid_t param_id) = 0;
  virtual void History_RemoveParameter (cbt1::oid_t param_id) = 0;
};

class Server
  : public Pingable
{
  const uint8_t interface_idx_;
public:
  using servant_t = IServer_Servant;

  Server(uint8_t interface_idx)
    : interface_idx_(interface_idx)
    , Pingable(interface_idx + 1)
  {
  }
  void GetNetworkStatus (/*out*/std::vector<uint8_t>& network_status);
  void CreateItemManager (/*out*/Object*& im);
  void SendRawData (/*in*/::flat::Span<const uint8_t> data);
  void Write_1 (/*in*/uint8_t dev_addr, /*in*/uint16_t mem_addr, /*in*/uint8_t bit, /*in*/uint8_t value);
  void Write_q1 (/*in*/uint8_t dev_addr, /*in*/uint16_t mem_addr, /*in*/uint8_t bit, /*in*/uint8_t value, /*in*/uint8_t qvalue);
  void Write_8 (/*in*/uint8_t dev_addr, /*in*/uint16_t mem_addr, /*in*/uint8_t value);
  void Write_q8 (/*in*/uint8_t dev_addr, /*in*/uint16_t mem_addr, /*in*/uint8_t value, /*in*/uint8_t qvalue);
  void Write_16 (/*in*/uint8_t dev_addr, /*in*/uint16_t mem_addr, /*in*/uint16_t value);
  void Write_q16 (/*in*/uint8_t dev_addr, /*in*/uint16_t mem_addr, /*in*/uint16_t value, /*in*/uint8_t qvalue);
  void Write_32 (/*in*/uint8_t dev_addr, /*in*/uint16_t mem_addr, /*in*/uint32_t value);
  void Write_q32 (/*in*/uint8_t dev_addr, /*in*/uint16_t mem_addr, /*in*/uint32_t value, /*in*/uint8_t qvalue);
  void WriteBlock (/*in*/uint8_t dev_addr, /*in*/uint16_t mem_addr, /*in*/::flat::Span<const uint8_t> data);
  void ReadByte (/*in*/uint8_t dev_addr, /*in*/uint16_t addr, /*out*/uint8_t& value);
  void ReadBlock (/*in*/uint8_t dev_addr, /*in*/uint16_t addr, /*in*/uint8_t len, /*out*/std::vector<uint8_t>& data);
  bool AVR_StopAlgorithm (/*in*/uint8_t dev_addr, /*in*/uint16_t alg_addr);
  void AVR_ReinitIO (/*in*/uint8_t dev_addr);
  void AVR_SendRemoteData (/*in*/uint8_t dev_addr, /*in*/uint16_t page_num, /*in*/::flat::Span<const uint8_t> data);
  void AVR_SendPage (/*in*/uint8_t dev_addr, /*in*/uint8_t page_num, /*in*/::flat::Span<const uint8_t> data);
  void AVR_RemoveAlgorithm (/*in*/uint8_t dev_addr, /*in*/uint16_t alg_addr);
  void AVR_ReplaceAlgorithm (/*in*/uint8_t dev_addr, /*in*/uint16_t old_addr, /*in*/uint16_t new_addr);
  void AVR_WriteEeprom (/*in*/uint8_t dev_addr, /*in*/uint16_t mem_addr, /*in*/::flat::Span<const uint8_t> data);
  void AVR_WriteTwiTable (/*in*/uint8_t dev_addr, /*in*/uint8_t page_num, /*in*/::flat::Span<const uint8_t> data);
  void AVR_V_GetFlash (/*in*/const cbt1::oid_t& device_id, /*out*/std::vector<uint16_t>& data);
  bool AVR_V_StoreFlash (/*in*/const cbt1::oid_t& device_id);
  bool Notify_DeviceActivated (/*in*/const cbt1::oid_t& device_id);
  bool Notify_DeviceDeactivated (/*in*/const cbt1::oid_t& device_id);
  void Notify_ParameterRemoved (/*in*/const cbt1::oid_t& param_id);
  void Notify_TypeOrVariableChanged (/*in*/const cbt1::oid_t& param_id);
  void History_AddParameter (/*in*/const cbt1::oid_t& param_id);
  void History_RemoveParameter (/*in*/const cbt1::oid_t& param_id);
};

} // namespace nps

namespace server::helper {
template<::nprpc::IterableCollection T>
void assign_Advise_h(/*out*/::flat::Vector_Direct1<uint64_t>& dest, const T & src) {
  dest.length(src.size());
  memcpy(dest().data(), src.data(), src.size() * 8);
}
template<::nprpc::IterableCollection T>
void assign_GetNetworkStatus_network_status(/*out*/::flat::Vector_Direct1<uint8_t>& dest, const T & src) {
  dest.length(src.size());
  memcpy(dest().data(), src.data(), src.size() * 1);
}
template<::nprpc::IterableCollection T>
void assign_AVR_V_GetFlash_data(/*out*/::flat::Vector_Direct1<uint16_t>& dest, const T & src) {
  dest.length(src.size());
  memcpy(dest().data(), src.data(), src.size() * 2);
}
} // namespace server::helper

#endif