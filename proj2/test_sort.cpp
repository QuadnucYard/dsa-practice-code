#define DEBUG
#include "external_quick_sort.hpp"
int main() {
	external_quick_sorter<int> sorter(1 << 10);
	sorter("data/arr.in", "data/arr.out");
	return 0;
}