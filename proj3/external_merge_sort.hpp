#include <algorithm>
#include <filesystem>
#include <cassert>
#include "fbufstream.hpp"


/// @brief External sorting implemented by merge sort
/// @tparam T Value type of sorted file
template <class T>
class external_merge_sorter {
	using value_type = T;

public:
	/// @brief Constructor
	/// @param buffer_size Size of buffer elements.
	external_merge_sorter(size_t buffer_size) : buffer_size(buffer_size),
		input_buf1(buffer_size), input_buf2(buffer_size), output_buf(buffer_size * 2) {}

	/// @brief Sort array in binary file.
	/// @param input_path Path of input file.
	/// @param output_path Path of output file.
	void operator()(const std::filesystem::path& input_path, const std::filesystem::path& output_path) {
		namespace fs = std::filesystem;
		// Sort run: Sort all data by block
		input_buf1.open(input_path);
		output_buf.open(output_path);
		size_t tot_size = input_buf1.size();
		auto tmp = std::make_unique<value_type[]>(buffer_size);
		for (size_t i = 0; i < tot_size; i += buffer_size) {
			size_t n = std::min(tot_size - i, buffer_size);
			std::copy_n(ifbuf_iterator(input_buf1), n, tmp.get());
			std::stable_sort(tmp.get(), tmp.get() + n);
			std::copy_n(tmp.get(), n, ofbuf_iterator(output_buf));
		}
		output_buf.dump(); // Dump rest data in buffer to file
		input_buf1.close();
		output_buf.close();
		// Merge run
		fs::path pA = output_path, pB = output_path; // merge A to B
		pB.replace_filename(".tmp");
		for (size_t len = buffer_size; len < tot_size; len <<= 1) { // 输出段长度
			input_buf1.open(pA);
			input_buf2.open(pA);
			output_buf.open(pB);
			output_buf.seek(0);
			size_t i = 0;
			for (; i + len < tot_size; i += len << 1) { // Pos of merged block 
				size_t len2 = len << 1;
				input_buf1.seek(i, i + len);
				input_buf2.seek(i + len, std::min(i + len2, tot_size));
				std::merge(
					ifbuf_iterator(input_buf1), ifbuf_iterator<value_type>(),
					ifbuf_iterator(input_buf2), ifbuf_iterator<value_type>(),
					ofbuf_iterator(output_buf)
				);
			}
			// Move rest data to output file if any
			if (i < tot_size) {
				input_buf1.seek(i, tot_size);
				std::copy_n(ifbuf_iterator(input_buf1), tot_size - i, ofbuf_iterator(output_buf));
			}
			output_buf.dump(); // Dump rest data in buffer to file
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
	size_t buffer_size;
	ifbufstream<T> input_buf1;
	ifbufstream<T> input_buf2;
	ofbufstream<T> output_buf;
};