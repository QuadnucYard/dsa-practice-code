#include <fstream>
#include <filesystem>
#include <vector>


/// @brief Read binary file stored an array typed T. 
/// @tparam T Value type.
/// @param path The path of file.
/// @return A vector with file data.
template <class T>
std::vector<T> read_binary_file(const std::filesystem::path& path) {
	std::ifstream fin(path, std::ios_base::binary);
	fin.seekg(0, std::ios_base::end);
	auto size = fin.tellg();
	fin.seekg(0, std::ios_base::beg);
	std::vector<T> v(size / sizeof(T));
	fin.read(reinterpret_cast<char*>(v.data()), size);
	return v;
}