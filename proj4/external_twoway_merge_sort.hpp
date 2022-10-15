#pragma once
#include "replacement_selection.hpp"
#include "../common/fbufstream_iterator.hpp"
#include <algorithm>
#include <queue>


/// @brief External twoway merge sort implementation.
/// @tparam T Value type.
template <class T>
class external_twoway_merge_sorter {
	using value_type = T;

	/// @brief File segment with offset, pos, and file index.
	struct file_segment {
		size_t size;	// Segment length
		size_t pos;		// Segment offset
		size_t index;	// File index
		bool operator< (const file_segment& o) const { return size > o.size; }
	};

	/// @brief A struct with 3 buffers for merge run.
	struct buffer_group {
		ifbufstream<value_type, basic_buffer_tag> input_buf1;
		ifbufstream<value_type, basic_buffer_tag> input_buf2;
		ofbufstream<value_type, basic_buffer_tag> output_buf;
		buffer_group(size_t buffer_size) : input_buf1(buffer_size), input_buf2(buffer_size), output_buf(buffer_size) {}
	};

public:
	external_twoway_merge_sorter(size_t buffer_size) : buffer_size(buffer_size) {}

	void operator()(const std::filesystem::path& input_path, const std::filesystem::path& output_path) {
		this->input_path = input_path;
		this->output_path = output_path;
		segments = replacement_selection<value_type>(buffer_size)(input_path, get_merge_file(0)); // Call replacement selection
		merge();
	}

private:
	/// @brief Get the temporary merge file path by index.
	/// @param id File index.
	/// @return Path of merge file.
	fs::path get_merge_file(size_t id) {
		auto p = output_path;
		p.replace_filename(std::string("merge_") + std::to_string(id));
		return p;
	}

	/// @brief Merge segments.
	void merge() {
		// Get merge order.
		std::priority_queue<file_segment> ordseg;
		for (size_t sum = 0; size_t s : segments) {
			ordseg.push({ s, sum, 0 });
			sum += s;
		}
		// Start merge.
		buffer_group bufs(buffer_size);
		std::vector<std::pair<file_segment, file_segment>> merge_seq;
		size_t n = ordseg.size();
		for (size_t i = 1; i < n; i++) {
			file_segment s1 = ordseg.top(); ordseg.pop();
			file_segment s2 = ordseg.top(); ordseg.pop();
			ordseg.push({ s1.size + s2.size, 0, i });
			merge_run(bufs, i, s1, s2);
			merge_seq.push_back({ s1, s2 });
			// Remove used files.
			if (s1.index != 0) fs::remove(get_merge_file(s1.index));
			if (s2.index != 0) fs::remove(get_merge_file(s2.index));
		}
		// Finalize.
		fs::remove(get_merge_file(0)); // Remove the initial merge file.
		fs::rename(get_merge_file(n - 1), output_path); // Rename the last file to output file.
		best_merge_sequence = std::move(merge_seq);
	}

	/// @brief Merge file segments.
	void merge_run(buffer_group& b, size_t i, const file_segment& s1, const file_segment& s2) {
		b.input_buf1.open(get_merge_file(s1.index));
		b.input_buf2.open(get_merge_file(s2.index));
		b.output_buf.open(get_merge_file(i));
		b.input_buf1.seek(s1.pos, s1.pos + s1.size);
		b.input_buf2.seek(s2.pos, s2.pos + s2.size);
		std::merge(
			ifbufstream_iterator(b.input_buf1), ifbufstream_iterator<value_type>(),
			ifbufstream_iterator(b.input_buf2), ifbufstream_iterator<value_type>(),
			ofbufstream_iterator(b.output_buf)
		);
		b.input_buf1.close();
		b.input_buf2.close();
		b.output_buf.close();
	}

private:
	/// @brief Size of all buffers.
	size_t buffer_size;
	/// @brief Path of input file.
	fs::path input_path;
	/// @brief Path of output file.
	fs::path output_path;
	/// @brief Length of segments to be merged.
	std::vector<size_t> segments;
	/// @brief The best merge sequence.
	std::vector<std::pair<file_segment, file_segment>> best_merge_sequence;
};