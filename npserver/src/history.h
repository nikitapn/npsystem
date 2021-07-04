// Copyright (c) 2021 nikitapnn1@gmail.com
// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory

#pragma once

#include <npsys/fbdblock.h>
#include "protocol_base.h"
#include <npsys/other/online_value.h>
#include "listener.h"

/*
class history : public protocol::listener {
	std::unordered_map<
		nps::var_handle, 
		std::pair<npsys::fbd_slot_n, influxdb::Point>
	> ref_;

	npsys::history_l history_list_;
	std::unique_ptr<influxdb::InfluxDB> influxdb_;
public:
	void Init() noexcept;
	void AddParameter(oid_t id);
	void RemoveParameter(oid_t id);
	virtual void OnDataChanged(const nps::DataUpd_Array& ar) final;
private:
	void OnNodeEvent(oid_t id, odb::NodeEvent event, const ::cbt::bytestream* data);
};

extern std::unique_ptr<history> hist;
*/