#include "ds/interval_heap.hpp"
#include <algorithm>
#include <deque>
#include <iostream>
#include <random>

template <class T>
void print_heap(const qy::interval_heap<T>& h) {
	int i = 0, m = 2;
	for (const auto x : h) {
		std::cout << x << " ";
		if (++i == m) {
			i = 0;
			m *= 2;
			std::cout << std::endl;
		}
	}
	std::cout << std::endl;
	std::cout << std::endl;
}

int main() {
	using namespace qy;

	using H = interval_heap<int>;
	// Simple push
	{
		H h{10, 90, 15, 80, 30, 60, 20, 70, 15, 20, 45, 60, 35, 50, 25, 60, 30,
			50, 16, 19, 17, 17, 50, 55, 47, 58, 40, 45, 40, 43, 28, 55, 35};
		print_heap(h);
		h.push(82);
		print_heap(h);
		h.validate();
	}
	// Simple push 2
	{
		H h{10, 90, 15, 82, 30, 60, 20, 80, 15, 20, 45, 60, 35, 50, 25, 70,
			30, 50, 16, 19, 17, 17, 50, 55, 47, 58, 40, 45, 40, 43, 28, 55};
		h.push(8);
		print_heap(h);
	}
	// Simple pop
	{
		std::initializer_list<int> il{10, 90, 15, 82, 30, 60, 20, 80, 15, 20, 45,
									  60, 35, 50, 25, 70, 30, 50, 16, 19, 17, 17,
									  50, 55, 47, 58, 40, 45, 40, 43, 28, 55, 35};
		H h(il);
		print_heap(h);
		h.pop_min();
		print_heap(h);
	}
	// Pop all (max)
	{
		std::initializer_list<int> il{10, 90, 15, 82, 30, 60, 20, 80, 15, 20, 45,
									  60, 35, 50, 25, 70, 30, 50, 16, 19, 17, 17,
									  50, 55, 47, 58, 40, 45, 40, 43, 28, 55, 35};
		H h(il);
		std::vector<int> vec(il), vec2;
		while (!h.empty()) {
			vec2.push_back(h.top_max());
			h.pop_max();
			//print_heap(h);
		}
		std::ranges::sort(vec, std::greater());
		for (auto x : vec)
			printf("%d ", x);
		printf("\n");
		for (auto x : vec2)
			printf("%d ", x);
		printf("\n");
		assert(std::ranges::equal(vec, vec2));
	}
	// Pop all (min)
	{
		std::initializer_list<int> il{10, 90, 15, 82, 30, 60, 20, 80, 15, 20, 45,
									  60, 35, 50, 25, 70, 30, 50, 16, 19, 17, 17,
									  50, 55, 47, 58, 40, 45, 40, 43, 28, 55, 35};
		H h(il);
		std::vector<int> vec(il), vec2;
		while (!h.empty()) {
			vec2.push_back(h.top_min());
			h.pop_min();
			//print_heap(h);
		}
		std::ranges::sort(vec);
		for (auto x : vec)
			printf("%d ", x);
		printf("\n");
		for (auto x : vec2)
			printf("%d ", x);
		printf("\n");
		assert(std::ranges::equal(vec, vec2));
	}
	// Pop all (max-min)
	{
		std::initializer_list<int> il{10, 90, 15, 82, 30, 60, 20, 80, 15, 20, 45,
									  60, 35, 50, 25, 70, 30, 50, 16, 19, 17, 17,
									  50, 55, 47, 58, 40, 45, 40, 43, 28, 55, 35};
		H h(il);
		std::vector<int> vec(il);
		std::deque<int> deq;
		while (!h.empty()) {
			deq.push_back(h.top_min());
			h.pop_min();
			if (!h.empty()) {
				deq.push_front(h.top_max());
				h.pop_max();
			}
			//print_heap(h);
		}
		std::ranges::sort(vec);
		for (auto x : vec)
			printf("%d ", x);
		printf("\n");
		for (auto x : deq)
			printf("%d ", x);
		printf("\n");
		printf("%d\n", std::ranges::equal(vec, deq));
	}
	// Make heap
	{
		std::vector<int> v{10, 90, 15, 80, 30, 60, 20, 70, 15, 20, 45, 60, 35, 50, 25, 60, 30,
						   50, 16, 19, 17, 17, 50, 55, 47, 58, 40, 45, 40, 43, 28, 55, 35};
		std::ranges::shuffle(v, std::default_random_engine());
		for (auto x : v)
			printf("%d ", x);
		printf("\n");
		H h(v);
		print_heap(h);
		h.validate();
	}
	{
		H h{0, 15, 8, 12, 1, 13, 17};
		print_heap(h);
	}
	{
		H h{0, 193, 12, 117, 56, 136, 33, 173};
		print_heap(h);
	}
	return 0;
}