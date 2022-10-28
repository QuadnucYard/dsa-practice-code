#pragma once
#include "fbuf.hpp"
#include "shared_file.hpp"
#include <future>
#include <atomic>


/// @brief A special fstream using only 3 buffers for I/O. 
/// @tparam T Value type.
template <class T>
class async_iofbufstream {
public:
	using value_type = T;
	using self = async_iofbufstream<T>;
	constexpr static size_t value_size = sizeof(value_type);

	async_iofbufstream(size_t buffer_size) :
		buffer_size(buffer_size), m_buf(buffer_size), m_ibuf(buffer_size), m_obuf(buffer_size),
		m_ipos(buffer_size), m_opos(0), m_ispos(-1), m_ifpos(-1), m_ofpos(-1) {}
	async_iofbufstream(size_t buffer_size, shared_ifile& input_file, shared_ofile& output_file) : async_iofbufstream(buffer_size) {
		open(input_file, output_file);
	}
	~async_iofbufstream() { close(); }

	/// @brief Opens external input file and output file. It will immediately load the first block synchronously. 
	/// @param input_path Path of input file.
	/// @param output_path Path of output file.
	void open(shared_ifile& input_file, shared_ofile& output_file) {
		m_istream = &input_file;
		m_ostream = &output_file;
		m_ispos = m_ifpos = m_ofpos = 0;
		load();
		if (!this->ieof()) aload();
	}

	/// @brief Close the stream.
	void close() {
		if (m_ofut.valid()) m_ofut.get(), m_ofpos += buffer_size;
		dump();
		m_istream->close();
		m_ostream->close();
	}

	/// @brief Check whether encounters end of input file. It does not means there are no more elements to read.
	bool ieof() const { return m_ifpos >= m_istream->file_size() / value_size; }

	inline self& operator>> (value_type& x) {
		if (m_ipos == buffer_size) {
			swap_ibuffer();
			aload();
		}
		x = m_buf[m_ipos++];
		m_ispos++;
		return *this;
	}

	inline self& operator<< (const value_type& x) {
		m_buf[m_opos++] = x;
		if (m_opos == buffer_size) {
			swap_obuffer();
			adump();
		}
		return *this;
	}

	/// @brief Whether the file is read complete.
	inline operator bool() const { return m_ispos < m_istream->file_size() / value_size; }

private:

	/// @brief Load data from file to buffer.
	inline virtual void load() {
		m_ifpos += m_istream->read(this->m_buf, this->m_ispos);
		m_ipos = 0;
	}

	/// @brief Load data to background buffer.
	inline void aload() {
		m_ifut = std::async(std::launch::async, [this]() {
			return this->m_istream->read(this->m_ibuf, this->m_ifpos);
			});
	}

	/// @brief Swap main buffer with input buffer.
	inline void swap_ibuffer() {
		m_ifpos += m_ifut.get();
		std::swap(m_buf, m_ibuf);
		m_ipos = 0;
	}

	/// @brief Write all data in buffer to file.
	inline void dump() {
		m_ostream->write(m_buf, m_ofpos, this->m_opos);
		m_opos = 0;
	}

	/// @brief Launch async dump.
	inline void adump() {
		m_ofut = std::async(std::launch::async, [this]() {
			this->m_ostream->write(this->m_obuf, this->m_ofpos, this->buffer_size);
			});
	}

	/// @brief Swap main buffer with output buffer.
	inline void swap_obuffer() {
		if (m_ofut.valid()) m_ofut.get(), m_ofpos += buffer_size;
		std::swap(m_buf, m_obuf);
		m_opos = 0;
	}

private:

	/// @brief Buffer size.
	size_t buffer_size;
	/// @brief Pointer to input data
	size_t m_ipos;
	/// @brief Pointer to output data
	size_t m_opos;
	/// @brief Current element pos of file span. -1 indicates that it hasn't been loaded.
	std::streamoff m_ispos;
	/// @brief Current read pos of file span.
	std::streamoff m_ifpos;
	/// @brief Current write pos of file span.
	std::streamoff m_ofpos;
	/// @brief The istream.
	shared_ifile* m_istream;
	/// @brief The ostream.
	shared_ofile* m_ostream;
	/// @brief Main buffer array.
	std::vector<value_type> m_buf;
	/// @brief Buffer for async input.
	std::vector<value_type> m_ibuf;
	/// @brief Buffer for asnyc outout.
	std::vector<value_type> m_obuf;
	/// @brief Future for async reading.
	std::future<std::streamsize> m_ifut;
	/// @brief Future for async writing.
	std::future<void> m_ofut;
};