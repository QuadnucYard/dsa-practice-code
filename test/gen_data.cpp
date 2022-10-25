#pragma GCC optimize(3)
#pragma G++ optimize(3)
#include "tester.hpp"
#include <fstream>
#include <random>
#include <vector>
#include <algorithm>
#include <filesystem>
#include <limits>

namespace fs = std::filesystem;

template <class T>
void generate_data(size_t num, T rmin, T rmax, int seed, const std::string& name) {
	std::vector<T> a(num);
	std::mt19937 rng(seed);
	if constexpr (std::is_integral_v<T>) {
		std::uniform_int_distribution distrib(rmin, rmax);
		std::ranges::generate(a, [&]() { return distrib(rng); });
	} else if constexpr (std::is_floating_point_v<T>) {
		std::uniform_real_distribution distrib(rmin, rmax);
		std::ranges::generate(a, [&]() { return distrib(rng); });
	} else {
		throw std::logic_error("Not supported value type!");
	}

	auto data_path = std::filesystem::current_path() / "test" / "data";
	if (!std::filesystem::exists(data_path))
		std::filesystem::create_directory(data_path);

	fmt::print("Generate data: size={}, seed={}, name={}, at {}\n", num, seed, name, data_path.string());

	std::ofstream fin(data_path / (name + ".in"), std::ios_base::binary | std::ios_base::trunc);
	fin.seekp(fin.beg);
	fin.write(reinterpret_cast<const char*>(a.data()), a.size() * sizeof(T));
	fin.close();

	std::ranges::sort(a);

	std::ofstream fans(data_path / (name + ".ans"), std::ios_base::binary | std::ios_base::trunc);
	fans.seekp(fans.beg);
	fans.write(reinterpret_cast<const char*>(a.data()), a.size() * sizeof(T));
	fans.close();
}

template <class T>
void generate_limit_data(size_t num, int seed, const std::string& name) {
	generate_data(num, std::numeric_limits<T>::min(), std::numeric_limits<T>::max(), seed, name);
}

int main() {
	tester::init();
	//std::vector<int> sizes{ 10'000, 100'000, 1'000'000, 5'000'000, 10'000'000 };
	std::vector<int> sizes{ 100000, 200000, 500000, 800000, 1000000, 1500000, 2000000 };
	const int seed = time(0);
	for (int i = 0; i < sizes.size(); i++) {
		// generate_limit_data<int8_t>(sizes[i], seed, fmt::format("arr_i8_{}", i));
		// generate_limit_data<int16_t>(sizes[i], seed, fmt::format("arr_i16_{}", i));
		generate_limit_data<int32_t>(sizes[i], seed, fmt::format("arr_i32_{}", i));
		// generate_limit_data<int64_t>(sizes[i], seed, fmt::format("arr_i64_{}", i));
		// generate_limit_data<float>(sizes[i], seed, fmt::format("arr_f32_{}", i));
		// generate_limit_data<double>(sizes[i], seed, fmt::format("arr_f64_{}", i));
	}
	return 0;
}