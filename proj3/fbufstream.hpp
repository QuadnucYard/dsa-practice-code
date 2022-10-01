#pragma once
#include <fstream>
#include <iostream>
#include <filesystem>
#include <memory>


/// @brief ifstream with buffer
/// @tparam T Value type
template <class T>
class ifbufstream {
	using value_type = T;
	constexpr static size_t value_size = sizeof(T);
public:
	ifbufstream(ssize_t buffer_size) : buffer_size(buffer_size) {
		m_buf = std::make_unique<value_type[]>(buffer_size);
	}
	ifbufstream(std::filesystem::path path, size_t buffer_size) : ifbufstream(buffer_size) {
		open(path);
	}

	/// @brief Opens an external file.
	/// @param path Path of a file.
	void open(std::filesystem::path path) {
		m_stream.open(path, std::ios_base::in | std::ios_base::binary);
		m_pos = buffer_size;
		seek(0);
	}

	/// @brief Close the file. 
	void close() {
		m_stream.close();
	}

	/// @brief Changing the current read position, and set pos of EOF.
	/// @param first A file offset object.
	/// @param last Offset as end of file.
	void seek(std::streampos first, std::streampos last = -1) {
		m_first = m_spos = first;
		if (last < 0) {
			m_stream.seekg(0, m_stream.end);
			m_last = m_stream.tellg() / value_size + last + 1;
		} else {
			m_last = last;
		}
		m_stream.seekg(m_spos * value_size);
	}

	/// @brief Get the current read position, in number of elements.
	size_t tellg() { return m_stream.tellg() / value_size; }

	/// @brief Load data from file to buffer.
	void load() {
		m_stream.read(reinterpret_cast<char*>(m_buf.get()), buffer_size * value_size);
		m_pos = 0;
	}

	/// @brief Get size of file span.
	/// @return 
	size_t size() const { return m_last - m_first; }

	ifbufstream& operator>> (value_type& x) {
		if (m_pos == buffer_size) load();
		x = m_buf[m_pos++];
		m_spos += 1;
		return *this;
	}

	/// @brief Whether the file is read complete.
	operator bool() const { return m_spos != m_last; }

private:

	size_t buffer_size;
	std::ifstream m_stream;
	std::unique_ptr<value_type[]> m_buf; // Buffer array
	size_t m_pos;			// Current pos of buffer
	std::streampos m_first;	// First element pos of file span
	std::streampos m_last;	// Last element pos of file span
	std::streampos m_spos;	// Current element pos of file span
};


/// @brief Iterator for ifbufstream
/// @tparam T Value type
template <class T>
class ifbuf_iterator {
public:
	using iterator_category = std::input_iterator_tag;
	using value_type = T;
	using difference_type = ptrdiff_t;
	using pointer = const value_type*;
	using reference = const value_type&;

	using stream_type = ifbufstream<value_type>;

	ifbuf_iterator() : end_marker(false) {}
	ifbuf_iterator(stream_type& s) : stream(&s) { read(); }

	reference operator* () const { return value; }
	reference operator-> () const { return &value; }

	ifbuf_iterator& operator++ () { read(); return *this; }
	ifbuf_iterator& operator++ (int) { auto tmp = *this; read(); return tmp; }

	bool operator== (const ifbuf_iterator& o) { return end_marker == o.end_marker; }

private:
	stream_type* stream;
	value_type value;
	bool end_marker;
	void read() {
		end_marker = (bool)*stream;
		if (end_marker) *stream >> value;
		//end_marker = (bool)*stream;
	}
};


/// @brief ofstream with buffer
/// @tparam T Value type
template <class T>
class ofbufstream {
	using value_type = T;
	constexpr static size_t value_size = sizeof(T);
public:
	ofbufstream(ssize_t buffer_size) : buffer_size(buffer_size) {
		m_buf = std::make_unique<value_type[]>(buffer_size);
	}
	ofbufstream(std::filesystem::path path, size_t buffer_size) : ofbufstream(buffer_size) {
		open(path);
	}

	/// @brief Opens an external file.
	/// @param path Path of a file.
	void open(std::filesystem::path path) {
		m_stream.open(path, std::ios_base::out | std::ios_base::binary);
		m_pos = 0;
		seek(0);
	}

	/// @brief Close the file. 
	void close() {
		m_stream.close();
	}

	/// @brief Changing the current write position, in number of elements.
	/// @param first 
	void seek(std::streampos first) {
		m_first = m_spos = first;
		m_stream.seekp(m_spos * value_size);
	}

	/// @brief Getting the current write position, in number of elements.
	size_t tellp() { return m_stream.tellp() / value_size; }

	/// @brief Write all data in buffer to file.
	void dump() {
		m_stream.write(reinterpret_cast<char*>(m_buf.get()), m_pos * value_size);
		m_pos = 0;
	}

	ofbufstream& operator<< (const value_type& x) {
		m_buf[m_pos++] = x;
		m_spos += 1;
		if (m_pos == buffer_size) dump();
		return *this;
	}

private:

	size_t buffer_size;
	std::ofstream m_stream;
	std::unique_ptr<value_type[]> m_buf; // Buffer array
	size_t m_pos;			// Current pos of buffer
	std::streampos m_first;	// First element pos of file span
	std::streampos m_spos;	// Current element pos of file span
};


/// @brief Iterator for ofbufstream
/// @tparam T Value type
template <class T>
class ofbuf_iterator {
public:
	using iterator_category = std::output_iterator_tag;
	using value_type = void;
	using difference_type = void;
	using pointer = void;
	using reference = void;

	using stream_type = ofbufstream<T>;

	ofbuf_iterator(stream_type& s) : stream(&s) {}

	ofbuf_iterator& operator= (const T& value) {
		*stream << value;
		return *this;
	}

	ofbuf_iterator& operator* () { return *this; }
	ofbuf_iterator& operator++ () { return *this; }
	ofbuf_iterator& operator++ (int) { return *this; }

private:
	stream_type* stream;
};