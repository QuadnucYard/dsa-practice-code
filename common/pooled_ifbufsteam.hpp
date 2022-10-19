#pragma once
#include "fbuf.hpp"
#include <future>
#include <memory>
#include <list>
#include <cassert>


template <class T>
class ifbufstream_pool;

/// @brief Async ifstream with linked buffer.
/// It read the first block synchronously. The next buffers rely on pool control.
/// @tparam T Value type
template <class T>
class pooled_ifbufstream : public base_ifbufstream<T> {
public:
	using value_type = T;
	using base = base_ifbufstream<T>;
	using buffer_type = base::buffer_type;

	using base::base;
	pooled_ifbufstream(size_t buffer_size, const std::filesystem::path& path) : base(buffer_size, path) {}
	pooled_ifbufstream(const pooled_ifbufstream& o) : pooled_ifbufstream(o.buffer_size) {}

	/// @brief Changing the current read position, and set pos of EOF. It will result in buffer reload.
	/// @param first A file offset object.
	/// @param last Offset as end of file.
	void seek(std::streamoff first, std::streamoff last = -1) override {
		if (this->m_spos != first) {
			base::seek(first, last);
			this->load();
			m_buf_queue.emplace_back(this->buffer_size);
			std::swap(this->m_buf, m_buf_queue.front()); // Make queue front as input
		} else {
			this->m_last = last;
		}
	}

	inline pooled_ifbufstream& operator>> (value_type& x) {
		//if (this->m_spos == -1) this->seek(0);
		if (this->m_pos == this->buffer_size) {
			swap_buffer();
		}
		x = this->m_buf_queue.front()[this->m_pos++];
		this->m_spos++;
		return *this;
	}

private:

	/// @brief Swap two buffers.
	inline void swap_buffer() {
		std::swap(this->m_buf, m_buf_queue.front());
		m_buf_queue.pop_front();
		if (m_buf_queue.empty()) {
			throw std::logic_error(std::string("Buffer not available! "));
		}
		this->m_pos = 0;
	}

	/// @brief Queue of buffers.
	std::list<buffer_type> m_buf_queue;

	friend class ifbufstream_pool<value_type>;
};


/// @brief Pool of ifbufstream.
/// @tparam T Value type.
template <class T>
class ifbufstream_pool {
public:
	using value_type = T;
	using stream_type = pooled_ifbufstream<value_type>;
	using buffer_type = stream_type::buffer_type;

	ifbufstream_pool(size_t buffer_count, size_t buffer_size) :
		m_bufs(buffer_count, stream_type{ buffer_size }) {}

	/// @brief Get buffer stream by subscript.
	/// @param index Index of buffer stream.
	/// @return The stream.
	inline stream_type& operator[](size_t index) {
		return m_bufs[index];
	}

	/// @brief Collect empty buffers, and allocate buffers as needed.
	void collect_allocate() {
		// Collect empty buffers.
		for (int i = 0; i < m_bufs.size(); i++) {
			if (!m_bufs[i].m_buf.empty()) {
				m_free_bufs.push_back(std::move(m_bufs[i].m_buf));
			}
		}
		// It should have free buffers.
		assert(!m_free_bufs.empty());
		// Await input loading.
		if (m_bufuture.valid()) m_bufuture.get();
		// Find the buffer with least last key.
		auto p = m_bufs.end();
		for (auto it = m_bufs.begin(); it != m_bufs.end(); ++it) {
			if (!it->eof() && (p == m_bufs.end() || it->m_buf_queue.back().back() < p->m_buf_queue.back().back())) {
				p = it;
			}
		}
		// If find any buffer that need supplement, load for it.
		if (p != m_bufs.end()) {
			// Get free buffer, and push it to the back of that queue.
			p->m_buf_queue.push_back(std::move(m_free_bufs.front()));
			m_free_bufs.pop_front();
			// Launch async load.
			m_bufuture = std::async(std::launch::async, [this, p]() {
				auto&& loading_buf = p->m_buf_queue.back();
				auto siz = std::min(p->m_last - p->m_spos, (ptrdiff_t)loading_buf.size());
				p->m_stream.read(reinterpret_cast<char*>(loading_buf.data()), siz * p->value_size);
				});
		}
	}

private:
	/// @brief Buffered streams.
	std::vector<stream_type> m_bufs;
	/// @brief Queue of free buffers.
	std::list<buffer_type> m_free_bufs;
	/// @brief Future for async load.
	std::future<void> m_bufuture;
};