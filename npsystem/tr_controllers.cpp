// Copyright (c) 2021 nikitapnn1@gmail.com
// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory

#include "stdafx.h"
#include "tr_item.h"
#include "tr_network.h"
#include "tr_alg.h"
#include <avr_info/avr_info.h>
#include "dlgnewcontroller.h"
#include "dlgloadingmanager.h"
#include "io_avr.h"
#include <npsys/variable.h>
#include "network_ext.h"
#include "assignedalgorithm.h"
#include "block.h"
#include "listcontrolview.h"
#include "avrassigned.h"
#include "configurator.h"
#include "avrassigned.h"
#include "tr_hardware.h"
#include "tr_module.h"
#include "view_controller.h"

static npsys::controller_n CreateAvrController(
	npsys::controllers_l& controllers,
	oid_t controller_id,
	const std::string& type,
	const avrinfo::Model model,
	bool virt);

extern UINT g_clf_blocks;

class CTreeAssignedSegmentValue
	: public CTemplateItemHardwareStatus<CTreeSegmentValueT<
	CTemplateItem<CTreeAssignedSegmentValue, CTreeItemAbstract,
	CategoryElement<npsys::CI2CModuleSegmentValue, npsys::CI2CModuleSegment::list_t>>, true>, CTreeAssignedSegmentValue> {
	using base = CTemplateItemHardwareStatus<CTreeSegmentValueT<
		CTemplateItem<CTreeAssignedSegmentValue, CTreeItemAbstract,
		CategoryElement<npsys::CI2CModuleSegmentValue, npsys::CI2CModuleSegment::list_t>>, true>, CTreeAssignedSegmentValue>;
public:
	CTreeAssignedSegmentValue(npsys::CI2CModuleSegmentValue* data, npsys::CI2CModuleSegment::list_t& list, npsys::CI2CModuleSegment& segment)
		: base(data, list, segment) {}

	io::Status ConvertStatus() {
		return data_->ass->GetStatus();
	}

	void ConstructMenu(CMenu* menu) noexcept {
		menu->AppendMenu(MF_STRING, ID_CREATE_LINK_FROM_MODULE, L"Copy As Link");
		menu->AppendMenu(MF_SEPARATOR);
		__super::ConstructMenu(menu);
	}

	void HandleRequest(REQUEST* req) noexcept {
		switch (req->ID_MENU) {
		case ID_CREATE_LINK_FROM_MODULE:
			CopyModuleValueAsLink();
			break;
		default:
			__super::HandleRequest(req);
		}
	}

	virtual COnlineTreeItem* CreateOnlineFromThis(std::vector<std::unique_ptr<COnlineTreeItem>>& nodes, CTreeViewCtrl tree, HTREEITEM hParent) {
		if (data_->ass->var.fetch()) {
			auto& var = data_->ass->var;
			ASSERT(var->IsQuality());
			nodes.push_back(std::move(
				std::make_unique<COnlineTreeItem>(GetName(), m_hIcon, *var.get())
			));
			nodes.back()->Insert(tree, hParent);
			online_item_ = nodes.back().get();
			return nodes.back().get();
		}
		return __super::CreateOnlineFromThis(nodes, tree, hParent);
	}

	virtual void PostDelete(CContainer* parent) noexcept {
		data()->ass->sig_node_removed();
		data()->ass->sig_node_removed.disconnect_all_slots();
		data()->ass->Release();
		if (parent)
		{
			EraseFromList();
			segment_.RecalcAll();
			StoreItem();
			CalcAndUpdateHardwareStatusAll(parent);
		}
	}
private:
	void CopyModuleValueAsLink() {
		auto segment = data_->parent;
		auto mod = segment->parent_module.fetch();

		auto type = data_->parent->GetType();
		auto factory = CBlockFactory();
		npsys::fbd_block_n block;
		if (type == io::SegmentType::WRITE) {
			factory.CreateOutput(block);
			auto output = static_cast<COutput*>(block->e_block.get());
			output->SetParamType(PARAMETER_TYPE::P_EXTERNAL_REF);
			output->GetSlot()->SetSlotType(
				new CModuleValue(mod,
					data_->GetId(),
					npsys::remote::ExtLinkType::Write)
			);
			output->GetSlot()->CommitTypeChanges();
		} else if (type == io::SegmentType::READ) {
			factory.CreateInput(block);
			auto input = static_cast<CInput*>(block->e_block.get());
			input->SetParamType(PARAMETER_TYPE::P_EXTERNAL_REF);
			input->GetSlot()->SetSlotType(
				new CModuleValue(mod,
					data_->GetId(),
					npsys::remote::ExtLinkType::Read)
			);;
			input->GetSlot()->CommitTypeChanges();
		} else {
			return;
		}

		auto elems = std::make_unique<CBlockCompositionWrapper>();
		elems->Push(block->e_block.get());

		omembuf omf;
		{
			boost::archive::binary_oarchive_special oa(omf);
			oa << elems;
		}

		if (!::OpenClipboard(m_treeview)) return;
		::EmptyClipboard();

		auto len = omf.length();
		HGLOBAL hMem = ::GlobalAlloc(GMEM_MOVEABLE, len);
		ASSERT(hMem);

		void* ptr = ::GlobalLock(hMem);
		memcpy(ptr, omf.c_str(), len);
		::GlobalUnlock(hMem);

		::SetClipboardData(g_clf_blocks, hMem);
		::CloseClipboard();
	}
};

class CTreeAssignedSegment : public CTemplateItemHardwareStatus
	<
		CTreeSegmentT<
			ItemListContainer<CTreeAssignedSegment, 
			std::vector, 
			CTreeAssignedSegmentValue,
			CategoryElement<npsys::CI2CModuleSegment, npsys::CAssignedI2CModule::list_t>
	>	>, CTreeAssignedSegment> 
{
	using base = CTemplateItemHardwareStatus<CTreeSegmentT<
		ItemListContainer<CTreeAssignedSegment,std::vector, CTreeAssignedSegmentValue,
		CategoryElement<npsys::CI2CModuleSegment, npsys::CAssignedI2CModule::list_t>>>, CTreeAssignedSegment>;
public:
	CTreeAssignedSegment(npsys::CI2CModuleSegment* data, npsys::CAssignedI2CModule::list_t& list)
		: base(data, list) {}

	void HandleRequest(REQUEST* req) noexcept {
		switch (req->ID_MENU) {
		case ID_CREATE_ITEM:
			__super::HandleRequest(req);
			CalcAndUpdateHardwareStatusAll();
			break;
		default:
			__super::HandleRequest(req);
		}
	}
};

class CTreeI2CAssignedModule
	: public CTemplateItemHardwareStatus<
	CTreeModuleT<CTreeI2CAssignedModule, CTreeAssignedSegment, npsys::i2c_assigned_module_n, npsys::i2c_assigned_module_l>,
	CTreeI2CAssignedModule
	> {
public:
	CTreeI2CAssignedModule(npsys::i2c_assigned_module_n& n, npsys::i2c_assigned_module_l& l, npsys::controller_n& ctrl)
		: CTemplateItemHardwareStatus(n, l)
		, ctrl_(ctrl) {
		SetIcon(AlphaIcon::Module);
	}

	void ConstructMenu(CMenu* menu) noexcept {
		menu->AppendMenu(MF_STRING, ID_UPLOAD_MODULE, L"Upload");
		menu->AppendMenu(MF_SEPARATOR);
		__super::ConstructMenu(menu);
	}

	void HandleRequest(REQUEST* req) noexcept {
		switch (req->ID_MENU) {
		case ID_UPLOAD_MODULE:
		{
			CDlg_LoadingManager dlg(
				std::make_unique<CDlg_LoadingManager::CTaskUploadOneI2CModule>(ctrl_, n_)
			);
			dlg.DoModal(g_hMainWnd);
			break;
		}
		default:
			__super::HandleRequest(req);
		}
	}
protected:
	npsys::controller_n& ctrl_;
};

class CTreeAssignedI2CModulesCat : public CTemplateItemHardwareStatus<
	ItemListContainer<CTreeAssignedI2CModulesCat, std::vector, CTreeItemAbstract, FixedElement, Manual>,
	CTreeAssignedI2CModulesCat> {
public:
	CTreeAssignedI2CModulesCat(npsys::controller_n& ctrl)
		: ctrl_(ctrl) {
		SetIcon(AlphaIcon::Container);
		name_ = "I2C";
		LoadChilds();
	}
	virtual oid_t GetId() const noexcept final {
		return ctrl_.id() + constants::ASSIGNED_I2C_MODULES_INC;
	}
	virtual bool IsRemovable() { return false; }
protected:
	npsys::controller_n& ctrl_;
	virtual void LoadChildsImpl() noexcept {
		ctrl_->assigned_modules.fetch_all_nodes();
		for (auto& mod : ctrl_->assigned_modules) {
			insert(new CTreeI2CAssignedModule(mod, ctrl_->assigned_modules, ctrl_));
		}
	}
};

class CTreeAssignedModulesCat : public CTemplateItemHardwareStatus<
	ItemListContainer<CTreeAssignedModulesCat, std::vector, CTreeItemAbstract, FixedElement, Manual>,
	CTreeAssignedModulesCat> {
public:
	CTreeAssignedModulesCat(npsys::controller_n& ctrl) : ctrl_(ctrl) {
		SetIcon(AlphaIcon::Container);
		name_ = "ASSIGNED MODULES";
		LoadChilds();
	}
	virtual oid_t GetId() const noexcept final {
		return ctrl_.id() + constants::ASSIGNED_MODULES_INC;
	}
	virtual bool IsRemovable() { return false; }
protected:
	npsys::controller_n& ctrl_;
	virtual void LoadChildsImpl() noexcept {
		insert(new CTreeAssignedI2CModulesCat(ctrl_));
	}
};

class CTreeAssignedAlgorithm :
	public CTemplateItemHardwareStatus<
	CTemplateItem<CTreeAssignedAlgorithm, CTreeItemAbstract, DatabaseElement<npsys::assigned_algorithm_n, npsys::assigned_algorithms_l>>,
	CTreeAssignedAlgorithm> {
public:
	CTreeAssignedAlgorithm(npsys::assigned_algorithm_n& n, npsys::assigned_algorithms_l& l)
		: CTemplateItemHardwareStatus(n, l) {
		SetIcon(AlphaIcon::Algorithm);
		n_->item = this;
		ASSERT_FETCH(n_->alg);
	}
	virtual bool SetName(const std::string& name) { return false; }
	void ConstructMenu(CMenu* menu) noexcept {
		UINT loaded = (n_->alg->GetStatus() >= npsys::CAlgorithm::status_loaded) ? 0xFFFFFFFF : 0;
		menu->AppendMenu(MF_STRING, _ID_UPLOAD_ALGORITHM_, L"Upload");
		menu->AppendMenu(MF_STRING | (MF_DISABLED & ~loaded), _ID_UNLOAD_ALGORITHM_, L"Unload");
		menu->AppendMenu(MF_STRING | (MF_DISABLED & ~loaded), ID_STOP_ALGORITHM, L"Stop Algorithm");
		menu->AppendMenu(MF_SEPARATOR);
		menu->AppendMenu(MF_STRING | (MF_DISABLED & loaded), ID_ITEM_DELETE, L"Delete");
	}
	void HandleRequest(REQUEST* req) noexcept {
		switch (req->ID_MENU)
		{
		case _ID_UPLOAD_ALGORITHM_:
		{
			if (!n_->alg.fetch()) {
				std::cout << "Error: algorithm could not be found..." << std::endl;
			}
			n_->alg->UploadToDevice(true);
			break;
		}
		case _ID_UNLOAD_ALGORITHM_:
		{
			if (!n_->alg.fetch()) {
				std::cout << "Error: algorithm could not be found..." << std::endl;
			}
			n_->alg->UnloadFromDevice();
			break;
		}
		case ID_STOP_ALGORITHM:
		{
			auto avr = n_->dev.fetch().cast<npsys::controller_n_avr>();
			int addr = static_cast<npsys::CAVRAssignedAlgorithm*>(n_.get())->Start()
				* avr->GetFirmwareInfo().pmem.pagesize / 2;
			try {
				if (avr->StopAlgorithm(addr) == true) {
					MessageBoxA(g_hMainWnd, "Algorithm has been stopped", "", MB_ICONINFORMATION);
				} else {
					MessageBoxA(g_hMainWnd, "Algorithm has not been found", "", MB_ICONWARNING);
				}
			} catch (nps::NPNetCommunicationError& ex) {
				std::cout << "Communication failed: " << TranslateError(ex.code).data() << std::endl;
			} catch (std::exception& ex) {
				std::cout << "Caught Exception: " << ex.what()<< std::endl;
			}
			break;
		}
		default:
			__super::HandleRequest(req);
		}
	}
	virtual bool IsRemovable() {
		bool ok = (n_->alg->GetStatus() == npsys::CAlgorithm::status_assigned);
		if (!ok) MessageBoxA(g_hMainWnd, "Unable to delete assigned algorithm:\r\nUnload from controller first.",
			"", MB_ICONINFORMATION);
		return ok;
	}
	io::Status ConvertStatus() {
		using Status = npsys::CAlgorithm::Status;
		auto status = n_->alg->GetStatus();
		switch (status)
		{
		case Status::status_not_assigned:
			ASSERT(FALSE);
			return io::Status::assigned;
		case Status::status_assigned:
		case Status::status_loaded:
		case Status::status_not_actual:
			return static_cast<io::Status>(status - 1);
		default:
			ASSERT(FALSE);
			return io::Status::assigned;
		}
	}
protected:
	virtual void PostDelete(CContainer* parent) noexcept {
		auto dev = n_->dev.fetch();
		n_->alg->SetStatus(npsys::CAlgorithm::status_not_assigned);
		n_->alg->assigned_alg = {};
		n_->alg->assigned_dev = {};
		n_->alg->HardwareSpecific_Clear();
		n_->alg.store();
		__super::PostDelete(parent);
	}
};

class CTreeAssignedAlgorithmCat :
	public CTemplateItemHardwareStatus<
	ItemListContainer<CTreeAssignedAlgorithmCat, std::vector, CTreeAssignedAlgorithm, FixedElement, Manual>,
	CTreeAssignedAlgorithmCat> {
public:
	CTreeAssignedAlgorithmCat(npsys::controller_n& ctrl)
		: ctrl_(ctrl) {
		SetIcon(AlphaIcon::Container);
		name_ = "ASSIGNED ALGORIHMS";
		LoadChilds();
	}

	virtual COnlineTreeItem* CreateOnlineFromThis(std::vector<std::unique_ptr<COnlineTreeItem>>& nodes, CTreeViewCtrl tree, HTREEITEM hParent) {
		return nullptr;
	}

	virtual CItemView* CreateView(CMyTabView& tabview) final {
		return CreateViewForItem<CListControlView>(this, tabview);
	};
	virtual CViewAdapter* GetViewAdapter() {
		CViewAdapter* pAdapter = new CViewAdapter(this);

		pAdapter->m_columns.emplace_back(L"Object","%s",40 );
		pAdapter->m_columns.emplace_back(L"Address","0x%.4x",15 );
		pAdapter->m_columns.emplace_back(L"First page","%d",15 );
		pAdapter->m_columns.emplace_back(L"End page","%d",15 );
		pAdapter->m_columns.emplace_back(L"Size","%d",15 );

		LoadChilds();
		auto avr = ctrl_.cast<npsys::controller_n_avr>();

		for (auto& item : m_items) {
			auto assigned = item->n_.cast<npsys::avr_assigned_algorithm_n>();
			pAdapter->m_data.emplace_back(
				(LPARAM)item,
				(LPARAM)item->GetName().c_str(),
				avr->PageToByte(assigned->Start()),
				assigned->Start(),
				assigned->End(),
				assigned->AllocSizeP()
			);
		}
		return pAdapter;
	}
	virtual void ConstructMenu(CMenu* menu) noexcept {
		if (ctrl_->firmware_status
			== npsys::CController::FIRMWARE_LOADED_DEVICE_ACTIVATED) {
			menu->AppendMenu(MF_STRING, ID_UPLOAD_ALL_ALGORITHMS, L"Upload Algorithms");
		}
	}
	virtual void HandleRequest(REQUEST* req) noexcept {
		switch (req->ID_MENU) {
		case ID_UPLOAD_ALL_ALGORITHMS:
		{
			Configurator cfg(ctrl_);
			cfg.UploadAllAlgoritms();
			break;
		}
		default:
			break;
		}
	}
	virtual bool IsRemovable() { return false; }
protected:
	virtual void LoadChildsImpl() noexcept {
		ctrl_->assigned_algs.fetch_all_nodes();
		for (auto& i : ctrl_->assigned_algs) {
			insert(new CTreeAssignedAlgorithm(i, ctrl_->assigned_algs));
		}
	}
	virtual oid_t GetId() const noexcept final {
		return ctrl_.id() + constants::ASSIGNED_ALGORITHMS_INC;
	}
	npsys::controller_n& ctrl_;
};

#include "tr_controller.h"


CTreeController::CTreeController(npsys::controller_n& n, npsys::controllers_l& l)
	: CTemplateItemHardwareStatus(n, l) {
	SetIcon(AlphaIcon::Controller_nl);
	n_->item = this;
	LoadChilds();
}

void CTreeController::ConstructMenu(CMenu* menu) noexcept {
	n_->ConstructMenu(menu);
}

CItemView* CTreeController::CreateView(CMyTabView& tabview) {
	return CreateViewForItem<CControllerView>(this, tabview);
};

void CTreeController::HandleRequest(REQUEST* req) noexcept {
	if (!n_->HandleRequest(req)) __super::HandleRequest(req);
}

CTreeHardware* CTreeController::GetHardware() noexcept {
	LoadChilds();
	return static_cast<CTreeHardware*>(m_items[constants::HARDWARE_INC - 1]);
}

CTreeAssignedAlgorithmCat* CTreeController::GetSoftware() noexcept {
	LoadChilds();
	return static_cast<CTreeAssignedAlgorithmCat*>(m_items[constants::ASSIGNED_ALGORITHMS_INC - 1]);
}

CTreeAssignedI2CModulesCat* CTreeController::GetI2CModules() noexcept {
	LoadChilds();
	auto modules = static_cast<CTreeAssignedModulesCat*>(m_items[constants::ASSIGNED_MODULES_INC - 1]);
	modules->LoadChilds();
	auto i2c = static_cast<CTreeAssignedI2CModulesCat*>(
		modules->GetElementById(n_.id() + constants::ASSIGNED_I2C_MODULES_INC));
	ASSERT(i2c);
	i2c->LoadChilds();
	return i2c;
}

int	CTreeController::Move(CTreeAlgorithm* alg, int action) {
	if (!action) return true;
	auto linker = n_->CreateLinker();
	auto ok = linker->AssignAlgorithm(alg->n_);
	if (ok) {
		auto last = *(n_->assigned_algs.end() - 1);
		auto software = GetSoftware();
		if (software->IsChildsLoaded() == true) {
			software->AddItem(new CTreeAssignedAlgorithm(last, n_->assigned_algs));
			CalcAndUpdateHardwareStatus();
			m_treeview.Invalidate();
		} else {
			m_treeview.Expand(GetHTR(), TVE_EXPAND);
			m_treeview.Expand(software->GetHTR(), TVE_EXPAND);
		}
	}
	return ok;
}

int	CTreeController::Move(CTreeI2CModule* mod, int action) {
	if (!action) return true;
	auto linker = n_->CreateLinker();
	npsys::i2c_assigned_module_n assigned;
	auto ok = linker->AssignI2CModule(mod->n_, assigned);
	if (ok) {
		GetI2CModules()->AddItem(new CTreeI2CAssignedModule(assigned, n_->assigned_modules, n_));
		CalcAndUpdateHardwareStatus();
		m_treeview.Invalidate();
	}
	return ok;
}

INT_PTR CTreeController::ShowProperties() { return n_->ShowProperties(); }

void CTreeController::LoadChildsImpl() noexcept {
	insert(n_->CreateHardware(n_));
	insert(new CTreeAssignedAlgorithmCat(n_));
	insert(new CTreeAssignedModulesCat(n_));
}

void CTreeController::PostDelete(CContainer* parent) noexcept {
	if (n_->firmware_status >= npsys::CController::FIRMWARE_LOADED_DEVICE_ACTIVATED) {
		try {
			if (npsys_rpc->server->Notify_DeviceDeactivated(n_.id())) n_.remove();
		} catch (::nprpc::ExceptionObjectNotExist&) {
			std::cerr << "Unable to perform the operation. Object was destroyed\n";
			return;
		} catch (::nprpc::ExceptionTimeout&) {
			std::cerr << "No connection with server. Unable to perform the operation\n";
			return;
		} catch (::nprpc::Exception& ex) {
			std::cerr << ex.what() << '\n';
		}
	}
	__super::PostDelete(parent);
}

bool CTreeController::IsRemovable() {
	return n_->status == 0;
}

COnlineTreeItem* CTreeController::CreateOnlineFromThis(std::vector<std::unique_ptr<COnlineTreeItem>>& nodes, CTreeViewCtrl tree, HTREEITEM hParent) {
	auto ptr = __super::CreateOnlineFromThis(nodes, tree, hParent);
	n_->CreateOnlineCustomItems(tree, ptr->htr, nodes);
	return ptr;
}

// CTreeNetwork

oid_t CTreeNetwork::GetId() const noexcept {
	return npsys::NETWORK_ID_;
}

void CTreeNetwork::LoadChildsImpl() noexcept {
	controllers_.fetch_all_nodes();
	for (auto& i : controllers_) {
		insert(new CTreeController(i, controllers_));
	}
}

void CTreeNetwork::HandleRequest(REQUEST* req) noexcept {
	switch (req->ID_MENU) {
	case ID_CREATE_ITEM: {
		std::string type;
		CDlg_NewController dlg(type);
		if (dlg.DoModal() != IDOK) break;
		AppendController(type);
		break;
	}
	case ID_UPLOAD_FIRMWARE:
		// UnAdvise();
		// Advise();
		// g_pSystem->m_pPNetwork->CalcAndUpdateHardwareStatus();
		// m_treeview.Invalidate();
		break;
	default:
		__super::HandleRequest(req);
	}
}

void CTreeNetwork::ConstructMenu(CMenu* menu) noexcept {
	menu->AppendMenu(MF_STRING, ID_CREATE_ITEM, L"New Controller");
}

void CTreeNetwork::AppendController(const std::string& type) {
	using npsys::CController;
	using npsys::controller_n;

	oid_t id = odb::Database::get()->next_oid(30);
	odb::Batch batch;
	controller_n ctrl;
	if (type == "atmega8") {
		ctrl = CreateAvrController(controllers_, id, type, avrinfo::Model::ATMEGA8, false);
	} else if (type == "atmega16") {
		ctrl = CreateAvrController(controllers_,id, type, avrinfo::Model::ATMEGA16, false);
	} else if (type == "atmega64") {

	} else if (type == "atmega8_virtual") {
		ctrl = CreateAvrController(controllers_,id, type, avrinfo::Model::ATMEGA8_VIRTUAL, true);
	} else if (type == "atmega16_virtual") {
		ctrl = CreateAvrController(controllers_,id, type, avrinfo::Model::ATMEGA16_VIRTUAL, true);
	} else if (type == "atmega64_virtual") {

	} else {
		ASSERT(FALSE);
		std::cerr << "Unknown device type...\n";
		return;
	}

	controllers_.add_node(ctrl);
	controllers_.store();

	batch.exec();

	AddItem(new CTreeController(ctrl, controllers_));
	CalcAndUpdateHardwareStatus();
}

npsys::controller_n CreateAvrController(
	npsys::controllers_l& controllers,
	oid_t controller_id,
	const std::string& type,
	const avrinfo::Model model,
	bool virt)
{
	using npsys::CController;
	using npsys::CAVRController;
	using npsys::controller_n;
	using npsys::controller_n_avr;
	int dev_addr;

	auto const& pinfo = avrinfo::AVRInfo::get_instance().GetPeripheralInfo(model);
	auto const& info = avrinfo::AVRInfo::get_instance().GetLatestInfo(model);

	std::vector<int> addresses;
	if (controllers.size() != 0) {
		std::for_each(controllers.begin(), controllers.end(), [&addresses](const auto& c) {
			addresses.push_back(c->dev_addr);
		});

		std::sort(addresses.begin(), addresses.end());

		if (addresses.size() == 1) {
			dev_addr = (addresses[0] == 3 ? 4 : 3);
		} else {
			if (addresses[0] != 3) {
				dev_addr = 3;
			} else {
				dev_addr = addresses[addresses.size() - 1] + 1;
				for (size_t ix = 0; ix < addresses.size() - 1; ++ix) {
					if (addresses[ix] + 1 != addresses[ix + 1]) {
						dev_addr = addresses[ix] + 1;
						break;
					}
				}
			}
		}
	} else {
		dev_addr = 3;
	}

	controller_n_avr avr;
	avr.create_locally_with_id(controller_id,
		new CAVRController("NEW_CONTROLLER", type, dev_addr, model, info.version, virt));

	const char* letter = pinfo.port;
	const std::string port_s = "PORT";
	int ix_pin = 0;

	do {
		auto port = *avr->ports.add_value(new npsys::CAVRPort(port_s + *letter, avr));
		for (; pinfo.pins_cfg[ix_pin].port_letter == *letter &&
			pinfo.pins_cfg[ix_pin].port_letter; ix_pin++) {
			int bit = static_cast<int>(pinfo.pins_cfg[ix_pin].pin_n);
			auto var = odb::create_node<npsys::variable_n>(
				npsys::variable::IO_SPACE | npsys::variable::VT_DISCRETE);
			var->SetDev(avr.cast<npsys::device_n>());
			port->pins_.add_value(new npsys::CAVRPin(
				"P" + std::string(1, *letter) + std::to_string(bit),
				avr, port, var, bit));
			var.store();
		}
		port->pins_.store();
	} while (*++letter);
	avr.store();
	return avr.cast<controller_n>();
}

