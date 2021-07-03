#pragma once

#include "header.h"
#include "access.h"
#include <npdb/memento.h>
#include <avr_info/avr_info.h>
#include <npsys/other/remote.h>
#ifdef _CONFIGURATOR_
#	include <npsys/other/write_default_value_request.h>
#endif
#include <npsys/other/uploadable.h>

#ifdef _CONFIGURATOR_
#	ifndef __NETWORK_EXT_H__
#		error "network.h should not be included directry"
#	endif // __NETWORK_EXT_H__

class COutsideReference;
class CAVR5DynamicLinker;
class CAlgorithmVariableLoader;

namespace net {
struct remote_data;
};
struct REQUEST;
struct twitab_t;
class CTime;
class CBlockSchedule;
#endif // _CONFIGURATOR_

namespace npsys {

class CNetworkDevice 
	: public odb::common_interface<CNetworkDevice> 
	, public odb::systree_item_observable<CNetworkDevice>
	, public odb::self_node_member<npsys::device_n>
	, public IUploadable {
#ifdef _CONFIGURATOR_
	friend ::detail::CVariableLoader;
	friend class CAssignedAlgorithm;
	friend CAlgorithmVariableLoader;
	friend CAVR5DynamicLinker;
	friend CMemoryManager;
#endif
	friend variable;
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive & ar, const unsigned int /*file_version*/) {
		ar & name_;
		ar & dev_addr;
		ar & allocated_vars_;
		ar & vars_owner_released_;
		ar & remote_data_changed_;
		ar & links_r_;
		ar & links_w_;
	}
	odb::node_list<variable_n> allocated_vars_;
	odb::node_list<variable_n> vars_owner_released_;
public:
	using link_list = remote::DeviceLinks::link_list;
	using link_map = std::unordered_map<oid_t, remote::DeviceLinks>;

	int32_t dev_addr = -1;
protected:
	bool remote_data_changed_ = false;
	link_map links_r_;
	link_map links_w_;
public:
	virtual void boost_serialization_always_abstact_base() = 0;
#ifdef _CONFIGURATOR_
	virtual std::unique_ptr<CDynamicLinker> CreateLinker() = 0;
	virtual void WriteDefaultValues(const std::vector<WriteDefaultValuesRequest>& requests) const = 0;
	virtual void UploadRemoteData() = 0;
	bool ActivateDevice(oid_t id) noexcept;
	void AddLink(npsys::variable_n& var, npsys::variable_n& ref_var, remote::ExtLinkType type);
	void DeleteLink(const remote::ExtLinkType type, const oid_t var_id, const bool is_bit);
	virtual void CreateOnlineCustomItems(CTreeViewCtrl tree, HTREEITEM parent, std::vector<std::unique_ptr<COnlineTreeItem>>& nodes) {}

	virtual const std::vector<std::tuple<int, uint16_t>>& InitInternalBlock(CTime*) = 0;
	virtual const std::vector<std::tuple<int, uint16_t>>& InitInternalBlock(CBlockSchedule*) = 0;

	CTreeItemAbstract* item = nullptr;
#endif // _CONFIGURATOR_
#ifdef _NPSERVER_
	virtual bool AddToNetwork(npsys::device_n device) = 0;
#endif // _NPSERVER_
	auto const& allocated_vars() const noexcept { return allocated_vars_; }
	auto const& vars_owner_released() const noexcept { return vars_owner_released_; }
	virtual std::unique_ptr<odb::IMemento> CreateMemento_Uploadable() noexcept final {
		return odb::MakeMemento(*this, remote_data_changed_, links_r_, links_w_);
	}

	CNetworkDevice() = default;
	CNetworkDevice(const std::string& name, int _dev_addr)
		: odb::systree_item_observable<CNetworkDevice>(name)
		, dev_addr(_dev_addr) {}
};

class CServer : public CNetworkDevice {
public:
	const std::string ip_address	= "192.168.1.70";
	const uint16_t port	= 28040;
	const uint16_t timeout = 1000;

	template<class Archive>
	void serialize(Archive & ar, const unsigned int /*file_version*/) {
		ar & boost::serialization::base_object<CNetworkDevice>(*this);
		ar & npsys::access::rw(ip_address);
		ar & npsys::access::rw(port);
		ar & npsys::access::rw(timeout);
	}

	virtual void boost_serialization_always_abstact_base() {}
#ifdef _CONFIGURATOR_
	virtual std::unique_ptr<CDynamicLinker> CreateLinker() {
		ASSERT(FALSE);
		return std::unique_ptr<CDynamicLinker>();
	};
	virtual void WriteDefaultValues(const std::vector<WriteDefaultValuesRequest>& m_requests) const {}

	virtual void UploadRemoteData() {}
	virtual const std::vector<std::tuple<int, uint16_t>>& InitInternalBlock(CTime*) { 
		static std::vector<std::tuple<int, uint16_t>> v;
		return v; 
	}
	virtual const std::vector<std::tuple<int, uint16_t>>& InitInternalBlock(CBlockSchedule*) { 
		static std::vector<std::tuple<int, uint16_t>> v;
		return v; 
	}
#endif // _CONFIGURATOR_
#ifdef _NPSERVER_
	virtual bool AddToNetwork(npsys::device_n device);
#endif // _NPSERVER_
};


class CController : public CNetworkDevice {
	friend boost::serialization::access;
	template<class Archive>
	void serialize(Archive & ar, const unsigned int /*file_version*/) {
		ar & boost::serialization::base_object<CNetworkDevice>(*this);
		ar & npsys::access::rw(firmware_status);
		ar & status;
		ar & npsys::access::rw(controller_type);
		ar & npsys::access::rw(controller_model);
		ar & virt_;
		ar & assigned_algs;
	}
public:
	enum FIRMWARE_STATUS {
		FIRMWARE_NOT_LOADED,
		FIRMWARE_LOADED,
		FIRMWARE_LOADED_DEVICE_ACTIVATED
	};
	const FIRMWARE_STATUS firmware_status = FIRMWARE_NOT_LOADED;
	uint32_t status = 0;
	const std::string controller_type = "";
	const avrinfo::Model controller_model = avrinfo::Model::UNKNOWN_MODEL;
	odb::node_list<assigned_algorithm_n> assigned_algs;
	i2c_assigned_module_l assigned_modules;
private:
	bool virt_;
public:
	bool IsVirtual() const noexcept { return virt_; }
#if defined(_CONFIGURATOR_) || defined(_NPSERVER_)
	virtual uint16_t GetA_Time() const noexcept = 0;
#endif
#ifdef _CONFIGURATOR_
	virtual CTreeHardware* CreateHardware(npsys::controller_n& self) noexcept = 0;
	virtual INT_PTR ShowProperties() noexcept = 0;
	virtual bool LoadFirmware() noexcept = 0;
	virtual void ConstructMenu(CMenu* menu) noexcept = 0;
	virtual bool HandleRequest(REQUEST* req) noexcept = 0;
	virtual bool UploadIO() noexcept = 0;
	virtual void IoConstructMenu(CMenu* menu) noexcept = 0;
	virtual bool IoHandleRequest(REQUEST* req) noexcept = 0;
	virtual std::unique_ptr<COutsideReference> CreatePinReference(remote::ExtLinkType type, 
	const std::string& port, const std::string& pin, odb::weak_node<algorithm_n> alg) = 0;
	virtual std::optional<npsys::i2c_assigned_module_l> GetAssignedModules() noexcept { return {}; }
#endif // _CONFIGURATOR_F
	CController() = default;
	CController(const std::string& name, const std::string& _controller_type, 
		int _addr, avrinfo::Model _controller_model, bool virt) 
		: CNetworkDevice(name, _addr)
		, firmware_status(FIRMWARE_NOT_LOADED)
		, status(0)
		, controller_type(_controller_type)
		, controller_model(_controller_model)
		, virt_(virt) 
	{
	}
};

class CAVRController : public CController {
	friend boost::serialization::access;
	template<class Archive>
	void serialize(Archive & ar, const unsigned int file_version) {
		ar & boost::serialization::base_object<CController>(*this);
		ar & ports;
		ar & assigned_ofs;
		ar & assigned_modules;
		if (file_version >= 1) {
			ar &  npsys::access::rw(version);
		}
	}
public:
	const int version = 0;
	odb::node_list<avr_port_n> ports;
	avr_assigned_object_files_l assigned_ofs;
	virtual void boost_serialization_always_abstact_base() {}
#if defined(_CONFIGURATOR_) || defined(_NPSERVER_)
	virtual uint16_t GetA_Time() const noexcept { return GetFirmwareInfo().rmem.lt; }
	const avrinfo::PeripheralInfo& GetPeripheralInfo() const { return avrinfo::AVRInfo::get_instance().GetPeripheralInfo(controller_model); }
	const avrinfo::FirmwareInfo& GetFirmwareInfo() const { return avrinfo::AVRInfo::get_instance().GetInfo(controller_model, version); }
#endif

#ifdef _CONFIGURATOR_
	std::vector<uint16_t> GetTaskTable() const;
	uint16_t PageToByte(uint16_t PageNum) const { return PageNum * GetFirmwareInfo().pmem.pagesize; }
	uint16_t PageToWord(uint16_t PageNum) const { return PageNum * GetFirmwareInfo().pmem.pagesize / 2; }
	uint16_t GetPageSizeInBytes() const { return GetFirmwareInfo().pmem.pagesize; }
	uint16_t GetPageSizeInWords() const { return GetFirmwareInfo().pmem.pagesize / 2; }
	virtual std::unique_ptr<CDynamicLinker> CreateLinker();
	virtual CTreeHardware* CreateHardware(npsys::controller_n& self) noexcept;
	virtual void CreateOnlineCustomItems(CTreeViewCtrl tree, HTREEITEM parent, std::vector<std::unique_ptr<COnlineTreeItem>>& nodes);
	virtual void WriteDefaultValues(const std::vector<WriteDefaultValuesRequest>& m_requests) const;

	void ShowTwiTable() const noexcept;
	void ShowTaskTable() const noexcept;
	void ShowRemoteTable() const noexcept;
	//
	virtual INT_PTR ShowProperties() noexcept;
	virtual void ConstructMenu(CMenu* menu) noexcept;
	virtual bool HandleRequest(REQUEST* req) noexcept;
	virtual void IoConstructMenu(CMenu* menu) noexcept;
	virtual bool IoHandleRequest(REQUEST* req) noexcept;

	std::string PrintRemoteTable(net::remote_data* pData) const;
	std::string PrintTwiTable(twitab_t* tab) const;
	virtual bool LoadFirmware() noexcept override;
	virtual void Dispatch(std::function<void(void*)> fn) { fn(this); }

	bool StopAlgorithm(uint16_t alg_addr);
	virtual int ReplaceAlg(int old_addr, int new_addr);
	virtual int RemoveAlg(uint16_t algAddr);
	virtual void UploadRemoteData() override;
	virtual bool UploadIO() noexcept;
	virtual const std::vector<std::tuple<int, uint16_t>>& InitInternalBlock(CTime*);
	virtual const std::vector<std::tuple<int, uint16_t>>& InitInternalBlock(CBlockSchedule*);

	virtual std::unique_ptr<COutsideReference> CreatePinReference(remote::ExtLinkType type, 
		const std::string& port, const std::string& pin, odb::weak_node<algorithm_n> alg);
	virtual std::optional<npsys::i2c_assigned_module_l> GetAssignedModules() noexcept { return assigned_modules; }
#ifdef DEBUG
	void ShowInternalData();
	void DeleteRDTable();
	void DeleteTaskTable();
	void DeleteI2CTable();
#endif // DEBUG
	int WriteRemoteData(uint8_t* data, int len);
	int WriteEeprom(uint16_t mem_addr, int len, const uint8_t* data);
	int WriteTwiTable(int page_num, int page_size, uint8_t* page_p);
	int WritePage(int page_num, int page_size, uint8_t* page_p);
	void ReinitIO();
	bool SetActivated() noexcept;
	/*
	bool AssignObject(const * object) override;
	virtual void AddToNetwork() override;
	virtual void AssignModule(CModule* pModule) override;
	virtual void AssignAlgorithm(CAlgorithm* pAlg) override;
	*/
	std::unique_ptr<Dlg_AVRMemoryViewer> flash_viewer_;

	bool IsFlashViewerWindow() noexcept {
		return (flash_viewer_ && (flash_viewer_->m_hWnd != NULL));
	}
#endif // _CONFIGURATOR_

#ifdef _NPSERVER_
	virtual bool AddToNetwork(npsys::device_n device);
#endif // _NPSERVER_

	CAVRController() = default;
	CAVRController(const std::string& name, const std::string& _type, 
		int _addr, avrinfo::Model _controller_model, int _version, bool _virt) 
		: CController(name, _type, _addr, _controller_model, _virt) 
		, version(_version)
	{ 
	}
};

class CVirtualAVRPCLINK : public CNetworkDevice {
	friend boost::serialization::access;
	template<class Archive>
	void serialize(Archive & ar, const unsigned int file_version) {
		ar & boost::serialization::base_object<CNetworkDevice>(*this);
	}
public:
#ifdef _NPSERVER_
	virtual bool AddToNetwork(npsys::device_n device);
#endif // _NPSERVER_
	CVirtualAVRPCLINK() = default;
	CVirtualAVRPCLINK(int create_new) 
		: CNetworkDevice("Virtual Bridge", 0) 
	{
	}
	virtual void boost_serialization_always_abstact_base() {}
};

} // namespace npsys

BOOST_CLASS_VERSION(npsys::CAVRController, 1)
