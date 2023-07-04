#pragma once
#include "./base_sorter.hpp"
#include "bufio/fbufstream.hpp"
#include "ds/loser_tree.hpp"
#include <vector>

namespace qy {

/// @brief Replacement selection algorithm for external merge sort to produce better initial merge segments.
/// @tparam T Value type.
template <class T>
class replacement_selection : public base_sorter {
	using value_type = T;

public:
	replacement_selection(size_t buffer_size, size_t loser_size) :
		base_sorter(buffer_size), loser_size(loser_size) {}

	replacement_selection(size_t buffer_size) : replacement_selection(buffer_size, buffer_size) {}

	std::vector<size_t> operator()(const fs::path& input_path, const fs::path& output_path) {
		// It can use buffer featuring both input and output.
		async_iofbufstream<value_type> iobuf(buffer_size, input_path, output_path);
		// Build loser tree, and insert elements reversely.
		loser_tree<std::pair<int, value_type>> lt(loser_size);
		for (ssize_t i = loser_size - 1; i >= 0; i--) {
			if (iobuf.ieof()) { // If input data is not enough, supplement with virtual segs.
				lt.push_at({2, 0}, i);
				continue;
			}
			value_type x;
			iobuf >> x;
			lt.push_at({1, x}, i);
		}
		// Get merge segments.
		std::vector<size_t> seg;
		for (int rc = 1, rmax = 1; rc <= rmax;) {
			size_t cnt = 0;
			while (lt.top().first ==
				   rc) { // While there still exists a record belonging to this round.
				int minimax = lt.top().second;
				if (iobuf.ieof()) {
					// When input EOF, add a virtual record in rmax+1 seg.
					lt.push({rmax + 1, 0});
				} else {
					// The input is not empty, then read in the next record.
					value_type x;
					iobuf >> x;
					if (x < minimax) {
						// If the new record is less than the minimal element in the last round, it belongs to the next round.
						rmax = rc + 1;
						lt.push({rmax, x});
					} else {
						// Else it belongs to this round.
						lt.push({rc, x});
					}
				}
				// Output the minimax and increment counter.
				iobuf << minimax;
				cnt++;
			}
			rc = lt.top().first; // Update rc. In fact, it just increment rc by 1.
			seg.push_back(cnt);
		}
		iobuf.close();
#ifdef LOGGING
		m_log["loser_size"] = loser_size;
		m_log["io"] = iobuf.get_log();
		m_log["seg"] = seg;
#endif
		return seg;
	}

private:
	size_t loser_size;
};

} // namespace qy