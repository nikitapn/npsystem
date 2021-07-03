#pragma once

#include <npsys/header.h>
#include <npdb/db.h>
#include <avr_info/avr_info.h>
#include "stuff.h"
#include "tr_request.h"
#include "global.h"
#include <avr_info/avr_info.h>

class CDlg_AvrPinConfig;

namespace npsys {

struct portc {
	uint8_t ddr;
	uint8_t port;
};

struct oriandival {
	uint8_t ori;
	uint8_t andi;
};

struct io_reg_addr {
	uint8_t port;
	std::string port_n;

	uint8_t ddr;
	std::string ddr_n;

	uint8_t pin;
	std::string pin_n;
};

class CAVRPort 
	: public odb::common_interface<odb::no_interface>
	, public odb::systree_item_const_name {
public:
	CAVRPort() = default;
	CAVRPort(std::string name, npsys::controller_n_avr& _ctrl) 
		: systree_item_const_name(name)
		, ctrl_(_ctrl) {}
	io_reg_addr GetControlRegisters(const avrinfo::PeripheralInfo& info, bool ioStart = true);
	virtual bool ChangeName(const std::string& name) { return false; };
	odb::node_list<avr_pin_n> pins_;
private:
	odb::weak_node<controller_n_avr> ctrl_;
	int unavailable_;
	int status_;

	friend boost::serialization::access;
	template<class Archive>
	void serialize(Archive & ar, const unsigned int /*file_version*/) {
		ar & name_;
		ar & ctrl_;
		ar & pins_;
		ar & unavailable_;
		ar & status_;
	}
};


// CPin
class CAVRPin
	: public odb::common_interface<odb::no_interface>
	, public odb::systree_item_const_name {
	friend CDlg_AvrPinConfig;
	friend class CBlockCompositionWrapper;
	friend boost::serialization::access;
	template<class Archive>
	void serialize(Archive & ar, const unsigned int /*file_version*/) {
		ar & name_;
		ar & ctrl;
		ar & port;
		ar & var_;
		ar & bit_;
		ar & use_;
		ar & last_loaded_use_;
		ar & type_;
		ar & inverted_;
		ar & status_;
	}
public:
	CAVRPin() = default;
	CAVRPin(
		std::string name,
		npsys::controller_n_avr& _ctrl,
		avr_port_n& _port,
		variable_n& _var,
		int _bit)
		: systree_item_const_name(name)
		, ctrl(_ctrl)
		, port(_port)
		, var_(_var)
		, bit_(_bit)
		, use_(avrinfo::PinPurpose::INPUTPU_PIN)
		, last_loaded_use_(avrinfo::PinPurpose::UNKNOWN_PIN)
		, type_(io::INPUT)
		, inverted_(false)
		, status_(io::Status::assigned) {}
public:
	virtual bool ChangeName(const std::string& name) { return false; };
	void SetPinPurpose(avrinfo::PinPurpose use) noexcept;
	avrinfo::PinPurpose GetPinPurpose(bool loaded = false) const noexcept { 
		return loaded ? last_loaded_use_ : use_; 
	}
	AlphaIcon GetIcon() const noexcept;
	oriandival GetDDRVal(bool loaded) const noexcept;
	oriandival GetPORTVal(bool loaded) const noexcept;
	
	int GetPinNum() const noexcept { return bit_; }
	
	npsys::variable_n& GetVariable() noexcept {
		ASSERT_FETCH(var_);
		return var_;
	}
	
	void UpdateVar(const avrinfo::FirmwareInfo& info, const io_reg_addr& reg);
	int GetAllStates() const noexcept;
	io::Status GetStatus() const noexcept { return status_; }

	int GetVariableType() const noexcept;
	io::PINTYPE GetPinType() const noexcept { return type_; }
	bool IsCanBeUploaded() const noexcept;
	bool IsInverted() const noexcept { return inverted_; }
	odb::weak_node<controller_n_avr> ctrl;
	odb::weak_node<avr_port_n> port;
	CTreeItemAbstract* item = nullptr;
private:
	static oriandival GetDDRValImpl(avrinfo::PinPurpose use, int bit) noexcept;
	static oriandival GetPORTValImpl(avrinfo::PinPurpose use, int bit, bool inverted) noexcept;


	variable_n var_;
	int bit_;
	avrinfo::PinPurpose use_;
	avrinfo::PinPurpose last_loaded_use_;
	io::PINTYPE type_;
	bool inverted_;
	io::Status status_;
};





} // namespace npsys