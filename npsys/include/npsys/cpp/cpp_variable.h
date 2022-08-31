// Copyright (c) 2021 nikitapnn1@gmail.com
// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory

namespace npsys {

int variable::GetDevAddr() const noexcept {
	if (dev_.is_invalid_id()) return -1;
	auto n = dev_.fetch();
	if (!n.loaded()) return -1;
	return n->dev_addr;
}

} // namespace npsys