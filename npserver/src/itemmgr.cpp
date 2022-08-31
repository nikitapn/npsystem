// Copyright (c) 2021 nikitapnn1@gmail.com
// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory

#include "stdafx.h"
#include "itemmgr.h"
#include "config.h"
#include "listener.h"
#include "itemmgr.h"
#include "protocol.h"

boost::pool<> ItemManagerImpl::pool_(sizeof(ItemManagerImpl), 32);

void ItemManagerImpl::keep_alive_timer(std::shared_ptr<std::atomic_bool> del) {
//	if (del->load() == true) return;

//	if (timer_.expires_at() <= boost::asio::deadline_timer::traits_type::now()) {
//		timer_.expires_at(boost::posix_time::pos_infin);
//		nplib::async<false>(strand_, [this]() { this->fire_keep_alive(); });
//	}

//	timer_.async_wait(std::bind(&ItemManagerImpl::keep_alive_timer, this, del));
}

ItemManagerImpl::ItemManagerImpl() noexcept
	: timer_(thread_pool::get_instance().ctx())
{
//	timer_.expires_at(boost::posix_time::pos_infin);
//	keep_alive_timer(deleted_);
}

// nprpc thread
void ItemManagerImpl::Advise(nprpc::flat::Span_ref<nps::flat::DataDef, nps::flat::DataDef_Direct> a, /*out*/nprpc::flat::Vector_Direct1<uint64_t> h) {
	h.length(a.size());
	auto span = h();
	t_advise({(nps::flat::DataDef*)a.data(), (nps::flat::DataDef*)a.data_end()}, span);
}

// nprpc thread
void ItemManagerImpl::UnAdvise(nprpc::flat::Span<uint64_t> a) {
}

// nprpc thread
void ItemManagerImpl::Activate(nprpc::Object* client) {
	assert(client);

	pd_client_.reset(nprpc::narrow<nps::DataCallBack>(client));

	if (g_cfg.log_level > 2) {
			std::cout <<"ItemManagerImpl::Activate()" << std::endl;
	}
	
//	pd_client_->_setTimeout(3, 0);
	
	proto->t_add_listener(static_cast<protocol::listener*>(this)).get();
}

// nprpc thread
void ItemManagerImpl::destroy() noexcept {
	if (g_cfg.log_level > 2) {
		std::cerr << "ItemManagerImpl::destroy()\n";
	}

	release_all();
	proto->t_remove_listener(this).get(); // wait for listener to be removed

//	boost::system::error_code ec;
//	timer_.cancel(ec);

	nplib::async<false>(strand_, [this]() { destroy_from_strand(); }); // last job in this strand
}

// strand context
void ItemManagerImpl::destroy_from_strand() noexcept {
	if (processing_on_data_changed_) {
		std::cerr << "ItemManagerImpl::destroy_from_strand().should not happen - processing_on_data_changed_ = true\n";
		nplib::async<false>(strand_, [this]() { destroy_from_strand(); });
		return;
	}
	if (g_cfg.log_level > 1) {
		std::cout <<
			"ItemManagerImpl::destroy_from_strand() " << "task_count: " << task_count() << std::endl;
	}
	pd_client_.reset();
	delete this;
}

// strand context
void ItemManagerImpl::OnDataChanged(const std::vector<nps::server_value>& ar) {
	if (is_deleted()) return;
	
	processing_on_data_changed_ = true;

//	boost::system::error_code ec;
//	timer_.expires_from_now(boost::posix_time::seconds(g_cfg.client_keep_alive_time), ec);

//	if (ec) {
//		if (g_cfg.log_level > 2) std::cout << "error start_keep_alive_timer: " << ec << std::endl;
//	}

	try {
		pd_client_->OnDataChanged(nprpc::flat::make_read_only_span(ar));
		death_counter_ = 0;
	} catch (nprpc::Exception& ex) {
		std::cerr << "ItemManagerImpl::OnDataChanged().NPRPC::Exception: " << ex.what() << '\n';
		death_counter_++;

		if (death_counter_ >= g_cfg.client_keep_alive_max_na) {
			if (g_cfg.log_level > 2) {
				std::cout << " Client Disconnected: Removing Servant" << std::endl;
			}
	//	deactivate
		}
	}

	processing_on_data_changed_ = false;
}

// strand context
void ItemManagerImpl::fire_keep_alive() {
	if (g_cfg.log_level > 2) {
		std::cout << "ItemManagerImpl::fire_keep_alive() " << std::endl;
	}

	try {
//		pd_client_->Ping();
		death_counter_ = 0;
		
		boost::system::error_code ec;
		timer_.expires_from_now(boost::posix_time::seconds(g_cfg.client_keep_alive_time), ec);
	} catch (nprpc::Exception& ex) {
		std::cerr << " NPRPC::Exception: " << ex.what() << '\n';
		death_counter_++;

		if (death_counter_ >= g_cfg.client_keep_alive_max_na) {
			if (g_cfg.log_level > 2) {
				std::cout << " Client Disconnected: Removing Servant" << std::endl;
			}
			// release();
		} else {
			boost::system::error_code ec;
			timer_.expires_from_now(boost::posix_time::seconds(g_cfg.client_keep_alive_time), ec);
		}
	}
}