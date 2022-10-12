#pragma once
#include "../common/fbufstream.hpp"
#include "../common/fbufstream_iterator.hpp"
#include "loser_tree.hpp"
#include <algorithm>
#include <vector>
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
		repsel_sort();
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

	/// @brief Get merge segments by replace-selection sort.
	void repsel_sort() {
		// Suppose the file contains more elements than buffer size.
		// It can use buffer featuring both input and output.
		ifbufstream<value_type, double_buffer_tag> input_buf(buffer_size, input_path);
		ofbufstream<value_type, double_buffer_tag> output_buf(buffer_size, get_merge_file(0));
		input_buf.seek(0);
		// Build loser tree, and insert elements reversely.
		loser_tree<std::pair<int, value_type>> lt(buffer_size);
		for (ssize_t i = buffer_size - 1; i >= 0; i--) {
			if (!input_buf) { // If input data is not enough, supplement with virtual segs.
				lt.push_at({ 2, 0 }, i);
				continue;
			}
			value_type x;
			input_buf >> x;
			lt.push_at({ 1, x }, i);
		}
		// Get merge segments.
		std::vector<size_t> seg;
		for (int rc = 1, rmax = 1; rc <= rmax;) {
			size_t cnt = 0;
			while (lt.top().first == rc) { // While there still exists a record belonging to this round.
				int minimax = lt.top().second;
				if (!input_buf) {
					// When input EOF, add a virtual record in rmax+1 seg.
					lt.push({ rmax + 1, 0 });
				} else {
					// The input is not empty, then read in the next record.
					value_type x;
					input_buf >> x;
					if (x < minimax) {
						// If the new record is less than the minimal element in the last round, it belongs to the next round.
						rmax = rc + 1;
						lt.push({ rmax, x });
					} else {
						// Else it belongs to this round.
						lt.push({ rc, x });
					}
				}
				// Output the minimax and increment counter. 
				output_buf << minimax;
				cnt++;
			}
			rc = lt.top().first; // Update rc. In fact, it just increment rc by 1.
			seg.push_back(cnt);
		}
		segments = std::move(seg);
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