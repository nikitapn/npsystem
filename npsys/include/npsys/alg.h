// Copyright (c) 2021 nikitapnn1@gmail.com
// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory

#pragma once

#include "header.h"
#include "access.h"
#include <npdb/memento.h>
#include <npsys/other/uploadable.h>

namespace npsys {
struct CAlgorithmViewPosition 
	: public odb::common_interface<odb::no_interface>
	, public odb::modified_flag {

	float x_offset;
	float y_offset;
	float scale;

	template<class Archive>
	void serialize(Archive & ar, const unsigned int /*file_version*/) {
		ar & x_offset;
		ar & y_offset;
		ar & scale;
	}

	void DefaultViewPosition() noexcept {
		x_offset = 0.0f;
		y_offset = 0.0f;
		scale = 1.5f;
	}

	void SavePosition(float _x_offset, float _y_offset, float _scale) noexcept {
		if (std::abs(x_offset - _x_offset) > 0.001f ||
			std::abs(y_offset - _y_offset) > 0.001f ||
			std::abs(scale - _scale) > 0.001f) {
			set_modified(true);
		}
		x_offset = _x_offset;
		y_offset = _y_offset;
		scale = _scale;
	}
};

using algorithm_view_position_n = odb::shared_node<CAlgorithmViewPosition>;

class CAlgorithm 
	: public odb::common_interface<odb::no_interface>
	, public odb::systree_item<CAlgorithm>
	, public IUploadable {
	friend boost::serialization::access;
	template<class Archive>
	void serialize(Archive & ar, const unsigned int file_version) {
		ar & name_;
		ar & cat;
		ar & assigned_dev;
		ar & assigned_alg;
		ar & status_;
		ar & editor_;
		ar & fbd_blocks;
		ar & npsys::access::rw(scan_period);
		if (file_version > 0) {
			ar & view_position;
		} else {
			assert(Archive::is_saving::value == false);
			view_position.create_locally_with_new_id();
			view_position->DefaultViewPosition();
		}
	}
public:
	static constexpr int _BOOST_CLASS_VERSION = 1;
	enum Status
	{
		status_not_assigned,
		status_assigned,
		status_loaded,
		status_not_actual
	};
	odb::node_list<fbd_block_n> fbd_blocks;
	odb::weak_node<assigned_algorithm_n> assigned_alg;
	odb::weak_node<algorithm_category_n> cat;
	odb::weak_node<device_n> assigned_dev;
	algorithm_view_position_n view_position;
	const uint16_t scan_period = 500;
protected:
	block_composition_n editor_;
public:
	Status GetStatus() const noexcept { return status_; }
	void SetStatus(Status status) noexcept {
		status_ = status; 
	}

	
	void SetScanPeriodMs(uint16_t scan_period);

	virtual std::unique_ptr<odb::IMemento> CreateMemento_Uploadable() noexcept final {
		return odb::MakeMemento(*this, status_);
	}
private:
	Status status_ = status_not_assigned;
};
} // namespace npsys

BOOST_CLASS_VERSION(npsys::CAlgorithm, npsys::CAlgorithm::_BOOST_CLASS_VERSION);
