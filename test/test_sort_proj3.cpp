// #pragma GCC optimize(3)
// #pragma GCC optimize("Ofast", "inline", "-ffast-math")
// #pragma GCC target("avx,sse2,sse3,sse4,mmx")
// #define DEBUG
#define LOGGING
#include "sort/external_merge_sort.hpp"
#include "utils/judge.hpp"

using namespace qy;

struct judge_impl {
	judge J;
	std::vector<size_t> buffer_sizes{
		/*1 << 10, 1 << 11, 1 << 12, 1 << 13, 1 << 14,1 << 15,*/ 1 << 16, 1 << 17, 1 << 18, 1 << 19,
		1 << 20};
	fs::path result_path;

	judge_impl() :
		result_path(fmt::format("test/out/result_{:%Y%m%d%H%M%S}.csv",
								fmt::localtime(std::time(nullptr)))) {
		J.init();
	}

	template <class T>
	void test() {
		for (size_t s : buffer_sizes) {
			J.test_sort(external_merge_sorter<T>(s));
			J.dump_result(result_path);
		}
	}
};

int main() {
	judge_impl J;
	J.test<int32_t>();
	return 0;
}