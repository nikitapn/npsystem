// Copyright (c) 2021 nikitapnn1@gmail.com
// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory

#pragma once

#include <npc/server.hpp>
#include <boost/pool/pool.hpp>
#include <boost/asio/deadline_timer.hpp>

#include "protocol_base.h"
#include "listener.h"

class ItemManagerImpl 
	: public nps::IItemManager_Servant
	, public protocol::listener
{
	inline static std::mutex pool_lock_;
	static boost::pool<> pool_;

	nprpc::ObjectPtr<nps::DataCallBack> pd_client_;
	boost::asio::deadline_timer timer_;
	int death_counter_ = 0;

	//void destroy_from_strand() noexcept;
	void fire_keep_alive();
	void keep_alive_timer(std::shared_ptr<std::atomic_bool> del);
public:
	// IPingable
	virtual void Ping() override {}
	// IItemManager_Servant
	virtual void Activate(nprpc::Object* client) override;
	virtual void Advise(::flat::Span_ref<nps::flat::DataDef, nps::flat::DataDef_Direct> a, /*out*/::flat::Vector_Direct1<uint64_t> h) override;
	virtual void UnAdvise(::flat::Span<uint64_t> a) override;
	// nprpc::ObjectServant
	virtual void destroy() noexcept override;
	// protocol::listener
	virtual void OnDataChanged(const std::vector<nps::server_value>& ar) override;

	static void* operator new(std::size_t size) {
		std::lock_guard<std::mutex> lk(pool_lock_);
		return pool_.malloc();
	}

	static void operator delete(void* ptr) {
		std::lock_guard<std::mutex> lk(pool_lock_);
		pool_.free(ptr);
	}
	
	ItemManagerImpl() noexcept;

	~ItemManagerImpl() {
		if (g_cfg.log_level > 2) {
			std::cerr << "ItemManagerImpl::~ItemManagerImpl()\n";
		}
	}
};