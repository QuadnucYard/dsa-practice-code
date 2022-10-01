#define DEBUG
#include "external_merge_sort.hpp"
int main() {
	external_merge_sorter<int> sorter(10000);
	sorter("data/arr.in", "data/arr.out");
	return 0;
}