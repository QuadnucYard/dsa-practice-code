#pragma once
#include "fbuf.hpp"
#include <future>
#include <atomic>


/// @brief A special fstream using only 3 buffers for I/O. 
/// @tparam T Value type.
template <class T>
class async_iofbufstream : public json_log {
public:
	using value_type = T;
	using self = async_iofbufstream<T>;
	constexpr static size_t value_size = sizeof(value_type);

	async_iofbufstream(size_t buffer_size) :
		buffer_size(buffer_size), m_buf(buffer_size), m_ibuf(buffer_size), m_obuf(buffer_size), m_ipos(buffer_size),
		m_opos(0), m_ispos(-1), m_isize(-1) {
#ifdef LOGGING
		this->m_log["in"] = 0;
		this->m_log["out"] = 0;
#endif
	}
	async_iofbufstream(size_t buffer_size, const fs::path& input_path, const fs::path& output_path) : async_iofbufstream(buffer_size) {
		open(input_path, output_path);
	}
	~async_iofbufstream() { close(); }

	/// @brief Opens external input file and output file. It will immediately load the first block synchronously. 
	/// @param input_path Path of input file.
	/// @param output_path Path of output file.
	void open(const fs::path& input_path, const fs::path& output_path) {
		m_istream.open(input_path, std::ios_base::binary);
		m_ostream.open(output_path, std::ios_base::binary);
		m_ispos = m_isize = 0;
		load();
		if (!m_istream.eof()) aload();
	}

	/// @brief Close the stream.
	void close() {
		if (m_ofut.valid()) m_ofut.get();
		dump();
		m_istream.close();
		m_ostream.close();
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
		if (m_opos == buffer_size) {
			swap_obuffer();
			adump();
		}
		return *this;
	}

private:

	/// @brief Load data from file to buffer.
	inline virtual void load() {
		m_istream.read(reinterpret_cast<char*>(m_buf.data()), m_buf.size() * value_size);
		m_ipos = 0;
		m_isize += m_istream.gcount() / value_size;
		m_ieof = m_istream.eof();
#ifdef LOGGING
		Json::inc(this->m_log, "in");
#endif
	}

	/// @brief Load data to background buffer.
	inline void aload() {
		m_ifut = std::async(std::launch::async, [this]() {
			this->m_istream.read(reinterpret_cast<char*>(this->m_ibuf.data()), this->m_ibuf.size() * this->value_size);
			this->m_isize += this->m_istream.gcount() / value_size;
			this->m_ieof = this->m_istream.eof();
			});
#ifdef LOGGING
		Json::inc(this->m_log, "in");
#endif
	}

	/// @brief Swap main buffer with input buffer.
	inline void swap_ibuffer() {
		m_ifut.get();
		std::swap(m_buf, m_ibuf);
		m_ipos = 0;
	}

	/// @brief Write all data in buffer to file.
	inline void dump() {
		m_ostream.write(reinterpret_cast<char*>(m_buf.data()), m_opos * value_size);
		m_opos = 0;
#ifdef LOGGING
		Json::inc(this->m_log, "out");
#endif
	}

	/// @brief Launch async dump.
	inline void adump() {
		m_ofut = std::async(std::launch::async, [this]() {
			this->m_ostream.write(reinterpret_cast<char*>(this->m_obuf.data()), this->m_obuf.size() * this->value_size);
			});
#ifdef LOGGING
		Json::inc(this->m_log, "out");
#endif
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
	/// @brief Current size of input file.
	std::atomic<std::streamsize> m_isize;
	/// @brief The istream.
	std::ifstream m_istream;
	/// @brief The ostream.
	std::ofstream m_ostream;
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