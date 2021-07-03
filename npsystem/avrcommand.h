#pragma once

#include <avr_info/avr_info.h>
#include <avr_firmware/net.h>

//#define TEST_MODE
//#define TEST_MODE2

class CMultiReqCmd {
public:
	virtual std::string_view msg_operation() const noexcept = 0;
	virtual std::string_view msg_start() const noexcept = 0;
	virtual std::string_view msg_error() const noexcept = 0;
	virtual void execute() const = 0;
	
	void set_error(uint8_t ec) const noexcept { error_code_ = ec; }
	int get_dev_addr() const noexcept { return m_dev_addr; }
	const avrinfo::FirmwareInfo& get_info() const noexcept { return info_; }

	CMultiReqCmd(int dev_addr, const avrinfo::FirmwareInfo& info) noexcept
		: m_dev_addr(dev_addr), info_(info) {}
	
	virtual ~CMultiReqCmd() = default;
protected:
	mutable uint8_t error_code_;
	const int m_dev_addr;
	const avrinfo::FirmwareInfo& info_;
};

// 0 - success, otherwise error code for specific command
int ExecMultiCmd(const CMultiReqCmd& cmd);

class CWritePageCmd : public CMultiReqCmd {
protected:
	const std::string begin_msg_;
	const int m_pageNum;
	const int m_dataSizeAligned;
	const unsigned char* m_data;
public:
	virtual std::string_view msg_operation() const noexcept;
	virtual std::string_view msg_start() const noexcept;
	virtual std::string_view msg_error() const noexcept;
	virtual void execute() const override;

	CWritePageCmd(int dev_addr, const avrinfo::FirmwareInfo& info, int page_num, int page_size, unsigned char* page_p);
};

class CWriteRemoteData : public CWritePageCmd {
public:
	virtual void execute() const final;

	CWriteRemoteData(int dev_addr, const avrinfo::FirmwareInfo& info, int page_num, int page_size, unsigned char* page_p)
		: CWritePageCmd(dev_addr, info, page_num, page_size, page_p) { }
};

class CWriteTwiTable : public CWritePageCmd {
public:
	virtual void execute() const final;

	CWriteTwiTable(int dev_addr, const avrinfo::FirmwareInfo& info, int page_num, int page_size, unsigned char* page_p)
		: CWritePageCmd(dev_addr, info, page_num, page_size, page_p) { }
};

class CReplaceAlgCmd : public CMultiReqCmd {
	const std::string begin_msg_;
	const int m_old_addr;
	const int m_new_addr;
public:
	virtual std::string_view msg_operation() const noexcept;
	virtual std::string_view msg_start() const noexcept;
	virtual std::string_view msg_error() const noexcept;
	virtual void execute() const final;

	CReplaceAlgCmd(int dev_addr, const avrinfo::FirmwareInfo& info, int old_addr, int new_addr);
};

class CRemoveAlgCmd : public CMultiReqCmd {
	const int m_alg_addr;
public:
	virtual std::string_view msg_operation() const noexcept;
	virtual std::string_view msg_start() const noexcept;
	virtual std::string_view msg_error() const noexcept;
	virtual void execute() const final;

	CRemoveAlgCmd(int dev_addr, const avrinfo::FirmwareInfo& info, unsigned short algAddr);
};

class CWriteEepromCmd : public CMultiReqCmd {
	const unsigned short m_addr;
	const int m_len;
	const unsigned char* m_data;
public:
	virtual std::string_view msg_operation() const noexcept;
	virtual std::string_view msg_start() const noexcept;
	virtual std::string_view msg_error() const noexcept;
	virtual void execute() const final;

	CWriteEepromCmd(int dev_addr, const avrinfo::FirmwareInfo& info, unsigned short addr, int len, const unsigned char* data);
};
