#include "matrix_file.hpp"
#include <cassert>
// template <class T>
// std::ostream& operator<< (std::ostream& out, const T& rhs) {
// 	return out << std::to_string(rhs);
// }
void test1() {
	matrix_file matA("data/matA.in");
	matrix_file matB("data/matB.in");
	matrix_file matC2("data/matC.ans");
	matrix_file matC("data/matC.out", matA.rows(), matB.cols());
	std::cout << matC.rows() << "," << matC.cols() << std::endl;
	//matC.matmul(matA, matB);
	assert(matC == matC2);
}
int main() {
	test1();
	return 0;
}