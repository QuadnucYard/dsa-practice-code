#include "ds/matrix_file.hpp"
#include <cassert>

void test1() {
	using namespace qy;

	matrix_file matA("proj1/data/matA.in");
	matrix_file matB("proj1/data/matB.in");
	matrix_file matC2("proj1/data/matC.ans");
	matrix_file matC("proj1/data/matC.out", matA.rows(), matB.cols());
	std::cout << matC.rows() << "," << matC.cols() << std::endl;
	matC.matmul(matA, matB);
	assert(matC == matC2);
}
int main() {
	test1();
	return 0;
}