#pragma once

#include <type_traits>
#include <list>
#include <vector>
#include <stack>
#include <array>
#include <assert.h>

namespace Iter
{
	template <class T>
	class Iterator
	{
	public:
		virtual ~Iterator() = default;
		virtual void Next() = 0;
		virtual T CurrentItem() = 0;
		virtual bool IsDone() = 0;
		T operator* (void);
	};
	template <class T>
	inline T Iterator<T>::operator* (void) {
		return CurrentItem();
	}

	template <class T> 
	class NullIterator : public Iterator<T>
	{
	public:
		virtual void Next() {};
		virtual T CurrentItem() {
			throw std::out_of_range("");
		}
		virtual bool IsDone() {
			return true;
		}
		void* operator new (std::size_t) {
			return &_instance;
		}
		void operator delete (void* ptr) { }
	private:
		static NullIterator<T> _instance;
	};

	template <class T>
	NullIterator<T> NullIterator<T>::_instance;

	template <class T>
	class ArrayIterator : public Iterator<T> {
		static_assert(std::is_pointer_v<T>, "T must be a pointer");
		T* cur_;
		T* end_;
	public:
		ArrayIterator() = default;
		
		ArrayIterator(T* _array, size_t count) {
			SetRange(_array, _array + count);
		}
		
		virtual T CurrentItem() {
			return (*cur_);
		}

		virtual void Next() {
			assert(cur_ != end_);
			cur_++;
		}
		
		virtual bool IsDone() { 
			return cur_ == end_;
		}
		
		void SetRange(T* begin, T* end) noexcept {
			cur_ = begin;
			end_ = end;
		}
	};

	template <class T>
	class OneElementIterator : public Iterator<T> {
		static_assert(std::is_pointer_v<T>, "T must be a pointer");
		T element_;
		bool done_ = false;
	public:
		OneElementIterator() 
			: done_(true) 
		{
		}
		
		OneElementIterator(T element)
			: element_(element)
			, done_(false) 
		{
		}
		
		virtual T CurrentItem() {
			return element_;
		}

		virtual void Next() {
			assert(!done_);
			done_ = true;
		}
		
		virtual bool IsDone() { 
			return done_; 
		}
		
		void Set(T element) noexcept {
			element_ = element;
			done_ = false;
		}
	};

	template <class T, template <typename, typename...> class stl_container, typename... Rest>
	class StlContainerIterator : public Iterator<T> {
	private:
		using container_t = stl_container<T, Rest...>;
		using iterator_t = typename container_t::iterator;
		iterator_t it_cur_;
		iterator_t end_;
	public:
		StlContainerIterator() = default;
		StlContainerIterator(container_t& items) {
			it_cur_ = items.begin();
			end_ = items.end();
		}
		virtual T CurrentItem() {
			return (*it_cur_);
		}
		virtual void Next() {
			assert(it_cur_ != end_);
			it_cur_++;
		}
		virtual bool IsDone() { return it_cur_ == end_; }
		void SetRange(iterator_t begin, iterator_t end) {
			it_cur_ = begin;
			end_ = end;
		}
	};

	template <class T, size_t N>
	class StlArrayIterator : public Iterator<T> {
	private:
		using container_t = std::array<T, N>;
		using iterator_t = typename container_t::iterator;
		iterator_t it_cur_;
		iterator_t end_;
	public:
		StlArrayIterator() = default;
		StlArrayIterator(container_t& items) {
			it_cur_ = items.begin();
			end_ = items.end();
		}
		virtual T CurrentItem() {
			return (*it_cur_);
		}
		virtual void Next() {
			assert(it_cur_ != end_);
			it_cur_++;
		}
		virtual bool IsDone() { return it_cur_ == end_; }
		void SetRange(iterator_t begin, iterator_t end) {
			it_cur_ = begin;
			end_ = end;
		}
	};

	template<class ChildType, bool bDynamiclyAllocated = false>
	class IteratorPtr
	{
	private:
		typename Iterator<ChildType>* iter_;
		IteratorPtr(const IteratorPtr&);
	public:
		explicit IteratorPtr(Iterator<ChildType>* iter) {
			iter_ = iter;
		}
		template<class U>
		explicit IteratorPtr(U* item) {
			static_assert(std::is_base_of_v<std::remove_pointer_t<ChildType>, U>, "Types are different.");
			iter_ = reinterpret_cast<Iterator<ChildType>*>(item->CreateIterator());
		}
		~IteratorPtr() {
			if constexpr (bDynamiclyAllocated) delete iter_;
		}
		Iterator<ChildType>* operator->() {
			return iter_;
		}
		ChildType operator*() {
			return iter_->CurrentItem();
		}
	};

	template <class T, bool bDynamiclyAllocated = false> 
	class PreorderIterator : public Iterator<T>
	{
	private:
		T _root;
		std::stack<Iterator<T>*, std::vector<Iterator<T>*>> iterators_;
		void First();
	public:
		PreorderIterator(T root);
		~PreorderIterator();
		virtual void Next();
		void NextSkipBranch() noexcept;
		virtual bool IsDone();
		// true - no elements left to traverse
		bool ExitBranch() noexcept;
		T CurrentItem();
	};

	template <class T, bool bDynamiclyAllocated>
	PreorderIterator<T, bDynamiclyAllocated>::PreorderIterator(T root) {
		_root = root;
		First();
	}
	template <class T, bool bDynamiclyAllocated>
	PreorderIterator<T, bDynamiclyAllocated>::~PreorderIterator() {
		while (!iterators_.empty()) {
			if constexpr (bDynamiclyAllocated) delete iterators_.top();
			iterators_.pop();
		}
	}
	template <class T, bool bDynamiclyAllocated>
	inline void PreorderIterator<T, bDynamiclyAllocated>::First() {
		Iterator<T>* iter_ptr = _root->CreateIterator();
		if (!iter_ptr->IsDone()) {
			iterators_.push(iter_ptr);
		} else {
			if constexpr (bDynamiclyAllocated) delete iter_ptr;
		}
	}
	template <class T, bool bDynamiclyAllocated>
	void PreorderIterator<T, bDynamiclyAllocated>::Next() {
		Iterator<T>* iter_ptr = iterators_.top()->CurrentItem()->CreateIterator();
		iterators_.push(iter_ptr);
		while (!iterators_.empty() && iterators_.top()->IsDone()) {
			if constexpr (bDynamiclyAllocated) delete iterators_.top();
			iterators_.pop();
			if (!iterators_.empty() && !iterators_.top()->IsDone()) {
				iterators_.top()->Next();
			}
		}
	}

	template <class T, bool bDynamiclyAllocated>
	void PreorderIterator<T, bDynamiclyAllocated>::NextSkipBranch() noexcept {
		while (!iterators_.empty()) {
			iterators_.top()->Next();
			if (iterators_.top()->IsDone()) {
				if constexpr (bDynamiclyAllocated) delete iterators_.top();
				iterators_.pop();
			} else {
				break;
			}
		}
	}

	template <class T, bool bDynamiclyAllocated>
	inline bool PreorderIterator<T, bDynamiclyAllocated>::IsDone() {
		return iterators_.empty();
	}

	template <class T, bool bDynamiclyAllocated>
	inline bool PreorderIterator<T, bDynamiclyAllocated>::ExitBranch() noexcept {
		auto size = iterators_.size();
		if (size) {
			if constexpr (bDynamiclyAllocated) delete iterators_.top();
			iterators_.pop();
		}
		return size == 1 ? true : false;
	}

	template <class T, bool bDynamiclyAllocated>
	T PreorderIterator<T, bDynamiclyAllocated>::CurrentItem() {
		assert(!iterators_.empty());
		return iterators_.top()->CurrentItem();
	}
	/*
	*****************************
	Postorder  (iterative)
	*****************************
	*/
	template <class T, bool bDynamiclyAllocated = false>
	class PostorderIterator : public Iterator<T>
	{
	private:
		T _root;
		std::stack<Iterator<T>*, std::vector<Iterator<T>*>> iterators_;
		void First();
	public:
		PostorderIterator(T root);
		~PostorderIterator();
		virtual void Next();
		virtual bool IsDone();
		T CurrentItem();
		virtual void Remove(T elem) { }
	};

	template <class T, bool bDynamiclyAllocated>
	PostorderIterator<T, bDynamiclyAllocated>::PostorderIterator(T root) {
		_root = root;
		First();
	}
	template <class T, bool bDynamiclyAllocated>
	PostorderIterator<T, bDynamiclyAllocated>::~PostorderIterator() {
		while (iterators_.size()) {
			if constexpr (bDynamiclyAllocated)
				delete iterators_.top();
			iterators_.pop();
		}
	}
	template <class T, bool bDynamiclyAllocated>
	inline void PostorderIterator<T, bDynamiclyAllocated>::First() {
		Iterator<T>* i = (Iterator<T>*)_root->CreateIterator();
		if (i->IsDone()) {
			if constexpr (bDynamiclyAllocated)
				delete i;
			return;
		}
		do {
			iterators_.push(i);
			i = (Iterator<T>*)i->CurrentItem()->CreateIterator();
		} while (!i->IsDone());
	}
	template <class T, bool bDynamiclyAllocated>
	void PostorderIterator<T, bDynamiclyAllocated>::Next() {
		iterators_.top()->Next();
		if (iterators_.top()->IsDone()) {
			if constexpr (bDynamiclyAllocated)
				delete iterators_.top();
			iterators_.pop();
			return;
		}
		Iterator<T>* i = (Iterator<T>*)iterators_.top()->CurrentItem()->CreateIterator();
		if (i->IsDone()) {
			if constexpr (bDynamiclyAllocated)
				delete i;
			return;
		}
		do {
			iterators_.push(i);
			i = (Iterator<T>*)i->CurrentItem()->CreateIterator();
		} while (!i->IsDone());
	}
	template <class T, bool bDynamiclyAllocated>
	inline bool PostorderIterator<T, bDynamiclyAllocated>::IsDone() {
		return iterators_.empty();
	}
	template <class T, bool bDynamiclyAllocated>
	T PostorderIterator<T, bDynamiclyAllocated>::CurrentItem() {
		assert(!iterators_.empty());
		return iterators_.top()->CurrentItem();
	}
}