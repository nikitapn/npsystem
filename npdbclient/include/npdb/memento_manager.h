#pragma once

#include "memento.h"
#include "base.h"
#include <functional>
#include <unordered_map>

namespace odb {

template<class T>
class MementoManager {
public:
	template<typename U>
	void AddMemento(U* obj) noexcept {
		assert(obj);
		static_assert(std::is_base_of_v<T, U>, "Type U doesn't support this type of memento");
		data_.push_back(std::move(
			static_cast<T*>(obj)->CreateMemento()
		));
	}
	
	void RestoreAll() const noexcept {
		for (auto& m : data_) m->Restore();
	}
	
	void SaveCurrentState() noexcept {
		data_saved_ = data_;
	}

	void RestoreSavedState() noexcept {
		data_ = data_saved_;
	}
protected:
	std::vector<std::shared_ptr<IMemento>> data_;
	std::vector<std::shared_ptr<IMemento>> data_saved_;
};

class NodeHolder {
public:
	template<typename Node>
	void AddNode(Node& n) noexcept {
		assert(n.loaded());
		nodes_.emplace((std::shared_ptr<void>)n, std::bind(&Node::store_impl_copy_ptr, n.data_));
	}
	
	void StoreAll() {
		for (auto& n : nodes_) n.second();
	}

	NodeHolder& operator +=(NodeHolder&& other) noexcept {
		std::copy(std::make_move_iterator(other.nodes_.begin()), std::make_move_iterator(other.nodes_.end()), std::inserter(nodes_, nodes_.end()));
		return *this;
	}
private:
	std::unordered_map<std::shared_ptr<void>, std::function<bool()>> nodes_;
};

template<class T>
class NodeMementoManager : public MementoManager<T> {
	using Base = MementoManager<T>;
public:
	struct Data {
		std::unique_ptr<IMemento> memento;
		std::function<bool()> store;

		Data(std::unique_ptr<IMemento>&& m, std::function<bool()>&& s)
			: memento(std::move(m))
			, store(std::move(s)) {}
	};

	NodeMementoManager(NodeHolder& holder)
		: holder_(holder) {}

	template<typename Node, typename U>
	void AddMemento(Node& n, U* u) noexcept {
		assert(u);
		static_assert(std::is_base_of_v<T, U>, "Type U doesn't support this type of memento");
		holder_.AddNode(n);
		MementoManager<T>::AddMemento(u);
	}

	template<typename Node>
	std::enable_if_t<!std::is_base_of_v<odb::detail::basic_list_abstract, Node>> AddMemento(Node& n) noexcept {
		static_assert(std::is_base_of_v<T, typename Node::type_t>, "Node doesn't support this type of memento");
		holder_.AddNode(n);
		MementoManager<T>::AddMemento(n.get());
	}

	template<typename List>
	std::enable_if_t<std::is_base_of_v<odb::detail::basic_list_abstract, List>> AddMemento(List& l) noexcept {
		l.init();
		holder_.AddNode(l.node());
		auto ptr = static_cast<MementoCommonSupport*>(l.node().get());
		Base::data_.push_back(std::move(
			std::move(ptr->CreateMemento())
		));
	}

	NodeMementoManager& operator +=(NodeMementoManager&& other) noexcept {
		std::copy_if(std::make_move_iterator(other.Base::data_.begin()), std::make_move_iterator(other.Base::data_.end()), std::inserter(Base::data_, Base::data_.end()), [this](auto const& pair) {
			return Base::data_.find(pair.first) == Base::data_.end();
		});
		return *this;
	}
private:
	NodeHolder& holder_;
};

using NodeListMementoManager = NodeMementoManager<MementoCommonSupport>;

} // namespace odb