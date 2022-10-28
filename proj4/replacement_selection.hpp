#pragma once
#include "../common/fbufstream.hpp"
#include "../proj4/loser_tree.hpp"
#include <vector>

/// @brief Replacement selection algorithm for external merge sort to produce better initial merge segments.
/// @tparam T Value type.
template <class T>
class replacement_selection {
	using value_type = T;

public:

	replacement_selection(size_t buffer_size) : buffer_size(buffer_size) {}

	std::vector<size_t> operator()(const fs::path& input_path, const fs::path& output_path) {
		shared_ifile input_file(input_path);
		shared_ofile output_file(output_path);
		// It can use buffer featuring both input and output.
		async_iofbufstream<value_type> iobuf(buffer_size, input_file, output_file);
		// Build loser tree, and insert elements reversely.
		loser_tree<std::pair<int, value_type>> lt(buffer_size);
		for (ssize_t i = buffer_size - 1; i >= 0; i--) {
			if (!iobuf) { // If input data is not enough, supplement with virtual segs.
				lt.push_at({ 2, 0 }, i);
				continue;
			}
			value_type x;
			iobuf >> x;
			lt.push_at({ 1, x }, i);
		}
		// Get merge segments.
		std::vector<size_t> seg;
		for (int rc = 1, rmax = 1; rc <= rmax;) {
			size_t cnt = 0;
			while (lt.top().first == rc) { // While there still exists a record belonging to this round.
				int minimax = lt.top().second;
				if (!iobuf) {
					// When input EOF, add a virtual record in rmax+1 seg.
					lt.push({ rmax + 1, 0 });
				} else {
					// The input is not empty, then read in the next record.
					value_type x;
					iobuf >> x;
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
				iobuf << minimax;
				cnt++;
			}
			rc = lt.top().first; // Update rc. In fact, it just increment rc by 1.
			seg.push_back(cnt);
		}
		return seg;
	}

private:
	size_t buffer_size;
};