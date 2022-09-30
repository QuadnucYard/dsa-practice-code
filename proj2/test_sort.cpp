#define DEBUG
#include "external_quick_sort.hpp"
external_quick_sorter<int, 1 << 10> sorter("data/tmp.bin");
int main() {
	sorter("data/arr.in", "data/arr.out");
	return 0;
}