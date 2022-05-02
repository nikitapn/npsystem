// Copyright (c) 2021 nikitapnn1@gmail.com
// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory

#pragma once

#include "header.h"
#include "access.h"
#include "cmodels.h"
#include <npdb/memento.h>
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
		void serialize(Archive& ar, const unsigned int /*file_version*/) {
			ar& name_;
			ar& dev_addr;
			ar& allocated_vars_;
			ar& vars_owner_released_;
			ar& remote_data_changed_;
			ar& links_r_;
			ar& links_w_;
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
		const std::string ip_address = "192.168.1.70";
		const uint16_t port = 28040;
		const uint16_t timeout = 1000;

		template<class Archive>
		void serialize(Archive& ar, const unsigned int /*file_version*/) {
			ar& boost::serialization::base_object<CNetworkDevice>(*this);
			ar& npsys::access::rw(ip_address);
			ar& npsys::access::rw(port);
			ar& npsys::access::rw(timeout);
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
		void serialize(Archive& ar, const unsigned int /*file_version*/) {
			ar& boost::serialization::base_object<CNetworkDevice>(*this);
			ar& npsys::access::rw(firmware_status);
			ar& status;
			ar& npsys::access::rw(controller_type);
			ar& npsys::access::rw(controller_model);
			ar& virt_;
			ar& assigned_algs;
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
		const hardware::Model controller_model = hardware::Model::UNKNOWN_MODEL;
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
			std::string_view port, std::string_view pin, odb::weak_node<fbd_control_unit_n> alg) = 0;
		virtual std::optional<npsys::i2c_assigned_module_l> GetAssignedModules() noexcept { return {}; }
#endif // _CONFIGURATOR_F
		CController() = default;
		CController(const std::string& name, const std::string& _controller_type,
			int _addr, hardware::Model _controller_model, bool virt)
			: CNetworkDevice(name, _addr)
			, firmware_status(FIRMWARE_NOT_LOADED)
			, status(0)
			, controller_type(_controller_type)
			, controller_model(_controller_model)
			, virt_(virt)
		{
		}
	};

} // namespace npsys