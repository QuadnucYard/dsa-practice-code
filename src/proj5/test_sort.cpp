#include "sort/external_multiway_merge_sort.hpp"
#include "utils/futils.hpp"
#include "utils/timer.hpp"
#include <iostream>

int main() {
	using namespace qy;

	// Issue: When input file is too large, it cannot open it.
	external_multiway_merge_sorter<int> sorter(1 << 12);
	func_timer_print(sorter, "proj5/data/arr.in", "proj5/data/arr.out", 0);
	auto v = read_binary_file<int>("proj5/data/arr.out");
	std::cout << "Is sorted? " << std::boolalpha << std::ranges::is_sorted(v) << std::endl;
	if (v.size() <= 1000) {
		for (size_t i = 0; i < v.size(); i++)
			std::cout << v[i] << " ";
		std::cout << std::endl;
	}
	auto ans = read_binary_file<int>("proj5/data/arr.ans");
	std::cout << "AC? " << (ans == v) << std::endl;
	return 0;
}
