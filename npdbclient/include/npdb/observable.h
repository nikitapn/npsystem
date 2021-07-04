// Copyright (c) 2021 nikitapnn1@gmail.com
// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory

#pragma once

#include <boost/signals2/signal.hpp>

namespace odb {

struct observable_removed {
	boost::signals2::signal<void()> sig_node_removed;

	auto observe_remove(std::function<void()>&& fn) {
		return sig_node_removed.connect(std::move(fn));
	}
};

} // namespace odb