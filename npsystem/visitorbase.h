#pragma once
#define ACCEPT_DECL(BlockClassName) \
	virtual void Accept (BlockClassName* pBlock);
#define ACCEPT_NONE(BlockClassName) \
	virtual void Accept (BlockClassName* pBlock) {}
#define FINDSPECIFICTYPE_IMPL(Visitor, BlockClassName) \
	void Visitor ## ::Accept(BlockClassName* pBlock) { \
		elems_.push_back(pBlock); \
	 } 
/////////////////////////////////////////////////

template <class T>
class COneTypeContainer {
protected:
	std::vector<T> elems_;
public:
	size_t size() const noexcept { return elems_.size(); }
	bool empty() const noexcept { return elems_.empty(); }
	void clear() noexcept { elems_.clear(); }
	T &operator[] (size_t i) { return elems_[i]; }
	auto begin() noexcept { return elems_.begin(); }
	auto end() noexcept { return elems_.end(); }
	auto begin() const noexcept { return elems_.begin(); }
	auto end() const noexcept { return elems_.end(); }
	std::vector<T>& get() noexcept { return elems_; }
};

template <class T, class VisitorT>
class CFindType
	: public VisitorT
	, public COneTypeContainer<T*> {
	static_assert(!std::is_abstract<T>::value, "T cannot be abstract.");
public:
	virtual void Accept(T* p) {
		this->elems_.push_back(p);
	}
};

template <class Visitor>
void Traversal(typename Visitor::InterfacePtr root, Visitor& visitor) {
	ATLASSERT(root != nullptr);
	Iter::PreorderIterator<typename Visitor::InterfacePtr> it(root);
	for (; !it.IsDone(); it.Next()) {
		(*it)->Visit(visitor);
	}
}

template <class Visitor>
void Traversal(typename Visitor::InterfacePtr root, std::initializer_list<Visitor*>&& visitors) {
	ATLASSERT(root != nullptr);
	Iter::PreorderIterator<typename Visitor::InterfacePtr> it(root);
	for (; !it.IsDone(); it.Next()) {
		for (auto& v : visitors) {
			(*it)->Visit(*v);
		}
	}
}
