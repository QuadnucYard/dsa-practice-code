#pragma once
#include <fstream>
#include <filesystem>
#include <vector>


namespace fs = std::filesystem;

struct basic_buffer_tag {};
struct double_buffer_tag {};


/// @brief Base buffer for stream. 
/// @tparam T Value type.
template <class T>
class fbuf {

public:
	using value_type = T;
	constexpr static size_t value_size = sizeof(T);

	fbuf(size_t buffer_size) : buffer_size(buffer_size), m_buf(buffer_size), m_pos(0), m_first(0), m_spos(-1) {}

protected:
	/// @brief Buffer size.
	size_t buffer_size;
	/// @brief Current pos of buffer.
	size_t m_pos;
	/// @brief First element pos of file span.
	std::streamoff m_first;
	/// @brief Current element pos of file span. -1 indicates that it hasn't been loaded..
	std::streamoff m_spos;
	/// @brief  Buffer array.
	/// It has been tested that vector<T> outperforms unique_ptr<T[]> about 4x without O2, but slightly slower than the latter with O2.
	std::vector<value_type> m_buf;
};