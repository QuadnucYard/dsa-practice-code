#pragma once
#include "fbuf.hpp"
#include <future>


/// @brief Base ifstream with buffer.
/// @tparam T Value type.
template <class T>
class base_ifbufstream : public fbuf<T> {

public:
	using value_type = T;

	base_ifbufstream(size_t buffer_size) : fbuf<T>(buffer_size) {}
	base_ifbufstream(size_t buffer_size, const std::filesystem::path& path) : base_ifbufstream(buffer_size) { open(path); }

	~base_ifbufstream() { close(); }

	/// @brief Opens an external file.
	/// @param path Path of a file.
	void open(const std::filesystem::path& path) {
		if (!fs::exists(path)) {
			throw std::runtime_error("File not found: " + path.string());
		}
		m_stream.open(path, std::ios_base::binary);
		if (!m_stream.is_open()) {
			throw std::runtime_error("Fail to open input file.");
		}
	}

	/// @brief Close the file. 
	void close() {
		m_stream.close();
	}

	/// @brief Changing the current read position, and set pos of EOF. It won't reload the block immediately.
	/// @param first A file offset object.
	/// @param last Offset as end of file.
	virtual void seek(std::streamoff first, std::streamoff last = -1) {
		this->m_first = this->m_spos = first;
		if (last < 0) {
			m_stream.clear(); // In case the stream has encountered EOF.
			m_stream.seekg(0, std::ios_base::end);
			m_last = m_stream.tellg() / this->value_size + last + 1;
		} else {
			m_last = last;
		}
		m_stream.seekg(this->m_spos * this->value_size, std::ios_base::beg);
		this->m_pos = this->buffer_size;
	}

	/// @brief Get the current read position, in number of elements.
	std::streamoff tellg() {
		std::streamoff t = m_stream.tellg();
		if (t != -1) return t / this->value_size;
		else return -1;
	}

	/// @brief Get size of file span.
	/// @return Span size.
	inline std::streamsize size() const { return this->m_last - this->m_first; }

	inline virtual base_ifbufstream& operator>> (value_type& x) {
		x = this->m_buf[this->m_pos++];
		this->m_spos++;
		return *this;
	}

	/// @brief Whether the file is read complete.
	inline operator bool() const { return this->m_spos != this->m_last; }

protected:

	/// @brief Load data from file to buffer.
	inline virtual void load() {
		m_stream.read(reinterpret_cast<char*>(this->m_buf.data()), this->m_buf.size() * this->value_size);
		this->m_pos = 0;
	}

	/// @brief The input file stream.
	std::ifstream m_stream;
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

	basic_ifbufstream(size_t buffer_size) : base(buffer_size) {}
	basic_ifbufstream(size_t buffer_size, const std::filesystem::path& path) : base(buffer_size, path) {}

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
	async_ifbufstream(size_t buffer_size, const std::filesystem::path& path) : base(buffer_size, path), m_buf2(buffer_size) {}

	/// @brief Changing the current read position, and set pos of EOF. It will result in buffer reload.
	/// @param first A file offset object.
	/// @param last Offset as end of file.
	void seek(std::streamoff first, std::streamoff last = -1) override {
		base::seek(first, last);
		this->load();
		if (!this->m_stream.eof()) aload();
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
		m_bufuture = std::async(std::launch::async, [this]() {
			this->m_stream.read(reinterpret_cast<char*>(this->m_buf2.data()), this->m_buf2.size() * this->value_size);
			});
	}

	/// @brief Swap two buffers.
	inline void swap_buffer() {
		m_bufuture.get();
		std::swap(this->m_buf, m_buf2);
		this->m_pos = 0;
	}

	/// @brief Background buffer
	std::vector<value_type> m_buf2;
	/// @brief Future for background reading.
	std::future<void> m_bufuture;
};


template <class T, class Tag> struct __ifbufstream_dispatcher {};
template <class T> struct __ifbufstream_dispatcher<T, basic_buffer_tag> { using type = basic_ifbufstream<T>; };
template <class T> struct __ifbufstream_dispatcher<T, double_buffer_tag> { using type = async_ifbufstream<T>; };

template <class T, class Tag>
using ifbufstream = __ifbufstream_dispatcher<T, Tag>::type;
