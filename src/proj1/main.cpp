#include "ds/matrix_file.hpp"
#include "utils/timer.hpp"
#include <cassert>

void test1() {
	using namespace qy;

	matrix_file matA("proj1/data/matA.in");
	matrix_file matB("proj1/data/matB.in");
	matrix_file matC2("proj1/data/matC.ans");
	matrix_file matC("proj1/data/matC.out", matA.rows(), matB.cols());
	std::cout << matC.rows() << "," << matC.cols() << std::endl;
	func_timer_print([&] {
		for (int i = 0; i < 10; i++)
			matC.matmul(matA, matB);
	});
	func_timer_print([&] {
		for (int i = 0; i < 1; i++)
			matC.matmul<1>(matA, matB);
	});
	func_timer_print([&] {
		for (int i = 0; i < 1; i++)
			matC.matmul<0>(matA, matB);
	});
	func_timer_print([&] {
		for (int i = 0; i < 1; i++)
			matC.matmul<2>(matA, matB);
	});
	std::cout << "Pass: " << std::boolalpha << (matC == matC2) << std::endl;
}
int main() {
	test1();
	/* char aaaa[1 << 20];
	qy::func_timer_print([&] {
		qy::unique_file f("proj1/data/test.out");
		for (int i = 0; i < (1 << 16); i++) {
			f.write_at(aaaa, 0);
		}
	}); */
	return 0;
}