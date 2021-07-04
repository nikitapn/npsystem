// Copyright (c) 2021 nikitapnn1@gmail.com
// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory

#include "stdafx.h"
#include "algext.h"
#include "avrconfigurator.h"
#include "network_ext.h"
#include "avrassigned.h"
#include "codegenerator_avr.h"
#include "library.h"
#include "block.h"
#include "exception.h"
#include "error.h"
#include "variableloader.h"
#include "tr_item.h"
#include "module.h"
#include "io_avr.h"
#include <npsys/fbdblock.h>
#include <nplib/utils/hexparser.h>
#include <avr_firmware/twi.h>
#include <npdb/memento_manager.h>
#include <boost/iostreams/stream.hpp>
#include <boost/iostreams/device/null.hpp>

extern CProgressBarCtrl* g_progress;

int CAVR5DynamicLinker::FindObjFromPath(const std::string& path, const std::string& obj_addr, const std::string& obj_size, ObjFile* obj) {
	std::stringstream ss;
	ss << obj_size;
	ss >> std::hex >> obj->size;
	ss.clear();
	ss << obj_addr;
	ss >> std::hex >> obj->addr;

	size_t i = path.find_last_of('\\');
	obj->name = path.substr(i + 1);

	return (obj->size != 0x00);
}

// LD

const std::string& LD::RelativeLibPath() {
	static const std::string library = "..\\libraries\\avr\\";
	return library;
}

std::string LD::GetAdditionalObjFiles(npsys::AdditionalLibs& libs) {
	std::string files;
	for (auto& n : libs) {
		const auto& l = npsys::libs.get_by_name(n);
		for (const auto& o : l.libobjects) {
			files += RelativeLibPath() + l.name + "\\obj\\" + o.first + " ";
		}
	}
	return files;
}

int LD::GenDependencies(
	npsys::DepTable& table,
	npsys::AdditionalLibs& libs)
{
	global.ChangeCurrentDirectory(CurrentDirectory::BUILD);

	std::string input = GetAdditionalObjFiles(libs) + "alg.o";
	std::string temp;

	{
		std::stringstream ss;
		[[maybe_unused]] DWORD dwRet = CreateProccessWindowlessA(
			CGccWrapperAvr::GetAvrToolchainPath("avr-ld") + "--cref " + input, "", ss);
		temp = ss.str();
	}
	//	std::cout << "avr-ld exit code: " << dwRet << std::endl;

	boost::replace_all(temp, RelativeLibPath(), "<");
	boost::replace_all(temp, "\\obj\\", ">");

	std::stringstream cref;
	cref << temp;

	//	std::cout << temp << std::endl;

	std::string line;
	while (std::getline(cref, line) && line != "Cross Reference Table\r");

	// Generate tree from Cross Reference Table
	table.clear();
	boost::regex expr_1("^([0-9a-zA-Z_><]+)(\\s*)([0-9a-zA-Z_><]+.o)"),
		expr_2("^(\\s*)([0-9a-zA-Z_]+.o)");
	boost::smatch what;
	std::string cur;
	while (std::getline(cref, line)) {
		if (boost::regex_search(line, what, expr_1)) {
			cur = what[3];
		} else if (boost::regex_search(line, what, expr_2)) {
			table[what[2]].push_back(cur);
		}
	}
	for (auto& i : table) {
		std::cout << i.first << ":\n";
		for (auto& j : i.second) {
			std::cout << "    " << j << "\n";
		}
	}
	return 1;
}

int LD::Link(npsys::DepTable& table, const std::string& output_file, std::ostream& out) {
	global.ChangeCurrentDirectory(CurrentDirectory::ROOT);

	const auto& info = avr_->GetFirmwareInfo();
	std::string input;
	npsys::ObjectDescList objects;

	for (auto& i : table) {
		std::cout << i.first << ":\n";
		for (auto& j : i.second) {
			if (j[0] == '<') {
				auto itBegin = j.begin();
				auto itEnd = j.begin() + j.find('>');
				std::string lib(itBegin + 1, itEnd);
				std::string objName(itEnd + 1, j.end());
				Collect(objects, npsys::libs.get_by_name(lib), objName);
			} else {
				input += j + " ";
			}
		}
	}

	// lib's o + me o
	for (auto& o : objects) {
		input += RelativeLibPath() + o.lib + "\\obj\\" + o.obj + " ";
	}

	// my Obj
	global.ChangeCurrentDirectory(CurrentDirectory::BUILD);
	WIN32_FIND_DATAA FindData;
	HANDLE hFind = FindFirstFileA("*.o", &FindData);
	input += FindData.cFileName + std::string(" ");
	while (FindNextFileA(hFind, &FindData)) {
		input += FindData.cFileName + std::string(" ");
	}
	FindClose(hFind);
	global.ChangeCurrentDirectory(CurrentDirectory::ROOT);
	CGccWrapperAvr gcc;
	gcc.SetDir(global.GetBuildDir());
	gcc.SetPartno(info.mccfg.partno);
	gcc.AppendOptions("-nostartfiles -nodefaultlibs -nostdlib -Wl,-static -Wl,-Map=\"map.map\" -o " + output_file);
	return gcc.Compile(input, out);
}

void LD::Collect(
	npsys::ObjectDescList& obj,
	const npsys::CLibrary& lib,
	std::string objName)
{
	if (lib.table.find(objName) == lib.table.end()) {
		obj.insert({ lib.name, objName });
		return;
	}

	const auto& dep = lib.table.find(objName)->second;

	for (const auto& j : dep) {
		if (j[0] == '<') {
			auto itBegin = j.cbegin();
			auto itEnd = j.cbegin() + j.find('>');
			std::string lib(itBegin + 1, itEnd);
			std::string libObjName(itEnd + 1, j.end());
			if (obj.find({ lib, libObjName }) == obj.end()) {
				Collect(obj, npsys::libs.get_by_name(lib), libObjName);
			}
		} else {
			if (lib.table.find(j) != lib.table.end()) {
				Collect(obj, lib, j);
			} else {
				obj.insert({ lib.name, j });
			}
		}
	}
	obj.insert({ lib.name, objName });
}

void LD::GetLibraryObjects(
	const npsys::DepTable& table,
	npsys::ObjectDescList& obj)
{
	for (auto& i : table) {
		std::cout << i.first << ":\n";
		for (auto& j : i.second) {
			if (j[0] == '<') {
				auto itBegin = j.begin();
				auto itEnd = j.begin() + j.find('>');
				std::string lib(itBegin + 1, itEnd);
				std::string objName(itEnd + 1, j.end());
				Collect(obj, npsys::libs.get_by_name(lib), objName);
			}
		}
	}
}

void LD::GetModuleObjects(std::string& stModuleObjects) {
	WIN32_FIND_DATAA FindData;
	HANDLE hFind = FindFirstFileA("*.o", &FindData);
	stModuleObjects += FindData.cFileName + std::string(" ");
	while (FindNextFileA(hFind, &FindData)) {
		stModuleObjects += FindData.cFileName + std::string(" ");
	}
	FindClose(hFind);
}

// CAVR5DynamicLinker

CAVR5DynamicLinker::CAVR5DynamicLinker(npsys::controller_n_avr avr)
	: avr_(avr)
{
	avr_.fetch();
}

bool CAVR5DynamicLinker::AssignAlgorithm(npsys::algorithm_n& alg) {
	if (alg->GetStatus() > 0) {
		MessageBoxA(g_hMainWnd, ("Algorithm \"" + alg->get_name()
			+ "\" is already assigned").c_str(), "", MB_ICONEXCLAMATION);
		return false;
	}

	if (avr_->firmware_status != npsys::CAVRController::FIRMWARE_LOADED_DEVICE_ACTIVATED) {
		MessageBoxA(g_hMainWnd, "The Firmware is not loaded to the controller", "", MB_ICONEXCLAMATION);
		return false;
	}

	odb::Batch batch;
	auto device = avr_.cast<npsys::device_n>();
	auto assigned = odb::create_node<npsys::avr_assigned_algorithm_n>(alg, device)
		.cast<npsys::assigned_algorithm_n>();

	assigned.store();

	//	CFindElementsThatContainsVariablesExludeExternalReferences varElements;
	//	Traversal<CElement>(alg->GetBlocks(), varElements);
	//	for (auto var : varElements.m_all_variables) {
	//		if (!(*var).fetch()) assert(false);
	//		(*var)->SetDev(avr_.cast<npsys::device_n>());
	//		(*var).store();
	//	}

	avr_->assigned_algs.add_node(assigned);
	avr_->assigned_algs.store();
	alg->assigned_alg = assigned;
	alg->assigned_dev = avr_;
	alg->SetStatus(npsys::CAlgorithm::status_assigned);
	alg->Save();

	return batch.exec();
}

bool CAVR5DynamicLinker::AssignI2CModule(const npsys::i2c_module_n& mod, npsys::i2c_assigned_module_n& assigned) {
	odb::Batch batch;
	avr_->assigned_modules.fetch_all_nodes();

	ASSERT_FETCH(mod->childs);

	auto ctrl = avr_.cast<npsys::controller_n>();
	assigned = odb::create_node<npsys::i2c_assigned_module_n>(*mod.get(), ctrl);

	for (auto& segment : assigned->childs) {
		segment->parent_module = assigned;
	}

	assigned.store();

	avr_->assigned_modules.add_node(assigned);
	avr_->assigned_modules.store();

	return batch.exec();
}


CDynamicLinker::Result
CAVR5DynamicLinker::UploadAlgorithm(npsys::algorithm_n& alg) {
	using npsys::variable;
	using vs = npsys::variable::Status;
	using namespace std::string_literals;
	
	bool was_errors = false;

	if (!alg.fetch()) {
		ASSERT(FALSE);
		return Result::R_CRITICAL_ERROR;
	}

	auto assigned_alg = alg->assigned_alg.fetch();
	if (!assigned_alg.loaded()) {
		ASSERT(FALSE);
		return Result::R_CRITICAL_ERROR;
	}

	auto device = avr_.cast<npsys::device_n>();

	odb::NodeHolder holder;
	odb::NodeMementoManager<IUploadable> mementoUploadable(holder);
	odb::NodeListMementoManager mementoList(holder);

	mementoUploadable.AddMemento(alg);
	mementoUploadable.AddMemento(assigned_alg);
	mementoUploadable.AddMemento(avr_);

	{
		odb::Batch batch;
		alg->DetermineTypes(alg);
		alg->Save();
		if (batch.exec() == false) {
			std::cout << ">Database Error: Algorithm was not saved properly" << std::endl;
			return Result::R_CRITICAL_ERROR;
		}
	}

	std::unordered_map<int, std::unique_ptr<CMemoryManager>> mms;
	auto get_memory_manager = [&mms](npsys::device_n& dev, npsys::algorithm_n& alg) {
		auto founded = mms.find(dev->dev_addr);
		if (founded != mms.end()) return founded->second.get();
		auto ptr = CMemoryManager::CreateForAlgorithm(dev, alg, false);
		auto raw_ptr = ptr.get();
		mms.emplace(
			std::make_pair(dev->dev_addr, std::move(ptr))
		);
		return raw_ptr;
	};

	auto additional_libs_loaded = false;
	std::vector<npsys::avr_assigned_object_file_n> all_additional_objects;
	std::vector<npsys::avr_assigned_object_file_n> new_additional_objects;

	auto save_assigned_objects = [&]() {
		for (auto& obj : new_additional_objects) {
			avr_->assigned_ofs.add_node(obj);
			obj.store();
		}
		avr_->assigned_ofs.store();
	};

	try {
		odb::Batch batch;
		global.ChangeCurrentDirectory(CurrentDirectory::ROOT);
		auto assigned = static_cast<npsys::CAVRAssignedAlgorithm*>(assigned_alg.get());
		auto const oldaddr = (alg->GetStatus() < npsys::CAlgorithm::status_loaded
			? 0
			: avr_->PageToWord(assigned->Start()));
		if (oldaddr) {
			auto tt = avr_->GetTaskTable();
			if (std::find(tt.begin(), tt.end(), oldaddr) == tt.end()) {
				throw old_algorithm_was_not_found(oldaddr);
			}
		}

		g_progress->OffsetPos(10);

		const auto& info = avr_->GetFirmwareInfo();

		npsys::controllers_l ctrlList;
		ctrlList.fetch_all_nodes();

		auto this_mm = static_cast<CAvrMemoryManager*>(get_memory_manager(
			device,
			alg));

		ASSERT(this_mm);

		auto blocks = alg->GetBlocks();
		CFindSlots slots;
		CFindElementsThatContainsVariablesExcludeReferences standaloneVarElems;
		CFindExternalReferences varExtRef;
		//CFindInternalBlockRef intRefBlocks;
		Traversal<CBlockVisitor>(blocks, { &standaloneVarElems, &varExtRef, &slots/*, &intRefBlocks*/});

		//for (auto block : intRefBlocks) {
		//	block->Init(holder, avr_.get());
		//}

		auto invalid_link_warning =
			"The followed parameters will not be loaded to the controller due to invalid values: \r\n"s;

		for (auto& i : varExtRef.m_invalidRef) {
			invalid_link_warning += "\t" + i.second->GetFullName() + "\r\n";
		}

		if (!varExtRef.m_invalidRef.empty()) {
			invalid_link_warning += "Do you want to continue?"sv;
			auto result = ::MessageBoxA(g_hMainWnd, invalid_link_warning.c_str(), "Attention", MB_ICONEXCLAMATION | MB_YESNO);
			if (result == IDNO) {
				return Result::E_CANCELED;
			}
		}

		for (auto var_ptr : standaloneVarElems.m_all_variables) {
			auto& var = *var_ptr;
			ASSERT_FETCH(var);
			if (var->GetStatus() != vs::good) {
				mementoUploadable.AddMemento(var);
			}
			if (var->GetStatus() == vs::not_loaded) {
				var->SetAlg(alg);
				var->SetDev(avr_);
				var->AddRef();
			} else if (var->GetStatus() == vs::good_allocated_from_another_algorithm) {
				var->AddRef();
				var->SetStatus(vs::good);
			}
		}

		mementoList.AddMemento(avr_->allocated_vars_);
		mementoList.AddMemento(avr_->vars_owner_released_);
		
		for (auto var : standaloneVarElems) var->Alloc(*this_mm);

		for (auto& i : varExtRef) {
			if (!i.first->IsLinkValid()) continue;

			SortedLink sorted;
			color_m color;

			auto slot = i.second->top->self_node.fetch();
			i.first->TopoSort(alg, slot, color, sorted);

			for (auto& it : mms) it.second->SaveState();
			mementoList.SaveCurrentState();
			mementoUploadable.SaveCurrentState();

			bool error = false;
			bool soft_error = false;

			for (auto it = sorted.rbegin(); it != sorted.rend(); ++it) {
				if ((*it).alg->GetStatus() == npsys::CAlgorithm::status_not_assigned) {
					auto alg_name = (*it).alg->get_name();
					std::cout << ">WARNING: The following parameters"
						" will not be loaded. Algorithm \"" << alg_name << "\" is not assigned." << std::endl;
					for (; it != sorted.rend(); ++it) {
						auto alg_name = (*it).alg->get_name();
						std::cout << ">\t/" << alg_name << '/' << i.second->GetFullName() << std::endl;
					}
					error = true;
				}

				if (error) {
					was_errors = true;
					break;
				}

				auto curalg = (*it).alg;
				ASSERT_FETCH(curalg);

				auto curdev = (*it).alg->assigned_dev.fetch();

				switch ((*it).slot_type2) {
				case Value:
				{
					auto var = (*it).slot_type->GetVariableAsNode();
					if ((*var)->GetStatus() == vs::not_loaded) {
						mementoUploadable.AddMemento(*var);
						mementoUploadable.AddMemento((*it).alg);
						(*var)->SetDev(curdev);
						(*var)->SetAlg(curalg);
						// (*var)->AddRef(); no need to
						mementoList.AddMemento(curdev->allocated_vars_);
						get_memory_manager(curdev, curalg)->AllocateMemory(*var);
						if (curalg != alg) {
							(*var)->SetStatus(vs::allocated_from_another_algorithm);
						}
					}
					break;
				}
				case Peripheral:
				{
					auto slot_type = static_cast<CAvrInternalPin*>((*it).slot_type);
					mementoUploadable.AddMemento((*it).slot, slot_type);

					auto pin = slot_type->pin_;
					ASSERT_FETCH(pin);
					auto var = pin->GetVariable();
					ASSERT_FETCH(var);

					mementoUploadable.AddMemento(var);

					bool need_release = false;
					if (pin->GetStatus() == io::not_relevant && slot_type->IsLoaded()) {
						need_release = true;
					}

					if (!slot_type->IsPinTypeEqualParamDirection()) {
						std::cout << ">WARNING: " << i.second->GetFullName() << 
							" will not be loaded: wrong pin configuration." << std::endl;

						if (slot_type->IsLoaded()) {
							slot_type->ReleaseMemory();
							soft_error = true;
						} else {
							error = true;
						}
						break;
					}

					if (need_release) {
						std::cout << ">WARNING: " << i.second->GetFullName() <<
							" will not be loaded: pin configuration is not loaded." << std::endl;

						slot_type->ReleaseMemory();
						soft_error = true;
						break;
					}

					if (slot_type->IsLoaded()) break; // already good

					if (pin->GetStatus() != io::relevant) {
						std::cout << ">WARNING: " << i.second->GetFullName() <<
							" will not be loaded: pin configuration is not loaded." << std::endl;
						error = true;
						break;
					}

					var->AddRef();
					slot_type->SetLoaded();

					break;
				}
				case InternalReference:
					// already has passed checking in IsLinkValid()
					// no error
					break;
				case ModuleValue:
				{
					auto mv = static_cast<CModuleValue*>((*it).slot_type);
					const bool was_loaded = !mv->last_loaded_var_.is_invalid_node();

					if (was_loaded) ASSERT_FETCH(mv->last_loaded_var_);

					if (was_loaded && 
						mv->last_loaded_var_->IsGood() &&
						mv->last_loaded_var_ == mv->value->ass->var
						) continue;

					if (alg != curalg && was_loaded) {
						std::cout << ">WARNING: " << i.second->GetFullName() <<
							" will not be loaded - module value is in another algorithm and has been loaded before." << std::endl;
						error = true;
						break;
					}

					if (was_loaded) {
						ASSERT(alg == curalg);

						mementoUploadable.AddMemento(mv->last_loaded_var_);
						mementoUploadable.AddMemento(assigned_alg);
						assigned_alg->AddVariableReferenceToRemoveList(mv->last_loaded_var_);
						
						mementoUploadable.AddMemento((*it).slot, mv);
						mv->last_loaded_var_ = {};
					}

					if (mv->value->ass->GetStatus() != io::relevant) {
						std::cout << ">WARNING: " << i.second->GetFullName() <<
							" will not be loaded: module value is not loaded." << std::endl;
						soft_error = true;
						break;
					}

					if (!mv->IsPinTypeEqualParamDirection()) {
						std::cout << ">WARNING: " << i.second->GetFullName() <<
							" will not be loaded: wrong parameter type." << std::endl;
						soft_error = true;
						break;
					}

					ASSERT(mv->last_loaded_var_.is_invalid_node());
					ASSERT_FETCH(mv->value->ass->var);

					mementoUploadable.AddMemento(mv->value->ass->var);
					mv->value->ass->var->AddRef();
					
					mementoUploadable.AddMemento((*it).slot, mv);
					mv->last_loaded_var_ = mv->value->ass->var;

					break;
				}
				case ExternalReference:
				{
					enum {
						ext_ref_loaded = 0x01,
						ext_ref_not_loaded = 0x02,
						ref_alg_is_assigned = 0x04,
						ref_alg_is_not_assigned = 0x08
					};

					auto& node = (*it).slot;
					auto slot = node->e_slot.get();
					auto ext = static_cast<CExternalReference*>((*it).slot_type);
					auto var_ptr = ext->GetVariableAsNode();
					auto ref_var_ptr = ext->GetRefVariableAsNode();
					auto ref_var = ref_var_ptr ? *ref_var_ptr : npsys::variable_n();

					if (ref_var->IsQuality() == false) {
						std::cout << ">WARNING: " << i.second->GetFullName() <<
									" will not be loaded: external links to parameters with no quality are not not allowed." << std::endl;
						error = true;
						break;
					}

					auto release_variables = [&]() -> void {
						ASSERT(curalg == alg);

						mementoUploadable.AddMemento(assigned_alg);
						mementoUploadable.AddMemento(node, ext);
						mementoUploadable.AddMemento(*var_ptr);

						if (ext->GetPlace() == CExternalReference::SAME_DEVICE) {
							assigned_alg->AddVariableReferenceToRemoveList((*var_ptr));
						} else {
							(*var_ptr)->OwnerRelease(&mementoList);
							assigned->AddExternalReferenceToRemoveList(ext);
							ext->link_.loaded_variable = {};
						}
						(*var_ptr) = odb::create_node<npsys::variable_n>(
							npsys::variable::VT_DISCRETE | npsys::variable::VQUALITY);
						
						ext->loaded_ = false;
					};

					int state =
						(ext->IsRefAlgorithmAssigned()
							? ref_alg_is_assigned
							: ref_alg_is_not_assigned) |
							(ext->loaded_
								? ext_ref_loaded
								: ext_ref_not_loaded);

					bool done = false;

					while (!done)
						switch (state)
						{
						case ref_alg_is_not_assigned:
							std::cout << ">WARNING: " << i.second->GetFullName()  <<
								" will not be loaded: the reference algorithm is not assigned." << std::endl;
							state |= ext->link_.loaded_variable.is_invalid_id()
								? ext_ref_not_loaded
								: ext_ref_loaded;
							break;
						case ref_alg_is_not_assigned | ext_ref_not_loaded:
							done = true;
							error = true;
							break;
						case ref_alg_is_not_assigned | ext_ref_loaded:
							if (curalg == alg) {
								release_variables();
								done = true;
								soft_error = true;
							} else {
								error = true;
							}
							break;
						case ref_alg_is_assigned | ext_ref_loaded:
							if (ext->GetPlace() == CExternalReference::SAME_DEVICE) {
								if (
									ext->variable_.id() == ref_var.id() &&
									ref_var->IsGood()
									) {
									done = true;
									break;
								}
							} else {
								ASSERT(!ext->link_.loaded_variable.is_invalid_id());
								if (
									ext->link_.loaded_variable.id() == ref_var.id() &&
									ref_var->IsGood()
									) {
									done = true;
									break;
								}
							}

							if (curalg != alg) {
								std::cout << ">WARNING: " << i.second->GetFullName() <<
									" will not be loaded: the reference variable type has changed and that variable is still in use and \"" 
									<< curalg->get_name() << "\" is not a load target." << std::endl;
								done = true;
								error = true;
								break;
							}

							release_variables();

							state = ref_alg_is_assigned | ext_ref_not_loaded;
							break;
						case ref_alg_is_assigned | ext_ref_not_loaded:
						{
							ASSERT(ref_var->GetStatus() == vs::good
								|| ref_var->GetStatus() == vs::allocated
								|| ref_var->GetStatus() == vs::allocated_from_another_algorithm
								|| ref_var->GetStatus() == vs::good_allocated_from_another_algorithm);

							auto link_type = slot->GetDirection() == CSlot::SLOT_DIRECTION::INPUT_SLOT
								? npsys::remote::ExtLinkType::Write
								: npsys::remote::ExtLinkType::Read;

							auto place = ref_var->GetDevAddr() == curdev->dev_addr
								? CExternalReference::SAME_DEVICE
								: CExternalReference::OTHER_DEVICE;

							if (link_type == npsys::remote::ExtLinkType::Write
								&& place == CExternalReference::OTHER_DEVICE) {
								std::cout << ">WARNING: " << i.second->GetFullName() <<
									" will not be loaded: external links to different devices are only for reading." << std::endl;
								done = true;
								error = true;
								break;
							}

							mementoUploadable.AddMemento(node, ext);
							ext->SetType(link_type);

							ASSERT(ext->variable_->GetStatus() == vs::not_loaded);
							ASSERT(ext->link_.loaded_variable.is_invalid_id());

							if (place == CExternalReference::SAME_DEVICE) {
								ext->SetPlace(place);
								mementoUploadable.AddMemento(ref_var);
								ref_var->AddRef();
								ext->variable_ = ref_var;
								ext->link_.loaded_variable = {};
							} else {
								ext->SetPlace(place);
								mementoUploadable.AddMemento(ref_var);
								ref_var->AddRef();
								mementoUploadable.AddMemento(ext->variable_);
								ext->variable_->AddRef();
								ext->variable_->SetDev(curdev);
								ext->variable_->SetAlg(curalg);
								ext->variable_->SetType((ref_var->GetType() | npsys::variable::VQUALITY) & (~npsys::variable::IO_SPACE));

								ext->link_.loaded_variable = ref_var;

								mementoList.AddMemento(curdev->allocated_vars_);
								get_memory_manager(curdev, curalg)->
									AllocateMemory(ext->variable_, ref_var->IsBit() ? ref_var->GetBit() : -1);

								mementoUploadable.AddMemento(curdev);
								curdev->AddLink(ext->variable_, ref_var, ext->GetType());
							}
							ext->loaded_ = true;
							done = true;
							break;
						}
						default:
							ASSERT(FALSE);
							break;
						}
					break;
				}
				default:
					ASSERT(FALSE);
					break;
				}
			}

			if (error) {
				for (auto& it : mms) it.second->RollbackState();
				mementoList.RestoreSavedState();
				mementoUploadable.RestoreSavedState();
			} else if (soft_error) {
				// ok
			}
		}

		g_progress->OffsetPos(10);
		CAVR5CodeGenerator code(alg, info);
		alg->Translate(&code);
		code.SaveToFile();

		const auto alg_id_file_path = "algorithms\\"s + std::to_string(alg->id()) + "\\alg.c"s; ;
		
		std::unique_ptr<CGccWrapperAvr> gcc = std::move(code.gcc_);

		gcc->SetDir(global.GetBuildDir());
		gcc->SetPartno(info.mccfg.partno);
		gcc->AppendOptions("-c -nostartfiles -nodefaultlibs -nostdlib -std=gnu99"s);

		std::ostringstream out;
		auto exit_code = gcc->Compile("..\\" + alg_id_file_path, out);
		if (exit_code) {
			std::cout << out.str() << std::endl;
			throw compile_error(exit_code);
		}

		if (code.libraries.size()) {
			LD ld(LD::Library, avr_);
			npsys::DepTable depTable;
			ld.GenDependencies(depTable, code.libraries);
			out.str("");
			auto error_code = ld.Link(depTable, "alg.a", out);
			if (!error_code) {
				auto result = CreateAdditionalObjects(depTable);
				all_additional_objects = std::move(std::get<0>(result));
				new_additional_objects = std::move(std::get<1>(result));
			} else {
				std::cout << out.str() << std::endl;
				throw compile_error(error_code);
			}
		}

		std::string defsym;
		if (all_additional_objects.size()) {
			UploadAdditionalObjects(this_mm, all_additional_objects, defsym);
			additional_libs_loaded = true;
		}

		g_progress->OffsetPos(10);

		gcc->Reset();
		gcc->SetDir(global.GetBuildDir());
		gcc->SetPartno(info.mccfg.partno);
		gcc->AppendInclude("..\\avr8-gnu-toolchain\\avr\\include\\"s);
		gcc->AppendOptions("-T./ls.x -std=gnu99 -nostartfiles -nodefaultlibs -nostdlib -Wl,-static -Wl,-Map=\"alg.map\""s + defsym);

		auto alg_size = gcc->GetSize("alg.o");
		auto page = this_mm->AllocatePageMemory(alg_size);

		gcc->CreateAlgorithmLinkerScript(avr_->PageToByte(page), info);

		out.str("");
		exit_code = gcc->Link("alg.o", "alg.elf", out);
		if (exit_code) {
			std::cout << out.str() << std::endl;
			throw compile_error(exit_code);
		}

		gcc->CreateLss("alg.elf");
		gcc->CreateHex("alg.elf");

		// Uploading algorithm
		HASHMD5 hash;
		if (GetFileMD5(alg_id_file_path, hash)) {
			ASSERT(FALSE);
		}
		
		bool changed =
			(
			(alg->GetStatus() == npsys::CAlgorithm::status_assigned) ||
				(hash != assigned->GetHash())
				);

		if (changed) {
			HexParserAlgorithm hp("build\\alg.hex", info.pmem.pagesize);

			ASSERT(hp.GetAlgSize() == alg_size);

			byte buffer[512];
			int cur_page = page;
			int lenght;

			while (hp.GetPageData(buffer, lenght)) {
				auto code = avr_->WritePage(cur_page, lenght, buffer);
				if (code == 0) {
					g_progress->OffsetPos(2);
					cur_page++;
				} else throw operation_write_page_failed(code);
			}
			if (avr_->IsVirtual()) {
				npsys_rpc->server->AVR_V_StoreFlash(avr_.id());
			}
		}
		
		// return Result::R_ERROR;

		g_progress->OffsetPos(10);

		if (changed && oldaddr) {
			if (!avr_->StopAlgorithm(oldaddr)) throw operation_stop_algorithm_failed();
		}

		g_progress->OffsetPos(2);

		// Uploading Defaults Values
		var_raw_ptr_list allocatedReferencesFromThisAlgorithm;

		for (auto& ref : varExtRef.m_externalRef) {
			if (!ref.first->IsLinkValid()) continue;
			auto var = ref.first->GetVariable();
			if ((var->GetAlg() == alg) && var->GetStatus() == vs::allocated) {
				ASSERT(!var->IsIO());
				allocatedReferencesFromThisAlgorithm.push_back(var);
			}
		}

		CAlgorithmVariableLoader varLoader(
			device,
			alg,
			standaloneVarElems.m_all_variables,
			allocatedReferencesFromThisAlgorithm
		);

		varLoader.Load();
		varLoader.Commit();

		g_progress->OffsetPos(5);

		assigned->ReleaseRemovedReferences();
		avr_->UploadRemoteData();

		g_progress->OffsetPos(10);

		if (changed) {
			// Replacing algorithm
			auto code = avr_->ReplaceAlg(oldaddr, avr_->PageToWord(page));
			if (code) throw operation_replace_algorithm_failed(code);
			assigned->SetRange(avr_->PageToByte(page), alg_size, avr_->GetPageSizeInBytes());
			assigned->SetHash(hash);
			if (avr_->IsVirtual()) {
				npsys_rpc->server->AVR_V_StoreFlash(avr_.id());
			}
			g_progress->OffsetPos(10);
		}

		alg->SetStatus(npsys::CAlgorithm::status_loaded);

		// .....................SAVING..................... //
		for (auto& i : mms) {
			i.second->Commit();
		}

		for (auto& slot : slots) {
			auto var = slot->GetSlotAssociateVariable();
			if (var == nullptr && !slot->top->var.is_invalid_node()) {
				slot->top->var = {};
				slot->top->set_modified();
				auto n = slot->top->self_node.fetch();
				holder.AddNode(n);
			} else if (var != nullptr && var->self_node.id() != slot->top->var.id()) {
				slot->top->var = var->self_node.fetch();
				slot->top->set_modified();
				auto n = slot->top->self_node.fetch();
				holder.AddNode(n);
			}
		}

		holder.StoreAll();

		if (additional_libs_loaded) {
			save_assigned_objects();
		}

		if (avr_->IsVirtual()) {
			npsys_rpc->server->AVR_V_StoreFlash(avr_.id());
		}
		
		// ................................................ //
		if (batch.exec()) {
			// Restart online session if needed
			alg->OnlineRestart();
			g_progress->OffsetPos(10);
			return was_errors ? Result::R_ERROR : Result::R_OK;
		} else {
			std::cerr << ">Error: could not update the database, changes will be saved locally...\n";
			// not impl...
			// not impl...
			// not impl...
			// not impl...
			// not impl...


			g_progress->OffsetPos(10);
			return Result::R_ERROR;
		}
	} catch (old_algorithm_was_not_found& ex) {
		ex;
		std::cerr << ">Existing algorithm was not found in the controller.\r\nAbort...\n";
	} catch (sram_bad_alloc& ex) {
		std::cerr << ">There is no free memory for variables in device \""
			<< ex.dev->get_name() << "\".\n";
	} catch (flash_bad_alloc&) {
		std::cerr << ">There is not enough flash memory for this algorithm...\n";
	} catch (operation_stop_algorithm_failed& ex) {
		ex;
	} catch (operation_write_page_failed& ex) {
		ex;
	} catch (operation_replace_algorithm_failed& ex) {
		ex;
	} catch (compile_error& ex) {
		ex;
	} catch (symbol_not_found& ex) {
		ex;
	} catch (function_not_found& ex) {
		ex;
	} catch (nprpc::Exception& ex) {
		std::cerr << ">NPRPC Error: " << ex.what() << '\n';
	} catch (std::exception& ex) {
		std::cerr << ">Unknown exception: " << ex.what() << '\n';
	}

	mementoList.RestoreAll();
	mementoUploadable.RestoreAll();

	odb::Batch batch;
	if (additional_libs_loaded) save_assigned_objects();
	batch.exec();
	
	return Result::R_CRITICAL_ERROR;
}

std::tuple<std::vector<npsys::avr_assigned_object_file_n>,
	std::vector<npsys::avr_assigned_object_file_n>>
	CAVR5DynamicLinker::CreateAdditionalObjects(npsys::DepTable& table) {
	npsys::ObjectDescList objects;

	LD ld(LD::Module, avr_);
	ld.GetLibraryObjects(table, objects);

	std::vector<npsys::avr_assigned_object_file_n> all;
	std::vector<npsys::avr_assigned_object_file_n> only_new;
	avr_->assigned_ofs.fetch_all_nodes();
	auto assigned_begin = avr_->assigned_ofs.begin();
	auto assigned_end = avr_->assigned_ofs.end();

	for (auto& o : objects) {
		auto it = std::find_if(assigned_begin, assigned_end, [&o](npsys::avr_assigned_object_file_n& aof) {
			return aof->desc == o;
		});
		if (it == assigned_end) {
			auto ofile = odb::create_node<npsys::avr_assigned_object_file_n>(avr_, o);
			auto& lib = npsys::libs.get_by_name(o.lib);
			auto& obj = lib.libobjects.find(o.obj)->second;
			std::copy(obj.expf.begin(), obj.expf.end(), std::back_inserter(ofile->expf));
			only_new.push_back(ofile);
			all.push_back(ofile);
		} else {
			all.push_back((*it));
		}
	}
	return std::make_tuple(std::move(all), std::move(only_new));
}

void CAVR5DynamicLinker::UploadAdditionalObjects(
	CAvrMemoryManager* this_mm,
	std::vector<npsys::avr_assigned_object_file_n>& additionalObjects,
	std::string& defsym)
{
	byte buffer[512];
	const int dev_addr = avr_->dev_addr;
	const int iPageSize = avr_->GetPageSizeInBytes();
	const auto& info = avr_->GetFirmwareInfo();

	CGccWrapperAvr gcc;
	std::vector<std::pair<std::string, npsys::CAVRAssignedObjectFile*>>
		moduleObjectsMustBeLoadFirst;
	std::string input;
	for (auto& obj : additionalObjects) {
		if (obj->status == io::Status::assigned) {
			std::string path = "..\\libraries\\avr\\" + obj->desc.lib + "\\obj\\" + obj->desc.obj;
			input += path + " ";
			moduleObjectsMustBeLoadFirst.push_back({ path, obj.get() });
		} else {
			defsym += obj->DefSym();
		}
	}

	std::string elf = "library.elf";

	// Library
	if (!moduleObjectsMustBeLoadFirst.empty()) {
		gcc.SetDir(global.GetBuildDir());
		gcc.SetPartno(info.mccfg.partno);
		gcc.AppendOptions("-T./ls.x -nostartfiles -nodefaultlibs -nostdlib -Wl,-static -Wl,-Map=\"library.map\"" + defsym);
		
		boost::iostreams::stream<boost::iostreams::null_sink> null_sink( (boost::iostreams::null_sink()) );
	
		gcc.Link(input, elf, null_sink);

		auto objectsSize = gcc.GetSize(elf);
		auto page = this_mm->AllocatePageMemory(objectsSize);

		gcc.CreateLinkerScript(avr_->PageToByte(page));

		//	gcc.AppendOptions("-Wl,-Ttext," + to_hex(m_pAvrCtrl->PageToWord(page)));
		gcc.Link(input, elf, null_sink);
		gcc.CreateLss(elf);
		gcc.CreateHex(elf);

		ParseObjectFunctions(moduleObjectsMustBeLoadFirst);

		// load ...
		std::cout << "Loading additional libraries..." << std::endl;

		HexParserAlgorithm hp("build\\library.hex", iPageSize);
		int cur_page = page;
		int lenght;
		while (hp.GetPageData(buffer, lenght)) {
			auto code = avr_->WritePage(cur_page, lenght, buffer);
			if (code == 0) {
				g_progress->OffsetPos(2);
				cur_page++;
			} else throw operation_write_page_failed(code);
		}

		std::cout << "Additional libraries have been loaded..." << std::endl;

		if (avr_->IsVirtual()) {
			npsys_rpc->server->AVR_V_StoreFlash(avr_.id());
		}
		for (auto& obj : moduleObjectsMustBeLoadFirst) {
			obj.second->status = io::Status::relevant;
		}
	}
	for (auto& obj : moduleObjectsMustBeLoadFirst) {
		defsym += obj.second->DefSym();
	}
}

void CAVR5DynamicLinker::ParseObjectFunctions(
	std::vector<std::pair<std::string, npsys::CAVRAssignedObjectFile*>>& objects)
{
	int p_size = avr_->GetPageSizeInBytes();
	CGccWrapperAvr gcc;
	gcc.SetDir(global.GetBuildDir());
	auto symbols = gcc.GetSymbolTable("library.elf", "-g");

	std::ifstream file(global.GetBuildDir() + "library.map");
	std::string mfile((std::istreambuf_iterator<char>(file)),
		std::istreambuf_iterator<char>());
	file.close();

	for (auto& obj : objects) {
		std::string tmp = obj.first;
		boost::replace_all(tmp, "\\", "\\\\");
		boost::replace_all(tmp, ".", "\\.");

		boost::regex expr("(0x\\w+)(\\s+)(0x\\w+)(\\s+)(" + tmp + ")$");
		boost::smatch what;
		int origin, size = 0;

		std::string::const_iterator start, end;
		start = mfile.begin();
		end = mfile.end();
		boost::match_flag_type flags = boost::match_default;

		while (boost::regex_search(start, end, what, expr, flags)) {
			if (what[3] == "0x0") {
				// update search position:
				start = what[0].second;
				// update flags:
				flags |= boost::match_prev_avail;
				flags |= boost::match_not_bob;
				continue;
			}
			origin = nplib::format::from_hex(what[1]);
			size = nplib::format::from_hex(what[3]);
			break;
		}

		ASSERT(size != 0);

		obj.second->SetRange(origin, size, p_size);
		// std::cout << ("%s : p - %d : size - %d\n", obj.o->GetName(), obj.o->m_object->Start(), obj.o->m_object->AllocSizeP());

		auto osymbols = gcc.GetSymbolTable(obj.first, "-g");
		for (auto& sym : osymbols) {
			if (sym.type == 'T') {
				auto& name = sym.name;
				auto it = std::find_if(symbols.begin(), symbols.end(), [&name](const Symbol& sym) {
					return name == sym.name;
				});
				if (it == symbols.end()) {
					throw symbol_not_found(name);
				}
				obj.second->SetFunaddr(it->name, it->addr);
			}
		}
	}
	std::string error;
	for (auto& obj : objects) {
		if (!obj.second->CheckFunctions(error)) {
			throw function_not_found(error);
		}
	}
}

CDynamicLinker::Result
CAVR5DynamicLinker::UnloadAlgorithm(npsys::algorithm_n& alg) {
	auto blocks = alg->GetBlocks();
	auto assigned = alg->assigned_alg.fetch();
	if (!assigned.loaded()) {
		std::cout << "Assigned algorithm not found" << std::endl;
		return Result::R_CRITICAL_ERROR;
	}
	auto avr_assigned = assigned.cast<npsys::avr_assigned_algorithm_n>();
	auto avr = alg->assigned_dev.fetch().cast<npsys::controller_n_avr>();
	ASSERT_FETCH(avr);
	try {
		odb::Batch first_batch;

		if (avr_->RemoveAlg(avr_->PageToWord(avr_assigned->Start()))) {
			std::cout << "Cannot find \"" << alg->get_name() << "\" in the controller.\r\n"
				"Algorithm will be deleted from controller database"<< std::endl;
		}

		g_progress->OffsetPos(10);

		CReleaseVariables releaser;
		CResetVariables cleaner(true);

		Traversal<CBlockVisitor>(blocks, { &releaser, &cleaner }); // order matters

		g_progress->OffsetPos(2);
		
		assigned->ReleaseRemovedReferences();

		g_progress->OffsetPos(10);

		auto device = avr_.cast<npsys::device_n>();
		CAlgorithmVariableLoader varLoader(device, alg);
		varLoader.Load();
		varLoader.Commit();

		g_progress->OffsetPos(30);

		first_batch.exec();

		// new values
		odb::Batch second_batch;
		avr_->UploadRemoteData();

		g_progress->OffsetPos(20);
		
		for (auto& block : alg->fbd_blocks) {
			for (auto& slot : block->slots) slot.store();
			block.store();
		}

		alg->SetStatus(npsys::CAlgorithm::status_assigned);
		
		alg.store();
		assigned.store();
		avr.store();

		second_batch.exec();

		g_progress->OffsetPos(20);

		return Result::R_OK;
	} catch (nps::NPNetCommunicationError& ex) {
		std::cout << "Communication failed: " << TranslateError(ex.code).data() << std::endl;
	} catch (std::exception& ex) {
		std::cout << "Caught Exception: " << ex.what() << std::endl;
	}

	return Result::R_CRITICAL_ERROR;
}

CDynamicLinker::Result
CAVR5DynamicLinker::UploadI2CModule(npsys::i2c_assigned_module_n& target) {
	static_assert(sizeof(twi_req_t) == 8, "twi_req_t size is not 8");
	static_assert(TWI_TABLE_SIZE < 64, "TWI_TABLE_SIZE >= 64");

	using npsys::CI2CModuleSegment;
	using npsys::CI2CModuleSegmentValue;

	const auto& info = avr_->GetFirmwareInfo();

	ASSERT_FETCH(target);
	ASSERT_FETCH(target->childs);
	
	for (auto& s : target->childs) {
		auto len = s->GetLenght();
		if (len > info.mccfg.twi_usefull_buffer_max) {
			std::cerr << "Error: Twi buffer is to small\n";
			return CDynamicLinker::Result::R_ERROR;
		}
	}

	unsigned char buffer[512];
	twitab_t* tab = (twitab_t*)buffer;
	
	avr_->assigned_modules.fetch_all_nodes();
	auto& all = avr_->assigned_modules;

	std::vector<twi_req_t> lrr;

	size_t ix = 0;
	for (auto& mod : all) {
		if (mod == target) continue;
		for (auto& twreq : mod->lrr) {
			tab->twi_req[ix++] = twreq;
		}
	}

	auto mm = CMemoryManager::CreateGeneric(avr_.cast<npsys::device_n>(), true);
	std::vector<std::pair<CI2CModuleSegment*, CVarContainer>> vcs;

	odb::NodeHolder holder;
	odb::NodeListMementoManager mementoList(holder);
	odb::NodeMementoManager<IUploadable> mementoUploadable(holder);

	mementoUploadable.AddMemento(avr_);

	avr_->allocated_vars_.fetch_all_nodes();
	mementoList.AddMemento(avr_->allocated_vars_);

	bool relevant_module = true;
	
	try {
		odb::Batch batch;
		for (auto& segment : target->childs) {
			CVarContainer vc;
			bool segment_not_relevant = false;
			for (auto& value : segment->values) {
				if (value->ass->GetStatus() != io::Status::relevant) {
					segment_not_relevant = true;
					relevant_module = false;
					break;
				}
			}
			for (auto& value : segment->values) {
				mementoUploadable.AddMemento(target->childs.node(), value->ass.get());
				
				if (value->ass->var.is_invalid_node()) {
					value->ass->var = odb::create_node<npsys::variable_n>(value->GetType());
					mementoUploadable.AddMemento(value->ass->var);
					value->ass->var->SetDev(avr_.cast<npsys::device_n>());
					value->ass->var->AddRef();
				} else {
					ASSERT_FETCH(value->ass->var);
				}
				
				if (segment_not_relevant) {
					// variable that wasn't affected must be removed also
					if (value->ass->GetStatus() == io::Status::relevant) {
						mementoUploadable.AddMemento(value->ass->var);
						value->ass->var->OwnerRelease(&mementoList);
						value->ass->var = odb::create_node<npsys::variable_n>(value->GetType());
						value->ass->var->SetDev(avr_.cast<npsys::device_n>());
						value->ass->var->AddRef();
					}
					mementoUploadable.AddMemento(value->ass->var);
				}

				vc.AddVariable(value->ass->var); // ! must be in the end
			}

			if (vc.Empty()) continue;

			if (segment_not_relevant) {
				mm->AllocateMemory(&vc);
			}

			vcs.emplace_back(segment.get(), vc);
		}

		for (auto& vc : vcs) {
			io::SegmentType segType = vc.first->GetType();
			twi_req_t* r = &tab->twi_req[ix++];
			if (target->addr_size() == 0) {
				r->fn = (segType == io::SegmentType::READ) ? TWI_REQ_READ : TWI_REQ_WRITE;
				r->addr = 0;
			} else {
				r->fn = (segType == io::SegmentType::READ) ? TWI_REQ_READ_BLOCK : TWI_REQ_WRITE_BLOCK;
				r->addr = vc.first->GetOffset();
				if (target->addr_size() == 2) r->addr |= (1 << 15);
			}
			r->dev_addr = target->slave_addr();
			r->data = vc.second.GetOrigin();
			r->len = vc.second.VarByteSize();
			lrr.push_back(*r);
		}

		if (relevant_module) {
			std::cout << ">Module is relevant. There is nothing to do." << std::endl;

			if (MessageBoxA(g_hMainWnd, "Do you wish to reload it?", "npsystem", MB_YESNO) == IDNO) {
				return Result::R_OK;
			}
		}

		tab->n = static_cast<uint8_t>(ix);

		
		int page_size = static_cast<int>(sizeof(twitab_t) + ix * sizeof(twi_req_t));
		avr_->WriteTwiTable(info.pmem.twi, page_size, buffer);

		for (auto& i : vcs) {
			for (auto& value : i.first->values) {
				value->ass->SetStatus(io::Status::relevant);
			}
		}

		mm->Commit();

		auto device = avr_.cast<npsys::device_n>();
		CSimpleVariableLoader loader(device);
		loader.Load();
		loader.Commit();

		target->lrr = lrr;
		target.store();

		holder.StoreAll();
		
		batch.exec();

		return Result::R_OK;
	} catch (nps::NPNetCommunicationError& ex) {
		std::cout << "Communication failed: " << TranslateError(ex.code).data() << std::endl;
	} catch (std::exception& ex) {
		std::cout << "Caught Exception: " << ex.what()<< std::endl;
	}

	mementoList.RestoreAll();
	mementoUploadable.RestoreAll();

	return Result::R_CRITICAL_ERROR;
}

bool CAVR5DynamicLinker::UnloadModule(npsys::i2c_assigned_module_n& mod) {
	return 0;
}
