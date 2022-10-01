#pragma once
#include <fstream>
#include <iostream>
#include <filesystem>
#include <memory>


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

	void open(std::filesystem::path path) {
		m_stream.open(path, std::ios_base::in | std::ios_base::binary);
		m_pos = buffer_size;
		seek(0);
	}

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

	void load() {
		m_stream.read(reinterpret_cast<char*>(m_buf.get()), buffer_size * value_size);
		m_pos = 0;
	}

	ifbufstream& operator>> (value_type& x) {
		if (m_pos == buffer_size) load();
		x = m_buf[m_pos++];
		m_spos += 1;
		return *this;
	}

	operator bool() const { return m_spos != m_last; }

private:

	size_t buffer_size;
	std::ifstream m_stream;
	std::unique_ptr<value_type[]> m_buf;
	size_t m_pos;
	std::streampos m_first;
	std::streampos m_last;
	std::streampos m_spos;
};


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

	void open(std::filesystem::path path) {
		m_stream.open(path, std::ios_base::out | std::ios_base::binary);
		m_pos = 0;
		seek(0);
	}

	void seek(std::streampos first) {
		m_first = m_spos = first;
		m_stream.seekp(m_spos * value_size);
	}

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
	std::unique_ptr<value_type[]> m_buf;
	size_t m_pos;
	std::streampos m_first;
	std::streampos m_spos;
};


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