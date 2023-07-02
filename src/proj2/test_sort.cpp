#define DEBUG
#include "external_quick_sort.hpp"
int main() {
	external_quick_sorter<int> sorter(100000);
	fs::current_path("E:/Project/dsa-practice-code/proj2");
	sorter("data/arr.in", "data/arr.out");
	return 0;
}