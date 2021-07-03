#pragma once

#include <npsys/variable.h>

class WriteDefaultValuesRequest;

class VarWrapper {
public:
	VarWrapper(npsys::variable* var)
		: var_(var)
	{
	}
	virtual ~VarWrapper() = default;
	
	const npsys::variable* var() const noexcept { return var_; }
	
	bool IsBit() const noexcept { return var_->IsBit(); }
	int GetBit() const noexcept { return var_->GetBit(); }
	int GetSizeWithQuality() const noexcept { return var_->GetSizeWithQuality(); }
	int GetAddr() const noexcept { return var_->GetAddr(); }
	
	void Commit() noexcept { 
		auto modified = var_->DefaultValue_Modified();
		var_->DefaultValue_SetModified(false); 
		if (modified) var_->self_node.fetch().store();
	}

	virtual bool IsModified() const noexcept { return var_->DefaultValue_Modified(); }
	virtual int InsertVariable(WriteDefaultValuesRequest* pRequest) const noexcept;
protected:
	npsys::variable* var_;
};

/* 
	simple values 
	default quality has not implemented yet
*/
class VarWrapper1 : public VarWrapper {
public:
	VarWrapper1(npsys::variable* var)
		: VarWrapper(var) 
	{
	}
	virtual int InsertVariable(WriteDefaultValuesRequest* pRequest) const noexcept;
};

/* 
	External Reference 
	everything is set to zero 
*/
class VarWrapper2 : public VarWrapper {
public:
	VarWrapper2(npsys::variable* var)
		: VarWrapper(var)
	{
	}
};

/* 
	Variables which were deleted 
	everything is set to zero
*/
class VarWrapper3 : public VarWrapper {
public:
	VarWrapper3(npsys::variable* var)
		: VarWrapper(var) 
	{
	}
	virtual bool IsModified() const noexcept { return true; }
};

class WriteDefaultValuesRequest {
	friend VarWrapper;
	friend VarWrapper1;
	friend VarWrapper2;
	friend VarWrapper3;
public:
	static constexpr auto MAX_DATA_LEN = 56;

	WriteDefaultValuesRequest(uint16_t offset) {
		m_offset = offset;
		memset(m_data, 0x00, MAX_DATA_LEN);
	}
	WriteDefaultValuesRequest(VarWrapper* wrapper) {
		m_offset = wrapper->GetAddr();
		memset(m_data, 0, MAX_DATA_LEN);
		wrapper->InsertVariable(this);
	}
	WriteDefaultValuesRequest(const WriteDefaultValuesRequest& other) {
		m_offset = other.m_offset;
		m_len = other.m_len;
		m_prOffset = other.m_prOffset;
		memcpy(m_data, other.m_data, MAX_DATA_LEN);
	}
	const uint8_t* GetData() const noexcept {
		return m_data;
	}
	uint16_t GetOffset() const noexcept {
		return m_offset;
	}
	int GetLength() const noexcept {
		return m_len;
	}
	bool IsByteAvailable() const noexcept {
		return m_len < MAX_DATA_LEN;
	}
private:
	int m_len = 0;
	uint16_t m_offset;
	uint16_t m_prOffset = 0;
	uint8_t m_data[MAX_DATA_LEN];
};