#define NO_IO // This test only need to simulate and count cache access.
#include "ds/matrix_file.hpp"
#include "utils/print.hpp"
#include "utils/timer.hpp"
#include <cassert>
#include <format>

class judge_mat {
public:
	template <int order, size_t cache_size>
	void test() {
		std::string root{"proj1/data"};
		for (int i = 0; i < 3; i++) {
			test_one<order, cache_size>(std::format("{}/mat_{}_A.in", root, i),
										std::format("{}/mat_{}_B.in", root, i),
										std::format("{}/mat_{}_C.out", root, i));
		}
	}

	template <int order, size_t cache_size>
	void test_one(const std::string& in1, const std::string& in2, const std::string& out) {
		using namespace qy;

		matrix_file<int, cache_size> matA(in1);
		matrix_file<int, cache_size> matB(in2);
		matrix_file<int, cache_size> matC(out, matA.rows(), matB.cols());
		auto t = func_timer([&]() { matC.template matmul<order>(matA, matB); }).count();
		print("{} {} {} {} {:.3f}\n", matA.count_in, matB.count_in, matC.count_in, matC.count_out,
			  t / 1e6);
	}
};

void test1() {
	judge_mat J;
	J.test<0, 1 << 10>();
	J.test<1, 1 << 10>();
	J.test<2, 1 << 10>();
	J.test<3, 1 << 10>();
	J.test<4, 1 << 10>();
	J.test<5, 1 << 10>();
}

int main() {
	test1();
	return 0;
}