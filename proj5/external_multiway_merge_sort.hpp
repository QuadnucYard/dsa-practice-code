#pragma once
#include "../proj4/replacement_selection.hpp"
#include "../common/fbufstream_iterator.hpp"
#include "../common/pooled_ifbufsteam.hpp"

/// @brief External multi-way merge sort implementation.
/// @tparam T Value type.
template <class T>
class external_multiway_merge_sorter {

public:
	using value_type = T;

	external_multiway_merge_sorter(size_t buffer_size) : buffer_size(buffer_size) {}

	void operator()(const fs::path& input_path, const fs::path& output_path) {
		auto tmp_path = output_path;
		tmp_path.replace_filename(".merge");
		segments = replacement_selection<value_type>(buffer_size)(input_path, tmp_path); // Call replacement selection

		/// Now merge.

		size_t merge_order = segments.size(); // Merge order
		size_t buffer_size_2 = buffer_size * 2 / merge_order + 1; // Add 1 for fear of zero trap.
		//printf("order = %d, size = %d\n", merge_order, buffer_size_2);
		ifbufstream_pool<value_type> pool(merge_order, buffer_size_2); // Buffer pool
		ofbufstream<value_type, double_buffer_tag> output_buf(buffer_size_2, output_path); // Output buffer
		// Init input buffers.
		for (size_t sum = 0, i = 0; i < merge_order; i++) {
			pool[i].open(tmp_path);
			pool[i].seek(sum, sum + segments[i]);
			sum += segments[i];
		}
		pool.collect_allocate(); // Maybe this is important

		// Loser tree. The 0-th of each element marks whether it is virtual.
		loser_tree<std::tuple<bool, value_type, int>> lt(merge_order);
		// Initialize loser tree
		for (ssize_t i = merge_order - 1; i >= 0; i--) {
			lt.push_at({ 0, pool[i].get(), i }, i);
		}
		// Continuously select the minimal element and output it.
		size_t st = 0;
		while (true) {
			// Collect & allocate at intervals
			if (++st == buffer_size_2) {
				pool.collect_allocate();
				st = 0;
			}
			auto [b, x, i] = lt.top();
			if (b) break; // It indicates that the merge is completed. Then end loop.
			output_buf << x; // Output.
			if (pool[i]) {
				// If this file is not exhausted, read in next value and push.
				pool[i] >> x;
				lt.push({ 0, x, i });
			} else {
				// Else push a virtual record. The value can be arbitrary.
				lt.push({ 1, {}, i });
			}
		}
		output_buf.close();
		fs::remove(tmp_path);
	}

	void operator()(const fs::path& input_path, const fs::path& output_path, int x) {
		auto tmp_path = output_path;
		tmp_path.replace_filename(".merge");
		segments = replacement_selection<value_type>(buffer_size)(input_path, tmp_path); // Call replacement selection

		/// Now merge.

		using ifbufstream_t = ifbufstream<value_type, double_buffer_tag>;
		size_t merge_order = segments.size(); // Merge order
		std::vector<ifbufstream_t> inputs(merge_order, { buffer_size }); // Input buffers.
		ofbufstream<value_type, double_buffer_tag> output_buf(buffer_size, output_path); // Output buffer
		// Init input buffers.
		for (size_t sum = 0, i = 0; i < merge_order; i++) {
			inputs[i].open(tmp_path);
			inputs[i].seek(sum, sum + segments[i]);
			sum += +segments[i];
		}

		// Loser tree. The 0-th of each element marks whether it is virtual.
		loser_tree<std::tuple<bool, value_type, int>> lt(merge_order);
		// Initialize loser tree
		for (ssize_t i = merge_order - 1; i >= 0; i--) {
			value_type x;
			inputs[i] >> x;
			lt.push_at({ 0, x, i }, i);
		}
		// Continuously select the minimal element and output it.
		while (true) {
			auto [b, x, i] = lt.top();
			if (b) break; // It indicates that the merge is completed. Then end loop.
			output_buf << x; // Output.
			if (inputs[i]) {
				// If this file is not exhausted, read in next value and push.
				inputs[i] >> x;
				lt.push({ 0, x, i });
			} else {
				// Else push a virtual record. The value can be arbitrary.
				lt.push({ 1, {}, i });
			}
		}
	}

private:
	/// @brief Size of all buffers.
	size_t buffer_size;
	/// @brief Length of segments to be merged.
	std::vector<size_t> segments;
};