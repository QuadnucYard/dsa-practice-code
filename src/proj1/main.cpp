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
	func_timer_print([&] { matC.matmul(matA, matB); });
	std::cout << "Pass: " << std::boolalpha << (matC == matC2) << std::endl;
}
int main() {
	test1();
	return 0;
}