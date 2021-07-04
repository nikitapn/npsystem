// Copyright (c) 2021 nikitapnn1@gmail.com
// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory

#pragma once

#include "color.h"
#include "element.h"
#include "vartype.h"
#include "visitorblocks.h"
#include "constants.h"

#include <npsys/variable.h>
#include <nplib/utils/variant.h>
#include <npsys/fbdblock.h>
#include <npsys/other/remote.h>
#include <npsys/other/online_value.h>
#include <npsys/other/uploadable.h>
#include <npdb/memento.h>

#include "cpp_deleter.h"

#include "propertygrid.h"
#include <bitset>

#include "serialization_atl_string.h"

class CBlockFactory;

enum DATABASE_ACTION {
	DATABASE_ACTION_CONFIGURABLE_SLOT_REMOVED
};

class DatabaseAction {
public:
	explicit DatabaseAction(oid_t node_id, DATABASE_ACTION action_id)
		: node_id_(node_id)
		, action_id_(action_id)
	{
	}
	virtual ~DatabaseAction() = default;
	virtual void Execute() noexcept = 0;
	oid_t GetNodeId() const noexcept { return node_id_; }
	DATABASE_ACTION GetActionId() const noexcept { return action_id_; }
private:
	const oid_t node_id_;
	const DATABASE_ACTION action_id_;
};

class DatabaseAction_ConfigurableSlotRemoved : public DatabaseAction {
public:
	explicit DatabaseAction_ConfigurableSlotRemoved(npsys::fbd_slot_n& slot)
		: DatabaseAction(slot.id(), DATABASE_ACTION_CONFIGURABLE_SLOT_REMOVED)
		, slot_(slot) {}
	virtual void Execute() noexcept;
private:
	npsys::fbd_slot_n slot_;
};

class ActionList {
public:
	template<class Action, class... Rest>
	void AddAction(Rest&&... rest) noexcept {
		actions_.push_back(std::move(std::make_unique<Action>(std::forward<Rest>(rest)...)));
	}
	// true - removed
	// false - not founded
	bool RemoveAction(oid_t node_id, DATABASE_ACTION action_id) noexcept {
		auto founded = std::find_if(actions_.begin(), actions_.end(), [=](auto& action) {
			return action->GetNodeId() == node_id && action->GetActionId() == action_id;
			});
		if (founded != actions_.end()) {
			actions_.erase(founded);
			return true;
		}
		return false;
	}
	void ExecuteAll() noexcept {
		for (auto& action : actions_) action->Execute();
		actions_.clear();
	}
private:
	std::vector<std::unique_ptr<DatabaseAction>> actions_;
};

class BlockProperty {
	friend boost::serialization::access;
	template<class Archive>
	void serialize(Archive& ar, unsigned int file_version) {
		if (file_version > 0) ar& name_;
		else {
			if constexpr (Archive::is_loading::value) {
				CStringW str; ar& str;
				name_ = narrow(str.GetString());
			} else {
				assert(false);
			}
		}
		ar& key_;
		ar& section_;
	}
protected:
	std::string name_;
	std::string key_;
	PRB::Section section_;
	std::bitset<16> dirty_;
public:
	BlockProperty() = default;
	BlockProperty(PRB::Section section, const std::string& name, std::string_view key)
		: section_(section)
		, name_(name)
		, key_(key)
	{
	}
	virtual ~BlockProperty() = default;
	const std::string& GetName() const noexcept {
		return name_;
	}
	const std::string& GetKey() const noexcept {
		return key_;
	}
	template<size_t Ix>
	bool GetDirty() noexcept {
		bool dirty = dirty_[Ix];
		dirty_[Ix] = false;
		return dirty;
	}
	virtual void FillPropertyGrid(CPropertyGrid* prop_grid) noexcept = 0;
	virtual void OnPropertyChanged(const npsys::CMyVariant& var) noexcept = 0;
	virtual std::string ToString() const noexcept = 0;
};

BOOST_CLASS_VERSION(BlockProperty, 1);

template<typename T>
class BlockPropertyT : public BlockProperty {
	friend boost::serialization::access;
	template<class Archive>
	void serialize(Archive& ar, unsigned int) {
		ar& boost::serialization::base_object<BlockProperty>(*this);
		ar& value_;
	}
	T value_;
public:
	BlockPropertyT() = default;
	BlockPropertyT(PRB::Section section, const std::string& name, std::string_view key, T defVal = {})
		: BlockProperty(section, name, key)
		, value_(defVal)
	{
	}
	void SetValue(const T& value) noexcept {
		value_ = value;
		dirty_.set();
	}
	void SetValue(const npsys::CMyVariant& value) {
		value_ = value.as<T>;
		dirty_.set();
	}
	const T& GetValue() const noexcept {
		return value_;
	}
	virtual void FillPropertyGrid(CPropertyGrid* grid) noexcept {
		grid->AddVariantItem(section_, name_, value_, (UINT_PTR)this);
	}
	virtual void OnPropertyChanged(const npsys::CMyVariant& var) noexcept {
		value_ = var.as<T>();
		dirty_.set();
	}
	virtual std::string ToString() const noexcept {
		const npsys::CMyVariant var = value_;
		return var.ToString();
	}
};

namespace npsys {
class CAssignedAlgorithm;
class CI2CModuleSegmentValue;
}

class CLine;
class COutputSlot;
class CBlockVisitor;
class CSlot;
class CInputSlot;
class COutputSlot;
class CAlgorithms;
class CCodeGenerator;

enum SLOT_SIGNAL {
	SIG_SLOT_DELETED,
	SIG_SLOT_DELETED_PERMANENT,
	SIG_SLOT_INSERTED,
	SIG_SLOT_PARENT_NAME_CHANGED
};

class CGeometryElement : public CElement {
	friend boost::serialization::access;
	template<class Archive>
	void save(Archive& ar, const unsigned int /*file_version*/) const {}
	template<class Archive>
	void load(Archive& ar, const unsigned int /*file_version*/) {
		CreateRgnShape();
		UpdateTransformedGeometry();
	}
	template<class Archive>
	void serialize(Archive& ar, const unsigned int file_version) {
		ar& boost::serialization::base_object<CElement>(*this);
		ar& m_matrixTransform;
		boost::serialization::split_member(ar, *this, file_version);
	}
protected:
	wrl::ComPtr<ID2D1Geometry> m_geometry;
	wrl::ComPtr<ID2D1TransformedGeometry> m_geometryTransformed;
	wrl::ComPtr<ID2D1GeometryRealization> m_geometryRealizationStroked;
	wrl::ComPtr<ID2D1GeometryRealization> m_geometryRealizationFilled;
	D2D1::Matrix3x2F m_matrixTransform;
	
	SolidColor m_colorFill = SolidColor::NoColor;
	SolidColor m_colorStroke = SolidColor::NoColor;
public:
	CGeometryElement();
	CGeometryElement(CGeometryElement&);
	virtual bool HitTest(const D2D1_POINT_2F& pt, UINT& flags);
	virtual	void Move(float x, float y);
	virtual void MoveDxDy(float dx, float dy);
	virtual void Transform(const D2D1::Matrix3x2F& matrix);
	virtual void Draw(CGraphics* graphics);
protected:
	void UpdateTransformedGeometry();
	virtual	void CreateRgnShape() noexcept = 0;
};

class CTextElement : public CElement {
public:
	enum ALIGMENT {
		T_CENTER = 1,
		T_LEFT = 2,
		T_RIGHT = 4,
		T_VCENTER = 8,
	};

	enum FONT_SIZE {
		FONT_SIZE_8,
		FONT_SIZE_12,
	};
private:
	template<class Archive>
	void save(Archive& ar, const unsigned int /*file_version*/) const {}

	template<class Archive>
	void load(Archive& ar, const unsigned int /*file_version*/) {
		SetText(text_);
	}

	friend boost::serialization::access;
	template<class Archive>
	void serialize(Archive& ar, const unsigned int file_version) {
		ar& boost::serialization::base_object<CElement>(*this);
		ar& m_align;

		/////////
		if (file_version > 0) ar& text_;
		else {
			if constexpr (Archive::is_loading::value) {
				CStringW str; ar& str;
				text_ = narrow(str);
			} else {
				assert(false);
			}
		}

		if (file_version > 1) ar& font_size_;

		boost::serialization::split_member(ar, *this, file_version);
	}
protected:
	virtual void OnTextChanged();
	void UpdatePosition();

	int m_align;
	FONT_SIZE font_size_ = FONT_SIZE_12;
	std::string text_;
	wrl::ComPtr<IDWriteTextLayout> textLayout_;
	CElementManipulator _manipulator;
public:
	DECLARE_TYPE_NAME("TEXT")
	DECLARE_VISITOR()

	virtual void Draw(CGraphics* pGraphics);
	virtual CManipulator* GetManipulator() { return &_manipulator; }

	void SetPosition(float x, float y, int align);
	void SetText(const std::string& text) {
		text_ = text;
		OnTextChanged();
	}

	CTextElement();
	CTextElement(std::string&& text, float x, float y, int align = (T_CENTER | T_VCENTER), FONT_SIZE font_size = FONT_SIZE_12);
};

BOOST_CLASS_VERSION(CTextElement, 2);

class CStaticTextElement : public CTextElement {
	friend boost::serialization::access;
	template<class Archive>
	void serialize(Archive& ar, const unsigned int file_version) {
		ar& boost::serialization::base_object<CTextElement>(*this);
	}

	static std::map<std::string, wrl::ComPtr<IDWriteTextLayout>> m_layouts;
protected:
	virtual void OnTextChanged();
public:
	DECLARE_TYPE_NAME("TEXT_STATIC")
	DECLARE_VISITOR()
	
	CStaticTextElement() = default;
	~CStaticTextElement();
};

class CDxEdit : public CElement {
	std::string text_;
	wrl::ComPtr<IDWriteTextLayout> text_layout_;
public:
	DECLARE_TYPE_NAME("EDIT")
	DECLARE_VISITOR()

	void SetText(const std::string& text);
	virtual void Draw(CGraphics* pGraphics);
	virtual CManipulator* GetManipulator() { return nullptr; }

	CDxEdit() {
		SetZOrder(6);
		SetHitAccess();
	}
};

class CGroup : public CGeometryElement {
	friend boost::serialization::access;
	template<class Archive>
	void save(Archive& ar, const unsigned int /*file_version*/) const {
	}
	template<class Archive>
	void load(Archive& ar, const unsigned int /*file_version*/) {
		CreateRgnShape();
	}
	template<class Archive>
	void serialize(Archive& ar, const unsigned int file_version) {
		ar& boost::serialization::base_object<CGeometryElement>(*this);
		ar& elements_;
		boost::serialization::split_member(ar, *this, file_version);
	}
protected:
	virtual	void CreateRgnShape() noexcept;

	std::vector<CElement*> elements_;
	using iterator_element = std::vector<CElement*>::iterator;
	Iter::StlContainerIterator<CElement*, std::vector> m_iterator;
public:
	virtual _Iterator* CreateIterator();
	virtual	void Move(float x, float y);
	virtual	void MoveDxDy(float dx, float dy);
	
	void AddGraphicElement(CElement* elem) {
		elements_.push_back(elem);
	}
	bool RemoveElement(CElement* elem) {
		return VectorFastErase(elements_, elem);
	}

	~CGroup();
};

class CFindElementsThatContainsDefinedVariables;
class CFindInternalReferences;
class CFindExternalReferences;
class CFindExternalReferencesToTheSameController;

class CValue;
class CAvrInternalPin;
class CSlotType;
class CSlot;
class CPin;
class CAssignedSegment;
class CAssignedSegmentValue;

namespace npsys {
class CAlgorithmExt;
}

enum SlotType {
	Value,
	InternalReference,
	ExternalReference,
	Peripheral,
	ModuleValue
};

struct SlotInfo {
	npsys::algorithm_n alg;
	npsys::fbd_slot_n slot;
	CSlotType* slot_type;
	SlotType slot_type2;
};

typedef std::deque<SlotInfo> SortedLink;
typedef std::unordered_map<const void*, int> color_m;

struct ParameterTrait {
	uint32_t traits;

	ParameterTrait()
		: traits(0) {}
	explicit ParameterTrait(uint32_t _traits)
		: traits(_traits) {}

	enum {
		PT_INVERTED = 1
	};

	bool IsInverted() const noexcept { return static_cast<bool>(traits & PT_INVERTED); }
};

class CSlotType {
	friend boost::serialization::access;
	template<class Archive>
	void serialize(Archive& ar, const unsigned int file_version) {}
public:
	virtual ~CSlotType() = default;
	virtual void Visit(CBlockVisitor& v) = 0;
	virtual npsys::variable_n* GetVariableAsNode() = 0;
	npsys::variable* GetVariable() noexcept {
		auto var = GetVariableAsNode();
		if (!var) return nullptr;
		return var->fetch() ? var->get() : nullptr;
	}
	virtual void ReleaseMemory() noexcept = 0;
	virtual int DefineTypeR() { return npsys::variable::VT_UNDEFINE; }
	virtual bool Equal(CSlotType*) { return false; }
	virtual bool Equal(CValue*) { return false; }
	virtual bool Equal(CAvrInternalPin*) { return false; }
	virtual bool CheckCycle(color_m& color, void* slot_ptr) {
		color[slot_ptr] = 2;
		return false;
	};
	virtual void TopoSort(
		npsys::algorithm_n& alg,
		npsys::fbd_slot_n& slot,
		color_m& color,
		SortedLink& sorted)
	{
		ASSERT(FALSE);
	};
	virtual void PrintDebugTooltip(std::stringstream& ss);
	virtual void FillPropertyGrid(CPropertyGrid*) noexcept {}
	virtual void OnPropertyChanged(WPARAM wParam, LPARAM lParam) {}
	virtual void ClearReferences() {}
	virtual ParameterTrait GetParameterTraits() { return {}; }
};

class CBlockInput : public CSlotType {
	friend boost::serialization::access;
	template<class Archive>
	void serialize(Archive& ar, const unsigned int file_version) {
		ar& boost::serialization::base_object<CSlotType>(*this);
	}
public:
	DECLARE_VISITOR()
	virtual npsys::variable_n* GetVariableAsNode();
	virtual void ReleaseMemory() noexcept override;
};

class CValue : public CSlotType, public COneVariable {
	friend boost::serialization::access;
	template<class Archive>
	void serialize(Archive& ar, const unsigned int file_version) {
		ar& boost::serialization::base_object<CSlotType>(*this);
		ar& boost::serialization::base_object<COneVariable>(*this);
	}
public:
	DECLARE_VISITOR()
	CValue() = default;
	explicit CValue(int type);
	virtual int DefineTypeR();
	virtual npsys::variable_n* GetVariableAsNode();
	virtual bool Equal(CValue* p);
	virtual bool Equal(CSlotType* p) { return p->Equal(this); }
	virtual void ReleaseMemory() noexcept override;
	virtual void TopoSort(
		npsys::algorithm_n& alg,
		npsys::fbd_slot_n& slot,
		color_m& color,
		SortedLink& sorted);
	virtual void FillPropertyGrid(CPropertyGrid* pPropertyGrid) noexcept;
	virtual void OnPropertyChanged(WPARAM wParam, LPARAM lParam);
};

class CValueRef : public CSlotType {
	friend boost::serialization::access;
	template<class Archive>
	void serialize(Archive& ar, const unsigned int file_version) {
		ar& boost::serialization::base_object<CSlotType>(*this);
	}
public:
	DECLARE_VISITOR()
	CValueRef() = default;
	CValueRef(npsys::variable_n& var) { variable_ = &var; }
	void SetReference(npsys::variable_n& var) noexcept {
		ATLASSUME(variable_ == nullptr);
		variable_ = &var;
	}
	virtual int DefineTypeR() {
		ASSERT(variable_);
		return (*variable_)->GetType();
	}
	virtual npsys::variable_n* GetVariableAsNode() {
		ASSERT(variable_);
		return variable_;
	}
	virtual void ReleaseMemory() noexcept override {}
protected:
	npsys::variable_n* variable_ = nullptr;
};

class CReference : public CSlotType {
	friend boost::serialization::access;
	template<class Archive>
	void serialize(Archive& ar, const unsigned int file_version) {
		ar& boost::serialization::base_object<CSlotType>(*this);
	}
public:
	virtual ~CReference() { CReference::ClearReferences(); }
	// shows whether slot type has valid reference or not
	bool IsLinkValid() const noexcept { return link_valid_; }
	const std::string& GetLinkName(bool* link_name_dirty = nullptr) noexcept {
		if (link_name_dirty) link_name_dirty_ = link_name_dirty;
		return link_name_;
	}
	virtual void ClearReferences() {
		con_.disconnect();
		link_valid_ = false;
	}
protected:
	bool link_valid_ = false;
	std::string link_name_;
	boost::signals2::connection con_;
	bool* link_name_dirty_ = nullptr;
};

class COutsideReference
	: public CReference {
	friend class CBlockCompositionWrapper;
	friend boost::serialization::access;
	template<class Archive>
	void serialize(Archive& ar, const unsigned int file_version) {
		ar& boost::serialization::base_object<CReference>(*this);
		ar& alg_;
	}
	bool initialized_ = false;
protected:
	virtual bool LoadLinkImpl() noexcept = 0;
public:
	COutsideReference() = default;
	COutsideReference(odb::weak_node<npsys::algorithm_n> alg)
		: alg_(alg) {}
	// called only once
	bool LoadLink() noexcept {
		if (initialized_) return link_valid_;
		initialized_ = true;
		return LoadLinkImpl();
	}
	virtual void ClearReferences() {
		CReference::ClearReferences();
		initialized_ = false;
	}
protected:
	odb::weak_node<npsys::algorithm_n> alg_;
};

class CInternalRef : public CReference {
	static constexpr int INVALID_LINK_ID = -1;
	
	friend boost::serialization::access;

	template<class Archive>
	void save(Archive& ar, const unsigned int file_version) const;
	
	template<class Archive>
	void load(Archive& ar, const unsigned int file_version);

	template<class Archive>
	void serialize(Archive& ar, const unsigned int file_version) {
		ar& boost::serialization::base_object<CReference>(*this);
		boost::serialization::split_member(ar, *this, file_version);
	}
public:
	DECLARE_VISITOR()
	CInternalRef();
	explicit CInternalRef(CSlot* slot_link)
		: slot_link_(nullptr)
		, saved_slot_link_(nullptr)
	{
		SetInternalLink(slot_link);
	}
	virtual ~CInternalRef();
	void SetInternalLink(CSlot* pSlotLink) noexcept;
	int GetSlotID() { return slot_id_; }
	virtual int DefineTypeR();
	virtual npsys::variable_n* GetVariableAsNode();
	virtual void ReleaseMemory() noexcept override;
	virtual bool CheckCycle(color_m& color, void* slot_ptr);
	virtual void TopoSort(
		npsys::algorithm_n& alg,
		npsys::fbd_slot_n& slot,
		color_m& color,
		SortedLink& sorted);
	void OnRefSlotStateChanged(SLOT_SIGNAL s) noexcept;
	virtual void ClearReferences() {
		CReference::ClearReferences();
		slot_link_ = nullptr;
		saved_slot_link_ = nullptr;
	}
protected:
	int slot_id_;
	CSlot* slot_link_;
	CSlot* saved_slot_link_;
};

class CAvrInternalPin
	: public COutsideReference
	, public IUploadable {
	friend class CBlockCompositionWrapper;
	friend class CAVR5DynamicLinker;
	friend boost::serialization::access;
	template<class Archive>
	void save(Archive& ar, const unsigned int /*file_version*/) const {}
	template<class Archive>
	void load(Archive& ar, const unsigned int /*file_version*/) {
		LoadLink();
	}
	template<class Archive>
	void serialize(Archive& ar, const unsigned int file_version) {
		ar& boost::serialization::base_object<COutsideReference>(*this);
		ar& dir_;
		ar& pin_;
		serialize_if_not_special(loaded_, ar, false);
		boost::serialization::split_member(ar, *this, file_version);
	}

	npsys::avr_pin_n pin_;
	npsys::remote::ExtLinkType dir_;
	bool loaded_;
protected:
	virtual bool LoadLinkImpl() noexcept final;
public:
	DECLARE_VISITOR()
	
	virtual ParameterTrait GetParameterTraits();
	virtual npsys::variable_n* GetVariableAsNode();
	virtual void ReleaseMemory() noexcept override;
	virtual void TopoSort(
		npsys::algorithm_n& alg,
		npsys::fbd_slot_n& slot,
		color_m& color,
		SortedLink& sorted);
	virtual int DefineTypeR();
	bool IsPinTypeEqualParamDirection() const noexcept;
	bool IsLoaded() const noexcept { return loaded_; }
	void SetLoaded() noexcept { loaded_ = true; }
	std::unique_ptr<odb::IMemento> CreateMemento_Uploadable() noexcept final {
		return odb::MakeMemento(*this, loaded_);
	}

	CAvrInternalPin() = default;
	CAvrInternalPin(
		npsys::avr_pin_n& pin,
		npsys::remote::ExtLinkType dir,
		odb::weak_node<npsys::algorithm_n> alg)
		: COutsideReference(alg)
		, pin_(pin)
		, dir_(dir)
		, loaded_(false)
	{
	}
};

class CModuleValue
	: public COutsideReference
	, public IUploadable {
	friend class CAVR5DynamicLinker;
	friend boost::serialization::access;
	template<class Archive>
	void save(Archive& ar, const unsigned int /*file_version*/) const {
	}
	template<class Archive>
	void load(Archive& ar, const unsigned int /*file_version*/) {
		LoadLink();
	}
	template<class Archive>
	void serialize(Archive& ar, const unsigned int file_version) {
		ar& boost::serialization::base_object<COutsideReference>(*this);
		ar& mod_;
		ar& segment_value_id_;
		serialize_if_not_special(last_loaded_var_, ar, {});
		ar& link_type_;
		boost::serialization::split_member(ar, *this, file_version);
	}

	boost::signals2::connection con_ctrl_name_;
	boost::signals2::connection con_module_name_;
	boost::signals2::connection con_module_segment_;
	boost::signals2::connection con_module_segment_value_;
public:
	DECLARE_VISITOR()
	CModuleValue() = default;
	CModuleValue(
		npsys::i2c_assigned_module_n& mod,
		oid_t segment_value_id,
		npsys::remote::ExtLinkType link_type);
	virtual npsys::variable_n* GetVariableAsNode();
	virtual void ReleaseMemory() noexcept override;
	virtual void TopoSort(
		npsys::algorithm_n& alg,
		npsys::fbd_slot_n& slot,
		color_m& color,
		SortedLink& sorted);

	virtual void ClearReferences() {
		COutsideReference::ClearReferences();
		con_ctrl_name_.disconnect();
		con_module_name_.disconnect();
		con_module_segment_.disconnect();
		con_module_segment_value_.disconnect();
	}

	virtual int DefineTypeR();
	bool IsPinTypeEqualParamDirection();

	virtual void PrintDebugTooltip(std::stringstream& ss);
	std::unique_ptr<odb::IMemento> CreateMemento_Uploadable() noexcept final {
		return odb::MakeMemento(*this, last_loaded_var_);
	}
	virtual ~CModuleValue() {
		std::cout << "~CModuleValue()" << std::endl;
	}
	npsys::CI2CModuleSegmentValue* value = nullptr;
private:
	// Stored
	npsys::i2c_assigned_module_n mod_;
	oid_t segment_value_id_;
	npsys::remote::ExtLinkType link_type_;
	npsys::variable_n last_loaded_var_;
protected:
	virtual bool LoadLinkImpl() noexcept final;
};

class CFixedValue : public CSlotType {
	friend boost::serialization::access;
	template<class Archive>
	void serialize(Archive& ar, const unsigned int file_version) {
		ar& boost::serialization::base_object<CSlotType>(*this);
		ar& variable_;
	}
public:
	DECLARE_VISITOR();
	virtual npsys::variable_n* GetVariableAsNode() {
		variable_.fetch();
		return &variable_;
	}
	virtual int DefineTypeR() {
		return variable_.fetch() ? variable_->GetType() : npsys::variable::VT_UNDEFINE;
	}
	virtual void ReleaseMemory() noexcept {
		if (variable_.is_invalid_node()) return;
		variable_.remove();
	}
	virtual void TopoSort(
		npsys::algorithm_n& alg,
		npsys::fbd_slot_n& slot,
		color_m& color,
		SortedLink& sorted);
	void Hardware_InitVariable(odb::weak_node<npsys::device_n> dev, int type, uint16_t addr);
private:
	npsys::variable_n variable_;
};

class CExternalReference
	: public COutsideReference
	, public COneVariable
	, public IUploadable {
	friend npsys::CAssignedAlgorithm;
	friend class CAVR5DynamicLinker;
	friend class CBlockCompositionWrapper;
	friend boost::serialization::access;
	template<class Archive>
	void serialize(Archive& ar, const unsigned int file_version) {
		ar& boost::serialization::base_object<COutsideReference>(*this);
		ar& boost::serialization::base_object<COneVariable>(*this);
		ar& link_;
		ar& place_;
		ar& type_;
		serialize_if_not_special(loaded_, ar, false);
		//	LoadLink(); recursive overflow if strong connected links exist
	}
public:
	DECLARE_VISITOR()

	enum Place {
		SAME_DEVICE,
		OTHER_DEVICE
	};

	struct LinkData {
		friend class boost::serialization::access;
		template<class Archive>
		void serialize(Archive& ar, const unsigned int /*file_version*/) {
			serialize_default_override(slot, ar);
			ar& alg;
			ar& loaded_variable;
		}

		odb::weak_node<npsys::algorithm_n> alg;
		npsys::fbd_slot_n slot;
		// previously loaded referenced variable
		odb::weak_node<npsys::variable_n> loaded_variable;

		LinkData() = default;
		LinkData(npsys::fbd_slot_n& _slot, npsys::algorithm_n& _alg)
			: slot(_slot)
			, alg(_alg)
		{
		}
	};

	CExternalReference() = default;
	CExternalReference(
		npsys::fbd_slot_n& ref_slot,
		npsys::algorithm_n& ref_alg,
		npsys::algorithm_n& alg);
	virtual int DefineTypeR();
	virtual npsys::variable_n* GetVariableAsNode();
	virtual bool Equal(CExternalReference* p);
	virtual void ReleaseMemory() noexcept override;
	virtual bool CheckCycle(color_m& color, void* slot_ptr);
	virtual void TopoSort(
		npsys::algorithm_n& alg,
		npsys::fbd_slot_n& slot,
		color_m& color,
		SortedLink& sorted);
	// link stuff
	virtual void PrintDebugTooltip(std::stringstream& ss);
	CSlot* GetRefSlot() noexcept;
	npsys::variable_n* GetRefVariableAsNode() noexcept;
	Place GetPlace() const noexcept { return place_; }
	void SetPlace(Place place) noexcept { place_ = place; }
	npsys::remote::ExtLinkType GetType() const noexcept { return type_; }
	void SetType(npsys::remote::ExtLinkType type) noexcept { type_ = type; }
	void OnRefSlotStateChanged(SLOT_SIGNAL s) noexcept;
	bool IsRefAlgorithmAssigned() noexcept;
	std::unique_ptr<odb::IMemento> CreateMemento_Uploadable() noexcept final {
		return odb::MakeMemento(*this, variable_, link_, loaded_);
	}
protected:
	virtual bool LoadLinkImpl() noexcept final;
	LinkData link_;
	Place place_;
	npsys::remote::ExtLinkType type_;
	bool loaded_;
};

// CSlot
class CSlot : public CGeometryElement {
public:
	enum SLOT_DIRECTION {
		INPUT_SLOT,
		OUTPUT_SLOT
	};
private:
	using base = CGeometryElement;
	friend class CBlockFactory;
	friend class ChangeSlotTypeCommand;
	friend class CSlotManipulator;
	friend class CParameterUpdater;
	friend class npsys::CFBDSlot;
	friend class npsys::CFBDBlock;
	friend class CBlock;
	friend class COutput;
	friend class CInput;
	friend boost::serialization::access;
	template<class Archive>
	void serialize(Archive& ar, const unsigned int file_version) {
		ar& boost::serialization::base_object<CGeometryElement>(*this);
		ar& id_;
		ar& slot_type_;
		ar& index_;
		ar& history_;
	}

	void SetParent(CBlock* parent) noexcept { parent_ = parent; }
protected:
	boost::signals2::signal<void(SLOT_SIGNAL)> sig_state_chaged_;

	void UpdateSlotPosition() noexcept;
	virtual	void CreateRgnShape() noexcept;

	boost::container::small_vector<CLine*, 5> lines_;
	CBlock* parent_ = nullptr;

	union {
		struct {
			bool st_deleted_from_editor : 1;
		};
		uint32_t st_state = 0;
	};
	// Online
	std::string _stValue;
	online_value value_;
	std::unique_ptr<CSlotType> slot_type_saved_;
	std::unique_ptr<CSlotType> slot_type_;
	bool activated_ = false;

	class CSlotManipulator : public CManipulator {
	public:
		CSlotManipulator(CSlot* slot) : _slot(slot) {}
		virtual S_CURSOR MouseButtonDown(CPoint point, CAlgorithmView* pWnd);
		virtual void Drag(CPoint point, CAlgorithmView* pWnd);
		virtual void MouseButtonUP(CPoint point, CAlgorithmView* pWnd);
		virtual void Draw(CGraphics* pGraphics);
	private:
		CSlot* _slot;
		std::unique_ptr<CLine, cpp_line_delete> line_;
		bool first_slot_no_error_;
		CSlot* _tmp_slot = nullptr;
		int begin_at_;
		CFindSlots fs;
	} manipulator_;

	struct {
		wrl::ComPtr<IDWriteTextLayout> text_layout;
		npsys::strings_n string;
		union {
			D2D1_ROUNDED_RECT rc;
			D2D1_POINT_2F pt;
		};
	} Online;

	bool history_ = false;
	int m_bHasLines = FALSE;
	int id_;
	size_t index_;
	CStaticTextElement text_;
	Iter::OneElementIterator<CElement*> iterator_;

	CSlot(int id, CSlotType* slot_type);
public:
	npsys::CFBDSlot* top;

	using line_container = boost::container::small_vector<CLine*, 5>;
	using line_iterator = line_container::iterator;

	virtual SLOT_DIRECTION GetDirection() const = 0;
	//Editor's methods
	virtual _Iterator* CreateIterator();
	virtual AlphaCursor GetCursor() final { return AlphaCursor::Pen; };
	virtual	void Draw(CGraphics* pGraphics);
	virtual CManipulator* GetManipulator() { return &manipulator_; }
	virtual bool ConnectToLine(CLine* line) = 0;
	virtual void AddLine(CLine*) = 0;
	virtual void DisconnectLine(CLine* pLine) = 0;
	virtual int DefineTypeR() = 0;
	virtual bool IsNode() const noexcept final { return true; }
	virtual int	IsConnected() = 0;
	//online
	virtual void SetValue(const nps::server_value* value);
	virtual void DrawOnlineValue(CGraphics* pGraphics);
	virtual void AlignOnlineValue() = 0;
	virtual void AlignText() = 0;
	//
	virtual npsys::variable* GetSlotAssociateVariable() = 0;
	virtual	void Move(float x, float y);
	virtual	void MoveDxDy(float dx, float dy);
	virtual const std::string& GetName() const noexcept { return top->get_name(); }
	virtual void SetName(const std::string& name) noexcept final {
		top->set_name(name);
	}
	virtual void PrintDebugTooltip(std::stringstream& ss);

	std::string GetFullName();
	line_iterator begin() { return lines_.begin(); }
	line_iterator end() { return lines_.end(); }
	int GetId() const noexcept { return id_; }
	void SetSlotType(CSlotType* pSlotType);
	CValue* CreateValueTypeSlot(int type) { return new CValue(type); }
	CInternalRef* CreateInternalTypeSlot() { return new CInternalRef; }
	//SlotType Methods
	CSlotType* GetSlotType() { ASSERT(slot_type_); return slot_type_.get(); }
	void OnSlotAddedToEditor() noexcept {
		st_deleted_from_editor = 0; // order is important!
		sig_state_chaged_(SIG_SLOT_INSERTED); // order is important!

	};
	void OnSlotDeletedFromEditor() noexcept {
		st_deleted_from_editor = true; // order is important!
		sig_state_chaged_(SIG_SLOT_DELETED); // order is important!
	};
	// index - index from the top
	// (x, y) - block top left corner
	void SetSlotPosition(size_t index, float x, float y, float block_width) noexcept;
	// (x, y) - block top left corner
	void SetInputSlotPosition(const D2D1_RECT_F& block_rect) noexcept;
	// (x, y) - block top left corner
	void SetOutputSlotPosition(const D2D1_RECT_F& block_rect) noexcept;
	size_t GetSlotIndex() const noexcept { return index_; }
	npsys::variable_n* GetVariableAsNode() {
		return slot_type_->GetVariableAsNode();
	}
	npsys::variable* GetVariable() {
		auto var = GetVariableAsNode();
		return ((var && var->loaded()) ? var->get() : nullptr);
	}
	CBlock* GetParentBlock() const noexcept {
		return parent_;
	}
	void BeforeStartOnline();
	void AfterStopOnline();
	bool IsModified() noexcept {
		return static_cast<bool>(st_state);
	}
	void ResetState() noexcept {
		st_state = 0;
	}
	void ParentNameChanged() noexcept {
		sig_state_chaged_(SIG_SLOT_PARENT_NAME_CHANGED);
	}
	online_value& OnlineGetValue() { return value_; }
	void CommitTypeChanges();
	void DeletePermanent() noexcept {
		sig_state_chaged_(SIG_SLOT_DELETED_PERMANENT);
		sig_state_chaged_.disconnect_all_slots();
		slot_type_->ReleaseMemory();
	}
	boost::signals2::connection AdviceToEvents(std::function<void(SLOT_SIGNAL)>&& fn) noexcept {
		return sig_state_chaged_.connect(fn);
	}
	bool IsDeletedFromEditor() const noexcept { return st_deleted_from_editor; }
	CStaticTextElement* GetTextElement() noexcept { return &text_; }
	void ClearReferences() noexcept {
		lines_.clear();
		GetSlotType()->ClearReferences();
	}

	CSlot();
	CSlot(CSlot&);
	~CSlot();
};

template<class Archive>
void CInternalRef::save(Archive& ar, const unsigned int /*file_version*/) const {
	if (slot_link_) {
		ar & slot_link_->GetId();
	} else {
		ar & INVALID_LINK_ID;
	}
}

template<class Archive>
void CInternalRef::load(Archive& ar, const unsigned int /*file_version*/) {
	ar & slot_id_;
	slot_link_ = nullptr;
	saved_slot_link_ = nullptr;
}

class CSlotGroup : public CGeometryElement {
	friend boost::serialization::access;
	template<class Archive>
	void save(Archive& ar, const unsigned int /*file_version*/) const {}
	template<class Archive>
	void load(Archive& ar, const unsigned int /*file_version*/) {
		CreateRgnShape();
	}
	template<class Archive>
	void serialize(Archive& ar, const unsigned int file_version) {
		ar& boost::serialization::base_object<CGeometryElement>(*this);
		ar& elements_;
		boost::serialization::split_member(ar, *this, file_version);
	}
public:
	DECLARE_TYPE_NAME("SLOT_GROUP")
	DECLARE_VISITOR()
	CSlotGroup() = default;
	CSlotGroup(CSlot* slot);
	virtual ~CSlotGroup();
	virtual _Iterator* CreateIterator();
	virtual	void Move(float x, float y);
	virtual	void MoveDxDy(float dx, float dy);
	CSlot* GetSlot() { return static_cast<CSlot*>(elements_[0]); }
	CStaticTextElement* GetText() { return static_cast<CStaticTextElement*>(elements_[1]); }
protected:
	virtual	void CreateRgnShape() noexcept;
	//
	Iter::StlArrayIterator<CElement*, 2> iterator_;
	std::array<CElement*, 2> elements_;
};

class CInputSlot : public CSlot {
	friend class CBlockFactory;
	friend boost::serialization::access;
	template<class Archive>
	void serialize(Archive& ar, const unsigned int file_version) {
		ar& boost::serialization::base_object<CSlot>(*this);
	}

	CInputSlot(int id, CSlotType* slot_type)
		: CSlot(id, slot_type) {}
public:
	DECLARE_TYPE_NAME("INPUT_SLOT")
	DECLARE_VISITOR()
	
	virtual bool ConnectToLine(CLine* line);
	virtual void AddLine(CLine* line);
	virtual void AlignOnlineValue();
	virtual void DisconnectLine(CLine* line);
	virtual SLOT_DIRECTION GetDirection() const final { return CSlot::INPUT_SLOT; }
	virtual int DefineTypeR();
	virtual npsys::variable* GetSlotAssociateVariable();
	virtual void AlignText();
	virtual int IsConnected();

	CSlot* GetAnotherSlot();
	npsys::variable* GetInputVariable();

	CInputSlot() = default;
};

class COutputSlot : public CSlot {
	friend class CBlockFactory;
	friend boost::serialization::access;
	template<class Archive>
	void serialize(Archive& ar, const unsigned int file_version) {
		ar& boost::serialization::base_object<CSlot>(*this);
	}

	COutputSlot(int id, CSlotType* slot_type)
		: CSlot(id, slot_type) {}
public:
	DECLARE_TYPE_NAME("OUTPUT_SLOT")
	DECLARE_VISITOR()
	
	virtual bool ConnectToLine(CLine* line);
	virtual void AddLine(CLine*);
	virtual int IsConnected() { return !lines_.empty(); }
	virtual int DefineTypeR() { return slot_type_->DefineTypeR(); }
	virtual void DisconnectLine(CLine* pLine);
	virtual void SetValue(const nps::server_value* value);
	virtual void AlignOnlineValue();
	virtual npsys::variable* GetSlotAssociateVariable() {
		auto var = GetVariableAsNode();
		return var ? var->get() : nullptr;
	}
	virtual void AlignText();
	virtual SLOT_DIRECTION GetDirection() const final { return CSlot::OUTPUT_SLOT; }

	void CalcLinesOffset();

	COutputSlot() = default;
};

class CInplaceBlockProperty : public CGroup {
	BlockProperty& prop_;
public:
	DECLARE_TYPE_NAME("INPLACE_BLOCK_PROPERTY")

	virtual void Visit(CBlockVisitor& v) { v.Accept(this); }
	virtual bool IsSerializable() const noexcept { return false; }
	virtual CManipulator* GetManipulator() { return nullptr; }
	virtual void Draw(CGraphics* graphics) {
		if (prop_.GetDirty<0>()) {
			GetEditElem()->SetText(prop_.ToString());
		}
	}

	CStaticTextElement* GetTextElem() { return static_cast<CStaticTextElement*>(elements_[0]); }
	CDxEdit* GetEditElem() { return static_cast<CDxEdit*>(elements_[1]); }

	CInplaceBlockProperty(BlockProperty& prop, const D2D1_RECT_F& rect)
		: prop_(prop) {

		rect_ = rect;

		D2D1_RECT_F rc;

		auto text_elem = new CStaticTextElement;
		text_elem->SetText(prop_.GetName());
		text_elem->SetPosition(rect.left, rect.top, CTextElement::T_VCENTER | CTextElement::T_LEFT);
		text_elem->GetRect(rc);
		AddGraphicElement(text_elem);

		auto var_elem = new CDxEdit();
		var_elem->SetRect(rect.right - 50, rc.top, 50, rc.bottom - rc.top);
		var_elem->SetText(prop_.ToString());
		AddGraphicElement(var_elem);

		SetZOrder(6);
		CreateRgnShape();
	}
};

class CBlock
	: public CGroup
	, public CGraphElement 
{
	friend class CBlockFactory;
	friend class CTreeBlock;
	friend class npsys::CFBDBlock;
	friend class CBlockManipulator;
	friend class CGroupOfBlocksManipulator;
	friend class CSlotManipulator;

	friend boost::serialization::access;
	template<class Archive>
	void save(Archive& ar, const unsigned int /*file_version*/) const {
		ar << elements_.size() - top->slots.i_size() - top->slots.o_size() - inplace_prop_count_;
		for (auto& e : elements_) {
			if (!e->IsNode() && e->IsSerializable()) ar << e;
		}
	}
	template<class Archive>
	void load(Archive& ar, const unsigned int /*file_version*/) {
		size_t size;
		ar >> size;
		while (size--) {
			CElement* e;
			ar >> e;
			elements_.push_back(e);
		}
		CreateInplaceProperties();
	}
	template<class Archive>
	void serialize(Archive& ar, const unsigned int file_version) {
		ar& boost::serialization::base_object<CGeometryElement>(*this);
		ar& execute_order_; // remove this
		ar& prop_list_;
		boost::serialization::split_member(ar, *this, file_version);
	}

	class CBlockManipulator : public CElementManipulator {
	public:
		CBlockManipulator(CBlock* pBlock)
			: CElementManipulator(reinterpret_cast<CElement*>(pBlock)) {
		}
		virtual void Drag(CPoint point, CAlgorithmView* pWnd);
		virtual void MouseButtonUP(CPoint point, CAlgorithmView* pWnd);
		virtual void Draw(CGraphics* pGraphics);
	protected:
		std::vector<CLine*> m_lines;
	} manipulator_;
protected:
	virtual	void CreateRgnShape() noexcept;
	virtual void CreateInplaceProperties() noexcept {}

	size_t inplace_prop_count_ = 0;
	std::vector<std::unique_ptr<BlockProperty>> prop_list_;
	int execute_order_ = -1;
	bool fresh_ = false;
	BlockColor m_colorIndex = BlockColor::Standart;

	CBlock();
public:
	npsys::CFBDBlock* top;

	virtual void Translate(CCodeGenerator* pGenerator) = 0;
	virtual Command* DeleteElement(CBlockComposition* pBlocks, CElement::MyQuadrantNode* qtRoot);
	virtual AlphaCursor GetCursor() final { return IsSelected() ? AlphaCursor::Drag : AlphaCursor::Arrow; }
	virtual AlphaCursor GetOnlineCursor() { return AlphaCursor::LinkSelect; }
	virtual void Check(CBlockVisitor& v) {}
	virtual void Draw(CGraphics* pGraphics);
	virtual void InsertToWrapper(CBlockCompositionWrapper* wrapper) noexcept final;
	virtual	int	GetPriority() { return 0; }
	virtual void DeletePermanent();
	virtual	MoveCommand* CreateMoveCommand(const D2D1_POINT_2F& delta, CElement::MyQuadrantNode* qtRoot);
	virtual CManipulator* GetManipulator() { return &manipulator_; }
	virtual const std::string& GetName() const noexcept final { return top->get_name(); }
	virtual void SetName(const std::string& name) noexcept;
	virtual bool IsNode() const noexcept final { return true; }
	virtual void PrintDebugTooltip(std::stringstream& ss);
	virtual void FillPropertyGrid(CPropertyGrid* pPropertyGrid) noexcept;
	virtual void OnPropertyChanged(WPARAM wParam, LPARAM lParam) noexcept;
	void Expand(int nCount);
	void UpdateBezierLines();
	void AddGraphicElement(CElement* elem) {
		CGroup::AddGraphicElement(elem);
	}
	void AddGraphicElement(CInplaceBlockProperty* prop) {
		CGroup::AddGraphicElement(prop);
		inplace_prop_count_++;
	}
	void Hardware_InitInternal(CSlot::SLOT_DIRECTION slot_dir, size_t slot_index, 
		int var_type, uint16_t addr, odb::weak_node<npsys::device_n> dev);
	size_t InputHasValidVariable() noexcept;
	std::vector<CLine*> GetLines() const;
	void CollectLines(std::vector<CLine*>& lines) const;
	void SetExecuteOrder(int execute_order) { execute_order_ = execute_order; }
	int GetExecuteOrder() const noexcept { return execute_order_; }
	void MountSlot(npsys::fbd_slot_n&& slot);
	CTextElement* GetTextName() { return static_cast<CTextElement*>(elements_[0]); }
	static constexpr size_t CustomElementIdxBegin() { return 2; }
	CElement* FirstCustomElement() noexcept {
		ASSERT(CustomElementIdxBegin() < elements_.size());
		return elements_[CustomElementIdxBegin()];
	}

	void UpdateSize();

	void OnBlockAddedToEditor() noexcept {
		for (auto& slot : top->slots) slot->e_slot->OnSlotAddedToEditor();
	};

	void OnBlockDeletedFromEditor() noexcept {
		for (auto& slot : top->slots) slot->e_slot->OnSlotDeletedFromEditor();
	};

	template<typename T>
	void AddProperty(PRB::Section section, const std::string& name, std::string_view key, T defValue = {}) {
		prop_list_.push_back(std::move(
			std::make_unique<BlockPropertyT<T>>(section, name, key, defValue)
		));
	}

	template<typename T>
	BlockPropertyT<T>* GetPropertyByKey(std::string_view key) {
		for (auto& i : prop_list_) {
			if (i->GetKey() == key) {
				ASSERT(dynamic_cast<BlockPropertyT<T>*>(i.get()));
				return static_cast<BlockPropertyT<T>*>(i.get());
			}
		}
		throw std::runtime_error("Property with key \"" + std::string(key) + "\" does not exist.");
		return nullptr;
	}

	~CBlock();
};

class CDynamicBlock : public CBlock {
	friend class CBlockFactory;
	friend boost::serialization::access;
	template<class Archive>
	void serialize(Archive& ar, const unsigned int file_version) {
		ar& boost::serialization::base_object<CBlock>(*this);
	}
public:
	virtual CMenuHandle GetContextMenu();
	void ExpandReduceInputs(size_t new_count, CBlockFactory& factory);
};

#define DECLARE_TRANSLATE() \
	virtual void Translate(class CCodeGenerator*);

#define DECLARE_SIMPLE_BLOCK_CLASS(classname, blockname, colorIndex) \
class classname : public CBlock { \
	friend class CBlockFactory; \
	friend boost::serialization::access; \
	friend class CTypeDeterminant; \
	friend class CTACGenerator; \
	friend class CAVR5CodeGenerator; \
	template<class Archive> \
	void serialize(Archive & ar, const unsigned int file_version) { \
		ar & boost::serialization::base_object<CBlock>(*this); \
	} \
	classname() { \
		m_colorIndex = colorIndex; \
	} \
public:	\
	DECLARE_TYPE_NAME(blockname) \
	DECLARE_VISITOR()	\
	DECLARE_TRANSLATE() \
};

#define DECLARE_DYNAMIC_BLOCK_CLASS(classname, blockname, colorIndex) \
class classname : public CDynamicBlock { \
	friend class CBlockFactory; \
	friend class CTypeDeterminant; \
	friend class CTACGenerator; \
	friend class CAVR5CodeGenerator; \
	friend boost::serialization::access; \
	template<class Archive> \
	void serialize(Archive & ar, const unsigned int file_version) { \
		ar & boost::serialization::base_object<CDynamicBlock>(*this); \
	} \
	classname() { \
		m_colorIndex = colorIndex; \
	} \
public:	\
	DECLARE_TYPE_NAME(blockname) \
	DECLARE_VISITOR()	\
	DECLARE_TRANSLATE()	\
};

DECLARE_DYNAMIC_BLOCK_CLASS(COr, "OR", BlockColor::Standart)
DECLARE_DYNAMIC_BLOCK_CLASS(CAnd, "AND", BlockColor::Standart)
DECLARE_SIMPLE_BLOCK_CLASS(CRsTrigger, "RS", BlockColor::Standart)
DECLARE_SIMPLE_BLOCK_CLASS(CNot, "NOT", BlockColor::Standart)

DECLARE_SIMPLE_BLOCK_CLASS(CAdd, "ADD(+)", BlockColor::Math)
DECLARE_SIMPLE_BLOCK_CLASS(CSub, "SUB(-)", BlockColor::Math)
DECLARE_SIMPLE_BLOCK_CLASS(CMul, "MUL(*)", BlockColor::Math)
DECLARE_SIMPLE_BLOCK_CLASS(CDiv, "DIV(/)", BlockColor::Math)

#define DB_DELETE(Parent) \
virtual void DeletePermanent() noexcept override { \
	CBlock::DeletePermanent(); \
	Parent::ReleaseMemory(); \
}

class CBinaryEncoder : public CBlock {
	friend class CBlockFactory;
	friend class CTypeDeterminant;
	friend class CTACGenerator;
	friend class CAVR5CodeGenerator;
	friend boost::serialization::access;
	template<class Archive>
	void serialize(Archive& ar, const unsigned int file_version) {
		ar& boost::serialization::base_object<CBlock>(*this);
	}
public:
	DECLARE_TYPE_NAME("BINARY_ENCODER")
	DECLARE_VISITOR()
	DECLARE_TRANSLATE()
};

class CBinaryDecoder : public CBlock {
	friend class CBlockFactory;
	friend class CTypeDeterminant;
	friend class CTACGenerator;
	friend class CAVR5CodeGenerator;
	friend boost::serialization::access;
	template<class Archive>
	void serialize(Archive& ar, const unsigned int file_version) {
		ar& boost::serialization::base_object<CBlock>(*this);
	}
public:
	DECLARE_TYPE_NAME("BINARY_DECODER")
	DECLARE_VISITOR()
	DECLARE_TRANSLATE()
};

class CPositiveEdge : public CBlock, public COneVariable {
	friend class CBlockFactory;
	friend class CTypeDeterminant;
	friend class CTACGenerator;
	friend class CAVR5CodeGenerator;
	friend boost::serialization::access;
	template<class Archive>
	void serialize(Archive& ar, const unsigned int file_version) {
		ar& boost::serialization::base_object<CBlock>(*this);
		ar& boost::serialization::base_object<COneVariable>(*this);
	}
public:
	DECLARE_TYPE_NAME("PE")
	DECLARE_VISITOR()
	DECLARE_TRANSLATE()
	DB_DELETE(COneVariable)
};

class CCounter : public CBlock, public COneVariable {
	friend class CBlockFactory;
	friend class CTypeDeterminant;
	friend class CTACGenerator;
	friend class CAVR5CodeGenerator;
	friend boost::serialization::access;
	template<class Archive>
	void serialize(Archive& ar, const unsigned int file_version) {
		ar& boost::serialization::base_object<CBlock>(*this);
		ar& boost::serialization::base_object<COneVariable>(*this);
	}
public:
	DECLARE_TYPE_NAME("CNT")
	DECLARE_VISITOR()
	DECLARE_TRANSLATE()
	DB_DELETE(COneVariable)
};

class CNegativeEdge : public CBlock, public COneVariable {
	friend class CBlockFactory;
	friend class CTypeDeterminant;
	friend class CTACGenerator;
	friend class CAVR5CodeGenerator;
	friend boost::serialization::access;
	template<class Archive>
	void serialize(Archive& ar, const unsigned int file_version) {
		ar& boost::serialization::base_object<CBlock>(*this);
		ar& boost::serialization::base_object<COneVariable>(*this);
	}
public:
	DECLARE_TYPE_NAME("NE")
	DECLARE_VISITOR()
	DECLARE_TRANSLATE()
	DB_DELETE(COneVariable)
};

namespace boost::serialization {

template<class Archive>
inline void load_construct_data(Archive& ar, CNegativeEdge* t, const unsigned int) {
	::new(t)CNegativeEdge();
}

} // boost::serialization


class CAnyEdge : public CBlock, public COneVariable {
	friend class CBlockFactory;
	friend class CTypeDeterminant;
	friend class CTACGenerator;
	friend class CAVR5CodeGenerator;
	friend boost::serialization::access;
	template<class Archive>
	void serialize(Archive& ar, const unsigned int file_version) {
		ar& boost::serialization::base_object<CBlock>(*this);
		ar& boost::serialization::base_object<COneVariable>(*this);
	}
public:
	DECLARE_TYPE_NAME("AE")
	DECLARE_VISITOR()
	DECLARE_TRANSLATE()
	DB_DELETE(COneVariable)
};


class CDelay : public CBlock, public COneVariable {
	friend class CBlockFactory;
	friend class CTypeDeterminant;
	friend class CTACGenerator;
	friend class CAVR5CodeGenerator;
	friend boost::serialization::access;
	template<class Archive>
	void serialize(Archive& ar, const unsigned int file_version) {
		ar& boost::serialization::base_object<CBlock>(*this);
		ar& boost::serialization::base_object<COneVariable>(*this);
	}
protected:
	virtual void CreateInplaceProperties() noexcept;
public:
	DECLARE_TYPE_NAME("DELAY")
	DECLARE_VISITOR()
	DECLARE_TRANSLATE()
	DB_DELETE(COneVariable)
};

class CPID : public CBlock, public CVarContainer {
	friend class CBlockFactory;
	friend class CTypeDeterminant;
	friend class CTACGenerator;
	friend class CAVR5CodeGenerator;
	template<class Archive>
	void save(Archive& ar, const unsigned int /*file_version*/) const {
	}
	template<class Archive>
	void load(Archive& ar, const unsigned int /*file_version*/) {
		static_cast<CValueRef*>(top->slots.at_o(0).GetSlotType())->SetReference(variables_[0]);
	}
	friend boost::serialization::access;
	template<class Archive>
	void serialize(Archive& ar, const unsigned int file_version) {
		ar& boost::serialization::base_object<CBlock>(*this);
		ar& boost::serialization::base_object<CVarContainer>(*this);
		boost::serialization::split_member(ar, *this, file_version);
	}
	CPID() {
		m_colorIndex = BlockColor::Control;
	}
public:
	DECLARE_TYPE_NAME("PID")
	DECLARE_VISITOR()
	DECLARE_TRANSLATE()
	DB_DELETE(CVarContainer)
	/*
	typedef struct pid_s
	{
		float out;
		uint8_t reserved;
		float kp;
		float ki;
		float kd;
		float e_pr;
		float integsum;
		float dt;
	} pid_t;
	*/
private:
	enum { PID_OUT = 0, PID_KP, PID_KI, PID_KD };
};

class CAlarmHigh : public CBlock {
	friend class CBlockFactory;
	friend class CTypeDeterminant;
	friend class CTACGenerator;
	friend class CAVR5CodeGenerator;
	friend boost::serialization::access;
	template<class Archive>
	void serialize(Archive& ar, const unsigned int file_version) {
		ar& boost::serialization::base_object<CBlock>(*this);
	}
	CAlarmHigh() {
		m_colorIndex = BlockColor::Compare;
	}
public:
	DECLARE_TYPE_NAME("ALARM_HIGH")
	DECLARE_VISITOR()
	DECLARE_TRANSLATE()
};

class CAlarmLow : public CBlock {
	friend class CBlockFactory;
	friend class CTypeDeterminant;
	friend class CTACGenerator;
	friend class CAVR5CodeGenerator;
	friend boost::serialization::access;
	template<class Archive>
	void serialize(Archive& ar, const unsigned int file_version) {
		ar& boost::serialization::base_object<CBlock>(*this);
	}
	CAlarmLow() {
		m_colorIndex = BlockColor::Compare;
	}
public:
	DECLARE_TYPE_NAME("ALARM_LOW")
	DECLARE_VISITOR()
	DECLARE_TRANSLATE()
};

class CInternalBlockRef {
public:
	virtual void HardwareSpecific_Init(npsys::CNetworkDevice* device) = 0;
	virtual void HardwareSpecific_Clear() = 0;
};

class CTime
	: public CBlock
	, public CInternalBlockRef {
	friend class CBlockFactory;
	friend class CTypeDeterminant;
	friend class CTACGenerator;
	friend class CAVR5CodeGenerator;
	friend boost::serialization::access;
	template<class Archive>
	void serialize(Archive& ar, const unsigned int file_version) {
		ar& boost::serialization::base_object<CBlock>(*this);
	}
	CTime() {
		m_colorIndex = BlockColor::Standart;
	}
public:
	DECLARE_TYPE_NAME("TIME")
	DECLARE_VISITOR()
	DECLARE_TRANSLATE()
	
	virtual void HardwareSpecific_Init(npsys::CNetworkDevice* device);
	virtual void HardwareSpecific_Clear();
};

class CConfigurableBlock : public CBlock {
	friend class CDlg_ConfigurableBlock;
	friend class CFindSlots;
	friend boost::serialization::access;
	template<class Archive>
	void serialize(Archive& ar, const unsigned int file_version) {
		ar& boost::serialization::base_object<CBlock>(*this);
		ar& disabled_slots_;
	}
	std::vector<npsys::fbd_slot_n> disabled_slots_;
public:
	CInputSlot* GetInput(const std::string_view slot_name);
	COutputSlot* GetOutput(const std::string_view slot_name);
	void AddDisabledSlot(npsys::fbd_slot_n&&);
	virtual CMenuHandle GetContextMenu();
	void EnableSlot(npsys::fbd_slot_n&);
	void DisableSlot(npsys::fbd_slot_n&);
};

class CComparator : public CConfigurableBlock {
	friend class CBlockFactory;
	friend class CTypeDeterminant;
	friend class CTACGenerator;
	friend class CAVR5CodeGenerator;
	friend boost::serialization::access;
	template<class Archive>
	void serialize(Archive& ar, const unsigned int file_version) {
		ar& boost::serialization::base_object<CConfigurableBlock>(*this);
	}
public:
	DECLARE_TYPE_NAME("COMPARATOR")
	DECLARE_VISITOR()
	DECLARE_TRANSLATE()
};


class CBlockFunction : public CBlock {
	friend class CBlockFactory;
	friend class CTypeDeterminant;
	friend class CTACGenerator;
	friend class CAVR5CodeGenerator;
	friend class CDlg_BlockFunctionEquation;
	friend boost::serialization::access;
	template<class Archive>
	void serialize(Archive& ar, const unsigned int file_version) {
		ar& boost::serialization::base_object<CBlock>(*this);
		ar& equation_;
	}

	std::string equation_;
public:
	DECLARE_TYPE_NAME("FUNCTION")
	DECLARE_VISITOR()
	DECLARE_TRANSLATE()

	virtual CMenuHandle GetContextMenu();
};

class CSliderThing : public CGeometryElement {
	friend class CBlockFactory;

	CSliderManipulator manipulator_;
	CGroup* parent_ = nullptr;

	friend boost::serialization::access;
	template<class Archive>
	void serialize(Archive& ar, const unsigned int file_version) {
		ar& boost::serialization::base_object<CGeometryElement>(*this);
	}

	CSliderThing();
	CSliderThing(CScheduleSlider* parent);
public:
	DECLARE_TYPE_NAME("SCHEDULE_THING")
	DECLARE_VISITOR()

	// virtual void Draw(CGraphics* pGraphics);
	virtual CManipulator* GetManipulator() { return &manipulator_; }

	void AfterDeserialization(CGroup* parent) {
		parent_ = parent;
		manipulator_.AfterDeserialization(parent);
	}
	virtual CMenuHandle GetContextMenu();
	CGroup* GetParent() noexcept { return parent_; }
protected:
	virtual	void CreateRgnShape() noexcept;
};

class CScheduleSlider : public CGroup {
	friend class CBlockFactory;
	friend boost::serialization::access;
	template<class Archive>
	void save(Archive& ar, const unsigned int /*file_version*/) const {}
	template<class Archive>
	void load(Archive& ar, const unsigned int /*file_version*/) {
		for (size_t i = 1; i < elements_.size(); ++i) {
			static_cast<CSliderThing*>(elements_[i])->AfterDeserialization(this);
		}
	}
	template<class Archive>
	void serialize(Archive& ar, const unsigned int file_version) {
		ar& boost::serialization::base_object<CGroup>(*this);
		boost::serialization::split_member(ar, *this, file_version);
	}
	CScheduleSlider();
protected:
	virtual	void CreateRgnShape() noexcept;
public:
	DECLARE_TYPE_NAME("SCHEDULE_SLIDER")
	DECLARE_VISITOR()

	virtual CManipulator* GetManipulator() { return nullptr; }
	virtual CMenuHandle GetContextMenu();

	struct SliderInfo {
		float length;
		std::vector<float> positions;
	};

	CSliderTimeChart* GetTimeChart();
	SliderInfo GetSliderInfo();
};

class CSliderTimeChart : public CGroup {
	friend class CBlockFactory;
	friend boost::serialization::access;
	template<class Archive>
	void serialize(Archive& ar, const unsigned int file_version) {
		ar& boost::serialization::base_object<CGroup>(*this);
	}
	CSliderTimeChart();
protected:
	virtual	void CreateRgnShape() noexcept;
public:
	DECLARE_TYPE_NAME("SLIDER_TIME_CHART")
	DECLARE_VISITOR()

	virtual CManipulator* GetManipulator() { return nullptr; }
};

class CBlockSchedule
	: public CBlock
	, public CVarContainer
	, public CInternalBlockRef {
	friend class CBlockFactory;
	friend class CTypeDeterminant;
	friend class CTACGenerator;
	friend class CAVR5CodeGenerator;

	friend boost::serialization::access;
	template<class Archive>
	void serialize(Archive& ar, const unsigned int file_version) {
		ar& boost::serialization::base_object<CBlock>(*this);
		ar& boost::serialization::base_object<CVarContainer>(*this);
	}
public:
	DECLARE_TYPE_NAME("SCHEDULE")
	DECLARE_VISITOR()
	DECLARE_TRANSLATE()
	DB_DELETE(CVarContainer)

	virtual void HardwareSpecific_Init(npsys::CNetworkDevice* device);
	virtual void HardwareSpecific_Clear();
	virtual void PrintDebugTooltip(std::stringstream& ss);

	CScheduleSlider* GetSlider();
};

enum PARAM_SCOPE {
	GLOBAL,
	INTERNAL
};

class CParameter : public CBlock {
	friend class CDlg_BlockParam;
	friend boost::serialization::access;
public:
	enum PARAMETER_DIRECTION {
		INPUT_PARAMETER,
		OUTPUT_PARAMETER
	};
private:
	template<class Archive>
	void serialize(Archive& ar, const unsigned int file_version) {
		ar& boost::serialization::base_object<CBlock>(*this);
		ar& m_paramType;
		if (file_version > 0) {
			ar& quality_;
		}
	}

	PARAMETER_TYPE m_paramType;
	bool link_name_dirty_ = true;
	float link_text_width_;
	wrl::ComPtr<IDWriteTextLayout> link_text_layout_;
	bool quality_ = true;
protected:
	CParameter();
public:
	CSlot* GetSlot() {
		return (*top->slots.begin_in())->e_slot.get();
	}
	virtual void Translate(std::string& prog) {}
	virtual void Draw(CGraphics* pGraphics);
	virtual void PrintDebugTooltip(std::stringstream& ss);
	virtual bool ShowPropertiesDialog(CBlockComposition* blocks, npsys::algorithm_n& alg);
	virtual bool ShowOnlineDialog(npsys::CAlgorithmExt* alg);
	PARAMETER_TYPE GetParamType();
	PARAMETER_TYPE SetParamType(PARAMETER_TYPE);
	virtual PARAM_SCOPE GetScope();
	virtual CMenuHandle GetContextMenu();
	virtual PARAMETER_DIRECTION GetDirection() const noexcept = 0;
	virtual void FillPropertyGrid(CPropertyGrid* pPropertyGrid) noexcept;
	virtual void OnPropertyChanged(WPARAM wParam, LPARAM lParam) noexcept;
	bool GetQuaility() const noexcept {
		return quality_;
	}
};

class CInaccessibleParameter {
public:
	virtual PARAM_SCOPE GetScope();
};

class CInput : public CParameter {
	friend class CBlockFactory;
	friend class CTypeDeterminant;
	friend class CTACGenerator;
	friend class CAVR5CodeGenerator;
	friend boost::serialization::access;
	template<class Archive>
	void serialize(Archive& ar, const unsigned int file_version) {
		ar& boost::serialization::base_object<CParameter>(*this);
	}
	virtual PARAMETER_DIRECTION GetDirection() const noexcept { return CParameter::INPUT_PARAMETER; }
	virtual void SetName(const std::string& name) noexcept final;
	void MountSlot(npsys::fbd_slot_n&& slot);
	void AdjustBlock() noexcept;

public:
	DECLARE_TYPE_NAME("INPUT")
	DECLARE_VISITOR()
	DECLARE_TRANSLATE()
};

class COutput : public CParameter {
	friend class CBlockFactory;
	friend class CTypeDeterminant;
	friend class CTACGenerator;
	friend class CAVR5CodeGenerator;
	friend boost::serialization::access;
	template<class Archive>
	void serialize(Archive& ar, const unsigned int file_version) {
		ar& boost::serialization::base_object<CParameter>(*this);
	}
	virtual PARAMETER_DIRECTION GetDirection() const noexcept { return CParameter::OUTPUT_PARAMETER; }
	virtual void SetName(const std::string& name) noexcept final;
	void MountSlot(npsys::fbd_slot_n&& slot);
	void AdjustBlock() noexcept;
public:
	DECLARE_TYPE_NAME("OUTPUT")
	DECLARE_VISITOR()
	DECLARE_TRANSLATE()
	virtual int	GetPriority() { return 1; }
};


BOOST_CLASS_VERSION(CParameter, 1)




namespace npsys {
template<typename Archive>
void CFBDSlot::serialize2(Archive& ar, const unsigned int file_version) {
	ar & e_slot;
	if constexpr (Archive::is_loading::value) {
		e_slot->top = this;
		e_slot->GetTextElement()->SetText(name_);
		e_slot->AlignText();
	}
}

template<typename Archive>
void CFBDBlock::serialize2(Archive& ar, const unsigned int file_version) {
	ar & e_block;
	if constexpr (Archive::is_loading::value) {
		e_block->top = this;
		slots.fetch_all_nodes();
		for (auto& slot : slots) {
			slot->e_slot->SetParent(e_block.get());
			slot->e_slot->CreateRgnShape();
			slot->e_slot->UpdateTransformedGeometry();
			e_block->elements_.push_back(slot->e_slot.get());
		}
	}
}

} // namespace npsys