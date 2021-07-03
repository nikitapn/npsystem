#pragma once

#include <npdb/constants.h>
#include "constants.h"
#include "stuff.h"
#include "error.h"
#include "global.h"
#include "MyTabView.h"
#include "tr_request.h"
#include <bitset>

class CTreeItemAbstract;
class CContainer;
class CView;
class CViewAdapter;
class COnlineTreeItem;

enum class ELTYPE {
	FIXED, CATEGORY, DATABASE
};

template<typename T>
concept has_childs_node_list = requires(T x) { x.childs_node_list(); };

/*
template <typename T, typename U = void>
struct is_set : std::false_type {};

template <typename T>
struct is_set<T, std::is_same<T, std::set<
	typename T::key_type,
	typename T::key_compare,
	typename T::allocator_type> > > : std::true_type {
};

template <typename T>
constexpr auto is_set_v = is_set<T>::value;
*/

template<class T, class Item>
inline std::enable_if_t<!T::is_dialog, CItemView*> CreateViewForItem(Item* item, CMyTabView& tabview) {
	if constexpr(Item::is_container()) {
		item->LoadChilds();
	}
	auto view = new T(item, tabview);
	view->Create(tabview, CWindow::rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, 0);
	return view;
}

template<class T, class Item>
inline std::enable_if_t<T::is_dialog, CItemView*> CreateViewForItem(Item* item, CMyTabView& tabview) {
	if constexpr(Item::is_container()) {
		item->LoadChilds();
	}
	auto view = new T(item, tabview);
	view->Create(tabview, NULL);
	return view;
}

class InteractiveItem {
public:
	virtual void HandleRequest(REQUEST* req) noexcept = 0;

	virtual void ConstructMenu(CMenu* menu) noexcept {}
	virtual INT_PTR ShowProperties() { return IDCANCEL; }
	void ShowMenu(CPoint& pt, CWindow* pWnd);
	void DefaultMenu(CMenu* menu) const noexcept;
};

class CTreeItemAbstract : public InteractiveItem {
	friend class CItemView;
	template<class, class> friend class ItemTraits;
	CItemView* view_ = nullptr;
	enum FL {
		UpperCase = 0
	};
protected:
	HICON m_hIcon = NULL;
	HTREEITEM m_hTreeItem = NULL;
	std::string name_;
	COnlineTreeItem* online_item_ = nullptr;
	std::bitset<8> flags_;
	static constexpr bool has_hardware_status() noexcept { return false; }
public:
	static CTreeViewCtrl m_treeview;
	static CMyTabView* m_tabview;
	typedef Iter::Iterator<CTreeItemAbstract*>* IIteratorPtr;
public:
	virtual	~CTreeItemAbstract() = default;
	//
	virtual oid_t GetId() const noexcept = 0;
	virtual bool SetName(const std::string& name) {
		name_ = name;
		return true;
	}
	bool IsUpperCase() const noexcept { return !flags_.test(FL::UpperCase); }
	void UnsetUpperCase() noexcept { flags_.set(FL::UpperCase, true); }
	virtual void HandleRequest(REQUEST* req) noexcept;
	void ForceSetName(const std::string& name) noexcept { name_ = name; }
	const std::string& GetName() const { return name_; }
	virtual void LoadName() {}
	// double dispatch
	virtual int Move(CTreeItemAbstract*, int) { return 0; }
	virtual int	Move(class CTreeAlgorithm* alg, int) { return 0; }
	virtual int	Move(class CTreeI2CModule* mod, int) { return 0; }
	virtual bool IsRemovable() { return true; }
	// Window
	void ShowView() noexcept;
	virtual CItemView* CreateView(CMyTabView& tabview) { return nullptr; };
	virtual void CloseView();
	virtual IIteratorPtr CreateIterator();
	void SetIcon(AlphaIcon icon) noexcept { m_hIcon = global.GetIcon(icon); }
	HICON GetIcon() const noexcept { return m_hIcon; }
	HTREEITEM GetHTR() const { return m_hTreeItem; }
	CContainer* GetParent() const {
		auto htr = m_treeview.GetParentItem(m_hTreeItem);
		if (htr == NULL) return nullptr;
		return reinterpret_cast<CContainer*>(m_treeview.GetItemData(htr));
	}
	bool IsWindow() { return view_ != nullptr; }
	CItemView* GetWindow() { return view_; }
	virtual void Insert(HTREEITEM htrParent) {
		m_hTreeItem = m_treeview.InsertItem(wide(GetName()).c_str(), htrParent, TVI_LAST);
		m_treeview.SetItemData(m_hTreeItem, (DWORD_PTR)this);
	}
	void UpdateTreeLabel(const std::string& name) {
		name_ = name;
		UpdateTreeLabel();
	}
	void Detach() const noexcept {
		m_treeview.DeleteItem(m_hTreeItem);
	}
	CItemView* SetWindow(CItemView* view) noexcept {
		auto old = view_;
		view_ = view;
		return old;
	}
	virtual COnlineTreeItem* CreateOnlineFromThis(std::vector<std::unique_ptr<COnlineTreeItem>>& nodes, CTreeViewCtrl tree, HTREEITEM hParent);
	virtual void Visit(CBlockVisitor& v) {}
	virtual void Delete() noexcept;
	virtual bool ChangeName(const std::string& name);
	virtual CViewAdapter* GetViewAdapter() { ATLASSERT(FALSE); return NULL; }
	virtual io::Status CalcAndUpdateHardwareStatus() { return io::Status::assigned; }
	virtual bool HasHarwareStatus() const noexcept { return false; }
	virtual void GetViewTitle(std::string& title) = 0;
	virtual std::string	GetTitle() = 0;
	void UpdateListView();
	void UpdateTreeLabel();
	static void InvalidateTree() { m_treeview.Invalidate(); }
	virtual void DrawIcon(HDC hdc, int x, int y) {
		constexpr auto cx = constants::treeview::icon_cx;
		constexpr auto cy = constants::treeview::icon_cy;
		DrawIconEx(hdc, x, y, m_hIcon, cx, cy, 0, NULL, DI_IMAGE | DI_MASK);
	}
	virtual void StoreItem() = 0;
	static constexpr auto is_container() noexcept { return false; }
protected:
	static REQUEST GetCategoryKeeper(CTreeItemAbstract* pStart) {
		REQUEST req(ID_GET_CATEGORY_KEEPER);
		pStart->HandleRequest(&req);
		ATLASSERT(req.result_ptr);
		return req;
	}
	virtual void PostDelete(CContainer* parent) noexcept {};
	template<class, class, template <class, class...> class>
	friend class ItemContainer;
};

class CContainer : public CTreeItemAbstract {
public:
	static constexpr auto has_lazy_loading() noexcept { return false; }

	virtual bool LoadChilds(bool initiate_by_user = false) noexcept {
		if (childs_loaded_) return true;
		LoadChildsImpl();
		childs_loaded_ = true;
		return true;
	};
	virtual void AddItem(CTreeItemAbstract* item) = 0;
	virtual void EraseItem(CTreeItemAbstract* item) = 0;
	virtual bool HasItemWithName(const std::string& name) const noexcept = 0;
	// moves branch to this container
	virtual int MoveBranch(CTreeItemAbstract* branch) = 0;
	virtual int MoveBranchImpl(CTreeItemAbstract* branch);
	bool IsChildsLoaded() const noexcept {
		return childs_loaded_;
	}
	bool FindName(const std::string& name) noexcept {
		Iter::IteratorPtr<CTreeItemAbstract*> it(this);
		for (; !it->IsDone(); it->Next()) {
			auto item = it->CurrentItem();
			if (item->GetName() == name) return true;
		}
		return false;
	}
	void Expand() noexcept {
		m_treeview.Expand(m_hTreeItem, TVE_EXPAND);
	}
	virtual COnlineTreeItem* CreateOnlineFromThis(std::vector<std::unique_ptr<COnlineTreeItem>>& nodes, CTreeViewCtrl tree, HTREEITEM hParent);
	static constexpr auto is_container() noexcept { return true; }
protected:
	virtual void LoadChildsImpl() noexcept = 0;
	bool childs_loaded_ = false;
};

// class ItemTemplateContainer
template<class MostDerrived, class T, template <class, class...> class stl_container>
class ItemTemplateContainer : public CContainer {
	using self = ItemTemplateContainer;
protected:
	typedef typename stl_container<T*, typename std::allocator<T*> > container;
	typedef typename container::iterator iterator;
public:
	virtual CTreeItemAbstract::IIteratorPtr CreateIterator() final {
		// my single thread version don't use in multithread
		m_iterator.SetRange(m_items.begin(), m_items.end());
		return reinterpret_cast<CTreeItemAbstract::IIteratorPtr>(&m_iterator);
	}
	virtual void EraseItem(CTreeItemAbstract* item) { EraseItemImpl(item); };

	T* GetElementById(oid_t id) noexcept {
		auto end = m_items.end();
		auto it = std::find_if(m_items.begin(), end, [&id](const CTreeItemAbstract* pItem) {
			return pItem->GetId() == id; });
		return it == end ? NULL : (*it);
	}
	void insert(T* ptr) noexcept {
		ptr->LoadName();
		m_items.push_back(ptr);
	}
protected:
	void EraseItemImpl(CTreeItemAbstract* item) noexcept {
		iterator it = std::find(m_items.begin(), m_items.end(), item);
		m_items.erase(it);
	}

	Iter::StlContainerIterator<T*, stl_container> m_iterator;
	container m_items;
};

// class ItemTemplateContainer set
struct id_compare {
	bool operator() (const  CTreeItemAbstract* lhs, const  CTreeItemAbstract* rhs) const {
		return lhs->GetId() < rhs->GetId();
	}
};

template<class MostDerrived, class T>
class ItemTemplateContainer<MostDerrived, T, std::set> : public CContainer {
	using self = ItemTemplateContainer<MostDerrived, T, std::set>;
protected:
	typedef typename std::set<T*, id_compare> container;
	typedef typename container::iterator iterator;
public:
	virtual CTreeItemAbstract::IIteratorPtr CreateIterator() final {
		m_iterator.SetRange(m_items.begin(), m_items.end());
		return reinterpret_cast<CTreeItemAbstract::IIteratorPtr>(&m_iterator);
		//	return new Iter::StlContainerIterator<CTreeItemAbstract*, std::set, id_compare<T>>
		//		(*reinterpret_cast<std::set<CTreeItemAbstract*, id_compare<T>>*>(&m_items));
	}
	virtual void EraseItem(CTreeItemAbstract* item) { EraseItemImpl(item); };
	
	T* GetElementById(oid_t id) noexcept {
		auto it = m_items.find(static_cast<T*>(reinterpret_cast<CTreeItemAbstract*>((char*)&id - sizeof(void*))));
		return it == m_items.end() ? nullptr : (*it);
	}
	void insert(T* ptr) noexcept {
		ptr->LoadName();
		m_items.insert(ptr);
	}
protected:
	void EraseItemImpl(CTreeItemAbstract* item) noexcept {
		m_items.erase((T*)item);
	}

	Iter::StlContainerIterator<T*, std::set, id_compare> m_iterator;
	container m_items;
};

// class ItemContainer
template<class MostDerrived, class T, template <class, class...> class stl_container>
class ItemContainer : public ItemTemplateContainer<MostDerrived, T, stl_container> {
	using base = ItemTemplateContainer<MostDerrived, T, stl_container>;
	using self = ItemContainer;
public:
	void Clear() {
		Iter::PostorderIterator<CTreeItemAbstract*> it(this);
		for (; !it.IsDone(); it.Next()) {
			auto item = it.CurrentItem();
			item->CloseView();
			item->Detach();
			delete item;
		}
		this->m_items.clear();
	}
	virtual void AddItem(CTreeItemAbstract* item) final {
		bool need_expand_after_insert = (this->m_items.size() <= 1);
		this->m_treeview.Expand(this->m_hTreeItem, TVE_EXPAND);
		this->childs_loaded_ = true;
		this->insert(static_cast<T*>(item));
		item->Insert(this->m_hTreeItem);
		if (need_expand_after_insert) this->m_treeview.Expand(this->m_hTreeItem, TVE_EXPAND);
	}
	void InsertChilds() {
		for (auto item : this->m_items)
			item->Insert(this->m_hTreeItem);
	}
	virtual void Insert(HTREEITEM htrParent) {
		CTreeItemAbstract::Insert(htrParent);
		for (auto item : this->m_items) {
			item->Insert(this->m_hTreeItem);
		}
	}
	virtual void HandleRequest(REQUEST* req) noexcept {
		switch (req->ID_MENU) {
		case ID_CHANGE_NAME:
			req->result = FindName_(*((std::string*)req->param)) == this->m_items.end();
			break;
		default:
			__super::HandleRequest(req);
			//			if (req->done)
			//				return;
			//			for (auto& i : m_items) {
			//				i->HandleRequest(req);
			//				if (req->done) break;
			//			}
		};
	}
	virtual bool HasItemWithName(const std::string& name) const noexcept {
		return GetItemByName(name) != nullptr;
	}

	auto cbegin() const noexcept { return this->m_items.cbegin(); }
	auto cend() const noexcept { return this->m_items.cend(); }
	auto begin() noexcept { return this->m_items.begin(); }
	auto end() noexcept { return this->m_items.end(); }
	auto size()  const noexcept { return this->m_items.size(); }
	
	const T* GetItemByName(const std::string& name) const noexcept {
		const auto it = FindName_(name);
		return it == this->m_items.end() ? nullptr : (*it);
	}

	T* GetItemByName(const std::string& name) noexcept {
		auto it = FindName_(name);
		return it == this->m_items.end() ? nullptr : (*it);
	}

	virtual	void Delete() noexcept final {
		if (!base::m_items.empty()) {
			Iter::PostorderIterator<CTreeItemAbstract*> it(this);
			for (; !it.IsDone(); it.Next()) {
				auto item = (*it);
				item->CloseView();
				item->Detach();
				item->PostDelete(nullptr);
				delete item;
			}
		}
		CTreeItemAbstract::Delete();
	}
	virtual	bool IsRemovable() {
		if (!this->m_items.empty()) {
			Iter::PostorderIterator<CTreeItemAbstract*> it(this);
			for (; !it.IsDone(); it.Next()) {
				if (!(*it)->IsRemovable())
					return false;
			}
		}
		return true;
	};

	template<typename L, typename... Args>
	void CreateNewItem(L& lst, Args&&... args) requires (T::TT::ElementType::type == ELTYPE::DATABASE) {
		this->m_treeview.Expand(this->m_hTreeItem, TVE_EXPAND);
		auto n = lst.add_value(new typename T::TT::ElementType::node_t::type_t(std::forward<Args>(args)...));
		if (n) {
			AddItem(new T((*n), lst));
			lst.store();
			this->UpdateListView();
		}
	}

protected:
	virtual void LoadChildsImpl() noexcept {
		if constexpr (!std::is_same_v<CTreeItemAbstract, T>) {
			if constexpr (T::TT::ElementType::type == ELTYPE::DATABASE) {
				if constexpr (has_childs_node_list<MostDerrived>) {
					auto& lst = static_cast<MostDerrived*>(this)->childs_node_list();
					lst.fetch_all_nodes();
					for (auto& n : lst) {
						this->insert(new T(n, lst));
					}
				}
			}
		}
	}

	const auto FindName_(const std::string& name) const noexcept {
		return std::find_if(this->m_items.begin(), this->m_items.end(), [name](const auto item) {
			return item->GetName() == name; });
	}

	auto FindName_(const std::string& name) noexcept {
		return std::find_if(this->m_items.begin(), this->m_items.end(), [name](const auto item) {
			return item->GetName() == name; });
	}

	std::string GetNextName(const std::string& pattern) {
		std::string name;
		auto end = this->m_items.end();
		int i = 1;
		do {
			name = pattern + std::to_string(i++);
		} while (FindName_(name) != end);
		return name;
	}
};

template<class WrappedType, class ElType>
class ItemTraits {
public:
	using ElementType = ElType;
	using T = WrappedType; // not the most derrived
	using IsContainer = std::is_base_of<CContainer, T>;
	//
	static int MoveBranch(CTreeItemAbstract* pItem, T* pDestination) {
		return MoveBranch_(pItem, pDestination, IsContainer());
	}
	static int CalcAndUpdateHardwareStatus(CTreeItemAbstract* pItem) {
		return 0;
	}
private:
	static int MoveBranch_(CTreeItemAbstract* item, T* dest, std::true_type) {
		auto old_parent = item->GetParent();
		int result = dest->MoveBranchImpl(item);
		if (result) {
			auto ck_1 = CTreeItemAbstract::GetCategoryKeeper(dest);
			auto ck_2 = CTreeItemAbstract::GetCategoryKeeper(old_parent);
			if (ck_1.item == ck_2.item) {
				ck_1.item->StoreItem();
			} else {
				ck_1.item->StoreItem();
				ck_2.item->StoreItem();
			}
		}
		return result;
	}
	static int MoveBranch_(CTreeItemAbstract* pItem, T* pDestination, std::false_type) {
		return 0;
	}
};

struct FixedElement {
public:
	static constexpr ELTYPE type = ELTYPE::FIXED;

	static void GetViewTitle(std::string& title, CTreeItemAbstract* item) {
		title = item->GetParent()->GetTitle() + ":" + item->GetName();
	}
	static std::string GetTitle(CTreeItemAbstract* item) {
		return item->GetParent()->GetTitle();
	}
};

template<class T, class U>
struct CategoryElement {
public:
	using DataType = T;
	using list_t = U;

	static constexpr ELTYPE type = ELTYPE::CATEGORY;

	CategoryElement(DataType* data, list_t& list)
		: data_(data)
		, list_(list)
	{
	}
	
	static void GetViewTitle(std::string& title, CTreeItemAbstract* item) {
		title = item->GetTitle();
	}
	static std::string GetTitle(CTreeItemAbstract* item) {
		return item->GetParent()->GetTitle() + ":" + item->GetName();
	}

	const DataType* data() const noexcept { return data_; }
	DataType* data() noexcept { return data_; }
protected:
	DataType* data_;
	list_t& list_;
};

struct no_parent {};

template<typename List, typename Node>
struct ListWrapper {
	ListWrapper(List& _list)
		: list(_list) {}
	
	void Remove(Node& n) {
		list.fast_erase(n);
		list.store();
		n.remove();
	}

	List& list;
};

template<typename Node>
struct ListWrapper<no_parent, Node> {
	void Remove(Node& n) {}
};



template<typename Node, typename List = no_parent>
class DatabaseElement {
public:
	using node_t = Node;
	using node_list_t = List;
	using node_list_wrapper_t = ListWrapper<List, Node>;
	static constexpr auto has_parent = !std::is_same_v<List, no_parent>;
	static constexpr ELTYPE type = ELTYPE::DATABASE;
	
	static void GetViewTitle(std::string& title, CTreeItemAbstract* item) {
		title = item->GetName();
	}
	static std::string GetTitle(CTreeItemAbstract* item) {
		return item->GetName();
	}
};

template<class MostDerrived, class T, class ElType, typename SFINAE = void>
class CTemplateItemT;

template<class MostDerrived, class T, class ElType>
class CTemplateItemT<MostDerrived, T, ElType, std::enable_if_t<ElType::type == ELTYPE::FIXED> > : public T {
	using base = T;
public:
	virtual bool SetName(const std::string& name) final {
		return false;
	}
	virtual void StoreItem() final {}
};

template<class MostDerrived, class T, class ElType>
class CTemplateItemT<MostDerrived, T, ElType, std::enable_if_t<ElType::type == ELTYPE::CATEGORY> > 
	: public T, public ElType {
	using base = T;
public:
	using DataType = typename ElType::DataType;
	using list_t = typename ElType::list_t;

	CTemplateItemT(DataType* data, list_t& list)
		: ElType(data, list) {}

	virtual oid_t GetId() const noexcept final {
		return ElType::data_->GetId();
	}

	virtual bool SetName(const std::string& name) {
		if (base::SetName(name)) {
			ElType::data_->set_name(name);
			StoreItem();
			return true;
		}
		return false;
	}

	virtual void LoadName() {
		base::name_ = ElType::data_->get_name();
	}

	virtual void StoreItem() final {
		StoreItemImpl(base::GetParent());
	}

	virtual void EraseFromList() final {
		auto id = ElType::data_->GetId();
		auto founded = std::find_if(ElType::list_.begin(), ElType::list_.end(), [id](const auto& ptr) {
			return id == ptr->GetId();
		});

		ASSERT(founded != ElType::list_.end());

		ElType::list_.erase(founded);
	}
protected:
	void StoreItemImpl(CContainer* parent) {
		if (!parent) return;
		auto cb = base::GetCategoryKeeper(parent);
		if (cb.item) cb.item->StoreItem();
	}

	virtual void PostDelete(CContainer* parent) noexcept {
		auto id = ElType::data_->GetId();
		auto founded = std::find_if(ElType::list_.begin(), ElType::list_.end(), [id](const auto& ptr) {
			return id == ptr->GetId();
		});

		ASSERT(founded != ElType::list_.end());

		ElType::list_.erase(founded);

		StoreItemImpl(parent);
	};
};

template<class MostDerrived, class T, class ElType>
class CTemplateItemT<MostDerrived, T, ElType, std::enable_if_t<ElType::type == ELTYPE::DATABASE> > : public T {
	using base = T;
	using node_t = typename ElType::node_t;
	using node_list_t = typename ElType::node_list_t;
	using node_list_wrapper_t = typename ElType::node_list_wrapper_t;
public:
	CTemplateItemT(node_t& n)
		: n_(n) 
	{
	}
	CTemplateItemT(node_t& n, node_list_t& list)
		: n_(n)
		, list_wrapper_(list)
		
	{
	}

	node_t& node() noexcept { return n_; }

	virtual oid_t GetId() const noexcept final {
		return n_.id();
	}

	virtual bool SetName(const std::string& name) {
		if constexpr(odb::is_mutable_name<typename node_t::type_t>::value) {
			n_->set_name(name);
			n_.store();
			return true;
		}
		return false;
	}
	virtual void LoadName() {
		ASSERT(n_.loaded());
		base::name_ = n_->get_name();
	}
	virtual void StoreItem() {
		n_.store();
	}
	virtual void PostDelete(CContainer* parent) noexcept {
		list_wrapper_.Remove(n_);
	};
	node_t n_;
	node_list_wrapper_t list_wrapper_;
};

template<class MostDerrived, class T, class ElType>
class CTemplateItem : public CTemplateItemT<MostDerrived, T, ElType> {
	using self = CTemplateItem<MostDerrived, T, ElType>;
	using base = CTemplateItemT<MostDerrived, T, ElType>;
public:
	using TT = ItemTraits<T, ElType>;

	template<typename... Args>
	CTemplateItem(Args&&... args) 
		: base(std::forward<Args>(args)...) {}

	virtual void HandleRequest(REQUEST* req) noexcept {
		switch (req->ID_MENU) {
		case ID_ITEM_PROPERTIES:
			T::HandleRequest(req);
			if (req->result == IDOK) {
				base::StoreItem();
			}
			break;
		default:
			T::HandleRequest(req);
		};
	}

	virtual void GetViewTitle(std::string& title) final {
		ElType::GetViewTitle(title, this);
	}

	virtual std::string	GetTitle() final {
		return ElType::GetTitle(this);
	}

	virtual bool ChangeName(const std::string& name) {
		if (name == base::GetName()) return false;
		auto parent = base::GetParent();
		if (parent != nullptr) {
			if (static_cast<CContainer*>(parent)->FindName(name)) {
				return false;
			}
		}
		bool result = base::SetName(name);
		if (result) base::name_ = name;
		return result;
	};
};


template<class T, class MostDerrived>
class CTemplateItemHardwareStatusBase : public T {
	using base = T;
protected:
	static constexpr bool has_hardware_status() noexcept { return true; }
	io::Status status_ = io::Status::assigned;
public:
	template<typename... Args>
	CTemplateItemHardwareStatusBase(Args&&... args)
		: base(std::forward<Args>(args)...) {}

	virtual void DrawIcon(HDC hdc, int x, int y) {
		constexpr auto cx = constants::treeview::icon_cx;
		constexpr auto cy = constants::treeview::icon_cy;
		
		DrawIconEx(hdc, x, y, base::m_hIcon, cx, cy, 0, NULL, DI_IMAGE | DI_MASK);

		constexpr auto status_cx = constants::treeview::status_icon_cx;
		constexpr auto status_cy = constants::treeview::status_icon_cy;
		constexpr auto x_offset = -status_cx;
		constexpr auto y_offset = cy - status_cy;

		

		if (status_ != io::Status::relevant) {
			CIconHandle icon;
			switch (status_) {
			case io::Status::assigned: 
				icon = global.GetIcon(AlphaIcon::Empty);
				break;
			case io::Status::not_relevant: 
				icon = global.GetIcon(AlphaIcon::Exclam);
				break;
			case io::Status::unknown: 
				icon = global.GetIcon(AlphaIcon::Question);
				break;
			}
			ASSERT(icon);
			DrawIconEx(hdc, x + x_offset, y + y_offset, icon, status_cx, status_cy, 0, NULL, DI_IMAGE | DI_MASK);
		}


	}
	virtual INT_PTR ShowProperties() {
		auto result = __super::ShowProperties();
		if (result == IDOK) base::CalcAndUpdateHardwareStatus();
		return result;
	}
	void CalcAndUpdateHardwareStatusAll(CContainer* parent = nullptr) {
		base::CalcAndUpdateHardwareStatus();
		
		auto next = (parent == nullptr ? base::GetParent() : parent);
		while (next && next->HasHarwareStatus()) {
			next->CalcAndUpdateHardwareStatus();
			next = next->GetParent();
		}	
	}
	virtual bool HasHarwareStatus() const noexcept { return true; }
	virtual void PostDelete(CContainer* parent) noexcept {
		__super::PostDelete(parent);
		if (parent) { // delete was called from this item
			if (parent->HasHarwareStatus()) {
				CalcAndUpdateHardwareStatusAll(parent);
			}
		}
	}
};

template<class T, class MostDerrived>
class CTemplateItemHardwareStatusItem : public CTemplateItemHardwareStatusBase<T, MostDerrived> {
	using base = CTemplateItemHardwareStatusBase<T, MostDerrived>;
public:
	template<typename... Args>
	CTemplateItemHardwareStatusItem(Args&&... args)
		: base(std::forward<Args>(args)...) {}
	
	virtual io::Status CalcAndUpdateHardwareStatus() {
		base::status_ = static_cast<MostDerrived*>(this)->ConvertStatus();
		return base::status_;
	}

	io::Status ConvertStatus() {
		static_assert(false, "not implemented");
		return io::Status::assigned;
	}
};

template<class T, class MostDerrived>
class CTemplateItemHardwareStatusContainer 
	: public CTemplateItemHardwareStatusBase<T, MostDerrived> {
	using base = CTemplateItemHardwareStatusBase<T, MostDerrived>;
public:
	template<typename... Args>
	CTemplateItemHardwareStatusContainer(Args&&... args)
		: base(std::forward<Args>(args)...) {}
	
	virtual io::Status CalcAndUpdateHardwareStatus() {
		if (base::m_items.size() == 0) {
			base::status_ = io::Status::relevant;
			return io::Status::relevant;
		}
		int status = 0, zero = 0;
		for (auto item : base::m_items) {
			int s = item->CalcAndUpdateHardwareStatus();
			status |= s;
			if (s == 0) zero = 2;
		}
		if (status != 0) status |= zero;
		if (status == 3) status = io::Status::not_relevant;
		if (status > 3) status = io::Status::unknown;
		base::status_ = static_cast<io::Status>(status);
		return base::status_;
	}
	
	virtual bool LoadChilds(bool initiate_by_user = false) noexcept {
		if (base::childs_loaded_) return true;

		auto result = base::LoadChilds(initiate_by_user);
		if constexpr (base::has_lazy_loading()) {
			if (initiate_by_user) {
				base::CalcAndUpdateHardwareStatusAll();
			}
		} 

		return result;
	}
};

template<class T, class MostDerrived>
class CTemplateItemHardwareStatus 
	: public std::conditional_t<T::TT::IsContainer::value,
	CTemplateItemHardwareStatusContainer<T, MostDerrived>, 
	CTemplateItemHardwareStatusItem<T, MostDerrived>>
{
	using base = std::conditional_t<T::TT::IsContainer::value,
		CTemplateItemHardwareStatusContainer<T, MostDerrived>, 
		CTemplateItemHardwareStatusItem<T, MostDerrived>>;
public:
	template<typename... Args>
	CTemplateItemHardwareStatus(Args&&... args)
		: base(std::forward<Args>(args)...) {}
};

// store methods

struct Manual {};
struct TableChilds {};
struct DataChilds {};

template<
	class MostDerrived,
	template <class, class...> class stl_container,
	class T,
	class ElType,
	class StoreMethod = Manual,
	typename Unused = void
> class ItemListContainer;

template
<
	class MostDerrived,
	template <class, class...> class stl_container,
	class T,
	class ElType,
	class StoreMethod
>
class ItemListContainer<MostDerrived, stl_container, T, ElType, StoreMethod, std::enable_if_t<std::is_same_v<Manual, StoreMethod>>> 
	: public CTemplateItem<MostDerrived, ItemContainer<MostDerrived, T, stl_container>, ElType> {
	using self = ItemListContainer<MostDerrived, stl_container, T, ElType, StoreMethod, void>;
	using base = CTemplateItem<MostDerrived, ItemContainer<MostDerrived, T, stl_container>, ElType>;
public:
	using ChildType = T;
	using item_type = CTemplateItem<MostDerrived, ItemContainer<MostDerrived, T, stl_container>, ElType>;

	template<typename... Args>
	ItemListContainer(Args&&... args) 
		: base(std::forward<Args>(args)...) {}

	virtual int MoveBranch(CTreeItemAbstract* pItem) final {
		return 0;
		//return base::MoveBranch(pItem, this);
	}
};

template
<
	class MostDerrived,
	template <class, class...> class stl_container,
	class T,
	class ElType,
	class StoreMethod
>
class ItemListContainer<MostDerrived, stl_container, T, ElType, StoreMethod, std::enable_if_t<std::is_same_v<TableChilds, StoreMethod>>> 
	: public CTemplateItem<MostDerrived, ItemContainer<MostDerrived, T, stl_container>, ElType> {
	using self = ItemListContainer<MostDerrived, stl_container, T, ElType, StoreMethod, void>;
	using base = CTemplateItem<MostDerrived, ItemContainer<MostDerrived, T, stl_container>, ElType>;
public:
	using ChildType = T;
	using item_type = CTemplateItem<MostDerrived, ItemContainer<MostDerrived, T, stl_container>, ElType>;

	template<typename... Args>
	ItemListContainer(Args&&... args) 
		: base(std::forward<Args>(args)...) {}

	virtual int MoveBranch(CTreeItemAbstract* pItem) final {
		return 0;
	}
};

template
<
	class MostDerrived,
	template <class, class...> class stl_container,
	class T,
	class ElType,
	class StoreMethod
>
class ItemListContainer<MostDerrived, stl_container, T, ElType, StoreMethod, std::enable_if_t<std::is_same_v<DataChilds, StoreMethod>>> 
	: public CTemplateItem<MostDerrived, ItemContainer<MostDerrived, T, stl_container>, ElType> {
	using self = ItemListContainer<MostDerrived, stl_container, T, ElType, StoreMethod, void>;
	using base = CTemplateItem<MostDerrived, ItemContainer<MostDerrived, T, stl_container>, ElType>;
public:
	using ChildType = T;
	using item_type = CTemplateItem<MostDerrived, ItemContainer<MostDerrived, T, stl_container>, ElType>;

	static_assert(ElType::type == ELTYPE::DATABASE, "This container only supports database nodes");


	template<typename... Args>
	ItemListContainer(Args&&... args) 
		: base(std::forward<Args>(args)...) {}

	virtual void HandleRequest(REQUEST* req) noexcept {
		switch (req->ID_MENU) {
		case ID_GET_CATEGORY_KEEPER:
			req->item = this;
			break;
		case ID_SERIALIZE:
			req->item = this;
			StoreItem();
			break;
		default:
			__super::HandleRequest(req);
		}
	}
	virtual int MoveBranch(CTreeItemAbstract* pItem) final {
		CTreeItemAbstract* parent = pItem->GetParent();
		int result = base::MoveBranchImpl(pItem);
		if (result) {
			StoreItem();
			parent->StoreItem();
		}
		return result;
	}
protected:
	virtual void LoadChildsImpl() noexcept {
		if (this->n_->childs.fetch()) {
			for (auto& c : this->n_->childs) {
				this->insert(new T(c.get(), this->n_->childs));
			}
		}
	}
	virtual void StoreItem() {
		this->n_.store();
	}
};

class CFakeItem : public CTemplateItem<CFakeItem, CTreeItemAbstract, FixedElement> {
public:
	virtual oid_t GetId() const noexcept final {
		return odb::INVALID_ID;
	}
	virtual io::Status CalcAndUpdateHardwareStatus() { return io::Status::unknown; }
};

template
<
	class MostDerrived,
	template <class, class...> class stl_container,
	class T,
	class ElType,
	class StoreMethod = Manual
>
class LazyItemListContainer :
	public ItemListContainer<MostDerrived, stl_container, T, ElType, StoreMethod> {
	using base = ItemListContainer<MostDerrived, stl_container, T, ElType, StoreMethod>;
public:
	static constexpr auto has_lazy_loading() noexcept { return true; }

	LazyItemListContainer() {
		this->insert((T*)new CFakeItem());
	}

	template<typename... Args>
	LazyItemListContainer(Args&&... args)
		: base(std::forward<Args>(args)...) {
		this->insert((T*)new CFakeItem());
	}

	virtual bool LoadChilds(bool initiate_by_user = false) noexcept {
		if (base::childs_loaded_) return true;
		
		CWaitCursor wc;
		auto fake = *base::m_items.begin();
		base::m_items.clear();
		this->LoadChildsImpl();
		delete (CTreeItemAbstract*)fake;
		bool load_time = (base::m_hTreeItem == NULL);
		if (!load_time) {
			base::m_treeview.DeleteItem(base::m_treeview.GetChildItem(base::m_hTreeItem));
		}
		auto empty = base::m_items.empty();
		if (!empty && !load_time) {
			for (auto item : base::m_items) item->Insert(base::m_hTreeItem);
		} 
		base::childs_loaded_ = true;

		return !empty;
	}
};