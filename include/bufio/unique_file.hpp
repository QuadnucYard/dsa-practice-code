#pragma once
#include <filesystem>
#include <fstream>

namespace qy {

namespace fs = std::filesystem;

class unique_ifile {
public:
	unique_ifile() = default;

	unique_ifile(const fs::path& path) { open(path); }

	unique_ifile(const unique_ifile& o) = delete;

	~unique_ifile() { close(); }

	void close() {
#ifdef CFILE
		fclose(m_file);
#else
		m_file.close();
#endif
	}

	void open(const fs::path& path) {
		if (!fs::exists(path))
			throw std::runtime_error("File not found: " + path.string());
#ifdef CFILE
		m_file = fopen(path.string().c_str(), "rb");
		if (!m_file)
			throw std::runtime_error("Fail to open input file.");
#else
		m_file.open(path, std::ios_base::binary);
		if (!m_file.is_open())
			throw std::runtime_error("Fail to open input file.");
#endif
		m_path = path;
		m_size = fs::file_size(m_path);
	}

	inline uintmax_t file_size() const { return m_size; }

	template <std::ranges::contiguous_range _Range>
	inline std::streamsize read_at(_Range&& buffer, std::streamoff offset) {
		using T = std::ranges::range_value_t<_Range>;
#ifdef CFILE
		std::fseek(m_file, offset, SEEK_SET);
		return std::fread(std::ranges::data(buffer), sizeof(T), std::ranges::size(buffer), m_file);
#else
		m_file.clear(); // In case the stream has encountered EOF.
		m_file.seekg(offset);
		m_file.read(reinterpret_cast<char*>(std::ranges::data(buffer)),
					std::ranges::size(buffer) * sizeof(T));
		assert(m_file.gcount() != -1); // Bad input
		return m_file.gcount() / sizeof(T);
#endif
	}

private:
	fs::path m_path;
	uintmax_t m_size;
#ifdef CFILE
	FILE* m_file;
#else
	std::ifstream m_file;
#endif
};

class unique_ofile {
public:
	unique_ofile() = default;

	unique_ofile(const fs::path& path, bool trunc = true) { open(path, trunc); }

	unique_ofile(const unique_ofile& o) = delete;

	~unique_ofile() { close(); }

	void close() {
#ifdef CFILE
		fclose(m_file);
#else
		m_file.close();
#endif
	}

	void open(const fs::path& path, bool trunc = true) {
#ifdef CFILE
		m_file = fopen(path.string().c_str(), trunc ? "wb" : "rb+");
		if (!m_file)
			throw std::runtime_error("Fail to open output file.");
#else
		m_file.open(path, trunc ? std::ios_base::binary | std::ios_base::trunc
								: std::ios_base::binary | std::ios_base::out);
		if (!m_file.is_open())
			throw std::runtime_error("Fail to open output file.");
#endif
		m_path = path;
	}

	template <std::ranges::contiguous_range _Range>
	inline void write_at(_Range&& buffer, std::streamsize count, std::streamoff offset) {
		using T = std::ranges::range_value_t<_Range>;
#ifdef CFILE
		std::fseek(m_file, offset, SEEK_SET);
		std::fwrite(std::ranges::data(buffer), sizeof(T), count, m_file);
#else
		m_file.seekp(offset);
		m_file.write(reinterpret_cast<const char*>(std::ranges::data(buffer)), count * sizeof(T));
#endif
	}

	template <class T>
	inline void write_at(const T& o, std::streamoff offset) {
#ifdef CFILE
		std::fseek(m_file, offset, SEEK_SET);
		std::fwrite(reinterpret_cast<const char*>(&o), sizeof(T), 1, m_file);
#else
		m_file.seekp(offset * sizeof(T));
		m_file.write(reinterpret_cast<const char*>(&o), sizeof(T));
#endif
	}

private:
	fs::path m_path;
#ifdef CFILE
	FILE* m_file;
#else
	std::ofstream m_file;
#endif
};

class unique_file {
public:
	unique_file() = default;

	unique_file(const fs::path& path) { open(path); }

	unique_file(const unique_file& o) = delete;

	~unique_file() { close(); }

	void close() {
#ifdef CFILE
		fclose(m_file);
#else
		m_file.close();
#endif
	}

	void open(const fs::path& path, bool trunc = true) {
		if (!fs::exists(path))
			std::ofstream{path};
			// throw std::runtime_error("File not found: " + path.string());
#ifdef CFILE
		m_file = fopen(path.string().c_str(), trunc ? "wb+" : "rb+");
		if (!m_file)
			throw std::runtime_error("Fail to open input file.");
#else
		m_file.open(path, trunc ? std::ios_base::binary | std::ios_base::in | std::ios_base::out |
									  std::ios_base::trunc
								: std::ios_base::binary | std::ios_base::in | std::ios_base::out);
		if (!m_file.is_open())
			throw std::runtime_error("Fail to open input file.");
#endif
		m_path = path;
		m_size = fs::file_size(m_path);
	}

	inline uintmax_t file_size() const { return m_size; }

	inline void seekg(std::streamoff offset) {
#ifdef CFILE
		fseek(m_file, offset, SEEK_SET);
#else
		m_file.seekg(offset);
#endif
	}

	inline void seekp(std::streamoff offset) {
#ifdef CFILE
		fseek(m_file, offset, SEEK_SET);
#else
		m_file.seekp(offset);
#endif
	}

	template <std::ranges::contiguous_range _Range>
	inline std::streamsize read_at(_Range&& buffer, std::streamoff offset) {
		using T = std::ranges::range_value_t<_Range>;
#ifdef CFILE
		std::fseek(m_file, offset, SEEK_SET);
		return std::fread(std::ranges::data(buffer), sizeof(T), std::ranges::size(buffer), m_file);
#else
		m_file.clear(); // In case the stream has encountered EOF.
		m_file.seekg(offset);
		m_file.read(reinterpret_cast<char*>(std::ranges::data(buffer)),
					std::ranges::size(buffer) * sizeof(T));
		assert(m_file.gcount() != -1); // Bad input
		return m_file.gcount() / sizeof(T);
#endif
	}

	template <std::ranges::contiguous_range _Range>
	inline void write_at(_Range&& buffer, std::streamsize count, std::streamoff offset) {
		using T = std::ranges::range_value_t<_Range>;
#ifdef CFILE
		std::fseek(m_file, offset, SEEK_SET);
		std::fwrite(std::ranges::data(buffer), sizeof(T), count, m_file);
#else
		m_file.seekp(offset);
		m_file.write(reinterpret_cast<const char*>(std::ranges::data(buffer)), count * sizeof(T));
#endif
	}

	template <class T>
	inline void write_at(const T& o, std::streamoff offset) {
#ifdef CFILE
		std::fseek(m_file, offset, SEEK_SET);
		std::fwrite(reinterpret_cast<const char*>(&o), sizeof(T), 1, m_file);
#else
		m_file.seekp(offset * sizeof(T));
		m_file.write(reinterpret_cast<const char*>(&o), sizeof(T));
#endif
	}

private:
	fs::path m_path;
	uintmax_t m_size;
#ifdef CFILE
	FILE* m_file;
#else
	std::fstream m_file;
#endif
};

} // namespace qy