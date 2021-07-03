#pragma once

class CLib;

#include "Item.h"
#include "hash.h"


typedef TItem<CategoryElement> CLibObject;
typedef ItemListContainer<std::list, CLibObject, FixedElement, TableChilds> CLibObjectCategory;

struct LibObjectFile 
{
	int	id;
	int	idLib;
	CStringW stObjName;
	std::vector<string> expf;
	HASHMD5 md5;
	template<class Archive>
	void serialize(Archive & ar, const unsigned int file_version) {
		ar & id;
		ar & idLib;
		ar & stObjName;
		ar & expf;
		ar & md5;
	}
};

class LibBlob
{
	friend boost::serialization::access;
	template<class Archive>
	void serialize(Archive & ar, const unsigned int file_version) {
		ar & additionalLib;
		ar & table;
		ar & libobjects;
	}
public:
	//
	std::vector<int> additionalLib;
	DepTable table;
	std::map<string, LibObjectFile> libobjects;
};

class CLib : public ItemListContainer<std::list, CItem, 
	DatabaseElement<tb::lib_t, NameField<tb::lib_t::F__name>>,
	DataChilds<tb::lib_t::F__sub_tree>>
{
	friend class LD;
	friend class Factory;
protected:
	CLib(CContainer* parent, int id);
	CLib(CContainer* parent, const tb::lib_t& data);
public:
	DECLARE_VISITOR()
	virtual void HandleRequest(REQUEST* req) noexcept;
	void SaveData();
	virtual INT_PTR ShowProperties();
	virtual void ConstructMenu(CMenu& menu) noexcept;
	void Build();
	void CreateDependenceTree();
	virtual int	Move(CFilesContainer*, int);
	virtual int	Move(CFileItem*, int);
	prm::BlobData<LibBlob, tb::lib_t::F__data> m_library;
	CLibObjectCategory *pObjectFiles;
protected:
	virtual void PostDelete() noexcept;
};

template<>
struct table_adapter<CLib>
{
	using type = CLib;
	inline static auto db_load_one(int id) {
		return db->select<tb::lib_t, pr::single_where,
			LF::F_id, AF::F_name>(pr::make_single_where(
				pr::base::cond<tb::lib_t, op::eq, LF::F_id>(id)
			))[0];
	}
	inline static auto db_load(int for_key) {
		return db->select<tb::lib_t, pr::single_empty, LF::F_id, AF::F_name>();
	}
	inline static auto db_create(CContainer* parent, const tb::lib_t& data) {
		return CLib::Factory::Create<CLib>(parent, data);
	}
};

inline string remove_ext(const string& str) {
	size_t pos = str.find_last_of('.');
	return string(str.c_str(), pos);
}

// CLibraries

class CLibraries : public LazyItemListContainer<std::list, CLib, FixedElement, TableChilds>
{
	friend class Factory;
protected:
	CLibraries(CContainer* parent);
public:
	virtual void HandleRequest(REQUEST* req) noexcept;
	virtual void ConstructMenu(CMenu& menu) noexcept;
	void GetMap(std::map<string, CLib*>& map);
	void GetNameByID(std::map<int, string>& map);
	LibObjectFile* GetLibraryObjectFile(const struct LibObject& o);
};