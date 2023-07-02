#define DEBUG
#include "external_merge_sort.hpp"
#include "../common/utils.hpp"

int main() {
	external_merge_sorter<int, true> sorter(1 << 20);
	func_timer(sorter, "data/arr.in", "data/arr.out");
	return 0;
}