#pragma once

#if defined(_CONFIGURATOR_) || defined(_NPSERVER_)
#	include <avr_info/avr_info.h>
#endif

#include "network.h"

namespace npsys {

class CAVRController : public CController {
	friend boost::serialization::access;
	template<class Archive>
	void serialize(Archive& ar, const unsigned int file_version) {
		ar& boost::serialization::base_object<CController>(*this);
		ar& ports;
		ar& assigned_ofs;
		ar& assigned_modules;
		if (file_version >= 1) {
			ar& npsys::access::rw(version);
		}
	}
public:
	const int version = 0;
	odb::node_list<avr_port_n> ports;
	avr_assigned_object_files_l assigned_ofs;
	virtual void boost_serialization_always_abstact_base() { std::abort(); }
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
		std::string_view port, std::string_view pin, odb::weak_node<fbd_control_unit_n> alg);
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
	virtual void AssignAlgorithm(CControlUnit* pAlg) override;
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
		int _addr, hardware::Model _controller_model, int _version, bool _virt)
		: CController(name, _type, _addr, _controller_model, _virt)
		, version(_version)
	{
	}
};

class CVirtualAVRPCLINK : public CNetworkDevice {
	friend boost::serialization::access;
	template<class Archive>
	void serialize(Archive& ar, const unsigned int file_version) {
		ar& boost::serialization::base_object<CNetworkDevice>(*this);
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
	virtual void boost_serialization_always_abstact_base() { std::abort(); }
};

} // namespace npsys

BOOST_CLASS_VERSION(npsys::CAVRController, 1)