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