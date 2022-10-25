#pragma once
#include "../common/base_sorter.hpp"
#include "../common/fbufstream_iterator.hpp"
#include <algorithm>
#include <cassert>


/// @brief External sorting implemented by merge sort
/// @tparam T Value type of sorted file
/// @tparam multithread Whether use multithread io. Default is false.
template <class T>
class external_merge_sorter : public base_sorter {

public:
	using value_type = T;

	/// @brief Constructor
	/// @param buffer_size Size of buffer elements.
	external_merge_sorter(size_t buffer_size) : base_sorter(buffer_size),
		input_buf1(buffer_size), input_buf2(buffer_size), output_buf(buffer_size) {}

	/// @brief Sort array in binary file.
	/// @param input_path Path of input file.
	/// @param output_path Path of output file.
	void operator()(const fs::path& input_path, const fs::path& output_path) {
		// Sort run: Sort all data by block
		input_buf1.open(input_path);
		output_buf.open(output_path);
		input_buf1.seek(0);
		size_t tot_size = input_buf1.size();
		std::vector<value_type> tmp(buffer_size);
		for (size_t i = 0; i < tot_size; i += buffer_size) {
			size_t n = std::min(tot_size - i, buffer_size);
			std::copy_n(ifbufstream_iterator(input_buf1), n, tmp.begin());
			std::stable_sort(tmp.begin(), tmp.begin() + n);
			std::copy_n(tmp.begin(), n, ofbufstream_iterator(output_buf));
		}
		input_buf1.close();
		output_buf.close();

		// Merge run

		fs::path pA = output_path, pB = output_path; // merge A to B
		pB.replace_filename(".tmp");
		// To keep continuity, let buf2 read from middle.
		for (size_t len = buffer_size; len < tot_size; len <<= 1) { // Length of each input way.
			size_t half = (tot_size + len - 1) / (len << 1) * len; // Middle position.
			input_buf1.open(pA), input_buf2.open(pA);
			output_buf.open(pB);
			input_buf1.seek(0, half), input_buf2.seek(half);
			for (size_t i = 0; i < half; i += len) {
				input_buf1.seek(i, i + len);
				input_buf2.seek(i + half, std::min(i + half + len, tot_size));
				std::merge(
					ifbufstream_iterator(input_buf1), ifbufstream_iterator<value_type>(),
					ifbufstream_iterator(input_buf2), ifbufstream_iterator<value_type>(),
					ofbufstream_iterator(output_buf)
				);
			}
			// Move rest data to output file if any
			if (half * 2 < tot_size) {
				input_buf2.seek(half * 2, tot_size);
				std::copy(ifbufstream_iterator(input_buf2), ifbufstream_iterator<value_type>(), ofbufstream_iterator(output_buf));
			}
			input_buf1.close();
			input_buf2.close();
			output_buf.close();
			// Swap output path
			std::swap(pA, pB);
		}
		if (pA == output_path) { // That's what we want, just remove temp
			fs::remove(pB);
		} else { // Else rename temp to output
			fs::rename(pA, pB);
		}
	}

private:

#ifdef DEBUG
	/// @brief Check whether sorting goes wrong. Only for each block.
	void validate(const std::filesystem::path& output_path, size_t len) {
		std::ifstream fin(output_path, std::ios_base::binary);
		fin.seekg(0, fin.end);
		size_t n = fin.tellg() / sizeof(T);
		printf("n=%lld\n", n);
		T* a = new T[n];
		fin.seekg(0, fin.beg);
		fin.read(reinterpret_cast<char*>(a), n * sizeof(T));
		for (size_t i = 0; i < n; i += len) {
			printf("validate %lld %lld\n", i, std::min(i + len, n));
			assert(std::is_sorted(a + i, a + std::min(i + len, n)));
		}
		delete[] a;
	}
#endif

private:
	ifbufstream<value_type, basic_buffer_tag> input_buf1;
	ifbufstream<value_type, basic_buffer_tag> input_buf2;
	ofbufstream<value_type, basic_buffer_tag> output_buf;
};