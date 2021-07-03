
class AssignedModuleBlob : public ModuleBlob
{
	friend boost::serialization::access;
	template<class Archive>
	void serialize(Archive & ar, const unsigned int file_version) {
		ar & boost::serialization::base_object<ModuleBlob>(*this);
		ar & lrr;
	}
public:
	AssignedModuleBlob() {}
	AssignedModuleBlob(ModuleBlob& other) {
		slaveAddr = other.slaveAddr;
		addrSize = other.addrSize;
	}
	std::vector<twi_req_t> lrr; // loaded relevant requests
};

class CAssignedModule;
class CAssignedSegment;
class CAssignedSegmentValue;
// CAssignedModulesCategory
class CAssignedModulesCategory : public CTemplateItemHardwareStatus<
	ItemListContainer<std::list, CAssignedModule, FixedElement, TableChilds> >
{
public:
	CAssignedModulesCategory(CContainer* parent, const CString& name, int dev_id);
	virtual void HandleRequest(REQUEST* req) noexcept;
};

class CAssignedModule : public CTemplateItemHardwareStatus<
	CModuleTemplate<CAssignedSegment, 
	DatabaseElement<tb::ass_module, FixedName>,
	AssignedModuleBlob, tb::ass_module::F__data, tb::ass_module::F__sub_tree> >
{
	friend class CAVR5DynamicLinker;
public:
	DECLARE_VISITOR()
	CAssignedModule(CContainer* parent, CModule* pModule, int dev_id);
	CAssignedModule(CContainer* parent, int id, const string& name, int dev_id, int mod_id, int status);
	virtual CString GetTitle();
	virtual int Upload();
	virtual int Unload();
	virtual void ConstructMenu(CMenu& menu) noexcept;
	virtual void HandleRequest(REQUEST* req) noexcept;
protected:
	int m_dev_id;
	int m_idObject;
	CSegment* m_pSegment;
};

class CAssignedSegment : public CTemplateItemHardwareStatus<CSegment>
{
	using base = CTemplateItemHardwareStatus<CSegment>;
	friend boost::serialization::access;
	template<class Archive>
	void serialize(Archive & ar, const unsigned int file_version) {
		ar & boost::serialization::base_object<base>(*this);
	}
public:
	CAssignedSegment() {}
	CAssignedSegment(CContainer* parent, int id, const CString& name) :
		CTemplateItemHardwareStatus(parent, id, name) {}
	CAssignedSegment(const CSegment* other, CContainer* parent);
	virtual CSegmentValue* CreateChild(CContainer* parent, const CString& name, int id, uint16_t offset);
	virtual void ConstructMenu(CMenu& menu) noexcept;
	virtual void HandleRequest(REQUEST* req) noexcept;
	void ShowOnlineList();
};

class CAssignedSegmentValue : public CTemplateItemHardwareStatus<CSegmentValue>
{
	using inherited = CTemplateItemHardwareStatus<CSegmentValue>;
	friend boost::serialization::access;
	template<class Archive>
	void serialize(Archive & ar, const unsigned int file_version) {
		ar & boost::serialization::base_object<inherited>(*this);
		ar & m_variable;
	}
public:
	CAssignedSegmentValue() {}
	CAssignedSegmentValue(CContainer* parent, int id, const CString& name, uint16_t offset) :
		CTemplateItemHardwareStatus(parent, id, name, offset) { }
	CAssignedSegmentValue(const CSegmentValue* other, CContainer* parent);
	virtual INT_PTR ShowProperties();
	npsys::variable m_variable;
};