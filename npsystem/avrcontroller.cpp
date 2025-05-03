// Copyright (c) 2021 nikitapnn1@gmail.com
// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory

#include "stdafx.h"
#include "network_ext.h"
#include "control_unit_ext.h"
#include "stuff.h"
#include "error.h"
#include "global.h"
#include "avrconfigurator.h"
#include "tr_request.h"
#include "avrcommand.h"
#include "dlgavrmemoryviewer.h"
#include "dlgloadingmanager.h"
#include <avr_firmware/net.h>
#include <avr_firmware/core.h>
#include <avr_firmware/twi.h>
#include "tr_item.h"
//#include "dlgavrcontrollerproperties.h"
#include "dlgpinconfig.h"
#include "element.h"
#include "assignedalgorithm.h"
#include "genhelper.h"
#include <npdb/find.h>
#include "exception.h"
#include "config.h"
#include "avrassigned.h"
#include "io_avr.h"

extern UINT g_clf_blocks;

class CTreeAvrPin : public CTemplateItemHardwareStatus<
	CTemplateItem<CTreeAvrPin, CTreeItemAbstract, DatabaseElement<npsys::avr_pin_n>>,
	CTreeAvrPin> {
public:
	CTreeAvrPin(npsys::avr_pin_n& n)
		: CTemplateItemHardwareStatus(n) {
		n->item = this;
		SetIcon(n->GetIcon());
	}

	void ConstructMenu(CMenu* menu) noexcept {
		menu->AppendMenu(MF_STRING, ID_ITEM_PROPERTIES, L"Propetries");
		menu->AppendMenu(MF_STRING, ID_CREATE_LINK_FROM_PIN, L"Copy As Link");
	};

	virtual INT_PTR ShowProperties() {
		if (true || n_->GetVariable()->RefCount() == 0) {
			CDlg_AvrPinConfig dlg(n_);
			auto result = dlg.DoModal();
			if (result == IDOK) SetIcon(n_->GetIcon());
			return result;
		} else {
			MessageBoxA(g_hMainWnd, "The pin is being used.\nUnable to change configuration.",
				"", MB_ICONINFORMATION);
		}
		return IDCANCEL;
	}

	virtual void HandleRequest(REQUEST* req) noexcept {
		switch (req->ID_MENU) {
		case ID_CREATE_LINK_FROM_PIN:
		{
			CopyPinAsLink();
			break;
		}
		default:
			__super::HandleRequest(req);
			break;
		};
	}
	io::Status ConvertStatus() {
		return n_->GetStatus();
	}
	virtual bool IsRemovable() { return false; }
private:
	void CopyPinAsLink() {
		auto type = n_->GetPinType();
		auto factory = CFBDBlockFactory(nullptr);
		npsys::fbd_block_n block;
		if (type == io::PINTYPE::OUTPUT) {
			factory.CreateOutput(block);
			auto output = static_cast<COutput*>(block->e_block.get());
			output->SetParamType(PARAMETER_TYPE::P_EXTERNAL_REF);
			output->GetSlot()->SetSlotType(new CAvrInternalPin(n_, npsys::remote::ExtLinkType::Write, {}));
			output->GetSlot()->CommitTypeChanges();
		} else if (type == io::PINTYPE::INPUT) {
			factory.CreateInput(block);
			auto input = static_cast<CInput*>(block->e_block.get());
			input->SetParamType(PARAMETER_TYPE::P_EXTERNAL_REF);
			input->GetSlot()->SetSlotType(new CAvrInternalPin(n_, npsys::remote::ExtLinkType::Read, {}));
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
		assert(hMem);

		void* ptr = ::GlobalLock(hMem);
		memcpy(ptr, omf.c_str(), len);
		::GlobalUnlock(hMem);

		::SetClipboardData(g_clf_blocks, hMem);
		::CloseClipboard();
	}
};

class CTreeAvrPort : public CTemplateItemHardwareStatus<
	ItemListContainer<CTreeAvrPort, std::vector, CTreeAvrPin, DatabaseElement<npsys::avr_port_n>, Manual>,
	CTreeAvrPort> {
public:
	CTreeAvrPort(npsys::avr_port_n& n)
		: CTemplateItemHardwareStatus(n) {
		SetIcon(NPSystemIcon::Io);
		LoadChilds();
	}
	virtual bool IsRemovable() { return false; }
protected:
	virtual void LoadChildsImpl() noexcept {
		auto port_ptr = n_.get();
		if (port_ptr->pins_.fetch_all_nodes()) {
			for (auto& i : port_ptr->pins_) {
				insert(new CTreeAvrPin(i));
			}
		}
	}
};

// CTreeIO
class CTreeAvrIO : public CTemplateItemHardwareStatus<
	ItemListContainer<CTreeAvrIO, std::vector, CTreeAvrPort, FixedElement, Manual>, CTreeAvrIO> {
public:
	CTreeAvrIO(npsys::controller_n& ctrl) : ctrl_(ctrl) {
		SetIcon(NPSystemIcon::Io);
		name_ = "IO";
		LoadChilds();
	}
	virtual void ConstructMenu(CMenu* menu) noexcept {
		ctrl_->IoConstructMenu(menu);
	}
	virtual void HandleRequest(REQUEST* req) noexcept {
		if (!ctrl_->IoHandleRequest(req)) {
			__super::HandleRequest(req);
		}
	}
	bool DownloadIO() {}
	virtual oid_t GetId() const noexcept final {
		return ctrl_.id() + constants::IO_INC;
	}
	virtual bool IsRemovable() { return false; }
protected:
	npsys::controller_n& ctrl_;
	virtual void LoadChildsImpl() noexcept {
		auto avr = static_cast<npsys::CAVRController*>(ctrl_.get());
		avr->ports.fetch_all_nodes();
		for (auto& i : avr->ports) {
			insert(new CTreeAvrPort(i));
		}
	}
};

class CTreeAvrHardware : public CTreeHardware {
public:
	CTreeAvrHardware(npsys::controller_n& ctrl)
		: CTreeHardware(ctrl) {
		LoadChilds();
	}
protected:
	virtual void LoadChildsImpl() noexcept {
		insert(new CTreeAvrIO(ctrl_));
	}
};


namespace npsys {

bool CNetworkDevice::ActivateDevice(oid_t id) noexcept {
	try {
		return npsys_rpc->server->Notify_DeviceActivated(id);
	} catch (nprpc::Exception& ex) {
		std::cerr << ex.what() << '\n';
	}
	return false;
}

std::vector<uint16_t> CAVRController::GetTaskTable() const {
	std::vector<uint16_t> result;
	uint8_t nTask, nb;
	uint16_t tt_addr = GetFirmwareInfo().rmem.tt;
	
	npsys_rpc->server->ReadByte(dev_addr, tt_addr, nTask);
	nb = nTask * sizeof(task_t);

	std::vector<uint8_t> data;
	npsys_rpc->server->ReadBlock(dev_addr, tt_addr + 2, nb, data);

	task_t* t = reinterpret_cast<task_t*>(&data[0]);
	for (int i = 0; i < nTask; ++i) {
		result.push_back(t[i].addr);
	}

	return result;
}

void CAVRController::ShowTaskTable() const noexcept {
	try {
		auto tt = GetTaskTable();
		std::string str = "N = " + std::to_string(tt.size()) + "\r\n";
		for (size_t i = 0; i < tt.size(); ++i) {
			str += "T" + std::to_string(i) + " = " + to_hex(static_cast<uint16_t>(tt[i] * 2)) + "\r\n";
		}
		MessageBoxA(g_hMainWnd, str.c_str(), "Task table", 0);
	} catch (nps::NPNetCommunicationError& ex) {
		std::cerr << "Communication failed: " << TranslateError(ex.code).data() << '\n';
	} catch (nprpc::Exception& ex) {
		std::cerr << "NPRPC Exception: " << ex.what() << '\n';
	}
}

void CAVRController::ShowRemoteTable() const noexcept {
	try {
		const auto& info = GetFirmwareInfo();
		uint16_t rd_addr = info.rmem.rd;

		uint8_t nb = static_cast<uint8_t>(info.mccfg.rdata_max * sizeof(net::remote_data));

		std::vector<uint8_t> data;
		npsys_rpc->server->ReadBlock(dev_addr, rd_addr, nb, data);

		net::remote_data* rd = reinterpret_cast<net::remote_data*>(&data[0]);
		MessageBoxA(g_hMainWnd, PrintRemoteTable(rd).c_str(), "Remote Table", 0);
	} catch (nps::NPNetCommunicationError& ex) {
		std::cerr << "Communication failed: " << TranslateError(ex.code).data() << '\n';
	} catch (nprpc::Exception& ex) {
		std::cerr << "NPRPC Error: " << ex.what() << '\n';
	}
}
void CAVRController::ShowTwiTable() const noexcept {
	try {
		const auto& info = GetFirmwareInfo();
		auto const twi_addr = info.rmem.twi;
		uint8_t n;

		npsys_rpc->server->ReadByte(dev_addr, twi_addr, n);

		if (!n) {
			MessageBoxA(g_hMainWnd, "Empty", "Twi Table", 0);
			return;
		}

		uint8_t len = n * sizeof(twi_req_t);

		std::vector<uint8_t> data;
		npsys_rpc->server->ReadBlock(dev_addr, twi_addr, len, data);

		twitab_t* tab = reinterpret_cast<twitab_t*>(&data[0]);
		MessageBoxA(g_hMainWnd, PrintTwiTable(tab).c_str(), "Twi Table", 0);
	} catch (nps::NPNetCommunicationError& ex) {
		std::cerr << "Communication failed: " << TranslateError(ex.code).data() << '\n';
	} catch (nprpc::Exception& ex) {
		std::cerr << "NPRPC Error: " << ex.what() << '\n';
	}
}

std::string CAVRController::PrintRemoteTable(net::remote_data* data) const {
	if (!data[0].u32Val)
		return "REMOTE TABLE: no data";

	int req = 0;
	std::string str = "\r\nREMOTE TABLE:\r\nReq #" + std::to_string(req) + "\r\n";
	for (int i = 0; ; ++i) {
		if (data[i].val.is_bit) {
			str += string_format(
				"\t| to: %.2d | laddr: %.4d | raddr %.4d | bitN: %d | mar: %d |\r\n",
				data[i].val.device_addr, data[i].val.local_addr,
				data[i].val.remote_addr, data[i].val.bit_size, data[i].val.marker);
		} else {
			str += string_format(
				"\t| to: %.2d | laddr: %.4d | raddr %.4d | size: %d | mar: %d |\r\n",
				data[i].val.device_addr, data[i].val.local_addr,
				data[i].val.remote_addr, data[i].val.bit_size, data[i].val.marker);
		}
		if (data[i].val.marker == M_LAST_DATA_IN_REQUEST) {
			str += string_format("Req #%d\r\n", ++req);
		}
		if (data[i].val.marker == M_ARRAY_END) {
			break;
		}
	}
	return str;
}

std::string CAVRController::PrintTwiTable(twitab_t* tab) const {
	if (!tab->n) return "TWI TABLE: no data";

	std::stringstream ss;

	ss << "\r\nTWI TABLE (" << (int)tab->n << "):\r\n";
	twi_req_t* r = tab->twi_req;
	for (int i = 0; i < tab->n; ++i) {
		if (r[i].fn == TWI_REQ_WRITE) {
			ss << "\tWrite:\r\n";
		} else if (r[i].fn == TWI_REQ_WRITE_BLOCK) {
			ss << "\tWrite Block:\r\n";
		} else if (r[i].fn == TWI_REQ_READ) {
			ss << "\tRead Block:\r\n";
		} else if (r[i].fn == TWI_REQ_READ_BLOCK) {
			ss << "\tRead Block:\r\n";
		}
		ss << "\t\ttwi_addr: " << std::hex << (int)r[i].dev_addr << "\r\n";
		ss << "\t\tsl_mem_addr: " << std::hex << (int)(r[i].addr & 0x7fff) << "\r\n";
		ss << "\t\tmem_addr: " << std::hex << (int)(r[i].data) << "\r\n";
		ss << "\t\tlen: " << std::dec << (int)r[i].len << "\r\n";
	}
	return ss.str();
}

bool CAVRController::SetActivated() noexcept {
	auto avr = self_node.fetch();
	if (!ActivateDevice(avr.id())) return false;
	npsys::access::rw(firmware_status) = npsys::CController::FIRMWARE_LOADED_DEVICE_ACTIVATED;
	std::cerr << "Device has been activated!\n";
	avr.store();
	return true;
}

bool CAVRController::LoadFirmware() noexcept {
	std::string fuses;
	
	switch (controller_model) {
	case npsys::hardware::Model::ATMEGA8:
		fuses = " -U lfuse:w:0x2f:m -U hfuse:w:0xc9:m";
		break;
	case npsys::hardware::Model::ATMEGA16:
		fuses = " -U lfuse:w:0x2f:m -U hfuse:w:0xc9:m";
		break;
	default:
		assert(false);
		break;
	}

	// -c usbasp -p m8 -e -U flash:w:..\\atmega8\\build\\atmega8_4.hex:i
	// -U eeprom:w:..\\atmega16\\build\\atmega16_2.eep:i
	global.ChangeCurrentDirectory(CurrentDirectory::MODULE);
	std::string hex = "data\\avr\\firmware\\" + controller_type + "_" + std::to_string(dev_addr) + ".hex";
	std::string eeprom = "data\\avr\\firmware\\" + controller_type + "_" + std::to_string(dev_addr) + ".eep";
	std::string cmd = g_cfg.avrdude_exe + " -c " + g_cfg.avr_programmer + " -p " + GetFirmwareInfo().mccfg.partno + fuses +
		" -e -U flash:w:\"" + hex + "\":i -U eeprom:w:\"" + eeprom + "\":i ";
	
	bool ok = (CreateProccessWindowlessA(cmd, global.GetModuleDir(), std::cout) == 0) ? true : false;
	std::cout << std::endl;

	if (!ok) return false;

	npsys::access::rw(firmware_status) = FIRMWARE_LOADED;
	npsys::access::rw(version) = avrinfo::AVRInfo::get_instance().GetLatestInfo(controller_model).version;

	return SetActivated();
}

std::unique_ptr<CDynamicLinker>
CAVRController::CreateLinker() {
	return std::make_unique<CAVR5DynamicLinker>(
		self_node.fetch().cast<npsys::controller_n_avr>()
		);
}

INT_PTR CAVRController::ShowProperties() noexcept {
	//	CDlg_AVRControllerProperties dlg(this);
	//	return dlg.DoModal();
	return IDCANCEL;
}

void CAVRController::ConstructMenu(CMenu* menu) noexcept {
	if (firmware_status == FIRMWARE_NOT_LOADED) {
		if (!IsVirtual()) {
			menu->AppendMenu(MF_STRING, ID_UPLOAD_FIRMWARE, L"Upload Firmware");
		}
		menu->AppendMenu(MF_STRING, ID_ACTIVATE_DEVICE, L"Activate");
		menu->AppendMenu(MF_STRING, ID_ITEM_DELETE, L"Delete");
	} else {
		if (IsVirtual()) {
			menu->AppendMenu(MF_STRING | (IsFlashViewerWindow() ? MF_DISABLED : 0), ID_SHOW_AVR_MEMORY, L"Show Memory");
			menu->AppendMenu(MF_STRING, ID_VIRTUAL_START, L"Start");
			menu->AppendMenu(MF_STRING, ID_VIRTUAL_STOP, L"Stop");
			menu->AppendMenu(MF_SEPARATOR);
		}
		menu->AppendMenu(MF_STRING, ID_UPLOAD_CONTROLLER, L"Upload");
		menu->AppendMenu(MF_STRING, ID_UPLOAD_IO, L"Upload Io");
		menu->AppendMenu(MF_STRING, ID_ITEM_DELETE, L"Delete");
#ifdef DEBUG
		menu->AppendMenu(MF_SEPARATOR);
		menu->AppendMenu(MF_STRING, ID_DELETE_RD, L"Delete Remote Data");
		menu->AppendMenu(MF_STRING, ID_DELETE_TT, L"Delete Task Table");
		menu->AppendMenu(MF_STRING, ID_DELETE_I2C, L"Delete I2C Table");
		menu->AppendMenu(MF_STRING, ID_SHOW_INTERNAL_DATA, L"Show internal data");
#endif // DEBUG
		menu->AppendMenu(MF_SEPARATOR);
		menu->AppendMenu(MF_STRING, ID_SHOW_TT, L"Task table");
		menu->AppendMenu(MF_STRING, ID_SHOW_RD, L"Remote table");
		menu->AppendMenu(MF_STRING, ID_SHOW_TWI, L"Twi table");
		menu->AppendMenu(MF_STRING, ID_DELETE_ADDITIONAL_LIBS, L"Delete additional libraries");
	}
}

bool CAVRController::HandleRequest(REQUEST* req) noexcept {
	switch (req->ID_MENU) {
	case ID_SHOW_AVR_MEMORY:
		assert(IsFlashViewerWindow() == false);
		flash_viewer_.reset();
		try {
			flash_viewer_.reset(new Dlg_AVRMemoryViewer(id(), GetFirmwareInfo().pmem.pagesize));
			if (flash_viewer_->Create(g_hMainWnd)) {
				flash_viewer_->SetWindowText(wide("FLASH: " + name_).c_str());
				flash_viewer_->ShowWindow(SW_SHOWNORMAL);
			} else {
				flash_viewer_.reset();
			}
		} catch (::nprpc::Exception& ex) {
			std::cerr << "NPRPC Error: " << ex.what() << '\n';
		}
		break;
	case ID_VIRTUAL_START:

		break;
	case ID_VIRTUAL_STOP:
		break;
	case ID_SHOW_TT:
		ShowTaskTable();
		break;
	case ID_SHOW_RD:
		ShowRemoteTable();
		break;
	case ID_SHOW_TWI:
		ShowTwiTable();
		break;
	case ID_DELETE_ADDITIONAL_LIBS:
	{
		if (MessageBoxA(g_hMainWnd, "Are you sure?", "", MB_YESNO) == IDNO) break;
		auto self = self_node.fetch().cast<npsys::controller_n_avr>();
		self->assigned_ofs.clear_list(true);
		break;
	}
	case ID_UPLOAD_CONTROLLER:
		break;
	case ID_UPLOAD_IO:
		UploadIO();
		break;
	case ID_UPLOAD_FIRMWARE: 
	{
		auto self = self_node.fetch();
		auto ctrl = self.cast<npsys::controller_n>();
		CDlg_LoadingManager dlg(
			std::make_unique<CDlg_LoadingManager::CTaskUploadControllerFirmware>(ctrl)
		);
		dlg.DoModal(g_hMainWnd);
		break;
	}
	case ID_ACTIVATE_DEVICE: 
		SetActivated();
		break;
#ifdef DEBUG
	case ID_DELETE_RD:
		if (MessageBoxA(g_hMainWnd, "Delete R Table?", "", MB_OKCANCEL) == IDOK) DeleteRDTable();
		break;
	case ID_DELETE_TT:
		if (MessageBoxA(g_hMainWnd, "Delete T Table?", "", MB_OKCANCEL) == IDOK) DeleteTaskTable();
		break;
	case ID_DELETE_I2C:
		if (MessageBoxA(g_hMainWnd, "Delete I2C Table?", "", MB_OKCANCEL) == IDOK) DeleteI2CTable();
		break;
	case ID_SHOW_INTERNAL_DATA:
		ShowInternalData();
		break;
#endif // DEBUG
	default:
		return false;
	}
	return true;
}

const std::vector<std::tuple<int, uint16_t>>& CAVRController::InitInternalBlock(CTime* block) {
	const auto tm_addr = GetA_Time();
	constexpr auto type = npsys::nptype::NPT_U8 | npsys::nptype::VQUALITY | npsys::nptype::INTERNAL;

	static const std::vector<std::tuple<int, uint16_t>> map =
	{
		{ type, tm_addr + offsetof(npsystem::types::Time, sec) },
		{ type, tm_addr + offsetof(npsystem::types::Time, min) },
		{ type, tm_addr + offsetof(npsystem::types::Time, hour) },
		{ type, tm_addr + offsetof(npsystem::types::Time, wday) },
		{ type, tm_addr + offsetof(npsystem::types::Time, mday) },
		{ type, tm_addr + offsetof(npsystem::types::Time, mon) },
		{ type, tm_addr + offsetof(npsystem::types::Time, year) },
	};

	return map;
}

const std::vector<std::tuple<int, uint16_t>>& CAVRController::InitInternalBlock(CBlockSchedule* block) {
	const auto tm_addr = GetA_Time();
	constexpr auto type = npsys::nptype::NPT_U8 | npsys::nptype::VQUALITY | npsys::nptype::INTERNAL;

	static const std::vector<std::tuple<int, uint16_t>> map =
	{
		{ type, tm_addr + offsetof(npsystem::types::Time, hour) },
		{ type, tm_addr + offsetof(npsystem::types::Time, min) },
		{ type, tm_addr + offsetof(npsystem::types::Time, sec) },
	};

	return map;
}

#ifdef DEBUG
void CAVRController::ShowInternalData() {
	std::cout << get_name() << ":\n";

	std::cout << " allocated variables:\n";
	allocated_vars().fetch_all_nodes();
	for (auto& v : allocated_vars()) {
		std::cout << '\t' << *v.get() << '\n';
	}
	std::cout << " owner released variables:\n";
	vars_owner_released().fetch_all_nodes();
	for (auto& v : vars_owner_released()) {
		std::cout << '\t' << *v.get() << '\n';
	}

	assigned_algs.fetch_all_nodes();

	for (auto& assigned : assigned_algs) {
		//		std::cout << ' ' << assigned->get_name() << ".ReleasedVariables:\n";
		//		assigned->vars_released().fetch_all_nodes();
		//		for (auto& v : assigned->vars_released()) {
		//			std::cout << '\t' << *v.get() << '\n';
		//		}

		std::cout << ' ' << assigned->get_name() << ".ReleasedReferences:\n";
		assigned->references_released().fetch_all_nodes();
		for (auto& v : assigned->references_released()) {
			std::cout << '\t' << *v.get() << '\n';
		}
	}

	std::cout << "IO:\n";
	for (auto& port : ports) {
		std::cout << port->get_name() << ":\n";
		for (auto& pin : port->pins_) {
			std::cout << '\t' << pin->get_name() << ' ' << *pin->GetVariable().get() << '\n';
		}
	}
	std::cout << '\n';
	std::cout.flush();
}


#endif

void CAVRController::UploadRemoteData() {
	if (!remote_data_changed_) return;

	net::remote_data r_table[64] = { 0 };
	net::remote_data* p_table = r_table;

	// Read Bytes
	for (auto& d : links_r_) d.second.load_nodes();

	for (auto& d : links_r_) {
		auto& rByte = d.second.lbyte;
		if (rByte.empty()) continue;
		std::sort(rByte.begin(), rByte.end(), [](const auto& l1, const auto& l2) {
			return l1.second->GetAddr() < l2.second->GetAddr();
		});

		net::remote_data data;
		data.val.device_addr = rByte[0].second->GetDevAddr();
		data.val.data_rw_type = 1;
		data.val.is_bit = 0;

		size_t n = rByte.size();
		for (int i = 0; i < n; ++i) {
			int size = rByte[i].first->GetSize();
			int rem_addr = rByte[i].second->GetAddr();
			data.val.local_addr = rByte[i].first->GetAddr();
			data.val.remote_addr = rem_addr;
			data.val.bit_size = size;

			if ((i + 1 != n) && (rByte[i + 1].second->GetAddr() == rem_addr + size + 1)) {
				data.val.marker = M_REQ_CONTINUE;
			} else {
				data.val.marker = M_LAST_DATA_IN_REQUEST;
			}
			(*p_table++) = data;
		}

	}
	// Read Bits
	for (auto& d : links_r_) {
		auto& rBit = d.second.lbit;
		if (rBit.empty()) continue;
		std::sort(rBit.begin(), rBit.end(), [](const auto& l1, const auto& l2) {
			return l1.second->GetAddr() < l2.second->GetAddr();
		});

		net::remote_data data;
		data.val.device_addr = rBit[0].second->GetDevAddr();
		data.val.data_rw_type = 1;
		data.val.is_bit = 1;

		size_t n = rBit.size();
		for (int i = 0; i < n; ++i) {
			int rem_addr = rBit[i].second->GetAddr();
			data.val.local_addr = rBit[i].first->GetAddr();
			data.val.remote_addr = rem_addr;
			data.val.bit_size = rBit[i].first->GetBit();

			if (i + 1 != n && (rBit[i + 1].second->GetAddr() <= 1 + rem_addr)) {
				data.val.marker = M_REQ_CONTINUE;
			} else {
				data.val.marker = M_LAST_DATA_IN_REQUEST;
			}

			(*p_table++) = data;
		}
	}

	size_t len = (p_table - r_table) * sizeof(net::remote_data);

	if (len) {
		(p_table - 1)->val.marker = M_ARRAY_END;
	} else {
		p_table[0].u32Val = 0;
		len = sizeof(net::remote_data);
	}

	std::cout << PrintRemoteTable(r_table) << std::endl;

	WriteRemoteData(reinterpret_cast<unsigned char*>(r_table), (int)len);

	remote_data_changed_ = false;
}

#ifdef DEBUG
void CAVRController::DeleteRDTable() {
	try {
		net::remote_data data = { 0 };
		WriteRemoteData(reinterpret_cast<unsigned char*>(&data), sizeof(net::remote_data));
		std::cout << "Remote Data have been deleted"sv << std::endl;
		links_r_.clear();
		links_w_.clear();
		self_node.fetch().store();
	} catch (nprpc::Exception& ex) {
		std::cerr << "NPRPC Error: " << ex.what() << '\n';
	}
}
#endif

int CAVRController::WritePage(int page_num, int page_size, uint8_t* page_p) {
	return ExecMultiCmd(CWritePageCmd(dev_addr, GetFirmwareInfo(), page_num, page_size, page_p));
}
int CAVRController::WriteRemoteData(uint8_t* data, int len) {
	const auto& info = GetFirmwareInfo();
	return ExecMultiCmd(CWriteRemoteData(dev_addr, info, info.pmem.rd, len, data));
}
int CAVRController::WriteTwiTable(int page_num, int page_size, uint8_t* page_p) {
	return ExecMultiCmd(CWriteTwiTable(dev_addr, GetFirmwareInfo(), page_num, page_size, page_p));
}
int CAVRController::ReplaceAlg(int old_addr, int new_addr) {
	return ExecMultiCmd(CReplaceAlgCmd(dev_addr, GetFirmwareInfo(), old_addr, new_addr));
}
int CAVRController::RemoveAlg(uint16_t algAddr) {
	return ExecMultiCmd(CRemoveAlgCmd(dev_addr, GetFirmwareInfo(), algAddr));
}
bool CAVRController::StopAlgorithm(uint16_t alg_addr) {
	return npsys_rpc->server->AVR_StopAlgorithm(dev_addr, alg_addr);
}
void CAVRController::ReinitIO() {
	npsys_rpc->server->AVR_ReinitIO(dev_addr);
}
int CAVRController::WriteEeprom(uint16_t mem_addr, int len, const uint8_t* data) {
	return ExecMultiCmd(CWriteEepromCmd(dev_addr, GetFirmwareInfo(), mem_addr, len, data));
}

void CAVRController::WriteDefaultValues(const std::vector<WriteDefaultValuesRequest>& m_requests) const {
	auto& info = GetFirmwareInfo();
	for (const auto& req : m_requests) {
		uint16_t eeprom_offset = req.GetOffset() - info.rmem.noinit_start;
		ExecMultiCmd(CWriteEepromCmd(dev_addr, info, eeprom_offset, req.GetLength(), req.GetData()));
	}
	for (const auto& req : m_requests) {
		npsys_rpc->server->WriteBlock(dev_addr, req.GetOffset(), nprpc::flat::make_read_only_span(req.GetData(), req.GetLength()));
	}
}

CTreeHardware* CAVRController::CreateHardware(npsys::controller_n& self) noexcept {
	return new CTreeAvrHardware(self);
}

#ifdef DEBUG
void CAVRController::DeleteTaskTable() {
	try {
		auto tt = GetTaskTable();
		if (tt.size() > 2) {
			for (size_t i = 2; i < tt.size(); ++i) {
				RemoveAlg(tt[i]);
			}
		}
		std::cout << "Task table has been removed." << std::endl;
	} catch (nprpc::Exception& ex) {
		std::cout << "NPRPC Error: " << ex.what() << std::endl;
	}
}
void CAVRController::DeleteI2CTable() {
	try {
		twitab_t tab {0};

		const auto& info = GetFirmwareInfo();
		WriteTwiTable(info.pmem.twi, 1, &tab.n);

		std::cout << "I2C table has been removed." << std::endl;
	} catch (nprpc::Exception& ex) {
		std::cout << "NPRPC Error: " << ex.what() << std::endl;
	}
}
#endif

static void time_func(std::string& str, uint32_t ul) {
	auto sec = static_cast<float>(ul) * 0.010048f;
	std::array<char, 10> buffer;

	if (auto [p, ec] = std::to_chars(buffer.data(), buffer.data() + buffer.size(), sec, std::chars_format::fixed, 2);
		ec == std::errc()) {
		str.assign(buffer.data(), p - buffer.data());
		str += " s. or "sv;
	}

	if (auto [p, ec] = std::to_chars(buffer.data(), buffer.data() + buffer.size(), sec / 3600.0f, std::chars_format::fixed, 2);
		ec == std::errc()) {
		str += std::string_view(buffer.data(), p - buffer.data());
		str += " h."sv;
	}
}

static void cpu_func(std::string& str, uint32_t ul) {
	auto ival = static_cast<int>(ul & 0xFF) - ADC_TASK_DELAY;
	if (ival < 0) ival = 0;
	float x = float(ival);

	constexpr float y0 = 0.f;
	constexpr float y1 = 100.f;
	constexpr float x0 = 0.f;
	constexpr float x1 = 60.f;

	//	float y = ( y0 * (x1 - x) + y1 * (x - x0) ) / (x1 - x0);
	float y = y1 * x / x1;
	std::array<char, 10> buffer;
	if (auto [p, ec] = std::to_chars(buffer.data(), buffer.data() + buffer.size(), y, std::chars_format::fixed, 2);
		ec == std::errc()) 
	str.assign(buffer.data(), p - buffer.data());
}

void CAVRController::CreateOnlineCustomItems(CTreeViewCtrl tree, HTREEITEM parent,
	std::vector<std::unique_ptr<COnlineTreeItem>>& nodes) {
	const auto& info = GetFirmwareInfo();
	const auto& pinfo = GetPeripheralInfo();

	auto hIcon = global.GetIcon24x24(NPSystemIcon::Binary);

	nodes.push_back(std::make_unique<COnlineTreeItem>(
		"INTERNAL",
		hIcon
		));
	auto hRoot = nodes.back()->Insert(tree, parent);

	nodes.push_back(std::make_unique<COnlineTreeItem>(
		"COUNTERS",
		hIcon
		));
	auto hCnt = nodes.back()->Insert(tree, hRoot);

	nodes.push_back(std::make_unique<COnlineTreeItem>(
		"DATE",
		hIcon
		));
	auto hDate = nodes.back()->Insert(tree, hRoot);

	nodes.push_back(std::make_unique<COnlineTreeItem>(
		"IO",
		hIcon
		));
	auto hIo = nodes.back()->Insert(tree, hRoot);

	nodes.push_back(std::make_unique<COnlineTreeItem>(
		"ADC",
		hIcon
		));
	auto hAdc = nodes.back()->Insert(tree, hRoot);

	nodes.push_back(std::make_unique<COnlineTreeItem>(
		"CPU LOAD",
		hIcon,
		dev_addr,
		info.rmem.info + offsetof(net::RuntimeInfoController, cpu_load),
		sizeof(net::RuntimeInfoController::cpu_load),
		COnlineTreeItem::Type::FUNCTION,
		&cpu_func
		));
	nodes.back()->Insert(tree, hCnt);


	nodes.push_back(std::make_unique<COnlineTreeItem>(
		"E_SYNCLOST",
		hIcon,
		dev_addr,
		info.rmem.info + offsetof(net::RuntimeInfo, synclost),
		sizeof(net::RuntimeInfo::synclost),
		COnlineTreeItem::Type::HEX_UI1
		));
	nodes.back()->Insert(tree, hCnt);

	nodes.push_back(std::make_unique<COnlineTreeItem>(
		"E_SLAVE_TIMEOUT",
		hIcon,
		dev_addr,
		info.rmem.info + offsetof(net::RuntimeInfo, e_slave_timeout),
		sizeof(net::RuntimeInfo::e_slave_timeout),
		COnlineTreeItem::Type::HEX_UI1
		));
	nodes.back()->Insert(tree, hCnt);

	nodes.push_back(std::make_unique<COnlineTreeItem>(
		"E_RV_CNT",
		hIcon,
		dev_addr,
		info.rmem.info + offsetof(net::RuntimeInfo, rv_error_cnt),
		sizeof(net::RuntimeInfo::rv_error_cnt),
		COnlineTreeItem::Type::HEX_UI1
		));
	nodes.back()->Insert(tree, hCnt);

	nodes.push_back(std::make_unique<COnlineTreeItem>(
		"E_RV_ALL_CNT",
		hIcon,
		dev_addr,
		info.rmem.info + offsetof(net::RuntimeInfo, rv_error_all_cnt),
		sizeof(net::RuntimeInfo::rv_error_all_cnt),
		COnlineTreeItem::Type::HEX_UI1
		));
	nodes.back()->Insert(tree, hCnt);

	nodes.push_back(std::make_unique<COnlineTreeItem>(
		"T:SEC",
		hIcon,
		dev_addr,
		info.rmem.lt + offsetof(npsystem::types::Time, sec),
		1,
		COnlineTreeItem::Type::HEX_UI1
		));
	nodes.back()->Insert(tree, hDate);

	nodes.push_back(std::make_unique<COnlineTreeItem>(
		"T:MIN",
		hIcon,
		dev_addr,
		info.rmem.lt + offsetof(npsystem::types::Time, min),
		1,
		COnlineTreeItem::Type::HEX_UI1
		));
	nodes.back()->Insert(tree, hDate);

	nodes.push_back(std::make_unique<COnlineTreeItem>(
		"T:HOUR",
		hIcon,
		dev_addr,
		info.rmem.lt + offsetof(npsystem::types::Time, hour),
		1,
		COnlineTreeItem::Type::HEX_UI1
		));
	nodes.back()->Insert(tree, hDate);

	nodes.push_back(std::make_unique<COnlineTreeItem>(
		"T:WDAY",
		hIcon,
		dev_addr,
		info.rmem.lt + offsetof(npsystem::types::Time, wday),
		1,
		COnlineTreeItem::Type::HEX_UI1
		));
	nodes.back()->Insert(tree, hDate);

	nodes.push_back(std::make_unique<COnlineTreeItem>(
		"T:MDAY",
		hIcon,
		dev_addr,
		info.rmem.lt + offsetof(npsystem::types::Time, mday),
		1,
		COnlineTreeItem::Type::HEX_UI1
		));
	nodes.back()->Insert(tree, hDate);

	nodes.push_back(std::make_unique<COnlineTreeItem>(
		"T:MON",
		hIcon,
		dev_addr,
		info.rmem.lt + offsetof(npsystem::types::Time, mon),
		1,
		COnlineTreeItem::Type::HEX_UI1
		));
	nodes.back()->Insert(tree, hDate);

	nodes.push_back(std::make_unique<COnlineTreeItem>(
		"T:YEAR",
		hIcon,
		dev_addr,
		info.rmem.lt + offsetof(npsystem::types::Time, year),
		1,
		COnlineTreeItem::Type::HEX_UI1
		));
	nodes.back()->Insert(tree, hDate);


	nodes.push_back(std::make_unique<COnlineTreeItem>(
		"Uptime (sec)",
		hIcon,
		dev_addr,
		info.rmem.info + offsetof(net::RuntimeInfoController, u32_time),
		sizeof(net::RuntimeInfoController::u32_time),
		COnlineTreeItem::Type::FUNCTION,
		&time_func
		));
	nodes.back()->Insert(tree, hCnt);

	ports.fetch_all_nodes();
	for (auto& port : ports) {
		npsys::io_reg_addr reg = port->GetControlRegisters(pinfo, false);
		nodes.push_back(std::make_unique<COnlineTreeItem>(
			(std::string)reg.ddr_n.c_str(),
			hIcon,
			dev_addr,
			reg.ddr,
			1,
			COnlineTreeItem::Type::BOOL
			));
		nodes.back()->Insert(tree, hIo);

		nodes.push_back(std::make_unique<COnlineTreeItem>(
			(std::string)reg.port_n.c_str(),
			hIcon,
			dev_addr,
			reg.port,
			1,
			COnlineTreeItem::Type::BOOL
			));
		nodes.back()->Insert(tree, hIo);

		nodes.push_back(std::make_unique<COnlineTreeItem>(
			(std::string)reg.pin_n.c_str(),
			hIcon,
			dev_addr,
			reg.pin,
			1,
			COnlineTreeItem::Type::BOOL
			));
		nodes.back()->Insert(tree, hIo);
	}

	for (int i = 0; i < info.mccfg.adc_max; ++i) {
		nodes.push_back(std::make_unique<COnlineTreeItem>(
			"ADC #" + std::to_string(i),
			hIcon,
			dev_addr,
			info.rmem.adc_array + i * 2,
			2,
			COnlineTreeItem::Type::HEX_UI2
			));
		nodes.back()->Insert(tree, hAdc);
	}
	nodes.push_back(std::make_unique<COnlineTreeItem>(
		"ADC State",
		hIcon,
		dev_addr,
		info.rmem.eeprcfg,
		1,
		COnlineTreeItem::Type::HEX_UI1
		));
	nodes.back()->Insert(tree, hAdc);

	nodes.push_back(std::make_unique<COnlineTreeItem>(
		"ADC N",
		hIcon,
		dev_addr,
		info.rmem.eeprcfg + 1,
		1,
		COnlineTreeItem::Type::HEX_UI1
		));
	nodes.back()->Insert(tree, hAdc);
}

#define PADHEX(width, val) "0x" << std::setfill('0') << std::setw(width) << std::hex << (unsigned)val
// IN 1011 0AAd dddd AAAA
#define ASM_IN(reg, addr) ((0b10110 << 11) | ((uint16_t)(addr) & 0x0F) | (((uint16_t)(addr) & 0xF0) << 5) | ((uint16_t)(reg) << 4))
// ANDI 0111 KKKK dddd KKKK
#define ASM_ANDI(reg, val) ((0b0111 << 12) | (uint16_t)(val) & 0x0F | ((uint16_t)((val) & 0xF0) << 4) | ((reg) - 16) << 4)
// ORI 0110 KKKK dddd KKKK
#define ASM_ORI(reg, val) ((0b0110 << 12) | (uint16_t)(val) & 0x0F | ((uint16_t)((val) & 0xF0) << 4) | ((reg) - 16) << 4)
// OUT 1011 1AAr rrrr AAAA
#define ASM_OUT(addr, reg) ((0b10111 << 11) | ((uint16_t)(addr) & 0x0F) | (((uint16_t)(addr) & 0xF0) << 5) | ((uint16_t)(reg) << 4))
// RET 1001 0101 0000 1000
#define ASM_RET 0x9508


bool CAVRController::IoHandleRequest(REQUEST* req) noexcept {
	switch (req->ID_MENU) {
	case ID_UPLOAD_IO:
		UploadIO();
		break;
	case ID_ITEM_PROPERTIES:
		// MessageBoxA(g_hMainWnd, "IO Properies", NULL, MB_OK);
		break;
	default:
		return false;
	};
	return true;
}

void CAVRController::IoConstructMenu(CMenu* menu) noexcept {
	if (firmware_status != FIRMWARE_NOT_LOADED) {
		menu->AppendMenu(MF_STRING, ID_UPLOAD_IO, L"Upload");
		menu->AppendMenu(MF_SEPARATOR);
		menu->AppendMenu(MF_STRING, ID_ITEM_PROPERTIES, L"Properties");
	}
}

bool CAVRController::UploadIO() noexcept {
	bool ok = false;

	const auto& info = GetFirmwareInfo();
	const auto& pinfo = GetPeripheralInfo();

	oriandival val;
	std::unique_ptr<uint16_t[]> buf(new uint16_t[info.pmem.pagesize / 2]);
	uint16_t* ptr = buf.get();

	constexpr auto REFS0 = 6;
	constexpr auto REFS1 = 7;
	constexpr int admux_table[] = {
		(0 << REFS1) | (0 << REFS0), // AREF, Internal Vref turned off
		(0 << REFS1) | (1 << REFS0), // AVCC with external capacitor at AREF pin
		(1 << REFS1) | (1 << REFS0), // Internal 2.56V Voltage Reference with external capacitor at AREF pin
	};

	std::vector<int> adc;

	for (auto& port : ports) {
		oriandival port_val{ 0 }, ddr_val{ 0 };
		for (auto& pin : port->pins_) {
			bool can_load = pin->IsCanBeUploaded();
			if (!can_load) {
				std::cout << ">WARNING: Pin " << pin->get_name() 
					<< " will not be loaded - there are existing links using it" << std::endl;
			}
			val = pin->GetDDRVal(!can_load);
			ddr_val.ori |= val.ori;
			ddr_val.andi |= val.andi;
			val = pin->GetPORTVal(!can_load);
			port_val.ori |= val.ori;
			port_val.andi |= val.andi;
			if (pin->GetPinPurpose(!can_load) == avrinfo::PinPurpose::ANALOG_PIN) {
				if (info.version < 2 && pin->adc_reference_voltage != avrinfo::AdcReferenceVoltage::AREF) {
					std::cerr << "This firmware version does not support ADC reference voltage other than AREF.\n";
					return false;
				}
				adc.push_back(pin->GetPinNum() | admux_table[static_cast<int>(pin->adc_reference_voltage)]);
			}
		}

		port_val.andi ^= 0xff;
		ddr_val.andi ^= 0xff;

		io_reg_addr addr = port->GetControlRegisters(pinfo);

		(*ptr++) = ASM_IN(16, addr.port);
		(*ptr++) = ASM_ANDI(16, port_val.andi);
		(*ptr++) = ASM_ORI(16, port_val.ori);
		(*ptr++) = ASM_OUT(addr.port, 16);

		(*ptr++) = ASM_IN(16, addr.ddr);
		(*ptr++) = ASM_ANDI(16, ddr_val.andi);
		(*ptr++) = ASM_ORI(16, ddr_val.ori);
		(*ptr++) = ASM_OUT(addr.ddr, 16);

		//	uint8_t *p = (uint8_t*)(ptr - 4);
		//	for (int i = 0; i < 8; i++)
		//		DBG_PRINT("%.2x", p[i]);
		//	DBG_PRINT("\n");
		//	return;
	}
	(*ptr++) = ASM_RET;

	size_t size = (ptr - buf.get()) * 2;

	/*
	std::ofstream ofs("io_" + GetChipModel() + ".txt");
	size_t sz = (ptr - buf.get());

	ofs << "#ifdef __ASSEMBLER__" << std::endl;
	ofs << ".macro PORT_INIT_SEQ" << std::endl;

	for (size_t i = 0, row = 0; i < sz; ++i) {
		if (row == 0) {
			ofs << "\t.word ";
		}
		ofs << PADHEX(2, buf[i]);
		if (row++ >= 4) {
			row = 0;
			ofs << std::endl;
		} else if (i != sz - 1) {
			ofs << ", ";
		} else if (i == sz - 1) {
			ofs << std::endl;
		}
	}

	ofs << ".endm" << std::endl;
	ofs << "#endif // __ASSEMBLER__" << std::endl;
	ofs.close();
	*/

	try {
		WritePage(info.pmem.io, static_cast<int>(size), reinterpret_cast<uint8_t*>(buf.get()));
		ReinitIO();
		//info.version;
		eeprcfg_t eep;
		if (!adc.empty()) {
			eep.adc_state = ADC_INIT;
			eep.adc_n = static_cast<uint8_t>(adc.size());
			for (size_t i = 0; i < adc.size(); ++i) {
				eep.admux_value[i] = adc[i];
			}
		} else {
			memset(&eep, 0, sizeof(eeprcfg_t));
			eep.adc_state = ADC_DISABLE;
		}

		npsys_rpc->server->WriteBlock(dev_addr, info.rmem.eeprcfg, nprpc::flat::make_read_only_span((uint8_t*)&eep, sizeof(eeprcfg_t)));

		WriteEeprom(info.emem.eeprcfg, sizeof(eeprcfg_t), (uint8_t*)&eep);

		odb::Batch batch;
		for (auto& port : this->ports) {
			io_reg_addr regs = port->GetControlRegisters(pinfo, false);
			for (auto& pin : port->pins_) {
				if (pin->IsCanBeUploaded()) {
					pin->UpdateVar(info, regs);
					pin.store();
				}
			}
		}
		batch.exec();
		item->CalcAndUpdateHardwareStatus();
		item->m_treeview.Invalidate();
		ok = true;
	} catch (nps::NPNetCommunicationError& ex) {
		std::cerr << "Communication failed: " << TranslateError(ex.code).data() << '\n';
	} catch (nprpc::Exception& ex) {
		std::cerr << "NPRPC Error: " << ex.what() << '\n';
	}
	return ok;
}

std::unique_ptr<COutsideReference> CAVRController::CreatePinReference(remote::ExtLinkType type, 
	std::string_view port_name, std::string_view pin_name, odb::weak_node<fbd_control_unit_n> alg) {
	auto port = odb::utils::find_by_name(this->ports, port_name);
	if (!port) throw std::runtime_error("Port: \"" + std::string(port_name) + "\" does not exist in this controller");
	
	auto pin = odb::utils::find_by_name((*port)->pins_, pin_name);
	if (!pin) throw std::runtime_error("Pin: \"" + std::string(pin_name) + "\" does not exist in this port");

	auto pin_type = (*pin)->GetPinType();
	if ((pin_type == io::PINTYPE::INPUT && type == remote::ExtLinkType::Read) ||
	 (pin_type == io::PINTYPE::OUTPUT && type == remote::ExtLinkType::Write)) {
		// ok
	} else {
		throw std::runtime_error("Parameter type and pin direction do not match");
	}
	return std::make_unique<CAvrInternalPin>(*pin, type, alg);
}

}; // namespace npsys