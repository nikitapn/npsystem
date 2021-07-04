// Copyright (c) 2021 nikitapnn1@gmail.com
// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory

#include "stdafx.h"
#include "algext.h"
#include "avrconfigurator.h"
#include "codegenerator.h"
#include "block.h"
#include "line.h"
#include "graphics.h"
#include "assignedalgorithm.h"
#include "error.h"
#include "view_algorithm.h"
#include "dlgalgorithmproperties.h"
#include <npsys/corba.h>
#include "network_ext.h"
#include <npsys/algcat.h>
#include <npsys/algsubcat.h>
#include "dlgloadingmanager.h"
#include "config.h"

namespace npsys {
#include <npdb/debug.h>

CAlgorithmExt::CAlgorithmExt(std::string&& name, npsys::algorithm_category_n& cat) {
	this->set_name(name);
	this->editor_.create();
	this->view_position.create();
	this->view_position->DefaultViewPosition();
	this->view_position.store();
	this->fbd_blocks.init();
	this->fbd_blocks.store();
	this->editor_->SetAlgorithm(this, this->fbd_blocks);
	this->cat = cat;
	this->set_modified(false);
}

CBlockComposition* CAlgorithmExt::GetBlocks() {
	if (editor_.loaded()) return editor_.get();
	if (editor_.fetch()) {
		fbd_blocks.fetch_all_nodes();
		editor_->SetAlgorithm(this, fbd_blocks);
		CFindExternalReferences externalReferences;
		Traversal<CBlockVisitor>(editor_.get(), externalReferences);
		for (auto ref : externalReferences) {
			ref.first->LoadLink();
		}
		return editor_.get();
	}
	return nullptr;
}

void CAlgorithmExt::Unload() {
	StopOnline();
	while (editor_->Undo() != -1); // moving node elements back

	/*
	CFindExternalReferences externalReferences;
	Traversal<CElement>(editor_.get(), externalReferences);
	for (auto& ref : externalReferences) {
		ref.first->Disconnect();
	}
	*/

	editor_.free();
}

void CAlgorithmExt::HardwareSpecific_Clear() {
	auto editor = GetBlocks();
	CFindInternalBlockRef intRefBlocks;
	Traversal<CBlockVisitor>(editor, intRefBlocks );
	intRefBlocks.Clear();
}

void CAlgorithmExt::Save(bool after_uploading) noexcept {
	if (saving_in_progress_) return;
	saving_in_progress_ = true;

	auto editor = GetBlocks();

	if (after_uploading) {
		SetStatus(Status::status_loaded);
	} else if (IsUndoRedo() && (GetStatus() >= status_loaded)) {
		SetStatus(Status::status_not_actual);
	}

	editor_->PermanentExecuteCommands();
	actions.ExecuteAll();

	CFindVariableNodes vars;
	CFindSlots slots;
	CFindInternalBlockRef intRefBlocks;
	Traversal<CBlockVisitor>(editor, { &vars, &slots, &intRefBlocks });
	
	if (assigned_dev.is_invalid_id() == false) {
		auto dev = assigned_dev.fetch();
		intRefBlocks.InitInternalReferences(dev.get());
	}

	for (auto& block : fbd_blocks) {
		Iter::PreorderIterator<CElement*> it(block->e_block.get());
		for (; !it.IsDone(); it.Next()) {
			if ((*it)->IsCreatedNew()) {
				block->set_modified();
				break;
			}
		}
	}

	for (auto s : slots) {
		s->CommitTypeChanges();
	}

	for (auto& s : slots.disabled_slots) {
		if (s.loaded() && s->is_modified()) {
			s.store();
			auto var = s->e_slot->GetVariableAsNode();
			if (var && var->loaded() && (*var)->is_modified()) {
				var->store();
			}
		}
	}

	for (auto& v : vars) {
		if ((*v)->is_modified()) {
			(*v).store();
		}
	}

	for (auto& block : fbd_blocks) {
		for (auto& slot : block->slots) {
			if (slot->is_modified() || slot->e_slot->IsModified()) {
				slot->alg = this->self_node;
				slot->e_slot->ResetState();
				slot.store();
			}
		}
		if (block->is_modified()) {
			block.store();
		}
	}

	fbd_blocks.store();
	editor_.store();

	if (GetStatus() >= Status::status_assigned) {
		auto assigned = assigned_alg.fetch();
		assigned.store(); // store removed variables
	}

	auto self = self_node.fetch();
	self.store();

	if (GetStatus() >= Status::status_loaded) {
		auto device = assigned_dev.fetch();
		ASSERT(device.loaded());
		device.store();
	}

	saving_in_progress_ = false;
}

void CAlgorithmExt::ClearVariables() noexcept {
	auto blocks = GetBlocks();
	blocks->ClearVariables();
}

std::optional<npsys::fbd_slot_n>
CAlgorithmExt::FindSlot(
	const std::string& block_name, const std::string& slot_name) {
	fbd_blocks.fetch_all_nodes();
	for (auto& b : fbd_blocks) {
		if (b->e_block->GetName() != block_name) continue;
		b->slots.fetch_all_nodes();
		for (auto& s : b->slots) {
			if (s->e_slot->GetName() != slot_name) continue;
			return s;
		}
		return {};
	}
	return {};
}

std::optional<npsys::fbd_slot_n>
CAlgorithmExt::FindSlot(int slotId) {
	fbd_blocks.fetch_all_nodes();
	for (auto& b : fbd_blocks) {
		b->slots.fetch_all_nodes();
		for (auto& s : b->slots) {
			if (s->e_slot->GetId() == slotId) return s;
		}
	}
	return {};
}

void CAlgorithmExt::DetermineTypes(npsys::algorithm_n& self) {
	if (m_running) return;

	m_running = 1;

	CFindExternalReferences varExtRef;
	Traversal<CBlockVisitor>(GetBlocks(), varExtRef);
	SortedLink sorted;
	color_m color;

	for (auto& i : varExtRef) {
		if (!i.first->IsLinkValid()) continue;
		auto slot = i.second->top->self_node.fetch();
		i.first->TopoSort(self, slot, color, sorted);
	}

	for (auto it = sorted.crbegin(); it != sorted.crend(); ++it) {
		auto alg = (*it).alg;
		alg->DetermineTypes(alg);
	}

	CTypeDeterminant tp;
	Translate(&tp);

	m_running = 0;
}

void CAlgorithmExt::Separate(std::vector<CBlock*>* pV, CBlock* block, char* color) {
	pV->push_back(block);
	color[block->GetTmpId()] = true;
	for (auto& s : block->top->slots) {
		for (auto& j : *s->e_slot) {
			if (color[j->GetTmpId()]) continue;

			color[j->GetTmpId()] = true;

			auto nextslot = j->GetAnotherSlot(s->e_slot.get());
			if (!nextslot) continue;

			auto nextblock = nextslot->GetParentBlock();
			if (!color[nextblock->GetTmpId()])
				Separate(pV, nextblock, color);
		}
	}
}

int CAlgorithmExt::CalcOrder(CBlock* block, int order, char* color) {
	std::stack<CBlock*> nextblocks;
	for (auto& s : block->top->slots) {
		for (auto l : *s.get()->e_slot) {
			int direction;
			auto next = l->GetAnotherSlot(s->e_slot.get(), &direction)->GetParentBlock();
			if (color[l->GetTmpId()]) continue;
			color[l->GetTmpId()] = true;
			if (direction > 0) {
				if (color[next->GetTmpId()]) continue;
				else order = CalcOrder(next, order, color);
			} else if (direction < 0) {
				nextblocks.push(next);
			}
		}
	}

	color[block->GetTmpId()] = true;
	block->SetExecuteOrder(order);
	int neworder = order;

	while (nextblocks.size()) {
		neworder = TryExecuteNext(nextblocks.top(), order, color);
		nextblocks.pop();
		if (neworder != -1) order = neworder;
	}
	return ++order;
}
int	CAlgorithmExt::TryExecuteNext(CBlock* block, int order, char* color) {
	std::stack<CBlock*, std::vector<CBlock*>> nextblocks;
	for (auto& s : block->top->slots) {
		for (auto& l : *s.get()->e_slot) {
			int direction;
			auto next = l->GetAnotherSlot(s->e_slot.get(), &direction)->GetParentBlock();
			if (direction > 0) {
				if (color[next->GetTmpId()]) continue;
				else return -1;
			} else if (direction < 0) {
				nextblocks.push(next);
			}
		}
	}
	color[block->GetTmpId()] = true;
	block->SetExecuteOrder(++order);
	while (nextblocks.size()) {
		int neworder = TryExecuteNext(nextblocks.top(), order, color);
		nextblocks.pop();
		if (neworder != -1) order = neworder;
	}
	return order;
}
void CAlgorithmExt::Translate(CCodeGenerator* pGenerator) {
	pGenerator->Reset();

	CBlocksInspector logic;
	CLinesInspector lines;

	Traversal<CBlockVisitor>(editor_.get(), { &logic, &lines });
	int id = 0;

	for (auto i : logic) i->SetTmpId(id++);
	for (auto i : lines) i->SetTmpId(id++);

	char color[500];
	memset(color, 0x00, sizeof(color));

	std::vector<std::vector<CBlock*>> chains;
	int k = 0;
	for (auto& i : logic) {
		if (!color[i->GetTmpId()]) {
			chains.push_back({});
			Separate(&chains[k++], i, color);
		}
	}

	memset(color, 0x00, sizeof(color));

	for (auto& i : chains) {
		int order = 0;
		for (auto& j : i) {
			if (!color[j->GetTmpId()]) {
				order = CalcOrder(j, order, color);
			}
		}
		std::sort(i.begin(), i.end(), [](const CBlock* b1, const CBlock* b2) {
			return b1->GetExecuteOrder() < b2->GetExecuteOrder(); });
	}
	for (auto& i : chains) {
		for (auto& j : i) {
			j->Translate(pGenerator);
		}
	}
}

bool CAlgorithmExt::IsUndoRedo() {
	return editor_.loaded()
		&& (editor_->IsRedoCommands() || editor_->IsUndoCommands());
}

void CAlgorithmExt::CloseView() {
	StopOnline();
	// __super::CloseView();
}

void CAlgorithmExt::DrawOnlineValues(CGraphics* pGraphics) {
	if (!online_) return;
	for (auto lo : m_loadedSlots) lo->DrawOnlineValue(pGraphics);
}

void CAlgorithmExt::StartOnline() {
	if (GetStatus() == status_not_assigned) {
		MessageBoxA(g_hMainWnd, "Algorithm is not assigned", "", MB_ICONINFORMATION);
		return;
	} else if (GetStatus() == status_assigned) {
		MessageBoxA(g_hMainWnd, "Algorithm is not loaded", "", MB_ICONINFORMATION);
		return;
	} else if (GetStatus() == status_not_actual) {
		MessageBoxA(g_hMainWnd, "Algorithm has been changed since it had been uploaded last time", "Warning", MB_ICONWARNING);
	}

	if (online_) return;

	m_loadedSlots.clear();
	Traversal<CBlockVisitor>(editor_.get(), m_loadedSlots);
	m_loadedSlots.BeforeOnline();

	if (m_loadedSlots.size()) {
		Advise();
	}
}

void CAlgorithmExt::UploadToDevice(bool mb_confirm) noexcept {
	if (GetStatus() < npsys::CAlgorithm::status_assigned) {
		MessageBoxA(g_hMainWnd, "Assign first", "npsystem", MB_ICONINFORMATION);
		return;
	}

	if (mb_confirm &&
		MessageBoxA(g_hMainWnd, "Algorithm will be uploaded", "npsystem", MB_OKCANCEL) == IDCANCEL) {
		return;
	}

	StopOnline();

	auto device = assigned_dev.fetch();
	if (!device.loaded()) return;

	auto self = self_node.fetch();
	using Task = CDlg_LoadingManager::CTaskLoadOneAlgorithm;
	CDlg_LoadingManager manager(
		std::make_unique<Task>(device, self));
	manager.DoModal(g_hMainWnd);
}

void CAlgorithmExt::UnloadFromDevice() noexcept {
	if (GetStatus() < npsys::CAlgorithm::status_loaded) {
		MessageBoxA(g_hMainWnd, "Algorithm is not loaded!", "npsystem", MB_ICONINFORMATION);
		return;
	}

	StopOnline();

	auto device = assigned_dev.fetch();
	if (!device.loaded()) return;

	auto self = self_node.fetch();
	using Task = CDlg_LoadingManager::CTaskUnloadOneAlgorithm;
	CDlg_LoadingManager manager(
		std::make_unique<Task>(device, self));
	manager.DoModal(g_hMainWnd);
}

void CAlgorithmExt::DeletePermanent() noexcept {
	fbd_blocks.fetch_all_nodes();
	for (auto& block : fbd_blocks) {
		block->e_block->DeletePermanent();
	}
	editor_.remove();
	fbd_blocks.remove();
	view_position.remove();
}

void CAlgorithmExt::StopOnline() {
	if (!online_) return;
	
	m_loadedSlots.AfterOnline();
	m_loadedSlots.clear();

	UnAdvise();
	ref_.clear();
	online_ = false;
}

#ifdef DEBUG
void CAlgorithmExt::ShowBlocks() {
	std::cout << get_name() << ":\n Slots:\n";
	fbd_blocks.fetch_all_nodes();
	for (auto& block : fbd_blocks) {
		block->slots.fetch_all_nodes();
		for (auto& slot : block->slots) {
			std::cout << '\t' << block->get_name() << "(" << block.id() << ")."
				<< slot->get_name() << "(" << slot.id() << "):";
			auto var = slot->e_slot->GetVariable();
			if (var) {
				std::cout << ' ' << *var << '\n';
			} else {
				std::cout << '\n';
			}
		}
	}

	CFindVariableNodes vars;
	Traversal<CBlockVisitor>(GetBlocks(), vars);
	std::cout << " block variables:\n";
	for (auto var : vars) {
		std::cout << '\t' << *((*var).get()) << '\n';
	}

	if (assigned_alg.is_invalid_id() == false) {
		std::cout << " released references:\n";
		auto assigned = assigned_alg.fetch();
		auto const& released = assigned->references_released();
		released.fetch_all_nodes();
		for (const auto& ref : released) {
				std::cout << '\t' << *ref.get() << '\n';
		}
	}

	std::cout.flush();
}
#endif // DEBUG

void CAlgorithmExt::OnlineRestart() {
	if (IsOnline()) {
		StopOnline();
		StartOnline();
	}
}

size_t CAlgorithmExt::AdviseImpl() {
	auto device = assigned_dev.fetch();
	if (!device.loaded()) {
		assert(false);
		return 0;
	}
	auto dev_addr = device->dev_addr;
	{
		nprpc::Object* obj;
		npsys_rpc->server->CreateItemManager(obj);
		item_manager_.reset(nprpc::narrow<nps::ItemManager>(obj));
		assert(item_manager_);
		assert(item_manager_->policy_lifespan() == nprpc::Policy_Lifespan::Transient);
		if (!item_manager_) return 0;
		item_manager_->add_ref();
	}

	//m_mr->_setTimeout(g_cfg.server_timeout_sec+3, 0);
	
	auto this_oid = npsys_rpc->callback_poa->activate_object(this);
	item_manager_->Activate(this_oid);

	auto const size = m_loadedSlots.size();

	if (!size) return 0;

	std::vector<nps::DataDef> a(size);
	
	for (size_t i = 0; i < size; ++i) {
		auto slot = m_loadedSlots[i];

		slot->SetSelect(false);
		auto var = slot->GetSlotAssociateVariable();

		slot->OnlineGetValue().set_type(var->GetType());
		slot->SetValue(nullptr);

		a[i].dev_addr = dev_addr;
		a[i].mem_addr = var->GetAddr();
		a[i].type = var->GetType();
	}

	std::vector<nps::var_handle> handles;
	item_manager_->Advise(flat::make_read_only_span(a), handles);

	for (size_t i = 0; i < size; ++i) {
		ref_[handles[i]].push_back(m_loadedSlots[i]);
	}

	online_ = true;

	window_.Invalidate();
	
	return size;
}

// called from message loop
void CAlgorithmExt::UpdateData(std::vector<nps::server_value>* upd) {
	if (upd) {
		size_t size = upd->size();
		for (size_t i = 0; i < size; ++i) {
			const nps::server_value& v = (*upd)[i];
			for (auto& s : ref_[v.h]) s->SetValue(&v);
		}
		delete upd;
	} else {
		for (auto& r : ref_) {
			for (auto& s : r.second) {
				s->SetValue(nullptr);
			}
		}
	}
	window_.Invalidate();
}

// called from CORBA thread
void CAlgorithmExt::OnDataChangedImpl(::flat::Span_ref<nps::flat::server_value, nps::flat::server_value_Direct> a) {
	auto const len = a.size();
	auto upd = new std::vector<nps::server_value>(len);
	memcpy(&(*upd)[0], a.data(), len * sizeof(nps::server_value));
	window_.PostMessage(WM_DATA_UPDATED, (WPARAM)upd, 0);
}

// called monitor thread
void CAlgorithmExt::OnConnectionLost() noexcept {
	window_.PostMessage(WM_CONNECTION_LOSS, 0, 0);
}

} // namespace npsys