#pragma once
#include <fstream>
#include <filesystem>
#include <vector>
#include <mutex>

namespace fs = std::filesystem;

class shared_ifile {
public:
	shared_ifile() = default;
	shared_ifile(const fs::path& path) { open(path); }

	~shared_ifile() { close(); }

	void close() {
#ifdef CFILE
		fclose(m_file);
#else
		m_file.close();
#endif
	}

	void open(const fs::path& path) {
		if (!fs::exists(path)) {
			throw std::runtime_error("File not found: " + path.string());
		}
#ifdef CFILE
		m_file = fopen(path.string().c_str(), "rb");
		if (!m_file) {
			throw std::runtime_error("Fail to open input file.");
		}
#else
		m_file.open(path, std::ios_base::binary);
		if (!m_file.is_open()) {
			throw std::runtime_error("Fail to open input file.");
		}
#endif
		m_path = path;
	}

	inline bool eof() const {
#ifdef CFILE
		return feof(m_file);
#else
		return m_file.eof();
#endif
	}

	inline uintmax_t file_size() const {
		return fs::file_size(m_path);
	}

	template <class T>
	std::streamsize read(std::vector<T>& buffer, std::streamoff offset) {
		std::lock_guard lock(m_mutex);
#ifdef CFILE
		fseek(m_file, offset * sizeof(T), SEEK_SET);
		return fread(buffer.data(), sizeof(T), buffer.size(), m_file);
#else
		m_file.clear();  // In case the stream has encountered EOF.
		m_file.seekg(offset * sizeof(T));
		m_file.read(reinterpret_cast<char*>(buffer.data()), buffer.size() * sizeof(T));
		if (m_file.gcount() == -1) throw std::runtime_error("Bad input");
		return m_file.gcount() / sizeof(T);
#endif
	}

private:
	fs::path m_path;
#ifdef CFILE
	FILE* m_file;
#else
	std::ifstream m_file;
#endif
	std::mutex m_mutex;
};


class shared_ofile {
public:
	shared_ofile() = default;
	shared_ofile(const fs::path& path) { open(path); }

	~shared_ofile() { close(); }

	void close() {
#ifdef CFILE
		fclose(m_file);
#else
		m_file.close();
#endif
	}

	void open(const fs::path& path) {
#ifdef CFILE
		m_file = fopen(path.string().c_str(), "wb");
		if (!m_file) {
			throw std::runtime_error("Fail to open output file.");
		}
#else
		m_file.open(path, std::ios_base::binary);
		if (!m_file.is_open()) {
			throw std::runtime_error("Fail to open output file.");
		}
#endif
	}

	template <class T>
	void write(const std::vector<T>& buffer, std::streamoff offset, std::streamsize count) {
		std::lock_guard lock(m_mutex);
#ifdef CFILE
		fseek(m_file, offset * sizeof(T), SEEK_SET);
		fwrite(buffer.data(), sizeof(T), count, m_file);
#else
		m_file.seekp(offset * sizeof(T));
		m_file.write(reinterpret_cast<const char*>(buffer.data()), count * sizeof(T));
#endif
	}

private:
#ifdef CFILE
	FILE* m_file;
#else
	std::ofstream m_file;
#endif
	std::mutex m_mutex;
};

