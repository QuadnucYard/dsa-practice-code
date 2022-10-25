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
		buffer_size(buffer_size), m_buf(buffer_size), m_ibuf(buffer_size), m_obuf(buffer_size), m_ipos(buffer_size),
		m_opos(0), m_ispos(-1), m_isize(-1) {}
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
		m_ispos = m_isize = 0;
		load();
		if (!m_istream->eof()) aload();
	}

	/// @brief Close the stream.
	void close() {
		if (m_ofut.valid()) m_ofut.get();
		dump();
		m_istream->close();
		m_ostream->close();
	}

	/// @brief Check whether encounters end of input file.
	bool ieof() const { return m_ieof && m_ispos == m_isize; }

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
		m_ospos++;
		if (m_opos == buffer_size) {
			swap_obuffer();
			adump();
		}
		return *this;
	}

private:

	/// @brief Load data from file to buffer.
	inline virtual void load() {
		m_isize += m_istream->read(this->m_buf, this->m_ispos);
		m_ipos = 0;
		m_ieof = m_istream->eof();
	}

	/// @brief Load data to background buffer.
	inline void aload() {
		m_ifut = std::async(std::launch::async, [this]() {
			this->m_isize += this->m_istream->read(this->m_ibuf, this->m_ispos);
			this->m_ieof = this->m_istream->eof();
			});
	}

	/// @brief Swap main buffer with input buffer.
	inline void swap_ibuffer() {
		m_ifut.get();
		std::swap(m_buf, m_ibuf);
		m_ipos = 0;
	}

	/// @brief Write all data in buffer to file.
	inline void dump() {
		m_ostream->write(m_buf.data(), m_ospos, this->m_ospos - this->m_opos);
		m_opos = 0;
	}

	/// @brief Launch async dump.
	inline void adump() {
		m_ofut = std::async(std::launch::async, [this]() {
			this->m_ostream->write(this->m_obuf, this->m_ospos - this->m_opos, this->m_opos);
			});
	}

	/// @brief Swap main buffer with output buffer.
	inline void swap_obuffer() {
		if (m_ofut.valid()) m_ofut.get();
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
	/// @brief Current element pos of file span.
	std::streamoff m_ospos;
	/// @brief Current size of input file.
	std::atomic<std::streamsize> m_isize;
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
	std::future<void> m_ifut;
	/// @brief Future for async writing.
	std::future<void> m_ofut;
	/// @brief Mark whether inpout eof.
	std::atomic<bool> m_ieof;
};