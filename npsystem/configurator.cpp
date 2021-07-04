// Copyright (c) 2021 nikitapnn1@gmail.com
// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory

#include "stdafx.h"
#include "configurator.h"
#include "network_ext.h"
#include "assignedalgorithm.h"

void Configurator::UploadAllAlgoritms() noexcept {
	auto& algs = ctrl_->assigned_algs;
	algs.fetch_all_nodes();
	if (algs.size() == 0) return;

	std::weak_ptr<int> w;
	



}