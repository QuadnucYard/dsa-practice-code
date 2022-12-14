#include "external_twoway_merge_sort.hpp"
#include "../common/futils.hpp"
#include <iostream>

int main() {
	external_twoway_merge_sorter<int> sorter(1000);
	//fs::current_path("E:/Project/dsa-practice-code/proj4");
	std::cout << fs::current_path() << std::endl;
	sorter("data/arr.in", "data/arr.out");
	auto v = read_binary_file<int>("data/arr.out");
	std::cout << "Is sorted? " << std::boolalpha << std::ranges::is_sorted(v) << std::endl;
	if (v.size() <= 1000) {
		for (int i = 0; i < v.size(); i++) std::cout << v[i] << " ";
		std::cout << std::endl;
	}
	return 0;
}

