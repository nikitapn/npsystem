// Copyright (c) 2021 nikitapnn1@gmail.com
// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory

#pragma once

#include <nplib/utils/helper.h>
#include <boost/container/small_vector.hpp>

class CElement;

template<typename num_type>
struct TQuadrantPoint {
	num_type x;
	num_type y;
};

template<typename num_type>
struct TQuadrantRect {
	num_type left;
	num_type top;
	num_type right;
	num_type bottom;
	num_type width() const { return right - left; }
	num_type height() const { return bottom - top; }
};

template<typename T, typename num_type>
struct QuadrantAdapter;

struct QuadrantAllocator {
	boost::pool<> pool_nodes;
	boost::pool<> pool_arrays;

	QuadrantAllocator(size_t size_of_q_node, size_t size_of_items_array) 
		: pool_nodes(size_of_q_node, 16)
		, pool_arrays(size_of_items_array, 32)
	{
	}
};

template<typename Ty, typename num_type = float, size_t _MAX_ELEMENTS_IN_SQUARE_ = 10>
class QuadrantNode {
public:
	static constexpr size_t MAX_ELEMENTS_IN_SQUARE = _MAX_ELEMENTS_IN_SQUARE_;
	using Node = QuadrantNode<Ty, num_type, MAX_ELEMENTS_IN_SQUARE>;
	using Iterator = Iter::Iterator<Node*>;
	using QuadrantRect = TQuadrantRect<num_type>;
	using QuadrantPoint = TQuadrantPoint<num_type>;
private:
	typedef QuadrantAdapter<Ty, num_type> _adapter;
	typedef Ty* _Ptr;

	size_t next_ix;
	_Ptr *items_;

	Node* childs[4];
	Node*& ne = childs[0];
	Node*& nw = childs[1];
	Node*& se = childs[2];
	Node*& sw = childs[3];

	QuadrantRect _area;
	QuadrantPoint _middle;
	QuadrantAllocator& alloc_;

	void __forceinline _InsertElement(_Ptr pElem) {
		const QuadrantRect& rc = _adapter::GetBounds(pElem);
		if (rc.top > _middle.y) {
			if (rc.left > _middle.x) {
				se->AddElement(pElem);
			} else {
				sw->AddElement(pElem);
				if (rc.right > _middle.x)
					se->AddElement(pElem);
			}
		} else {
			if (rc.left > _middle.x) {
				ne->AddElement(pElem);
				if (rc.bottom > _middle.y)
					se->AddElement(pElem);
			} else {
				nw->AddElement(pElem);
				if (rc.bottom > _middle.y)
					sw->AddElement(pElem);
				if (rc.right > _middle.x) {
					ne->AddElement(pElem);
					if (rc.bottom > _middle.y)
						se->AddElement(pElem);
				}
			}
		}
	}

	void __forceinline _DeleteElement(_Ptr pElem) {
		const QuadrantRect& rc = _adapter::GetBounds(pElem);
		if (rc.top > _middle.y) {
			if (rc.left > _middle.x) {
				se->DeleteElement(pElem);
			} else {
				sw->DeleteElement(pElem);
				if (rc.right > _middle.x)
					se->DeleteElement(pElem);
			}
		} else {
			if (rc.left > _middle.x) {
				ne->DeleteElement(pElem);
				if (rc.bottom > _middle.y)
					se->DeleteElement(pElem);
			} else {
				nw->DeleteElement(pElem);
				if (rc.bottom > _middle.y)
					sw->DeleteElement(pElem);
				if (rc.right > _middle.x) {
					ne->DeleteElement(pElem);
					if (rc.bottom > _middle.y)
						se->DeleteElement(pElem);
				}
			}
		}
	}
public:
	QuadrantNode(const QuadrantRect& area, QuadrantAllocator& alloc)
		: next_ix(0)
		, items_(nullptr)
		, _area(area)
		, alloc_(alloc) 
	{ 
	}

	void AddElement(_Ptr element) {
		if (next_ix == -1) {
			_InsertElement(element);
		} else if (next_ix < MAX_ELEMENTS_IN_SQUARE) {
			if (items_ == nullptr) {
				items_ = static_cast<_Ptr*>(alloc_.pool_arrays.malloc());
			}
			nplib::hlp::insert_sort_asc<MAX_ELEMENTS_IN_SQUARE>(&items_[0], &items_[0] + next_ix, element, [](auto a, auto b) {
				return _adapter::GetZOrder(a) > _adapter::GetZOrder(b);
				});
			next_ix++;
		} else {
			next_ix = -1;
			
			_middle.x = _area.left + (_area.right - _area.left) / 2;
			_middle.y = _area.top + (_area.bottom - _area.top) / 2;

			nw = new (alloc_.pool_nodes.malloc()) Node({ _area.left, _area.top, _middle.x, _middle.y }, alloc_);
			ne = new (alloc_.pool_nodes.malloc()) Node({_middle.x, _area.top, _area.right, _middle.y}, alloc_);
			sw = new (alloc_.pool_nodes.malloc()) Node({_area.left, _middle.y, _middle.x, _area.bottom}, alloc_);
			se = new (alloc_.pool_nodes.malloc()) Node({ _middle.x, _middle.y, _area.right, _area.bottom}, alloc_);

			for (size_t i = 0; i < MAX_ELEMENTS_IN_SQUARE; ++i) {
				_InsertElement(items_[i]);
			}
			_InsertElement(element);

			alloc_.pool_arrays.free(items_);
			items_ = nullptr;
		}
	}

	template<typename... Args>
	_Ptr FindElement(num_type x, num_type y, Args&&... args) const {
		if (next_ix == -1) {
			if (y < _middle.y) {
				if (x > _middle.x) return ne->FindElement(x, y, std::forward<Args>(args)...);
				else return nw->FindElement(x, y, std::forward<Args>(args)...);
			} else {
				if (x > _middle.x) return se->FindElement(x, y, std::forward<Args>(args)...);
				else return sw->FindElement(x, y, std::forward<Args>(args)...);
			}
		} else {
			for (size_t i = next_ix - 1; i != std::numeric_limits<size_t>::max(); --i) {
				if (_adapter::IsHitAccess(items_[i]) && _adapter::HitTest(items_[i], x, y, std::forward<Args>(args)...))
					return items_[i];
			}
		}
		return nullptr;
	}

	void DeleteElement(_Ptr pElem) {
		if (next_ix == -1) {
			_DeleteElement(pElem);
		} else {
			for (size_t i = 0; i < next_ix; ++i) {
				if (pElem == items_[i]) {
					for (size_t j = i; j < next_ix - 1; ++j) items_[j] = items_[j + 1];
					next_ix--;
					break;
				}
			}
		}
	}

	template<typename Context>
	void Draw(const Context& ctx, const QuadrantRect& vArea, int render_frame) {
		if (_area.left > vArea.right || _area.right < vArea.left ||
			_area.top > vArea.bottom || _area.bottom < vArea.top) {
			return;
		}

		if (next_ix == -1) {
			nw->Draw(ctx, vArea, render_frame);
			ne->Draw(ctx, vArea, render_frame);
			sw->Draw(ctx, vArea, render_frame);
			se->Draw(ctx, vArea, render_frame);
		} else {
			for (size_t i = 0; i < next_ix; ++i) {
				auto element = items_[i];
				if (_adapter::IsRendered(element, render_frame)) continue;
				
				const QuadrantRect& rc = _adapter::GetBounds(element);
				if (rc.left > vArea.right || rc.right < vArea.left ||
					rc.top > vArea.bottom || rc.bottom < vArea.top) {
					continue;
				}

				_adapter::Draw(element, ctx);
				_adapter::SetRenderedState(element, render_frame);
			}
		}
	}

	const QuadrantRect& GetBounds() const noexcept {
		return _area;
	}

	Iterator* CreateIterator() {
		if (next_ix == -1) {
			return new Iter::ArrayIterator<Node*>(childs, 4);
		} else {
			return new Iter::NullIterator<Node*>();
		}
		return nullptr;
	}
};