// Copyright (c) 2021 nikitapnn1@gmail.com
// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory

#include "stdafx.h"
#include "avrcommand.h"
#include "error.h"
#include <npsys/common_rpc.hpp>
#include "config.h"
#include <avr_info/avr_info.h>
#include <avr_firmware/net.h>


int ExecMultiCmd(const CMultiReqCmd& cmd) {
	static constexpr auto timeout = 1000UL;

	auto const& info = cmd.get_info();
	std::cout << cmd.msg_operation() << std::endl;
	npsys_rpc->server->Write_8(cmd.get_dev_addr(), info.rmem.info + offsetof(net::RuntimeInfo, result), 0xFF);
	
	std::cout << cmd.msg_start() << std::endl;
	
	{
		ObjectTimeout new_timeout(npsys_rpc->server.get(), 5000);
		cmd.execute();
	}

	Sleep(timeout);
	
	std::cout << ">\tVerifying operation..." << std::endl;
	
	uint8_t ec;
	npsys_rpc->server->ReadByte(cmd.get_dev_addr(), info.rmem.info + offsetof(net::RuntimeInfo, result), ec);
	cmd.set_error(ec);

	if (ec == 0x00) {
		std::cout << ">\tSuccess..." << std::endl;
	} else {
		std::cout << cmd.msg_error() << std::endl;
	}

	return ec;
}

// CWritePageCmd
CWritePageCmd::CWritePageCmd(int dev_addr, const avrinfo::FirmwareInfo& info, int page_num, int page_size, unsigned char* page_p) 
	: CMultiReqCmd(dev_addr, info)
	, m_pageNum(page_num)
	, m_dataSizeAligned(page_size + page_size % 2)
	, m_data(page_p)
	,begin_msg_(">Page sending procedure has been started (number: " 
		+ std::to_string(page_num) + ", size: " + std::to_string(page_size) + ").")
{
}

std::string_view CWritePageCmd::msg_operation() const noexcept {
	return begin_msg_;
}

std::string_view CWritePageCmd::msg_start() const noexcept {
	return ">\tSending page..."sv;
}

std::string_view CWritePageCmd::msg_error() const noexcept {
	if (error_code_ == NET_WRITE_PAGE_FAIL) return ">Page Write Fail..."sv;
	return ">\tVerifying fail..."sv;
}

void CWritePageCmd::execute() const {
	npsys_rpc->server->AVR_SendPage(m_dev_addr, m_pageNum, nprpc::flat::make_read_only_span(m_data, m_dataSizeAligned));
}

// CWriteRemoteData
void CWriteRemoteData::execute() const {
	npsys_rpc->server->AVR_SendRemoteData(m_dev_addr, m_pageNum, nprpc::flat::make_read_only_span(m_data, m_dataSizeAligned));
}

// CWriteTwiTable
void CWriteTwiTable::execute() const {
	npsys_rpc->server->AVR_WriteTwiTable(m_dev_addr, m_pageNum, nprpc::flat::make_read_only_span(m_data, m_dataSizeAligned));
}

// CReplaceAlgCmd
CReplaceAlgCmd::CReplaceAlgCmd(int dev_addr, const avrinfo::FirmwareInfo& info, int old_addr, int new_addr) 
	: CMultiReqCmd(dev_addr, info)
	, m_old_addr(old_addr)
	, m_new_addr(new_addr)
	, begin_msg_(">Replacing procedure has been started (old address: " 
		+ to_hex(old_addr) + ", new address: " + to_hex(new_addr) + ").")
{
}

std::string_view CReplaceAlgCmd::msg_operation() const noexcept {
	return begin_msg_;
}

std::string_view CReplaceAlgCmd::msg_start() const noexcept {
	return ">\tBegin Replace..."sv;
}

std::string_view CReplaceAlgCmd::msg_error() const noexcept {
	if (error_code_ == NET_ALGORTIHM_NOT_FOUND) return ">Replaced algorithm was not found"sv;
	return ">\tVerifying fail..."sv;
}

void CReplaceAlgCmd::execute() const {
	npsys_rpc->server->AVR_ReplaceAlgorithm(m_dev_addr, m_old_addr, m_new_addr);
}

// CRemoveAlgCmd
CRemoveAlgCmd::CRemoveAlgCmd(int dev_addr, const avrinfo::FirmwareInfo& info, unsigned short algAddr) 
	: CMultiReqCmd(dev_addr, info)
	, m_alg_addr(algAddr) 
{
}

std::string_view CRemoveAlgCmd::msg_operation() const noexcept {
	return ">Remove procedure has been started."sv;
}

std::string_view CRemoveAlgCmd::msg_start() const noexcept {
	return ">\tBegin Remove..."sv;
}

std::string_view CRemoveAlgCmd::msg_error() const noexcept {
	if (error_code_ == NET_ALGORTIHM_NOT_FOUND) return ">The algorithm was not found"sv;
	return ">\tVerifying fail..."sv;
}

void CRemoveAlgCmd::execute() const {
	npsys_rpc->server->AVR_RemoveAlgorithm(m_dev_addr, m_alg_addr);
}

// CWriteEeprom
CWriteEepromCmd::CWriteEepromCmd(int dev_addr, const avrinfo::FirmwareInfo& info, unsigned short addr, 
	int len, const unsigned char* data) 
	: CMultiReqCmd(dev_addr, info)
	, m_addr(addr)
	, m_len(len)
	, m_data(data) 
{
}

std::string_view CWriteEepromCmd::msg_operation() const noexcept {
	return ">Eeprom update procedure has been started."sv;
}

std::string_view CWriteEepromCmd::msg_start() const noexcept {
	return ">\tBegin update..."sv;
}

std::string_view CWriteEepromCmd::msg_error() const noexcept {
	return ">\tVerifying fail..."sv;
}

void CWriteEepromCmd::execute() const {
	npsys_rpc->server->AVR_WriteEeprom(m_dev_addr, m_addr, nprpc::flat::make_read_only_span(m_data, m_len));
}
