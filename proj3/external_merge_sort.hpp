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

	using base_sorter::base_sorter;

	/// @brief Sort array in binary file.
	/// @param input_path Path of input file.
	/// @param output_path Path of output file.
	void operator()(const fs::path& input_path, const fs::path& output_path) {

		// Sort run: Sort all data by block
		shared_ifile input_file(input_path);
		shared_ofile output_file(output_path);

		ifbufstream<value_type, basic_buffer_tag> input_buf1(buffer_size);
		ifbufstream<value_type, basic_buffer_tag> input_buf2(buffer_size);
		ofbufstream<value_type, basic_buffer_tag> output_buf(buffer_size);
		input_buf1.open(input_file);
		output_buf.open(output_file);
		input_buf1.seek(0);

		size_t tot_size = input_buf1.size();
		std::vector<value_type> tmp(buffer_size);
		for (size_t i = 0; i < tot_size; i += buffer_size) {
			size_t n = std::min(tot_size - i, buffer_size);
			std::copy_n(ifbufstream_iterator(input_buf1), n, tmp.begin());
			std::stable_sort(tmp.begin(), tmp.begin() + n);
			std::copy_n(tmp.begin(), n, ofbufstream_iterator(output_buf));
		}
		input_buf1.close(); output_buf.close();
		input_file.close(); output_file.close();

		// Merge run

		fs::path pA = output_path, pB = output_path; // merge A to B
		pB.replace_filename(".tmp");
		// To keep continuity, let buf2 read from middle.
		for (size_t len = buffer_size; len < tot_size; len <<= 1) { // Length of each input way.
			size_t half = (tot_size + len - 1) / (len << 1) * len; // Middle position.
			input_file.open(pA); output_file.open(pB);
			input_buf1.open(input_file); input_buf2.open(input_file);
			output_buf.open(output_file);
			//input_buf1.seek(0, half), input_buf2.seek(half);
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
			input_buf1.close(); input_buf2.close(); output_buf.close();
			input_file.close(); output_file.close();
			// Swap output path
			std::swap(pA, pB);
		}
		if (pA == output_path) { // That's what we want, just remove temp
			fs::remove(pB);
		} else { // Else rename temp to output
			fs::rename(pA, pB);
		}
	}

};