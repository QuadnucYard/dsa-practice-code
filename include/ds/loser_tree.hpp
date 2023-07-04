#pragma once
#include <vector>
#include <algorithm>

namespace qy {

/// @brief A Loser Tree for external merge sort.
/// @tparam _Tp Value type.
/// @tparam _Nm Number of elements.
template <class _Tp>
class loser_tree {
	using value_type = _Tp;

public:
	/// @brief Construct empty tree by default.
	loser_tree(size_t size) : m_tree(size, 0), m_data(size, value_type{}) {}

	/// @brief Get the top element.
	/// @return The top element.
	const value_type& top() const { return m_data[m_tree[0]]; }

	/// @brief Push new value to the tree.
	/// @param value Data value.
	void push(const value_type& value) { push_at(value, m_tree[0]); }

	/// @brief Push new value to the tree at a specific position.
	/// @param value Data value.
	/// @param i Position.
	void push_at(const value_type& value, size_t i) {
		m_data[i] = value;
		adjust((i + m_data.size()) >> 1, i); // Adjust from parent
	}

	size_t size() const { return m_data.size(); }

	auto begin() const { return m_data.begin(); }

	auto end() const { return m_data.end(); }

private:
	/// @brief Maintain the loser tree.
	/// @param i Current node index.
	/// @param winner Upcoming winner index.
	void adjust(size_t i, size_t winner) {
		for (; i > 0; i >>= 1) {
			// If the upcoming winner is current loser, just swap.
			if (m_data[m_tree[i]] < m_data[winner]) {
				std::swap(m_tree[i], winner);
			}
		}
		m_tree[0] = winner;
	}

private:
	std::vector<size_t> m_tree;		// Loser/Winner indices in the tree.
	std::vector<value_type> m_data; // Stored data.
};

} // namespace qy