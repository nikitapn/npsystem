// Copyright (c) 2021 nikitapnn1@gmail.com
// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory

#include "stdafx.h"
#include "history.h"
#include <npsys/network.h>
#include <npsys/variable.h>

/*
void history::OnNodeEvent(oid_t id, odb::NodeEvent event, const ::cbt::bytestream* data) {
	switch (event) {
	case odb::NodeEvent::CHANGED:
		std::cout << "node changed: " << id << std::endl;
		break;
	case odb::NodeEvent::DELETED:
		std::cout << "node deleted: " << id << std::endl;
		break;
	default:
		assert(false);
		break;
	}
}

void history::Init() noexcept {
	npsys::controllers_l controllers;
	controllers.fetch_all_nodes();

//	{
//		history_list_.init();
//		bool repaired = false;
//		for (auto it = history_list_.begin(); it != history_list_.end();) {
//			if (auto founded = std::find(std::next(it), history_list_.end(), *it); founded != history_list_.end()) {
//				it = history_list_.erase(it);
//				repaired = true;
//			} else {
//				it++;
//			}
//		}
//		if (repaired) history_list_.store();
//	}


	try {
		influxdb_ = std::move(influxdb::InfluxDBFactory::Get("http://192.168.1.2:8086/write?db=test"));
	} catch (std::exception& ex) {
		std::cout << ex.what() << std::endl;
		std::cout << "History initialization failed." << std::endl;
		return;
	}

	if (history_list_.fetch_all_nodes() == false) {
		history_list_.init();
		history_list_.store();
	}

//	{
//		odb::weak_node<npsys::device_n> temp;
//		std::stringstream ss;
//		{
//			boost::archive::binary_oarchive oa{ ss };
//			oa& npsys::device_n(5080);
//		}
//		{
//			boost::archive::binary_iarchive ia{ ss };
//			ia& temp;
//		}
//		auto temp_n = temp.fetch();
//	}

	std::vector<npsys::fbd_slot_n> added_slots;
	for (auto& param : history_list_) {
		param.advise(std::bind(&history::OnNodeEvent, this, 
			std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
		if (param->var.fetch()) {
			added_slots.push_back(param);
		}
	}

	if (added_slots.size() == 0) return;

	auto size = static_cast<uint32_t>(added_slots.size());

	nps::DataDef_Array ar;
	ar.length(size);

	size = 0;

	for (auto& param : history_list_) {
		if (param->var.loaded()) {
			auto& var = param->var;
			auto dev_addr = var->GetDevAddr();
			if (dev_addr == -1) continue;
			ar[size].dev_addr = dev_addr;
			ar[size].mem_addr = var->GetAddr();
			ar[size].type = var->GetType();
			param->online.set_type(var->GetType());
			size++;
		}
	}

	ar.length(size);

	t_advise(ar);
	
	for (uint32_t ix = 0; ix < size; ++ix) {
		ref_.emplace(
			ar[ix].h,
			std::make_pair(added_slots[ix], influxdb::Point(added_slots[ix]->id()))
		);
	}

	std::cout << size << " history items added for reading." << std::endl;
}

void history::AddParameter(oid_t id) {
	assert(id >= odb::FIRST_ID);

	boost::asio::post(strand_, [id, this]() {
		npsys::fbd_slot_n slot(id);

		do {
			if (slot.fetch() == false) {
				std::cerr << "slot does not exist... slot_id = " << id << std::endl;
				break;
			}

			slot.advise(std::bind(&history::OnNodeEvent, this, 
					std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));

			auto& var = slot->var;
			if (var.fetch() && var.loaded() && var->IsGood()) {
				nps::DataDef_Array ar;
				ar.length(1);
				
				ar[0].dev_addr = var->GetDevAddr();
				ar[0].mem_addr = var->GetAddr();
				ar[0].type = var->GetType();
				slot->online.set_type(var->GetType());
				
				advise(ar);

				ref_.emplace(
					ar[0].h,
					std::make_pair(slot, influxdb::Point(slot->id()))
				);
			}

			history_list_.add_node(slot);
			history_list_.store();
		} while (0);
	});
}

void history::RemoveParameter(oid_t id) {
	assert(id >= odb::FIRST_ID);

	boost::asio::post(strand_, [id, this]() {
		npsys::fbd_slot_n slot(id);
		do {
			if (!slot.fetch()) {
				std::cerr << "slot does not exist... slot_id = " << id << std::endl;
				break;
			}
			auto founded = std::find_if(ref_.begin(), ref_.end(), [id](auto& pair) {
				return pair.second.first.id() == id;
				});

			if (founded == ref_.end()) {
				std::cerr << "advised item was not found... slot_id = " << id << std::endl;
				break;
			}
			release_one(founded->first);
		} while (0);

		history_list_.erase(slot);
		history_list_.store();
	});
}

// called from io_context strand
void history::OnDataChanged(const nps::DataUpd_Array& ar) {
	auto size = ar.length();
	for (unsigned ix = 0; ix < size; ++ix) {
		auto& x = ref_[ar[ix].h];
		x.first->online = ar[ix];
		auto val = x.first->online.to_influx_db();
		if (val.length()) {
			x.second.WriteValue(std::move(val));
			influxdb_->write(x.second);
		}
	}
}

*/