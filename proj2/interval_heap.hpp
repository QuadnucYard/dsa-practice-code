#pragma once
#include <iostream>
#include <vector>
#include <ranges>
#include <cassert>

/**
 * @brief Double-end priority queue implemented by interval heap
 *
 * @tparam T Value type
 */
template <class T>
class interval_heap {
public:
	using value_type = T;
	using pointer = T*;
	using reference = T&;
	using const_reference = const T&;

	interval_heap() = default;

	/**
	 * @brief Construct a new interval heap object from initialize list
	 *
	 * @param il Initialize list
	 */
	interval_heap(std::initializer_list<T> il) : m_data(il) {
		_make_heap();
	}

	/**
	 * @brief Construct a new interval heap object from iterator pair
	 *
	 * @tparam RandomIt Random access iterator
	 * @param first First iterator
	 * @param last Last iterator
	 */
	template <class RandomIt>
	interval_heap(RandomIt first, RandomIt last) : m_data(first, last) {
		_make_heap();
	}

	/**
	 * @brief Construct a new interval heap object from range
	 *
	 * @tparam Range random_access_range
	 * @param r Random access range
	 */
	template <std::ranges::random_access_range Range>
	interval_heap(Range r) : m_data(r) {
		_make_heap();
	}

	/**
	 * @brief Get whether heap is empty
	 */
	bool empty() const noexcept { return m_data.empty(); }

	/**
	 * @brief Get heap size
	 *
	 * @return size_t Size of this heap in elements
	 */
	size_t size() const noexcept { return m_data.size(); }

	// template <typename Self>
	// constexpr auto begin(this Self&& self) { return m_data.begin(); }

	// template <typename Self>
	// constexpr auto end(this Self&& self) { return m_data.end(); }

	constexpr auto begin() noexcept { return m_data.begin(); }

	constexpr auto begin() const noexcept { return m_data.cbegin(); }

	constexpr auto end() noexcept { return m_data.end(); }

	constexpr auto end() const noexcept { return m_data.cend(); }

	/**
	 * @brief Get minimum element
	 *
	 * @return const_reference Minimum element
	 */
	const_reference top_min() const {
		size_t i = m_data.size();
		if (i == 0) throw std::out_of_range("Heap empty!");
		return m_data[0];
	}

	/**
	 * @brief Get maximum element
	 *
	 * @return const_reference Maximum element
	 */
	const_reference top_max() const {
		size_t i = m_data.size();
		if (i == 0) throw std::out_of_range("Heap empty!");
		else if (i == 1) return m_data[0];
		else return m_data[1];
	}

	/**
	 * @brief Push value into heap
	 *
	 * @param x Value to be pushed
	 */
	void push(const_reference x) {
		size_t i = m_data.size();
		if ((i & 1) && x < m_data.back()) { // Need swap
			auto t = std::move(m_data.back());
			m_data.back() = x;
			m_data.push_back(std::move(t));
			i--;
		} else {
			m_data.push_back(x);
		}
		if (i & 1) { // Large
			_push_heap(i, x, std::less());
		} else { // Small
			_push_heap(i, x, std::greater());
		}
	}

	/**
	 * @brief Pop minimum value from heap
	 */
	void pop_min() {
		size_t i = m_data.size();
		if (i == 0) throw std::out_of_range("Heap empty!");
		else if (i == 1) {
			m_data.pop_back();
			return;
		}
		_pop_heap(0, i - 1, std::greater());
	}

	/**
	 * @brief Pop maximum value from heap
	 */
	void pop_max() {
		size_t i = m_data.size();
		if (i == 0) throw std::out_of_range("Heap empty!");
		else if (i == 1) {
			m_data.pop_back();
			return;
		}
		_pop_heap(1, i - 1, std::less());
	}

	/**
	 * @brief Check whether the heap is valid using assert
	 */
	void validate() {
		// Interval validation
		for (size_t i = 1; i < m_data.size(); i += 2) {
			assert(!(m_data[i] < m_data[i - 1]));
		}
		// Inclusive relation
		for (size_t i = 2; i < m_data.size(); i++) {
			if (i & 1) { // Large
				assert(!(m_data[parent(i)] < m_data[i]));
			} else {
				assert(!(m_data[parent(i)] > m_data[i]));
			}
		}
	}

private:

	inline static size_t left_child(size_t i) noexcept { return (i << 1) + 2 - (i & 1); }
	inline static size_t right_child(size_t i) noexcept { return (i << 1) + 4 - (i & 1); }
	inline static size_t parent(size_t i) noexcept { return i - (i >> 2 << 1) - 2; }

	template<class Comp>
	void _push_heap(size_t holeIndex, const_reference value, Comp cmp) {
		size_t parentIndex = parent(holeIndex);
		while (holeIndex > 1 && cmp(m_data[parentIndex], value)) {
			m_data[holeIndex] = std::move(m_data[parentIndex]);
			holeIndex = parentIndex;
			parentIndex = parent(holeIndex);
		}
		m_data[holeIndex] = std::move(value);
	}

	template<class Comp>
	void _adjust_heap(size_t holeIndex, value_type&& value, Comp cmp) {
		size_t len = m_data.size();
		size_t firstChild = left_child(holeIndex);
		while (firstChild < len) {
			size_t secondChild = right_child(holeIndex);
			if (secondChild < len && cmp(m_data[firstChild], m_data[secondChild]))
				firstChild = secondChild;
			if (cmp(value, m_data[firstChild])) {
				m_data[holeIndex] = std::move(m_data[firstChild]);
				if (cmp(value, m_data[holeIndex ^ 1])) {
					std::swap(value, m_data[holeIndex ^ 1]);
				}
				holeIndex = firstChild;
				firstChild = left_child(holeIndex);
			} else break;

		}
		m_data[holeIndex] = std::move(value);
		if ((holeIndex ^ 1) < len && cmp(value, m_data[holeIndex ^ 1])) {
			std::swap(m_data[holeIndex], m_data[holeIndex ^ 1]);
		}
	}

	template<class Comp>
	void _pop_heap(size_t topIndex, size_t resultIndex, Comp cmp) {
		value_type value = std::move(m_data[resultIndex]);
		m_data.pop_back();
		_adjust_heap(topIndex, std::move(value), cmp);
	}

	void _make_heap() {
		const size_t len = m_data.size();
		// First order the elements in the root
		for (ptrdiff_t parent = len - 1; parent > 0; --parent) {
			if ((parent & 1) && m_data[parent] < m_data[parent - 1]) {
				std::swap(m_data[parent], m_data[parent - 1]);
			}
		}
		// Then reinsert the points
		for (ptrdiff_t parent = ((len + 2) >> 2 << 1) - 1; parent >= 0; --parent) {
			value_type value = std::move(m_data[parent]);
			if (parent & 1) {
				_adjust_heap(parent, std::move(value), std::less());
			} else {
				_adjust_heap(parent, std::move(value), std::greater());
			}
		}
	}

private:
	std::vector<value_type> m_data;
};