// Copyright (c) 2021 nikitapnn1@gmail.com
// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory

#pragma once

#include "header.h"
#include "fbdblock.h"

namespace npsys {

class WebCategory 
	: public odb::common_interface<odb::no_interface>
	, public odb::systree_item<WebCategory>
	, public odb::modified_flag {

	friend boost::serialization::access;
	template<class Archive>
	void serialize(Archive & ar, const unsigned int /*file_version*/) {
		ar & name_;
		ar & items;
	}
public:
	web_item_l items;

	WebCategory() = default;
	template<typename T>
	WebCategory(T&& name) 
		: systree_item(std::forward<T>(name)) 
	{
	}
};

class WebItem 
	: public odb::common_interface<odb::no_interface>
	, public odb::systree_item<WebItem>
	, public odb::modified_flag {

	friend boost::serialization::access;
	template<class Archive>
	void serialize(Archive & ar, const unsigned int /*file_version*/) {
		ar & name_;
		ar & slot;
		ar & description;
	}
public:
	fbd_slot_n slot;
	std::string description;

	WebItem() = default;
	template<typename T>
	WebItem(T&& name) 
		: systree_item(std::forward<T>(name)) 
	{
	}
};

} // namespace npsys