#pragma once

#include "element.h"
#include "line.h"
#include "sfc_block_visitor.hpp"
#include "cpp_deleter.h"

class CSFCLine;
class CSFCBlock;
class CSFCSlot;
class CSFCBlockComposition;

// CSFCBus
class CSFCBus 
	: public CLineBase 
	, public IRecreatable {
protected:
	float x_middle_ = 0.0f;
	float y_ = 0.0f;
	boost::container::small_vector<CSFCLine*, 4> lines_;
public:

	static constexpr auto c_hl = 3.0f;
	static constexpr auto c_extra_width = 10.0f;
	static constexpr auto c_offset_from_slot = 20.0f;

	virtual bool IsSerializable() const noexcept { return false; }
	void SetPosition(float x_middle, float y) noexcept { x_middle_ = x_middle; y_ = y; }
	size_t LinesCount() const noexcept { return lines_.size(); }
	void Clear() noexcept { lines_.clear(); }
	void Create(float x1, float x2);
	
	template<typename... T>
	void Connect(T*... lines);
	
	template<typename T>
	void GetLines(T& c) {
		std::copy(lines_.begin(), lines_.end(), std::back_inserter(c));
	}

	virtual CManipulator* GetManipulator() { return &g_default_manipulator; }
	virtual void CreateArrow(ID2D1GeometrySink* sink) = 0;
	virtual void CalcLinesPosition() = 0;
	CSFCLine* DisconnectLine(CSFCLine* line);

	// Recreatable
	virtual void Recreate() { CalcLinesPosition(); };
	virtual CElement* GetBase() { return this; };
};

class CSFCBusIn 
	: public CSFCBus {
public:
	DECLARE_TYPE_NAME("BUS_IN")
	DECLARE_VISITOR()

	virtual void CreateArrow(ID2D1GeometrySink* sink);
	virtual void CalcLinesPosition();
};

class CSFCBusOut
	: public CSFCBus {
public:
	DECLARE_TYPE_NAME("BUS_OUT")
	DECLARE_VISITOR()

	virtual void CreateArrow(ID2D1GeometrySink* sink);
	virtual void CalcLinesPosition();
};

// CSFCSlot
class CSFCSlot 
	: public CGeometryElement
	, public CLineConnectableT<CSFCLine> {
	friend class CSFCBlockFactory;
	friend boost::serialization::access;
	template<class Archive>
	void serialize(Archive& ar, const unsigned int file_version) {
		ar& boost::serialization::base_object<CGeometryElement>(*this);
		ar& id_;
	}
protected:
	class CSFCSlotManipulator : public CManipulator {
		CSFCSlot* target_ = nullptr;
		CSFCFindConnectable fs_;
		SlotDirection begin_at_;
		std::unique_ptr<CSFCLine, cpp_sfc_line_delete> line_;
	public:
		void SetTarget(CSFCSlot* target) { target_ = target; }

		virtual S_CURSOR MouseButtonDown(CPoint point, CAlgorithmView* pWnd);
		virtual int Drag(CPoint point, CAlgorithmView* pWnd);
		virtual void MouseButtonUP(CPoint point, CAlgorithmView* pWnd);
		virtual void Draw(CGraphics* pGraphics);
	};

	inline static CSFCSlotManipulator manipulator_;
	int id_;

	CSFCBlock* parent_ = nullptr;
	CSFCBus* bus_ = nullptr;
	CSFCLine* line_ = nullptr;

	virtual	void CreateRgnShape() noexcept {}
	virtual void UpdateLinePosition() = 0;
	virtual void UpdateBusPosition() = 0;
	virtual	void UpdateLineOrBus() noexcept = 0;
	virtual void DisconnectBus(CSFCBlockComposition* blocks) = 0;
public:
	void SetParent(CSFCBlock* parent) noexcept { parent_ = parent; }
	bool IsBusConnected() const noexcept { return bus_ != nullptr; }
	
	auto GetId() const noexcept { return id_; }
	auto GetParentBlock() const noexcept { ASSERT(parent_); return parent_; }
	auto GetLine() noexcept { return line_; }
	CSFCBus* GetBus() const noexcept { return bus_; }
	bool IsConnected() const noexcept { return line_ != nullptr; }
	void DisconnectLine(CSFCLine* line, CSFCBlockComposition* blocks, CElement::MyQuadrantNode* qtRoot);

	virtual SlotDirection GetDirection() const = 0;
	virtual CManipulator* GetManipulator() { 
		manipulator_.SetTarget(this);
		return &manipulator_;
	}

	virtual	void Move(float x, float y);
	virtual	void MoveDxDy(float dx, float dy);

	virtual bool HitTestWhileConnectingLine(const D2D1_POINT_2F& pt, UINT& flags) noexcept {
		return CGeometryElement::HitTest(pt, flags);
	}

	virtual NPSystemCursor GetCursor() final { return NPSystemCursor::Pen; };
	virtual void Draw(CGraphics* graphics);
	virtual void PrintDebugTooltip(std::stringstream& ss);

	template<typename T>
	void ConnectLine(CSFCLine* l1, CSFCBlockComposition* blocks);

	virtual void CollectGeometries(std::unordered_set<IRecreatable*>& lines) = 0;

	CSFCSlot();
	CSFCSlot(int id);
};

// CSFCInputSlot

class CSFCInputSlot : public CSFCSlot {
	friend boost::serialization::access;
	template<class Archive>
	void serialize(Archive& ar, const unsigned int file_version) {
		ar& boost::serialization::base_object<CSFCSlot>(*this);
	}
protected:
	virtual	void UpdateLineOrBus() noexcept;
public:
	DECLARE_TYPE_NAME("SFC_INPUT_SLOT")
	DECLARE_VISITOR()

	virtual void UpdateLinePosition();
	virtual void UpdateBusPosition();

	virtual bool ConnectToLine(CSFCLine* line, bool checking);
	virtual SlotDirection GetDirection() const { return SlotDirection::INPUT_SLOT; }
	virtual void DisconnectBus(CSFCBlockComposition* blocks);

	virtual void CollectGeometries(std::unordered_set<IRecreatable*>& lines);

	CSFCInputSlot() = default;
	CSFCInputSlot(int id) : CSFCSlot(id) {}
};

// CSFCOutputSlot

class CSFCOutputSlot : public CSFCSlot {
	friend boost::serialization::access;
	template<class Archive>
	void serialize(Archive& ar, const unsigned int file_version) {
		ar& boost::serialization::base_object<CSFCSlot>(*this);
	}
protected:
	virtual	void UpdateLineOrBus() noexcept;
public:
	DECLARE_TYPE_NAME("SFC_OUTPUT_SLOT")
	DECLARE_VISITOR()

	virtual void UpdateLinePosition();
	virtual void UpdateBusPosition();
	virtual void DisconnectBus(CSFCBlockComposition* blocks);

	virtual bool ConnectToLine(CSFCLine* line, bool checking);
	virtual SlotDirection GetDirection() const { return SlotDirection::OUTPUT_SLOT; }

	virtual void CollectGeometries(std::unordered_set<IRecreatable*>& lines);

	CSFCOutputSlot() = default;
	CSFCOutputSlot(int id) : CSFCSlot(id) {}
};

// CSFCLine
class CSFCLine 
	: public CLineT<CSFCSlot, CSFCInputSlot, CSFCOutputSlot, CSFCLine>
	, public IRecreatable {
	using inherited = CLineT<CSFCSlot, CSFCInputSlot, CSFCOutputSlot, CSFCLine>;
	friend boost::serialization::access;
	template<class Archive>
	void serialize(Archive& ar, const unsigned int file_version) {
		ar& boost::serialization::base_object<inherited>(*this);
	}
	static constexpr auto c_line_r = 4.0f;
public:
	DECLARE_VISITOR()
	DECLARE_TYPE_NAME("SFC_LINE");

	virtual void ConnectSlots(bool recreate_dependencies, CBlockCompositionBase* blocks, CElement::MyQuadrantNode* qtRoot = nullptr) final;
	virtual void Disconnect(CBlockCompositionBase* blocks, CElement::MyQuadrantNode* qtRoot) final;

	void CreatePredraw();
	void CreateSingle();
	void CreateForBus();

	// Recreatable
	virtual void Recreate();
	virtual CElement* GetBase() { return this; };
};

class CSFCBlock
	: public CGroup
	, public CGraphElement
	, public Properties
	, public CLineConnectableT<CSFCLine>
	, public IConnectableToLine
{
	friend class CSFCBlockFactory;
	friend boost::serialization::access;
	template<class Archive>
	void serialize(Archive& ar, const unsigned int file_version) {
		ar& boost::serialization::base_object<CGroup>(*this);
		ar& prop_list_;
	}

	class CSFCBlockManipulator : public CElementManipulator {
	public:
		virtual int Drag(CPoint point, CAlgorithmView* pWnd);
		virtual void MouseButtonUP(CPoint point, CAlgorithmView* pWnd);
		virtual void Draw(CGraphics* pGraphics);

		CSFCBlockManipulator() : CElementManipulator(nullptr) {}
	private:
		std::unordered_set<IRecreatable*> geometries_;
	};

	inline static CSFCBlockManipulator manipulator_;
protected:
	virtual void CreateInplaceProperties() noexcept {}

	CSFCBlock() {
		SetHitAccess();
		SetResizeable(true);
	} 
public:
	virtual CManipulator* GetManipulator() {
		manipulator_.SetTarget(this);
		return &manipulator_;
	}
	// CLineConnectableT<CSFCLine>
	virtual bool HitTestWhileConnectingLine(const D2D1_POINT_2F& pt, UINT& flags) noexcept final {
		return CSFCBlock::HitTest(pt, flags);
	}
	// CElement
	virtual void Draw(CGraphics* pGraphics);
	virtual NPSystemCursor GetCursor() final { return IsSelected() ? NPSystemCursor::Drag : NPSystemCursor::Arrow; }
	virtual std::unique_ptr<Command_MoveCommand> CreateMoveCommand(const D2D1_POINT_2F& delta, CElement::MyQuadrantNode* qtRoot);
	virtual std::unique_ptr<Command> DeleteElement(CBlockComposition* blocks, MyQuadrantNode* qtRoot);

	void CollectLines(CSFCSlot* slot, boost::container::small_vector<CSFCLine*, 8>& lines);

	virtual boost::container::small_vector<CSFCLine*, 8> GetLines() = 0;

	virtual void DeletePermanent();

//	virtual void Translate(CCodeGenerator* pGenerator) = 0;
//	virtual Command* DeleteElement(CFBDBlockComposition* pBlocks, CElement::MyQuadrantNode* qtRoot);
//	virtual NPSystemCursor GetCursor() final { return IsSelected() ? NPSystemCursor::Drag : NPSystemCursor::Arrow; }
//	virtual NPSystemCursor GetOnlineCursor() { return NPSystemCursor::LinkSelect; }
//	virtual void Check(CBlockVisitor& v) {}
//	virtual void InsertToWrapper(CBlockCompositionWrapper* wrapper) noexcept final;
//	virtual	int	GetPriority() { return 0; }
//	virtual void DeletePermanent();
//	virtual	Command_MoveCommand* CreateMoveCommand(const D2D1_POINT_2F& delta, CElement::MyQuadrantNode* qtRoot);
//	virtual CManipulator* GetManipulator() { return &manipulator_; }
//	virtual const std::string& GetName() const noexcept final { return top->get_name(); }
//	virtual void SetName(const std::string& name) noexcept;
//	virtual bool IsNode() const noexcept final { return true; }
//	virtual void PrintDebugTooltip(std::stringstream& ss);
//	virtual void FillPropertyGrid(CPropertyGrid* pPropertyGrid) noexcept;
//	virtual void OnPropertyChanged(WPARAM wParam, LPARAM lParam) noexcept;
//	void Expand(int nCount);
//	void UpdateBezierLines();
//	void Hardware_InitInternal(CSlot::SlotDirection slot_dir, size_t slot_index, 
//		int var_type, uint16_t addr, odb::weak_node<npsys::device_n> dev);
//	size_t InputHasValidVariable() noexcept;
//	std::vector<CLine*> GetLines() const;
//	void CollectLines(std::vector<CLine*>& lines) const;
//	void SetExecuteOrder(int execute_order) { execute_order_ = execute_order; }
//	int GetExecuteOrder() const noexcept { return execute_order_; }
//	void MountSlot(npsys::fbd_slot_n&& slot);
//	CTextElement* GetTextName() { return static_cast<CTextElement*>(elements_[0]); }
//	static constexpr size_t CustomElementIdxBegin() { return 2; }
//	CElement* FirstCustomElement() noexcept {
//		ASSERT(CustomElementIdxBegin() < elements_.size());
//		return elements_[CustomElementIdxBegin()];
//	}
//
//	void UpdateSize();
//
//	void OnBlockAddedToEditor() noexcept {
//		for (auto& slot : top->slots) slot->e_slot->OnSlotAddedToEditor();
//	};
//
//	void OnBlockDeletedFromEditor() noexcept {
//		for (auto& slot : top->slots) slot->e_slot->OnSlotDeletedFromEditor();
//	};
};

class CSFCBlock2 
	: public CSFCBlock {
	friend class CSFCBlockFactory;
	friend boost::serialization::access;
	
	template<class Archive>
	void save(Archive& ar, const unsigned int /*file_version*/) const {}
	
	template<class Archive>
	void load(Archive& ar, const unsigned int /*file_version*/) {
		GetInputSlot()->SetParent(this);
		GetOutputSlot()->SetParent(this);
	}
	
	template<class Archive>
	void serialize(Archive& ar, const unsigned int file_version) {
		ar& boost::serialization::base_object<CSFCBlock>(*this);
		boost::serialization::split_member(ar, *this, file_version);
	}
public:
	CTextElement* GetTextName() noexcept { return static_cast<CTextElement*>(elements_[0]); }
	const CTextElement* GetTextName() const noexcept { return static_cast<const CTextElement*>(elements_[0]); }

	virtual void SetName(const std::string& name) noexcept;
	virtual const std::string& GetName() const noexcept;

	auto GetInputSlot() noexcept {
		ASSERT(dynamic_cast<CSFCInputSlot*>(elements_[1]) != nullptr);
		return static_cast<CSFCInputSlot*>(elements_[1]);
	}

	auto GetOutputSlot() noexcept {
		ASSERT(dynamic_cast<CSFCOutputSlot*>(elements_[2]) != nullptr);
		return static_cast<CSFCOutputSlot*>(elements_[2]);
	}

	// IConnectableToLine
	virtual void CollectAffectedGeometries(std::unordered_set<IRecreatable*>& lines) {
		GetInputSlot()->CollectGeometries(lines);
		GetOutputSlot()->CollectGeometries(lines);
	}

	virtual boost::container::small_vector<CSFCLine*, 8> GetLines() {
		boost::container::small_vector<CSFCLine*, 8> lines;
		CollectLines(GetInputSlot(), lines);
		CollectLines(GetOutputSlot(), lines);
		return lines;
	}

	virtual bool ConnectToLine(CSFCLine* line, bool checking) final;
};

class CSFCBlockBegin
	: public CSFCBlock {
	friend class CSFCBlockFactory;
	friend boost::serialization::access;

	template<class Archive>
	void save(Archive& ar, const unsigned int /*file_version*/) const {}
	
	template<class Archive>
	void load(Archive& ar, const unsigned int /*file_version*/) {
		GetOutputSlot()->SetParent(this);
	}

	template<class Archive>
	void serialize(Archive& ar, const unsigned int file_version) {
		ar& boost::serialization::base_object<CSFCBlock>(*this);
		boost::serialization::split_member(ar, *this, file_version);
	}
protected:
	virtual	void CreateRgnShape() noexcept;
public:
	DECLARE_TYPE_NAME("BEGIN")
	DECLARE_VISITOR()
	DECLARE_TRANSLATE_SFC()

	auto GetOutputSlot() noexcept {
		ASSERT(dynamic_cast<CSFCOutputSlot*>(elements_[1]) != nullptr);
		return static_cast<CSFCOutputSlot*>(elements_[1]);
	}

	virtual bool ConnectToLine(CSFCLine* line, bool checking) final;

	// IConnectableToLine
	virtual void CollectAffectedGeometries(std::unordered_set<IRecreatable*>& lines) {
		GetOutputSlot()->CollectGeometries(lines);
	}

	virtual boost::container::small_vector<CSFCLine*, 8> GetLines() {
		boost::container::small_vector<CSFCLine*, 8> lines;
		CollectLines(GetOutputSlot(), lines);
		return lines;
	}
};

class CSFCBlockTerm
	: public CSFCBlock {
	friend class CSFCBlockFactory;
	friend boost::serialization::access;

	template<class Archive>
	void save(Archive& ar, const unsigned int /*file_version*/) const {}
	
	template<class Archive>
	void load(Archive& ar, const unsigned int /*file_version*/) {
		GetInputSlot()->SetParent(this);
	}

	template<class Archive>
	void serialize(Archive& ar, const unsigned int file_version) {
		ar& boost::serialization::base_object<CSFCBlock>(*this);
		boost::serialization::split_member(ar, *this, file_version);
	}
protected:
	virtual	void CreateRgnShape() noexcept;
public:
	DECLARE_TYPE_NAME("TERM")
	DECLARE_VISITOR()
	DECLARE_TRANSLATE_SFC()

	auto GetInputSlot() noexcept {
		ASSERT(dynamic_cast<CSFCInputSlot*>(elements_[0]) != nullptr);
		return static_cast<CSFCInputSlot*>(elements_[0]);
	}

	virtual bool ConnectToLine(CSFCLine* line, bool checking) final;

	// IConnectableToLine
	virtual void CollectAffectedGeometries(std::unordered_set<IRecreatable*>& lines) {
		GetInputSlot()->CollectGeometries(lines);
	}

	virtual boost::container::small_vector<CSFCLine*, 8> GetLines() {
		boost::container::small_vector<CSFCLine*, 8> lines;
		CollectLines(GetInputSlot(), lines);
		return lines;
	}
};


class CSFCBlockStep 
	: public CSFCBlock2 {
	friend class CSFCBlockFactory;
	friend class CTypeDeterminant;
	friend class CTACGenerator;
	friend class CAVR5CodeGenerator;
	friend boost::serialization::access;
	template<class Archive>
	void serialize(Archive& ar, const unsigned int file_version) {
		ar& boost::serialization::base_object<CSFCBlock2>(*this);
	}
	CSFCBlockStep() = default;
protected:
	virtual	void CreateRgnShape() noexcept;
public:
	DECLARE_TYPE_NAME("STEP")
	DECLARE_VISITOR()
	DECLARE_TRANSLATE_SFC()

	virtual bool SetFarCorner(float x, float y);
};

class CSFCBlockTransition 
	: public CSFCBlock2 {
	friend class CSFCBlockFactory;
	friend class CTypeDeterminant;
	friend class CTACGenerator;
	friend class CAVR5CodeGenerator;
	friend boost::serialization::access;
	template<class Archive>
	void serialize(Archive& ar, const unsigned int file_version) {
		ar& boost::serialization::base_object<CSFCBlock2>(*this);
	}
	CSFCBlockTransition() = default;
protected:
	virtual	void CreateRgnShape() noexcept;
public:
	DECLARE_TYPE_NAME("TRANSITION")
	DECLARE_VISITOR()
	DECLARE_TRANSLATE_SFC()
};