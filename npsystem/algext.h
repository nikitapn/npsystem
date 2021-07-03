#pragma once

#include <npsys/alg.h>
#include <npsys/fat_data_callback.h>
#include "blockcomposition.h"
#include "View.h"
#include "visitorblocks.h"

struct DrawStruct;
class CFindLoadedSlots;
class CBlockComposition;
class CCodeGenerator;
class CItemController;
class CSlot;
class CBlock;
class CGraphics;
class CAlgorithmFactory;
class CTreeAlgorithm;

namespace npsys {
class CAlgorithmExt 
	: public CAlgorithm
	, public IFatDataCallBack
	, public odb::self_node_member<algorithm_n>
	, public odb::modified_flag 
{
	CWindow window_;
	CFindLoadedSlots m_loadedSlots;
	bool online_ = false;
	bool saving_in_progress_ = false;
	int	m_running = 0;
	std::map<nps::var_handle, std::vector<CSlot*>> ref_;
protected:
	// CORBA
	virtual size_t AdviseImpl();
	virtual void OnDataChangedImpl(::flat::Span_ref<nps::flat::server_value, nps::flat::server_value_Direct> a);
	virtual void OnConnectionLost() noexcept;
public:
	CTreeAlgorithm* tree_algorithm = nullptr;
	ActionList actions;

	virtual void CloseView();

	void DrawOnlineValues(CGraphics* pGraphics);
	void UpdateData(std::vector<nps::server_value>* upd);
	//
	void Translate(CCodeGenerator*);
	void Separate(std::vector<CBlock*>*, CBlock*, char*);
	int CalcOrder(CBlock*, int, char*);
	int TryExecuteNext(CBlock*, int, char*);
	//
	void UploadToDevice(bool mb_confirm = false) noexcept;
	void UnloadFromDevice() noexcept;
	CBlockComposition* GetBlocks();
	//						
	bool IsOnline() const noexcept { return online_; }
	void StartOnline();
	void StopOnline();
	void OnlineRestart();
	//
	void Save(bool after_uploading = false) noexcept;
	bool IsUndoRedo();
	void SetWindow(CWindow window) noexcept { window_ = window; }
	void Unload();
	void ClearVariables() noexcept;
	void HardwareSpecific_Clear();
	void DetermineTypes(npsys::algorithm_n& self);
	std::optional<npsys::fbd_slot_n> FindSlot(const std::string& block_name, const std::string& slot_name);
	std::optional<npsys::fbd_slot_n> FindSlot(int slotId);
	void DeletePermanent() noexcept;
#ifdef DEBUG
	void ShowBlocks();
#endif // DEBUG

	CAlgorithmExt() = default;
	CAlgorithmExt(std::string&& name, npsys::algorithm_category_n& cat);
};

} // namespace npsys

BOOST_CLASS_VERSION(npsys::CAlgorithmExt, npsys::CAlgorithm::_BOOST_CLASS_VERSION);