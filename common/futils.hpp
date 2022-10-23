#pragma once
#include <fstream>
#include <filesystem>
#include <vector>
#ifdef FMT_HEADER_ONLY
#include <fmt/core.h>
#include <fmt/ranges.h>
#endif

namespace fs = std::filesystem;


/// @brief Read binary file stored an array typed T. 
/// @tparam T Value type.
/// @param path The path of file.
/// @return A vector with file data.
template <class T>
std::vector<T> read_binary_file(const fs::path& path) {
	std::ifstream fin(path, std::ios_base::binary);
	fin.seekg(0, std::ios_base::end);
	auto size = fin.tellg();
	fin.seekg(0, std::ios_base::beg);
	std::vector<T> v(size / sizeof(T));
	fin.read(reinterpret_cast<char*>(v.data()), size);
	return v;
}

std::string read_binary_file(const fs::path& path) {
	std::ifstream fin(path, std::ios_base::binary);
	std::string s;
	s.resize((fs::file_size(path)));
	fin.read(s.data(), s.size());
	return s;
}

bool file_compare(const fs::path& out, const fs::path& ans) {
	return read_binary_file(out) == read_binary_file(ans);
}

#ifdef FMT_HEADER_ONLY
template <class T>
void print_binary_file(const fs::path& path) {
	auto&& v = read_binary_file<T>(path);
	fmt::print("{} [{}][{}/{}]\n", v, std::ranges::is_sorted(v), std::ranges::is_sorted_until(v) - v.begin(), v.size());
}
#endif