// Copyright (c) 2021 nikitapnn1@gmail.com
// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory

#pragma once

#include <npsys/header.h>
#include <nplib/utils/variant.h>
#include "color.h"
#include "stuff.h"
#include "quadrant.h"
#include "global.h"
#include "manipulator.h"
#include "blockpropertyid.h"
#include <bitset>
#include "serialization_atl_string.h"
#include "propertygrid.h"
#include "constants.h"
#include "command_i.hpp"

#define DECLARE_TYPE_NAME(type_name) \
	virtual std::string_view GetTypeName() const noexcept { return type_name; } 

#define DECLARE_TRANSLATE() \
	virtual void Translate(class CCodeGenerator*);

#define IMPLEMENT_TRANSLATE(class_name) \
void class_name ## ::Translate(CCodeGenerator* code_generator) { \
	code_generator->Generate(this); \
}

#define DECLARE_TRANSLATE_SFC() \
	virtual void SFC_Translate(class CSFCCodeGenerator*);

#define IMPLEMENT_TRANSLATE_SFC(class_name) \
void class_name ## ::SFC_Translate(CSFCCodeGenerator* code_generator) { \
	code_generator->Generate(this); \
}

#define DECLARE_VISITOR() \
public:	\
	virtual void Visit(CBlockVisitor& v);

#define IMPLEMENT_VISITOR(ClassName) \
	void ClassName ## ::Visit(CBlockVisitor& v) { v.Accept(this); }

class CGroup;
class Command_MoveCommand;
class CGraphics;
class CBlockComposition;
class CBlockCompositionWrapper;
class CPropertyGrid;
class CSizeElement;

namespace npsys {
class CFBDControlUnit;
}

enum class SlotDirection {
	INPUT_SLOT,
	OUTPUT_SLOT
};

#define EL_INVALIDATE 1
#define VALUE_CHANGED 2

class CElement {
public:
	friend QuadrantAdapter<CElement, float>;
	typedef QuadrantNode<CElement, float, 60> MyQuadrantNode;
	typedef Iter::Iterator<CElement*> _Iterator;
private:
	friend boost::serialization::access;
	template<class Archive>
	void serialize(Archive & ar, const unsigned int file_version) {
		ar & z_order_;
		ar & visible_;
		ar & rect_.left;
		ar & rect_.top;
		ar & rect_.right;
		ar & rect_.bottom;

		if constexpr (Archive::is_loading::value) {
			if constexpr (is_special_archive_v<Archive> == false) {
				flags_.set(created_new, false);
			}
		} else {
			flags_.set(created_new, false);
		}
	}

	static const std::string emptyString_;
	// Serializable
	int	z_order_;
	bool visible_;
	// Not Serializable
	std::bitset<8> flags_;
	enum { hovered = 0, selected, edit_mode, hit_access, resizeable, created_new, geometry_realization_dirty, geometry_dirty };
	int render_frame_;
	int last_draw_context_id_ = -1;
protected:
	D2D1_RECT_F rect_;

	auto IsGeometryRealizationDirty(int context_id) { 
		return flags_.test(geometry_realization_dirty) || last_draw_context_id_ != context_id;
	}

	void SetGeometryRealizationFlagDirty() { 
		flags_.set(geometry_realization_dirty, true); 
	}

	void SetGeometryRealizationFlagAfterDraw(int draw_context_id) { 
		flags_.set(geometry_realization_dirty, false); 
		last_draw_context_id_ = draw_context_id; 
	}

	auto IsGeometryDirty() const noexcept { 
		return flags_.test(geometry_dirty);
	}

	void SetGeometryFlag(bool value) noexcept { 
		flags_.set(geometry_dirty, value); 
	}
public:
	virtual std::string_view GetTypeName() const noexcept = 0;
	virtual void Draw(CGraphics* graphics) = 0;
	virtual void PrintDebugTooltip(std::stringstream& ss) {}
	virtual bool HitTest(const D2D1_POINT_2F& pt, UINT& flags);
	virtual void Visit(CBlockVisitor& v) = 0;
	virtual CManipulator* GetManipulator() { return 
		&g_default_manipulator; };
	virtual void FillPropertyGrid(CPropertyGrid* pPropertyGrid) noexcept;
	virtual void OnPropertyChanged(WPARAM wParam, LPARAM lParam) noexcept;
	virtual bool IsSerializable() const noexcept { return true; }
	virtual	_Iterator* CreateIterator();
	void GetRect(D2D1_RECT_F& rc) const noexcept { rc = rect_; }
	void SetRect(const D2D1_RECT_F& rc) noexcept {  rect_ = rc; }
	void SetRect(float x, float y, float width, float height) noexcept { 
		rect_ = D2D1::RectF(x, y, x + width, y + height); 
	}
	
	virtual bool SetFarCorner(float x, float y) {
		bool succeded = false;
		if (x - rect_.left > constants::block::GRIDSZ_F * 4.0f) {
			rect_.right = x;
			succeded = true;
		}

		if (y - rect_.top > constants::block::GRIDSZ_F * 4.0f) {
			rect_.bottom = y;
			succeded = true;
		}

		return succeded;
	}

	virtual	void Move(float x, float y);
	virtual	void MoveDxDy(float dx, float dy);
	virtual bool ShowPropertiesDialog(CBlockComposition* blocks, npsys::control_unit_n& unit) { return false; }
	virtual bool ShowOnlineDialog(npsys::CControlUnit* unit) { return false; }
	virtual std::unique_ptr<Command> DeleteElement(CBlockComposition* pBlocks, MyQuadrantNode* qtRoot) { ASSERT(FALSE); return NULL; }
	virtual NPSystemCursor GetCursor() { return NPSystemCursor::Arrow; }
	virtual NPSystemCursor GetOnlineCursor() { return NPSystemCursor::Arrow; }
	virtual const std::string& GetName() const noexcept { return emptyString_; }
	virtual void Transform(const D2D1::Matrix3x2F& matrix) {}
	virtual void InsertToWrapper(CBlockCompositionWrapper* wrapper) noexcept;

	virtual CMenuHandle GetContextMenu() { 
		CMenu menu;
		menu.CreatePopupMenu();
		
		menu.AppendMenu(MF_STRING, ID_EDIT_CUT, L"Cut");
		menu.AppendMenu(MF_STRING, ID_EDIT_COPY, L"Copy");
		menu.AppendMenu(MF_SEPARATOR);
		menu.AppendMenu(MF_STRING, ID_BLOCK_PROPERTIES, L"Properties");
	
		return menu.Detach();
	}

	virtual	std::unique_ptr<Command_MoveCommand> CreateMoveCommand(const D2D1_POINT_2F& delta, CElement::MyQuadrantNode* qtRoot);
	virtual bool IsNode() const noexcept { return false; }
	virtual void SetName(const std::string&) noexcept { ASSERT(FALSE); }


	bool IsSerializedByComposition() { return !IsNode() && IsSerializable(); }

	auto IsHover() const noexcept { return flags_.test(hovered); }
	void SetHover(BOOL hover) noexcept { flags_.set(hovered, hover); }
	
	auto IsResizeable() const noexcept { return flags_.test(resizeable); }
	void SetResizeable(bool _resizeable) noexcept { flags_.set(resizeable, _resizeable); }
	
	auto IsSelected() const noexcept { return flags_.test(selected); }
	void SetSelect(BOOL select) noexcept { flags_.set(selected, select); }

	auto IsEditState() const { return flags_.test(edit_mode); }
	void BeginEditState() { flags_.set(edit_mode, true);}
	void EndEditState() {  flags_.set(edit_mode, false); }
	auto IsHitAccess() { return flags_.test(hit_access); }
	void SetHitAccess() { flags_.set(hit_access, true); }
	void ResetHitAccess() { flags_.set(hit_access, false); }
	auto IsCreatedNew() { return flags_.test(created_new); }
	void SetZOrder(int z_order) { z_order_ = z_order; }
	int GetZOrder() { return z_order_; }
	bool IsVisible() const noexcept { return visible_; }
	void ShowElement(bool show) noexcept { visible_ = show; }

	const std::string GetTypeNameStr() const noexcept {
		auto str = GetTypeName(); 
		return std::string(str); 
	}
	float Width() const noexcept { return rect_.right - rect_.left; }
	const D2D1_RECT_F& GetBounds() { return rect_; }

	const D2D1_POINT_2F&	GetPosition() {
		return reinterpret_cast<D2D1_POINT_2F&>(rect_);
	}

	const D2D1_POINT_2F& GetCorner0() {
		return reinterpret_cast<D2D1_POINT_2F&>(rect_);
	}

	const D2D1_POINT_2F& GetCorner1() {
		return reinterpret_cast<D2D1_POINT_2F&>(rect_.right);
	}

	D2D1_POINT_2F GetMiddle() {
		return D2D1::Point2F(
			rect_.left + (rect_.right - rect_.left) / 2,
			rect_.top + (rect_.bottom - rect_.top) / 2);
	}

	D2D1::MySize2F GetSize() {
		return D2D1::MySize2F(
			rect_.right - rect_.left,
			rect_.bottom - rect_.top);
	}

	BOOL RectInRect(const D2D_RECT_F& rect) const {
		if ((rect_.left < rect.left) || (rect_.right > rect.right)) return FALSE;
		if ((rect_.top < rect.top) || (rect_.bottom > rect.bottom)) return FALSE;
		return TRUE;
	}

	CElement() {
		z_order_ = 1; // 0 for lines
		visible_ = true;
		rect_ = D2D1::RectF();
		render_frame_ = 0;
		flags_.set(created_new, true);
		flags_.set(geometry_dirty, true);
	}

	CElement(const CElement& el) {
		z_order_ = el.z_order_;
		visible_ = el.visible_;
		rect_ = el.rect_;
		flags_.set(hit_access, el.flags_[hit_access]);
		flags_.set(created_new, true);
		flags_.set(geometry_dirty, true);
		render_frame_ = 0;
	}

	virtual ~CElement() = default;
};


template<>
struct QuadrantAdapter<CElement, float> {
	typedef TQuadrantPoint<float> QuadrantPoint;
	typedef TQuadrantRect<float> QuadrantRect;

	inline static const QuadrantPoint& GetPosition(CElement* pElem) {
		return reinterpret_cast<QuadrantPoint&>(pElem->rect_);
	}
	inline static const QuadrantRect& GetBounds(CElement* pElem) {
		return reinterpret_cast<QuadrantRect&>(pElem->rect_);
	}
	inline static bool HitTest(CElement* pElem, float x, float y, UINT& flags) {
		return pElem->HitTest(D2D1::Point2F(x, y), flags);
	}
	inline static const QuadrantRect& GetVisibleArea(const D2D1_RECT_F& rect) {
		return reinterpret_cast<QuadrantRect&>(const_cast<D2D1_RECT_F&>(rect));
	}
	inline static const int IsRendered(CElement* pElem, int render_frame) {
		return pElem->render_frame_ == render_frame;
	}
	inline static const void SetRenderedState(CElement* pElem, int render_frame) {
		pElem->render_frame_ = render_frame;
	}
	inline static const int GetZOrder(CElement* pElem) {
		return pElem->z_order_;
	}
	inline static const int IsHitAccess(CElement* pElem) {
		return pElem->IsHitAccess();
	}
	inline static void Draw(CElement* elem, CGraphics* graphics) {
		return elem->Draw(graphics);
	}
};

inline void RemoveFromQuadrant(CElement* pElem, CElement::MyQuadrantNode* qtRoot) {
	Iter::PostorderIterator<CElement*> it(pElem);
	for (; !it.IsDone(); it.Next()) {
		qtRoot->DeleteElement(*it);
	}
	qtRoot->DeleteElement(pElem);
}

inline void PasteToQuadrant(CElement* pElem, CElement::MyQuadrantNode* qtRoot) {
	Iter::PostorderIterator<CElement*> it(pElem);
	for (; !it.IsDone(); it.Next()) {
		qtRoot->AddElement(*it);
	}
	qtRoot->AddElement(pElem);
}

class CElementManipulator : public CManipulator {
	
protected:
	bool was_draged_;
	CElement* element_;
	D2D1::MyPoint2F _offset; // distance between pointer click and top-left corner of the element
	D2D1::MyPoint2F _oldpos;
	CPoint _oldScreenCursor;
public:
	virtual S_CURSOR MouseButtonDown(CPoint point, CAlgorithmView* pWnd);

	// 0 drag no movement
	// 1 drag started
	// 2 drag
	virtual int Drag(CPoint point, CAlgorithmView* pWnd);
	virtual void MouseButtonUP(CPoint point, CAlgorithmView* pWnd);
	virtual void Draw(CGraphics* pGraphics);
	void SetTarget(CElement* target) { element_ = target; }

	CElementManipulator(CElement* element = nullptr) 
		: element_(element) 
		, was_draged_{false}
	{
	}
};

class CSliderManipulator : public CElementManipulator {
	CGroup* slider_;
public:
	virtual int Drag(CPoint point, CAlgorithmView* pWnd);
	void AfterDeserialization(CGroup* slider) { slider_ = slider; }

	CSliderManipulator(CElement* element, CGroup* slider) 
		: CElementManipulator(element) 
		, slider_(slider)
	{
	}
};

class CGraphElement {
	int	tmp_id_;
public:
	void SetTmpId(int id) noexcept { tmp_id_ = id; }
	int GetTmpId() const noexcept { return tmp_id_; }
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
	virtual bool SetFarCorner(float x, float y) {
		auto size_before = GetSize();
		auto middle = GetMiddle();
		bool succeded = CElement::SetFarCorner(x, y);
		if (succeded) {
			auto size_after = GetSize();
			m_matrixTransform = m_matrixTransform *
				D2D1::Matrix3x2F::Scale(
					{size_after.width / size_before.width, size_after.height / size_before.height}
					, middle
				) * D2D1::Matrix3x2F::Translation({
					(size_after.width - size_before.width) / 2.0f,
					(size_after.height - size_before.height) / 2.0f
					});
			UpdateTransformedGeometry();
		}
		return succeded;
	}
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
private:
	template<class Archive>
	void save(Archive& ar, const unsigned int /*file_version*/) const {}

	template<class Archive>
	void load(Archive& ar, const unsigned int /*file_version*/) {
		SetName(text_);
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
	virtual void OnTextChanged() noexcept;
	void UpdatePosition() noexcept;

	int m_align;
	FONT_SIZE_CENTERED font_size_ = FONT_SIZE_CENTERED::SIZE_12;
	std::string text_;
	wrl::ComPtr<IDWriteTextLayout> textLayout_;
	CElementManipulator _manipulator;
public:
	DECLARE_TYPE_NAME("TEXT")
	DECLARE_VISITOR()

	virtual void Draw(CGraphics* pGraphics);
	virtual CManipulator* GetManipulator() { return &_manipulator; }
	virtual const std::string& GetName() const noexcept { return text_; }
	const std::string& GetText() const noexcept { return text_; }

	void SetPosition(float x, float y, int align);
	virtual void SetName(const std::string& name) noexcept {
		text_ = name;
		OnTextChanged();
	}

	CTextElement();
	CTextElement(std::string&& text, float x, float y, int align = (T_CENTER | T_VCENTER), FONT_SIZE_CENTERED font_size = FONT_SIZE_CENTERED::SIZE_12);
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
	virtual void OnTextChanged() noexcept;
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

class Properties {
protected:
	std::vector<std::unique_ptr<BlockProperty>> prop_list_;
public:
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
};

class CSizeElement 
	: public CGeometryElement {
	class CSizeManipulator : public CElementManipulator {
	public:
		virtual int Drag(CPoint point, CAlgorithmView* pWnd);
		CSizeManipulator(CElement* element)
			: CElementManipulator(element)
		{
		}
	};
	CElement* target_;
	CSizeManipulator manipulator_;
protected:
	virtual	void CreateRgnShape() noexcept;
public:
	DECLARE_TYPE_NAME("SIZE_TOOL");
	
	CElement* GetTarget() {
		return target_;
	}
	
	void SetTarget(CElement* target);
	void UpdatePosition();

	virtual bool HitTest(const D2D1_POINT_2F& pt, UINT& flags) {
		return CElement::HitTest(pt, flags);
	}

	virtual void Visit(CBlockVisitor& v) {}
	virtual CManipulator* GetManipulator() { return &manipulator_; }

	CSizeElement();
};