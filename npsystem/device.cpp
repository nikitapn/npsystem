// Copyright (c) 2021 nikitapnn1@gmail.com
// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory

#include "stdafx.h"
#include "network_ext.h"

namespace npsys {

void CNetworkDevice::AddLink(npsys::variable_n& var, npsys::variable_n& ref_var, remote::ExtLinkType type) {

	ASSERT(type == remote::ExtLinkType::Read);

	auto destDeviceId = ref_var->GetDev().id();
	if (type == remote::ExtLinkType::Read) {
		if (ref_var->IsBit()) {
			links_r_[destDeviceId].lbit.push_back(
				std::make_pair(var, ref_var)
			);
		} else {
			links_r_[destDeviceId].lbyte.push_back(
				std::make_pair(var, ref_var)
			);
		}
	} else {
		if (ref_var->IsBit()) {
			links_w_[destDeviceId].lbit.push_back(
				std::make_pair(var, ref_var)
			);
		} else {
			links_w_[destDeviceId].lbyte.push_back(
				std::make_pair(var, ref_var)
			);
		}
	}
	remote_data_changed_ = true;
}

void CNetworkDevice::DeleteLink(const remote::ExtLinkType type, const oid_t var_id, const bool is_bit) {
	auto erase_link = [=] (link_map& links) {
		//
		auto erase = [](link_list& ll, oid_t var_id) {
			auto it = std::find_if(ll.begin(), ll.end(), [var_id](const auto& link) {
				return link.first.id() == var_id; 
			});
			if (it == ll.end()) return false;
			ll.erase(it);
			return true;
		};	
		for (auto& dev : links) {
			if (is_bit && erase(dev.second.lbit, var_id)) return true;
			if (!is_bit && erase(dev.second.lbyte, var_id)) return true;
		}
		return false;
	};

	bool removed = 
		((type == remote::ExtLinkType::Read) 
		? erase_link(links_r_) 
		: erase_link(links_w_));
	
	remote_data_changed_ |= removed;
	
	if (!removed) {
		std::cout << "link not found! var_id: " << var_id << std::endl;
	}

//	ASSERT(removed);
}

} // namespace npsys