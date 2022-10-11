#pragma once
#include "fbuf.hpp"
#include <future>


/// @brief Base ofstream with buffer
/// @tparam T Value type
template <class T>
class base_ofbufstream : public fbuf<T> {
public:
	using value_type = T;

	base_ofbufstream(size_t buffer_size) : fbuf<T>(buffer_size) {}
	base_ofbufstream(size_t buffer_size, const std::filesystem::path& path) : base_ofbufstream(buffer_size) { open(path); }

	~base_ofbufstream() { close(); }

	/// @brief Opens an external file.
	/// @param path Path of a file.
	void open(const std::filesystem::path& path) {
		m_stream.open(path, std::ios_base::out | std::ios_base::binary);
		seek(0);
	}

	/// @brief Close the file. 
	virtual void close() {
		dump();
		m_stream.close();
	}

	/// @brief Changing the current write position, in number of elements.
	/// @param first 
	void seek(std::streamoff first) {
		this->m_first = this->m_spos = first;
		m_stream.seekp(this->m_spos * this->value_size, std::ios_base::beg);
	}

	/// @brief Getting the current write position, in number of elements.
	std::streamoff tellp() {
		std::streamoff t = m_stream.tellp();
		if (t != -1) return t / this->value_size;
		else return -1;
	}

	inline virtual base_ofbufstream& operator<< (const value_type& x) {
		this->m_buf[this->m_pos++] = x;
		this->m_spos++;
		return *this;
	}

protected:

	/// @brief Write all data in buffer to file.
	inline void dump() {
		m_stream.write(reinterpret_cast<char*>(this->m_buf.data()), this->m_pos * this->value_size);
		this->m_pos = 0;
	}

	/// @brief The output file stream.
	std::ofstream m_stream;
};


/// @brief Basic ofstream with buffer
/// @tparam T Value type
template <class T>
class basic_ofbufstream : public base_ofbufstream<T> {

public:
	using value_type = T;
	using base = base_ofbufstream<T>;

	basic_ofbufstream(size_t buffer_size) : base(buffer_size) {}
	basic_ofbufstream(size_t buffer_size, const std::filesystem::path& path) : base(buffer_size, path) {}

	inline basic_ofbufstream& operator<< (const value_type& x) override {
		base::operator<<(x);
		if (this->m_pos == this->buffer_size) this->dump();
		return *this;
	}

};


/// @brief Async ofstream with double buffer.
/// @tparam T Value type
template <class T>
class async_ofbufstream : public base_ofbufstream<T> {

public:
	using value_type = T;
	using base = base_ofbufstream<T>;

	async_ofbufstream(size_t buffer_size) : base(buffer_size), m_buf2(buffer_size) {}
	async_ofbufstream(size_t buffer_size, const std::filesystem::path& path) : base(buffer_size, path), m_buf2(buffer_size) {}

	void close() override {
		if (m_bufuture.valid()) m_bufuture.get();
		base::close();
	}

	inline async_ofbufstream& operator<< (const value_type& x) override {
		base::operator<<(x);
		if (this->m_pos == this->buffer_size) {
			if (m_bufuture.valid()) m_bufuture.get();
			swap_buffer();
			adump();
		}
		return *this;
	}

private:
	/// @brief Launch async dump.
	inline void adump() {
		m_bufuture = std::async(std::launch::async, [this]() {
			this->m_stream.write(reinterpret_cast<char*>(this->m_buf2.data()), this->m_buf2.size() * this->value_size);
			});
	}

	/// @brief Swap two buffers.
	inline void swap_buffer() {
		std::swap(this->m_buf, m_buf2);
		this->m_pos = 0;
	}

	/// @brief Background buffer
	std::vector<value_type> m_buf2;
	/// @brief Future for background writing.
	std::future<void> m_bufuture;
};


template <class T, class Tag> struct __ofbufstream_dispatcher {};
template <class T> struct __ofbufstream_dispatcher<T, basic_buffer_tag> { using type = basic_ofbufstream<T>; };
template <class T> struct __ofbufstream_dispatcher<T, double_buffer_tag> { using type = async_ofbufstream<T>; };

template <class T, class Tag>
using ofbufstream = __ofbufstream_dispatcher<T, Tag>::type;
