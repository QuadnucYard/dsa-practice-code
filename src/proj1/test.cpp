#pragma GCC optimize(3)
#pragma GCC optimize("Ofast", "inline", "-ffast-math")
#pragma GCC target("avx,sse2,sse3,sse4,mmx")
#define MATMUL_ORDER 0
#define NO_IO
#include "matrix_file.hpp"
#include "../common/utils.hpp"
#include <cassert>

class judge_mat {

public:
	template <size_t cache_size>
	void test() {
		std::string root{ "data/" };
		for (int i = 2; i < 3; i++) {
			test_one<cache_size>(
				root + "mat_" + std::to_string(i) + "_A.in",
				root + "mat_" + std::to_string(i) + "_B.in",
				root + "mat_" + std::to_string(i) + "_C.out"
				);
		}
	}

	template <size_t cache_size>
	void test_one(const std::string& in1, const std::string& in2, const std::string& out) {
		matrix_file<int, cache_size> matA(in1);
		matrix_file<int, cache_size> matB(in2);
		matrix_file<int, cache_size> matC(out, matA.rows(), matB.cols());
		auto t = func_timer([&]() { matC.matmul(matA, matB); }).count();
		printf("%d %d %d %d %.3lf\n", matA.count_in, matB.count_in, matC.count_in, matC.count_out, t / 1e6);
	}
};

void test1() {
	judge_mat J;
	J.test<1 << 10>();
}
int main() {
	test1();
	return 0;
}