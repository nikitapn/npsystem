
// Assigned

// CAssignedModulesCategory
CAssignedModulesCategory::CAssignedModulesCategory(CContainer* parent, const CString& name, int dev_id) :
	CTemplateItemHardwareStatus(parent, ASSIGN_MODULES_ID + dev_id, name, AlphaIcon::Modules, dev_id) {
	LoadChilds();
}
void CAssignedModulesCategory::HandleRequest(REQUEST* req) noexcept {
	switch (req->ID_MENU) {
	case ID_UPLOAD_FIRMWARE:
		__super::HandleRequest(req);
		SetStatus(io::Status::relevant);
		break;
	default:
		__super::HandleRequest(req);
	};
}
// CAssignedModule

CAssignedModule::CAssignedModule(CContainer* parent, CModule* pModule, int dev_id) :
	CTemplateItemHardwareStatus(parent, db->sequence_next_val<tb::tree_sequence>(), pModule->GetName()) {
	m_dev_id = dev_id;
	m_idObject = pModule->GetId();

	tb::ass_module ass = {};
	
	ass.id = GetId();
	ass.dev_id = dev_id;
	ass.mod_id = pModule->GetId();

	db->insert<tb::ass_module>(ass);

	for (auto& i : pModule->m_items)
		m_items.push_back(new CAssignedSegment(i, this));
	
	SaveChilds();

	m_properties.reset(new AssignedModuleBlob(*pModule->m_properties.get()));
	m_properties.save(db.get(), GetId());
}

CAssignedModule::CAssignedModule(CContainer* parent, int id, const string& name, int dev_id, int mod_id, int status) :
	CTemplateItemHardwareStatus(parent, id, to_utf16(name)) {
	m_properties.load(db.get(), GetId());
	LoadChilds();
}

void CAssignedModule::ConstructMenu(CMenu& menu) noexcept {
	menu.AppendMenu(MF_STRING, ID_UPLOAD_MODULE, _T("Upload to device"));
	menu.AppendMenu(MF_STRING, ID_REMOVE_MODULE, _T("Remove from device"));
	menu.AppendMenu(MF_SEPARATOR);
	__super::ConstructMenu(menu);
	if (GetStatus() > io::Status::assigned) {
		menu.EnableMenuItem(ID_ITEM_DELETE, false);
	}
}
void CAssignedModule::HandleRequest(REQUEST* req) noexcept {
	switch (req->ID_MENU) {
	case ID_UPLOAD_FIRMWARE:
		__super::HandleRequest(req);
		SetStatus(io::Status::relevant);
		break;
	case ID_UPLOAD_MODULE:
		Upload();
		break;
	case ID_REMOVE_MODULE:
		Unload();
		break;
	default:
		__super::HandleRequest(req);
	};
}
CString CAssignedModule::GetTitle() {
	return GetParent()->GetTitle() + _T(":") + GetName();
}
int CAssignedModule::Upload() {
	CDevice* pDevice = GetDevice(this);
	CAVR5DynamicLinker linker((CAVRController*)pDevice);
	return linker.UploadModule(this);
}
int CAssignedModule::Unload() {
	CDevice* pDevice = GetDevice(this);
	CAVR5DynamicLinker linker((CAVRController*)pDevice);
	return linker.UnloadModule(this);
}

// CAssignedSegment

BOOST_CLASS_EXPORT_GUID(CAssignedSegment, "CAssignedSegment")

CAssignedSegment::CAssignedSegment(const CSegment* other, CContainer* parent) {
	SetId(db->sequence_next_val<tb::tree_sequence>());
	SetParent(parent);
	SetName(other->GetName());
	icon_ = other->icon_;
	m_offset = other->m_offset;
	m_type = other->m_type;
	for (auto i : other->m_items) {
		m_items.push_back(new CAssignedSegmentValue(i, this));
	}
}
CSegmentValue* CAssignedSegment::CreateChild(CContainer* parent, const CString& name, int id, uint16_t offset) {
	return new CAssignedSegmentValue(parent, id, name, offset);
}
void CAssignedSegment::ConstructMenu(CMenu& menu) noexcept {
	menu.AppendMenu(MF_STRING, ID_SHOW_ONLINE_LIST, _T("Show online values"));
	__super::ConstructMenu(menu);
}
void CAssignedSegment::HandleRequest(REQUEST* req) noexcept {
	switch (req->ID_MENU) {
	case ID_SHOW_ONLINE_LIST:
		ShowOnlineList();
		break;
	default:
		__super::HandleRequest(req);
	};
}
void CAssignedSegment::ShowOnlineList() {
	/*
	std::vector<OnlineListVar> vars;
	Iter::IteratorPtr<CAssignedSegmentValue*> it(this);
	for (; !it->IsDone(); it->Next()) {
		CAssignedSegmentValue* pValue = *it;
		vars.resize(vars.size() + 1);
		vars.back().name = pValue->GetName();
		vars.back().dev_addr = pValue->m_variable.dev_addr;
		vars.back().addr = pValue->m_variable.addr;
		vars.back().size = pValue->m_variable.GetSize();
		vars.back().var.vt = (OnlineListVar::TYPE)pValue->m_variable.defValue.vt;
	}
	Dlg_OnlineList dlg(vars);
	dlg.DoModal();
	*/
}
////////

BOOST_CLASS_EXPORT_GUID(CAssignedSegmentValue, "CAssignedSegmentValue")

CAssignedSegmentValue::CAssignedSegmentValue(const CSegmentValue* other, CContainer* parent) {
	SetId(db->sequence_next_val<tb::tree_sequence>());
	SetParent(parent);
	SetName(other->GetName());
	icon_ = other->icon_;
	m_var = other->m_var;
}

INT_PTR CAssignedSegmentValue::ShowProperties() {
	INT_PTR result = inherited::ShowProperties();
	if (result == IDOK) {
		SetStatus(io::Status::not_relevant);
		g_pSystem->m_pPNetwork->m_pControllerMgr->CalcAndUpdateStatus();
	}
	return result;
}