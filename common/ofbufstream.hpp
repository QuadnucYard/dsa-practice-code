#pragma once
#include "fbuf.hpp"
#include "shared_file.hpp"
#include <future>


/// @brief Base ofstream with buffer
/// @tparam T Value type
template <class T>
class base_ofbufstream : public fbuf<T> {
public:
	using value_type = T;

	base_ofbufstream(size_t buffer_size) : fbuf<T>(buffer_size) {}
	base_ofbufstream(size_t buffer_size, shared_ofile& file) : base_ofbufstream(buffer_size) { open(file); }

	~base_ofbufstream() { close(); }

	/// @brief Opens an external file.
	/// @param path Path of a file.
	void open(shared_ofile& file) {
		m_stream = &file;
		seek(0);
	}

	/// @brief Close the file. 
	virtual void close() {
		dump();
	}

	/// @brief Changing the current write position, in number of elements.
	/// @param first 
	inline void seek(std::streamoff first) {
		this->m_first = this->m_spos = first;
	}

	/// @brief Getting the current write position, in number of elements.
	inline std::streamoff tellp() {
		return this->m_spos;
	}

	inline virtual base_ofbufstream& operator<< (const value_type& x) {
		this->m_buf[this->m_pos++] = x;
		this->m_spos++;
		return *this;
	}

protected:

	/// @brief Write all data in buffer to file.
	inline void dump() {
		m_stream->write(this->m_buf, this->m_spos - this->m_pos, this->m_pos);
		this->m_pos = 0;
	}

	/// @brief The output file stream.
	shared_ofile* m_stream;
};


/// @brief Basic ofstream with buffer
/// @tparam T Value type
template <class T>
class basic_ofbufstream : public base_ofbufstream<T> {

public:
	using value_type = T;
	using base = base_ofbufstream<T>;

	using base::base;

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

	using base::base;

	void close() override {
		if (m_bufuture.valid()) m_bufuture.get();
		base::close();
	}

	inline async_ofbufstream& operator<< (const value_type& x) override {
		base::operator<<(x);
		if (this->m_pos == this->buffer_size) {
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
		if (m_bufuture.valid()) m_bufuture.get();
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
