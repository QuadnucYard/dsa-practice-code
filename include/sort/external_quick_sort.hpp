#pragma once
#include "./base_sorter.hpp"
#include "bufio/arraybuf.hpp"
#include "ds/interval_heap.hpp"
#include <algorithm>
#include <cassert>

namespace qy {

/// @brief External sorting implemented by quick sort
/// @tparam T Value type of sorted file
/// @tparam buffer_size Buffer size, aka number of buffer elements
template <class T>
class external_quick_sorter : public base_sorter {
	using buffer_type = arraybuf<T>;

	constexpr static size_t value_size = sizeof(T);

public:
	using value_type = T;

	/// @brief Constructor
	/// @param tmp_path Path of temporary file.
	external_quick_sorter(size_t buffer_size, size_t heap_size) :
		base_sorter(buffer_size),
		heap_size(heap_size),
		input_buf(buffer_size),
		small_buf(buffer_size),
		large_buf(buffer_size, true) {}

	external_quick_sorter(size_t buffer_size) :
		base_sorter(buffer_size),
		input_buf(buffer_size),
		small_buf(buffer_size),
		large_buf(buffer_size, true) {}

	/// @brief Sort array in binary file.
	/// @param input_path Path of input file.
	/// @param output_path Path of output file.
	void operator()(const fs::path& input_path, const fs::path& output_path) {
		// Open files
		ftemp.open(output_path, std::ios_base::binary | std::ios_base::in | std::ios_base::out);
		finput.open(input_path, std::ios_base::binary | std::ios_base::in);
		foutput.open(output_path, std::ios_base::binary | std::ios_base::in | std::ios_base::out |
									  std::ios_base::trunc);
		assert(ftemp.is_open());
		assert(finput.is_open());
		assert(foutput.is_open());
		// Get input size
		size_t src_size = fs::file_size(input_path);
		fs::resize_file(output_path, src_size);
		// Bind buffer to stream
		input_buf.bind(&finput);
		small_buf.bind(&foutput);
		large_buf.bind(&ftemp);
#ifdef LOGGING
		clear_log();
		m_log["rec"] = 0;
		m_log["heap_size"] = heap_size;
#endif
		// Perform sorting
		_sort(0, src_size / value_size, true);
		// End with closing stream
		finput.close();
		foutput.close();
#ifdef LOGGING
		m_log["input"] = input_buf.get_log();
		m_log["small"] = small_buf.get_log();
		m_log["large"] = large_buf.get_log();
#endif
	}

private:
	void _sort(size_t first, size_t last, bool initial = false) {
		if (first >= last)
			return;
		// Fill middle group
		size_t input_size = std::min(last - first, buffer_size);
		input_buf.load(first, input_size);
		middle_heap = interval_heap<T>(input_buf.begin(), input_buf.begin() + input_size);
		//middle_heap.validate();
		size_t cur = first + input_size;
		while (cur < last && cur - first < heap_size) { // Read another run to make room for output.
			input_size = std::min(last - cur, buffer_size);
			input_buf.load(cur, input_size);
			for (size_t i = 0; i < input_size; i++) {
				middle_heap.push(input_buf[i]);
			}
			cur += input_size;
		}

		// Now middle group is full. Prepare for IO
		large_buf.seekp(last);
		small_buf.seekp(first);
		size_t mid1 = first, mid2 = last, cur2 = last;
		// Repeat until read in all input data
		// It read data from both sides as needed.
		while (cur < cur2) {
			// 这边还要检查是否加载过多
			if (cur + buffer_size >= cur2 || cur - mid1 <= mid2 - cur2) {
				input_size = std::min(buffer_size, cur2 - cur);
				input_buf.load(cur, input_size);
				cur += input_size;
			} else {
				input_size = buffer_size;
				cur2 -= input_size;
				input_buf.load(cur2, input_size);
			}
			for (size_t i = 0; i < input_size; i++) {
				T value = input_buf[i];
				if (value <= middle_heap.top_min()) {
					small_buf << value;
					++mid1;
				} else if (value >= middle_heap.top_max()) {
					large_buf << value;
					--mid2;
				} else {
					// Replace the minimum element in middle group
					small_buf << middle_heap.top_min();
					++mid1;
					middle_heap.pop_min();
					middle_heap.push(value);
				}
#ifdef DEBUG
				if (cur != cur2 && (mid1 > cur || mid2 < cur2)) {
					throw std::logic_error("xxx");
				}
#endif
			}
		}
		// Write back all data in buffer
		//small_buf.dump();
		large_buf.dump();

#ifdef DEBUG
		assert(first <= mid1);
		assert(mid1 <= mid2);
		assert(mid2 <= last);
#endif
		// Now middle follows small. Write back directly.
		for (size_t i = mid1; i < mid2; i++) {
			small_buf << middle_heap.top_min();
			middle_heap.pop_min();
		}
		small_buf.dump();

#ifdef DEBUG
		validate(first, mid1, mid2, last);
#endif
		// Rebind input buffer to output file after the first run
		if (initial)
			input_buf.bind(&foutput);
#ifdef LOGGING
		this->jinc("rec");
#endif
		_sort(first, mid1);
		_sort(mid2, last);
	}

#ifdef DEBUG
	/// @brief Check whether sorting goes wrong
	void validate(size_t first, size_t mid1, size_t mid2, size_t last) {
		size_t n = last - first;
		T* a = new T[n];
		foutput.seekg(first * value_size);
		foutput.read(reinterpret_cast<char*>(a), n * value_size);
		bool f1 = std::is_sorted(a + mid1 - first, a + mid2 - first);
		if (!f1) {
			std::cerr << first << " " << mid1 << " " << mid2 << " " << last << std::endl;
		}
		// if (n < 256) {
		// 	std::cout << "val: ";
		// 	for (int i = 0; i < mid1 - first; i++) std::cout << a[i] << " ";
		// 	std::cout << "| ";
		// 	for (int i = mid1 - first; i < mid2 - first; i++) std::cout << a[i] << " ";
		// 	std::cout << "| ";
		// 	for (int i = mid2 - first; i < last - first; i++) std::cout << a[i] << " ";
		// 	std::cout << "\n";
		// }
		assert(f1);
		delete[] a;
	}
#endif

private:
	size_t heap_size;
	std::fstream finput;		  // Input file stream
	std::fstream foutput;		  // Output file stream
	std::fstream ftemp;			  // Temp file stream
	interval_heap<T> middle_heap; // Heap (depq) for middle group
	buffer_type input_buf;		  // Buffer for input, bound input or output file
	buffer_type small_buf;		  // Buffer for small, bound output file
	buffer_type large_buf;		  // Buffer for large, bound temp file
};

} // namespace qy