// Copyright (c) 2021 nikitapnn1@gmail.com
// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory

#pragma once

#include <chrono>
#include <atomic>
#include <npc/server.hpp>

class IFatDataCallBack 
	: public virtual nps::IDataCallBack_Servant
{
	enum class ConnectionStatus {
		unadvised,
		advised_connected,
		advised_not_connected,
	};
	ConnectionStatus con_status_ = ConnectionStatus::unadvised;
	std::chrono::steady_clock::time_point last_update_;
	std::atomic_bool ready_for_callbacks_;
protected:
	nprpc::ObjectPtr<nps::ItemManager> item_manager_;

	virtual size_t AdviseImpl() = 0;
	virtual void OnDataChangedImpl(nprpc::flat::Span_ref<nps::flat::server_value, nps::flat::server_value_Direct> a) = 0;
	virtual void OnConnectionLost() noexcept = 0;
public:
	virtual void Ping() final { }
	virtual void OnDataChanged(nprpc::flat::Span_ref<nps::flat::server_value, nps::flat::server_value_Direct> a) final;

	bool Advise() noexcept;
	void UnAdvise() noexcept;
	bool IsAdvised() const noexcept;
	void EventConnectionLost();
	void Check() noexcept;

	virtual ~IFatDataCallBack() {
		if (con_status_ != ConnectionStatus::unadvised){
			assert(false);
		}
	}
};