#pragma once
#include <fstream>

/// @brief A file buffer specialized for value type.
/// @tparam T Value type
/// @tparam buffer_size Size of buffer elements.
template <class T, size_t buffer_size>
class arraybuf {

	constexpr static size_t value_size = sizeof(T); // Size of value type

public:
	/// @brief Bind a file stream object.
	/// @param stream A file stream object.
	inline void bind(std::fstream* stream) {
		m_stream = stream;
		m_size = 0;
	}

	/// @brief Changing the current read position.
	/// @param pos A file position object.
	inline void seekg(std::streampos pos) {
		m_stream->seekg(pos * value_size, m_stream->beg);
	}

	/// @brief Changing the current write position.
	/// @param pos A file position object.
	inline void seekp(std::streampos pos) {
		m_stream->seekp(pos * value_size, m_stream->beg);
	}

	/// @brief Getting the current read position.
	/// @return 
	inline std::streampos tellg() const {
		return m_stream->tellg() / value_size;
	}

	/// @brief Getting the current write position.
	/// @return 
	inline std::streampos tellp() const {
		return m_stream->tellp() / value_size;
	}

	/// @brief Get item
	/// @param index 
	/// @return 
	inline const T operator[] (size_t index) const {
		return m_buf[index];
	}

	/// @brief Output value to buffer, and dumps if full
	/// @param x Value
	/// @return Self
	inline arraybuf& operator<< (const T& x) {
		m_buf[m_size++] = x;
		if (m_size == buffer_size) {
			dump();
		}
		return *this;
	}

	/// @brief Load data to buffer
	/// @param pos Start pos
	/// @param input_size Size of input elements
	inline void load(std::streampos pos, std::streamsize input_size) {
		auto tmp_pos = m_stream->tellp(); // Save write pos because load will change it
		m_stream->seekg(pos * value_size, m_stream->beg);
		m_stream->read(reinterpret_cast<char*>(m_buf), input_size * value_size);
		m_stream->seekp(tmp_pos); // Recover write pos
		m_size = input_size;
	}

	/// @brief Dump buffer data to file
	inline void dump() {
		m_stream->write(reinterpret_cast<const char*>(m_buf), m_size * value_size);
		m_size = 0;
	}

	/// @brief Transfer all data till write pos to the end of another file
	/// @param other Output file
	inline void transfer(std::fstream* other) {
		size_t total_size = tellp();
		m_stream->seekg(m_stream->beg);
		char* buf = reinterpret_cast<char*>(m_buf);
		while (total_size > 0) {
			std::streamsize input_size = std::min(buffer_size, total_size);
			m_stream->read(buf, input_size * value_size);
			other->write(buf, m_stream->gcount());
			total_size -= input_size;
		}
		m_size = 0;
	}

	inline const T* begin() const { return m_buf; }

	inline const T* end() const { return m_buf + m_size; }

	/// @brief Get buffer size
	/// @return Buffer size
	inline size_t size() const { return m_size; }

	/// @brief Get the bound filestream
	/// @return fstream
	inline const std::fstream* stream() const { return m_stream; }

private:
	std::fstream* m_stream; // File stream
	T m_buf[buffer_size]; 	// Buffer array
	size_t m_size; 			// Filled size
};