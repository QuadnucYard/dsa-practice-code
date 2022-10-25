#pragma GCC optimize(3)
#include <iostream>
#include "judge.hpp"
#include "../proj2/external_quick_sort.hpp"
#include "../proj3/external_merge_sort.hpp"
#include "../proj4/external_twoway_merge_sort.hpp"
#include "../proj5/external_multiway_merge_sort.hpp"

struct judge_impl {
	judge J;
	std::vector<size_t> buffer_sizes{ /*1 << 10, 1 << 11, 1 << 12, 1 << 13, 1 << 14,1 << 15,*/ 1 << 16, 1 << 17, 1 << 18, 1 << 19, 1 << 20 };
	fs::path result_path{ "test/data/result.csv" };

	judge_impl() {
		J.init();
	}

	template <class T>
	void test() {
		for (size_t s : buffer_sizes) {
			J.test_sort(external_merge_sorter<T>(s));
			J.test_sort(external_twoway_merge_sorter<T>(s));
			J.test_sort(external_multiway_merge_sorter<T>(s));
			J.dump_result(result_path);
		}
	}
};

int main() {
	judge_impl J;
	J.test<int>();
	return 0;
}