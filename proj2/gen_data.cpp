#include <iostream>
#include <fstream>
#include <random>
#include <vector>
#include <algorithm>
#include <filesystem>

const int N = 1 << 16;

int main() {

	std::vector<int> a(N);
	std::default_random_engine rng;
	std::uniform_int_distribution distrib(0, 0xffffff);
	std::ranges::generate(a, [&]() { return distrib(rng); });

	auto pdata = std::filesystem::current_path() / "data";
	if (!std::filesystem::exists(pdata))
		std::filesystem::create_directory(pdata);
	std::cout << pdata.string() << std::endl;

	std::ofstream fin(pdata / "arr.in", std::ios_base::binary | std::ios_base::trunc);
	std::cout << a.size() * sizeof(int) << std::endl;
	fin.seekp(fin.beg);
	fin.write(reinterpret_cast<const char*>(a.data()), a.size() * sizeof(int));
	fin.close();

	std::ranges::sort(a);

	std::ofstream fans("./data/arr.ans", std::ios_base::binary | std::ios_base::trunc);
	fans.seekp(fans.beg);
	fans.write(reinterpret_cast<const char*>(a.data()), a.size() * sizeof(int));
	fans.close();

	return 0;
}