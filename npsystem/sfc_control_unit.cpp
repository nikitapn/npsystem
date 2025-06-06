// Copyright (c) 2021 nikitapnn1@gmail.com
// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory

#include "stdafx.h"
#include "control_unit_ext.h"
#include "avrconfigurator.h"
#include "codegenerator.h"
#include "block.h"
#include "line.h"
#include "graphics.h"
#include "assignedalgorithm.h"
#include "error.h"
#include "view_algorithm.h"
#include "dlgalgorithmproperties.h"
#include <npsys/common_rpc.hpp>
#include "network_ext.h"
#include <npsys/algcat.h>
#include <npsys/algsubcat.h>
#include "dlgloadingmanager.h"
#include "config.h"

#include "sfc_block_composition.hpp"
#include "View.h"
#include "sfc_block_visitor.hpp"
#include "sfc_block.hpp"

namespace npsys {

CSFCControlUnit::CSFCControlUnit(std::string&& name, npsys::algorithm_category_n& cat) {
	this->set_name(name);
	this->editor_.create(std::true_type{});
	this->view_position.create();
	this->view_position->DefaultViewPosition();
	this->view_position.store();
	this->editor_->SetAlgorithm(this);
	this->cat = cat;
	this->set_modified(false);
}

CBlockComposition* CSFCControlUnit::GetBlocks() {
	if (editor_.loaded()) return editor_.get();
	if (editor_.fetch()) {
		editor_->SetAlgorithm(this);
		return editor_.get();
  }
	return nullptr;
}

void CSFCControlUnit::DeletePermanent() noexcept {
	if (editor_.fetch()) {
		CSFCBlocksInspector blocks;
		Traversal<CBlockVisitor>(editor_.get(), blocks);
		for (auto b : blocks) b->DeletePermanent();
	}
	editor_.remove();
	view_position.remove();
}

void CSFCControlUnit::UploadToDevice(bool mb_confirm) noexcept {

}

void CSFCControlUnit::UnloadFromDevice() noexcept {

}

void CSFCControlUnit::Unload() {

}

void CSFCControlUnit::HardwareSpecific_Clear() {

}

void CSFCControlUnit::Save(bool after_uploading) noexcept {
	auto editor = GetBlocks();

	if (after_uploading) {
		SetStatus(Status::status_loaded);
	} else if (IsUndoRedo() && (GetStatus() >= Status::status_loaded)) {
		SetStatus(Status::status_not_actual);
	}

	editor_->PermanentExecuteCommands();
	actions.ExecuteAll();

//	CFindVariableNodes vars;
//	CFindSlots slots;
//	CFindInternalBlockRef intRefBlocks;
//	Traversal<CBlockVisitor>(editor, vars, slots, intRefBlocks);
//	
//	if (assigned_dev.is_invalid_id() == false) {
//		auto dev = assigned_dev.fetch();
//		intRefBlocks.InitInternalReferences(dev.get());
//	}
//
//	for (auto& block : fbd_blocks) {
//		Iter::PreorderIterator<CElement*> it(block->e_block.get());
//		for (; !it.IsDone(); it.Next()) {
//			if ((*it)->IsCreatedNew()) {
//				block->set_modified();
//				break;
//			}
//		}
//	}
//
//	for (auto s : slots) {
//		s->CommitTypeChanges();
//	}
//
//	for (auto& s : slots.disabled_slots) {
//		if (s.loaded() && s->is_modified()) {
//			s.store();
//			auto var = s->e_slot->GetVariableAsNode();
//			if (var && var->loaded() && (*var)->is_modified()) {
//				var->store();
//			}
//		}
//	}
//
//	for (auto& v : vars) {
//		if ((*v)->is_modified()) {
//			(*v).store();
//		}
//	}
//
//	for (auto& block : fbd_blocks) {
//		for (auto& slot : block->slots) {
//			if (slot->is_modified() || slot->e_slot->IsModified()) {
//				slot->fbd_unit = this->self_node;
//				slot->e_slot->ResetState();
//				slot.store();
//			}
//		}
//		if (block->is_modified()) {
//			block.store();
//		}
//	}
//
//	fbd_blocks.store();

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
}

INT_PTR CSFCControlUnit::ShowProperties(control_unit_n& self) {
	return 0;
}

NPSystemIcon CSFCControlUnit::GetIcon() const noexcept {
	return NPSystemIcon::SFC;
}

void CSFCControlUnit::StartOnline() {

}

void CSFCControlUnit::StopOnline() {

}
void CSFCControlUnit::DrawOnlineValues(CGraphics* pGraphics) {

}

bool CSFCControlUnit::IsUndoRedo() {
	return editor_.loaded()
		&& (editor_->IsRedoCommands() || editor_->IsUndoCommands());
}

void CSFCControlUnit::UpdateData(std::vector<nps::server_value>* upd) {

}

size_t CSFCControlUnit::AdviseImpl() {
	return 0;
}

void CSFCControlUnit::OnDataChangedImpl(nprpc::flat::Span_ref<nps::flat::server_value, nps::flat::server_value_Direct> a) {
}
void CSFCControlUnit::OnConnectionLost() noexcept {
}

} // namespace npsys
