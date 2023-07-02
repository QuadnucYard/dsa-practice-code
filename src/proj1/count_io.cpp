#define MATMUL_ORDER 0
#define NO_IO // This test only need to simulate and count cache access.
#include "ds/matrix_file.hpp"
#include "utils/print.hpp"
#include "utils/timer.hpp"
#include <cassert>

class judge_mat {
public:
	template <size_t cache_size>
	void test() {
		std::string root{"proj1/data/"};
		for (int i = 0; i < 3; i++) {
			test_one<cache_size>(root + "mat_" + std::to_string(i) + "_A.in",
								 root + "mat_" + std::to_string(i) + "_B.in",
								 root + "mat_" + std::to_string(i) + "_C.out");
		}
	}

	template <size_t cache_size>
	void test_one(const std::string& in1, const std::string& in2, const std::string& out) {
		using namespace qy;

		matrix_file<int, cache_size> matA(in1);
		matrix_file<int, cache_size> matB(in2);
		matrix_file<int, cache_size> matC(out, matA.rows(), matB.cols());
		auto t = func_timer([&]() { matC.matmul(matA, matB); }).count();
		print("{} {} {} {} {:.3f}\n", matA.count_in, matB.count_in, matC.count_in, matC.count_out,
			  t / 1e6);
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