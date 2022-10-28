#pragma once
#include "fbuf.hpp"
#include "shared_file.hpp"
#include <future>
#include "futils.hpp"

/// @brief Base ifstream with buffer.
/// @tparam T Value type.
template <class T>
class base_ifbufstream : public fbuf<T> {

public:
	using value_type = T;
	using base = fbuf<T>;
	using base::buffer_type;

	using base::base;
	base_ifbufstream(size_t buffer_size, shared_ifile& file) : base_ifbufstream(buffer_size) { open(file); }
	base_ifbufstream(const base_ifbufstream& o) : base_ifbufstream(o.buffer_size) {}

	/// @brief Opens an external file.
	/// @param file Shared file object.
	void open(shared_ifile& file) {
		m_stream = &file;
		this->m_spos = -1;
	}

	/// @brief Changing the current read position, and set pos of EOF. It won't reload the block immediately.
	/// @param first A file offset object.
	/// @param last Offset as end of file.
	virtual void seek(std::streamoff first, std::streamoff last = -1) {
		if (last < 0) {
			m_last = m_stream->file_size() / this->value_size + last + 1;
		} else {
			m_last = last;
		}
		//if (this->m_spos != first) {
		this->m_first = this->m_spos = this->m_fpos = first;
			this->m_pos = this->buffer_size;
			//}
	}

	/// @brief Get the current read position, in number of elements.
	inline std::streamoff tellg() {
		return this->m_spos;
	}

	/// @brief Get whether file span is end.
	inline bool eof() {
		return this->m_fpos >= m_stream->file_size() / this->value_size || this->m_spos >= m_last;
	}

	/// @brief Get size of file span.
	/// @return Span size.
	inline std::streamsize size() const { return this->m_last - this->m_first; }

	inline const value_type& back() const { return this->m_buf.back(); }

	inline virtual base_ifbufstream& operator>> (value_type& x) {
		x = this->m_buf[this->m_pos++];
		this->m_spos++;
		return *this;
	}

	/// @brief Whether the file is read complete.
	inline operator bool() const { return this->m_spos != this->m_last; }

	/// @brief Read one element.
	/// @return Element value.
	inline value_type get() {
		value_type x;
		*this >> x;
		return x;
	}

protected:

	/// @brief Load data from file to buffer.
	inline virtual void load() {
		this->m_fpos += m_stream->read(this->m_buf, this->m_fpos);
		fmt::print("rd {} [{}]\n", this->m_buf, this->m_fpos);
		this->m_pos = 0;
	}

	/// @brief The input file stream.
	shared_ifile* m_stream;
	/// @brief Last element pos of file span.
	std::streamoff m_last;

};


/// @brief Basic ifstream with buffer.
/// @tparam T Value type.
template <class T>
class basic_ifbufstream : public base_ifbufstream<T> {

public:
	using value_type = T;
	using base = base_ifbufstream<T>;

	using base::base;

	inline basic_ifbufstream& operator>> (value_type& x) override {
		if (this->m_spos == -1) this->seek(0);
		if (this->m_pos == this->buffer_size) this->load();
		base::operator>>(x);
		return *this;
	}

};


/// @brief Async ifstream with double buffer.
/// It read the first block synchronously, and the next block asynchronously.
/// When reading, if the fore-buffer is empty, wait for back-buffer to complete loading, then swap buffer, and load new block. 
/// @tparam T Value type
template <class T>
class async_ifbufstream : public base_ifbufstream<T> {
public:
	using value_type = T;
	using base = base_ifbufstream<T>;

	async_ifbufstream(size_t buffer_size) : base(buffer_size), m_buf2(buffer_size) {}
	async_ifbufstream(size_t buffer_size, shared_ifile& file) : base(buffer_size, file), m_buf2(buffer_size) {}
	async_ifbufstream(const async_ifbufstream& o) : base(o.buffer_size), m_buf2(o.buffer_size) {}

	/// @brief Changing the current read position, and set pos of EOF. It will result in buffer reload.
	/// @param first A file offset object.
	/// @param last Offset as end of file.
	void seek(std::streamoff first, std::streamoff last = -1) override {
		//if (this->m_spos != first) {
			base::seek(first, last);
			this->load();
			aload();
			//} else {
			//	this->m_fpos = first;
			//	this->m_last = last;
			//}
	}

	inline async_ifbufstream& operator>> (value_type& x) {
		if (this->m_spos == -1) this->seek(0);
		if (this->m_pos == this->buffer_size) {
			swap_buffer();
			aload();
		}
		base::operator>>(x);
		return *this;
	}

private:

	/// @brief Load data to background buffer.
	inline void aload() {
		m_bufuture = std::async(std::launch::async, [this, offset = this->m_fpos]() {
			return this->m_stream->read(this->m_buf2, offset);
			});
	}

	/// @brief Swap two buffers.
	inline void swap_buffer() {
		this->m_fpos += m_bufuture.get();
		std::swap(this->m_buf, m_buf2);
		this->m_pos = 0;
	}

	/// @brief Background buffer
	std::vector<value_type> m_buf2;
	/// @brief Future for background reading.
	std::future<std::streamsize> m_bufuture;
};


template <class T, class Tag> struct __ifbufstream_dispatcher {};
template <class T> struct __ifbufstream_dispatcher<T, basic_buffer_tag> { using type = basic_ifbufstream<T>; };
template <class T> struct __ifbufstream_dispatcher<T, double_buffer_tag> { using type = async_ifbufstream<T>; };

template <class T, class Tag>
using ifbufstream = __ifbufstream_dispatcher<T, Tag>::type;
