#define DEBUG
#include "sort/external_quick_sort.hpp"

int main() {
	external_quick_sorter<int> sorter(100000);
	sorter("proj2/data/arr.in", "proj2/data/arr.out");
	return 0;
}