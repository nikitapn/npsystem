// Copyright (c) 2021 nikitapnn1@gmail.com
// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory

#pragma once

#include <npdb/find.h>

namespace npsys {

std::string CFBDSlot::path() const noexcept {
	do {
		auto a = fbd_unit.fetch();
		if (!a.loaded()) break;

		auto c = a->cat.fetch();
		if (!c.loaded()) break;

		auto b = block_parent.fetch();
		if (!b.loaded()) break;

		return '/' + c->get_name() + '/' + a->get_name() + '/' + b->get_name() + '.' + this->get_name();
	} while (true);
	return {};
}


std::vector<std::string> CFBDSlot::split_path(const std::string& str) {
	std::vector<std::string> res;

	size_t b = 0;
	auto e = str.find(L'/', b + 1);

	while (e != std::string::npos) {
		res.push_back(str.substr(b + 1, e - b - 1));
		b = e;
		e = str.find(L'/', b + 1);
	}

	res.push_back(str.substr(b + 1));

	return res;
}

fbd_slot_n CFBDSlot::get_by_path(const std::string& path) {
	return get_by_path(split_path(path));
}

fbd_slot_n CFBDSlot::get_by_path(const std::vector<std::string>& tok) {
	if (tok.size() < 3) throw std::runtime_error("Invalid link");

	const std::string& cat_name = tok[0];
	const std::string& alg_name = tok[1];
	const std::string& slot_full_name = tok[2];

	npsys::categories_n categories;
	ASSERT_FETCH(categories);

	auto cat = odb::utils::find_by_name(categories->alg_cats, cat_name);

	if (!cat) {
		throw std::runtime_error("There is no such category \"" + cat_name + '\"');
	}

	auto control_unit = odb::utils::find_by_name((*cat)->units, alg_name);
	if (!control_unit || (*control_unit)->GetLanguageType() != npsys::CControlUnit::Language::FBD) {
		throw std::runtime_error("There is no such FBD unit \"" + alg_name + '\"');
	}

	auto fbd_unit = (*control_unit).cast<fbd_control_unit_n>();

	auto ix = slot_full_name.find(L'.', 0);

	if (ix == std::string::npos) {
		throw std::runtime_error("Expected . after \"" + slot_full_name + '\"');
	}

	const auto block_name = slot_full_name.substr(0, ix);
	const auto slot_name = slot_full_name.substr(ix + 1);

	auto& blocks = fbd_unit->fbd_blocks;
	blocks.fetch_all_nodes();

	for (auto& block : blocks) {
		if (block->get_name() == block_name) {
			block->slots.fetch_all_nodes();
			for (auto& slot : block->slots) {
				if (slot->get_name() == slot_name) {
					return slot;
				}
			}
			throw std::runtime_error("There is no such slot \"" + slot_full_name + '\"');
		}
	}
	throw std::runtime_error("There is no such slot \"" + slot_full_name + '\"');
}

} // namespace npsys