#pragma GCC optimize(3)
#include <iostream>
#include "tester.hpp"
#include "../proj2/external_quick_sort.hpp"
#include "../proj3/external_merge_sort.hpp"
#include "../proj4/external_twoway_merge_sort.hpp"
#include "../proj5/external_multiway_merge_sort.hpp"

int main() {
	tester tst;
	tst.init();
	// tst.test_sort(external_quick_sorter<int>(1 << 16));
	// tst.test_sort(external_twoway_merge_sorter<int>(1 << 8)); // 太慢，而且有大量临时文件
	// tst.test_sort(external_merge_sorter<int>(1 << 16));
	// tst.test_sort(external_merge_sorter<int>(1 << 18));
	// tst.test_sort(external_merge_sorter<int>(1 << 14));
	// tst.test_sort(external_twoway_merge_sorter<int>(1 << 14));
	// tst.test_sort(external_twoway_merge_sorter<int>(1 << 16));
	// tst.test_sort(external_twoway_merge_sorter<int>(1 << 18));
	// tst.test_sort(external_twoway_merge_sorter<int>(1 << 20));
	// tst.test_sort(external_twoway_merge_sorter<float>(1 << 16)); WA
	// tst.test_sort(external_twoway_merge_sorter<double>(1 << 16)); WA
	tst.test_sort(external_merge_sorter<int>(1 << 8));
	tst.test_sort(external_merge_sorter<int>(1 << 16));
	// tst.test_sort(external_merge_sorter<float>(1 << 8));
	// tst.test_sort(external_merge_sorter<float>(1 << 16));
	tst.test_sort(external_multiway_merge_sorter<int>(1 << 10));
	tst.test_sort(external_multiway_merge_sorter<int>(1 << 12));
	tst.test_sort(external_multiway_merge_sorter<int>(1 << 14));
	tst.test_sort(external_multiway_merge_sorter<int>(1 << 16));
	tst.test_sort(external_multiway_merge_sorter<int>(1 << 18));
	tst.test_sort(external_multiway_merge_sorter<int>(1 << 20));
	return 0;
}