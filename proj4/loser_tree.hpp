#pragma once
#include <array>
#include <iostream>
#include <algorithm>
#include <ranges>

/// @brief A Loser Tree for external merge sort.
/// @tparam _Tp Value type.
/// @tparam _Nm Number of elements.
template <class _Tp, size_t _Nm>
class loser_tree {
	using value_type = _Tp;

public:
	/// @brief Construct empty tree by default. 
	loser_tree() {
		std::ranges::fill(m_tree, 0);
	}

	/// @brief Construct tree from iterator. The size of range should not less than _Nm.
	/// @tparam InputIt Input iterator type.
	/// @param first First iterator.
	/// @param last Last iterator.
	template <typename InputIt>
	loser_tree(InputIt first, InputIt last) : loser_tree(std::ranges::subrange(first, last)) {}

	/// @brief Construct tree from range. The size of range should not less than _Nm.
	/// @tparam _Range Input range type.
	/// @param _r Input range.
	template <std::ranges::input_range _Range>
	loser_tree(_Range&& _r) : loser_tree() {
		ptrdiff_t i = _Nm;
		for (auto&& x : _r) {
			push_at({ 1, x }, --i);
			if (i == 0) break;
		}
	}

	/// @brief Get the top element.
	/// @return The top element.
	const value_type& top() const {
		return m_data[m_tree[0]];
	}

	/// @brief Push new value to the tree.
	/// @param value Data value.
	void push(const value_type& value) {
		push_at(value, m_tree[0]);
	}

	/// @brief Push new value to the tree at a specific position.
	/// @param value Data value.
	/// @param i Position.
	void push_at(const value_type& value, size_t i) {
		m_data[i] = value;
		adjust((i + _Nm) >> 1, i); // Adjust from parent
	}

private:
	/// @brief Maintain the loser tree.
	/// @param i Current node index. 
	/// @param winner Upcoming winner index.
	void adjust(size_t i, size_t winner) {
		for (; i > 0; i >>= 1) {
			if (m_data[m_tree[i]] < m_data[winner]) { // 上来的胜者是败者
				std::swap(m_tree[i], winner);
			}
		}
		m_tree[0] = winner;
	}

private:
	std::array<size_t, _Nm> m_tree;			// Loser/Winner indices in the tree.
	std::array<value_type, _Nm> m_data;		// Stored data.
};