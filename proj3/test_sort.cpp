#define DEBUG
#include "external_merge_sort.hpp"
int main() {
	external_merge_sorter<int, false> sorter(1 << 20);
	sorter("data/arr.in", "data/arr.out");
	return 0;
}