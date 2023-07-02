#define DEBUG
#include "sort/external_merge_sort.hpp"
#include "utils/timer.hpp"

int main() {
	// using namespace qy;

	external_merge_sorter<int> sorter(1 << 20);
	func_timer(sorter, "proj3/data/arr.in", "proj3/data/arr.out");
	return 0;
}