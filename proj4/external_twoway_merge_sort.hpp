#pragma once
#include "replacement_selection.hpp"
#include "../common/base_sorter.hpp"
#include "../common/fbufstream_iterator.hpp"
#include <algorithm>
#include <queue>
#include "../common/futils.hpp"


/// @brief External twoway merge sort implementation.
/// @tparam T Value type.
template <class T>
class external_twoway_merge_sorter : public base_sorter {
	
public:
	using value_type = T;

private:
	/// @brief File segment with offset, pos, and file index.
	struct file_segment {
		size_t size;	// Segment length
		size_t pos;		// Segment offset
		bool operator< (const file_segment& o) const { return size > o.size; }
	};

public:

	using base_sorter::base_sorter;

	void operator()(const std::filesystem::path& input_path, const std::filesystem::path& output_path) {
		this->input_path = input_path;
		this->output_path = output_path;
		segments = replacement_selection<value_type>(buffer_size)(input_path, output_path); // Call replacement selection
		merge();
	}

private:

	/// @brief Merge segments.
	void merge() {
		// Get merge order.
		std::priority_queue<file_segment> ordseg;
		size_t sum = 0;
		for (size_t s : segments) {
			ordseg.push({ s, sum });
			sum += s;
		}print_binary_file<value_type>(output_path);
		// Start merge. To reduce number of files, just append all data to the end of output,
		// and write back to begin in the last merge.
		shared_ifile input_file(output_path);
		shared_ofile output_file(output_path, false);
		ifbufstream<value_type, double_buffer_tag> input_buf1(buffer_size);
		ifbufstream<value_type, double_buffer_tag> input_buf2(buffer_size);
		ofbufstream<value_type, double_buffer_tag> output_buf(buffer_size, output_file);
		output_buf.seek(sum);

		std::vector<std::pair<file_segment, file_segment>> merge_seq;
		size_t n = ordseg.size();
		for (size_t i = 1, offset = sum; i < n; i++) {
			file_segment s1 = ordseg.top(); ordseg.pop();
			file_segment s2 = ordseg.top(); ordseg.pop();
			fmt::print("i={}\n", i);
			input_buf1.open(input_file); input_buf2.open(input_file);
			input_buf1.seek(s1.pos, s1.pos + s1.size);
			input_buf2.seek(s2.pos, s2.pos + s2.size);
			// 一个严重的问题：  不应有的0被读进来了
			if (i == n - 1) output_buf.wait(), output_buf.seek(0); // 这里要特别处理一下，
			//print_binary_file<value_type>(output_path);
			std::merge(
				ifbufstream_iterator(input_buf1), ifbufstream_iterator<value_type>(),
				ifbufstream_iterator(input_buf2), ifbufstream_iterator<value_type>(),
				ofbufstream_iterator(output_buf)
			);
			output_buf.wait();
			ordseg.push({ s1.size + s2.size, offset });
			offset += s1.size + s2.size;
			merge_seq.push_back({ s1, s2 });
			input_buf1.close();
			input_buf2.close();
		}
		output_buf.close();
		output_file.close();
		// Finalize.
		print_binary_file<value_type>(output_path);
		fs::resize_file(output_path, sum * sizeof(value_type));
		best_merge_sequence = std::move(merge_seq);
	}

private:
	/// @brief Path of input file.
	fs::path input_path;
	/// @brief Path of output file.
	fs::path output_path;
	/// @brief Length of segments to be merged.
	std::vector<size_t> segments;
	/// @brief The best merge sequence.
	std::vector<std::pair<file_segment, file_segment>> best_merge_sequence;
};