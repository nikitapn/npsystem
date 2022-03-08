#pragma once

#include "blockcomposition.hpp"
#include "sfc_block_factory.hpp"

class CSFCBusIn;
class CSFCBusOut;

struct cpp_delete_CSFCBusIn {
	__declspec(noinline) void operator()(CSFCBusIn*);
};


struct cpp_delete_CSFCBusOut {
	__declspec(noinline) void operator()(CSFCBusOut*);
};


class CSFCBlockComposition 
	: public CBlockComposition
	, public odb::common_interface<odb::no_interface>
{
	friend class PasteClipboardCommand;
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive & ar, const unsigned int file_version) {
		ar & boost::serialization::base_object<CBlockCompositionBase>(*this);
		ar & block_factory_;
	}

	CSFCBlockFactory block_factory_;
	npsys::CSFCControlUnit* unit_ = nullptr;

	 std::tuple<
		std::vector<std::unique_ptr<CSFCBusIn, cpp_delete_CSFCBusIn>>,
		std::vector<std::unique_ptr<CSFCBusOut, cpp_delete_CSFCBusOut>>
	> free_busses_;
public:
	DECLARE_TYPE_NAME("SFC_EDITOR")
	DECLARE_VISITOR()
	
//	void AddBlock(npsys::fbd_block_n& block) {
//		Push(block->e_block.get());
//		block->e_block->OnBlockAddedToEditor();
//	}
	
//	void RemoveBlock(npsys::fbd_block_n& block) {
//		RemoveElement(block->e_block.get());
//		block->e_block->OnBlockDeletedFromEditor();
//	}

	template<typename T>
	T* GetBus() {
		T* ptr;
		auto& v = std::get<mp11::mp_find<std::tuple<CSFCBusIn, CSFCBusOut>, T>::value>(free_busses_);
		if (v.size()) {
			ptr = v.back().release();
			v.pop_back();
		} else {
			ptr = new T();
		}
		this->Push(ptr);
		return ptr;
	}

	template<typename T>
	void FreeBus(T* bus) {
		auto& v = std::get<mp11::mp_find<std::tuple<CSFCBusIn, CSFCBusOut>, T>::value>(free_busses_);
		v.emplace_back(bus);
		this->RemoveElement(bus);
	}

	void PopBlock() {
		Pop();
	}
	
	void SetAlgorithm(npsys::CSFCControlUnit* unit) noexcept {
		ATLASSERT(unit);
		ATLASSUME(!unit_);
		unit_ = unit;
		
		SFC_RestorePointers();
	}

	npsys::CSFCControlUnit* GetAlgorithm() noexcept {
		ATLASSUME(unit_ != nullptr);
		return unit_;
	}

	CSFCBlockFactory* GetBlockFactory() { return &block_factory_; }

	CSFCBlockComposition() 
		: block_factory_(this) 
	{
	}

	CSFCBlockComposition(std::true_type);
};