#pragma once
#include <iostream>
#include <fstream>
#include <filesystem>

/*
We use a code to determine loop order:
ijk 0
ikj 1
jik 2
jki 3
kij 4
kji 5
*/
#ifndef MATMUL_ORDER
#define MATMUL_ORDER 1
#endif

/**
 * @brief A matrix with cache on disk.
 * BUG: It fails to compile when cache_size differs.
 *
 * @tparam T Type of elements in the matrix.
 * @tparam cache_size Size of cache line in bytes.
 */
template <class T = int, size_t cache_size = 4096>
class matrix_file {

	static_assert(cache_size % sizeof(T) == 0); // To ensure memory alignment.

private:
	static const size_t header_size = sizeof(int) * 2; // Size of header(rows, cols) in bytes 
	static const size_t block_size = cache_size / sizeof(T); // Number of elements in cache line

public:
	/**
	 * @brief Construct a new matrix file object from existent file.
	 *
	 * @param path File of matrix.
	 * @param cache_size Size of cache line in bytes.
	 */
	matrix_file(std::filesystem::path path) :
		m_path(path)
	{
		m_file.open(path, std::ios_base::binary | std::ios::in | std::ios::out | std::ios::app);
		uint32_t tmp_rows, tmp_cols;
		m_file.read(reinterpret_cast<char*>(&tmp_rows), sizeof(uint32_t));
		m_file.read(reinterpret_cast<char*>(&tmp_cols), sizeof(uint32_t));
		m_rows = tmp_rows;
		m_cols = tmp_cols;
		m_blocks = (m_rows * m_cols + block_size - 1) / block_size;
		m_dirty = false;
		count_in = count_out = 0;
		load_block(0);
	}

	/**
	 * @brief Construct a new matrix file object with rows and cols.
	 *
	 * @param path Path to save the matrix file.
	 * @param rows Number of rows
	 * @param cols Number of columns
	 * @param cache_size Size of cache line in bytes.
	 */
	matrix_file(std::filesystem::path path, size_t rows, size_t cols) :
		m_path(path), m_rows(rows), m_cols(cols)
	{
		m_file.open(path, std::ios_base::binary | std::ios::in | std::ios::out | std::ios::trunc);
		m_file.seekp(0);
		m_file.write(reinterpret_cast<char*>(&m_rows), sizeof(uint32_t));
		m_file.write(reinterpret_cast<char*>(&m_cols), sizeof(uint32_t));
		m_blocks = (m_rows * m_cols + block_size - 1) / block_size;
		fill(0);
		count_in = count_out = 0;
	}

	matrix_file(const matrix_file& other) = delete;

	~matrix_file() {
		if (m_dirty) {
			store_block();
		}
		m_file.close();
	}

	/**
	 * @brief Get the number of rows.
	 *
	 * @return const size_t
	 */
	inline const size_t rows() const { return m_rows; }

	/**
	 * @brief Get the number of columns.
	 *
	 * @return const size_t
	 */
	inline const size_t cols() const { return m_cols; }

	/**
	 * @brief Get the element at (r,c).
	 *
	 * @param r
	 * @param c
	 * @return const T This element
	 */
	const T operator[] (size_t r, size_t c) const {
		load_block_on_demand(r, c);
		return m_cacheline[(r * m_cols + c) % block_size];
	}

	/**
	 * @brief Get the element at (r,c).
	 *
	 * @param r
	 * @param c
	 * @return const T This element
	 */
	T& operator[] (size_t r, size_t c) {
		load_block_on_demand(r, c);
		m_dirty = true;
		return m_cacheline[(r * m_cols + c) % block_size];
	}

	template <size_t S1, size_t S2>
	friend bool operator== (const matrix_file<T, S1>& lhs, const matrix_file<T, S2>& rhs) {
		if (lhs.rows() != rhs.rows() || lhs.cols() != rhs.cols()) return false;
		size_t n = lhs.rows(), m = lhs.cols();
		for (size_t i = 0; i < n; i++) {
			for (size_t j = 0; j < m; j++) {
				if (lhs[i, j] != rhs[i, j]) return false;
			}
		}
		return true;
	}

	/**
	 * @brief Fill matrix with value.
	 *
	 * @param val Value to fill.
	 */
	void fill(const T& val) {
		std::fill_n(m_cacheline, block_size, val);
		for (size_t i = 0; i < m_blocks; i++) {
			m_current_block = i;
			store_block();
		}
	}

	/**
	 * @brief Set data to the result of matmul of two matrices.
	 *
	 * @param lhs
	 * @param rhs
	 */
	template <size_t S1, size_t S2>
	void matmul(const matrix_file<T, S1>& lhs, const matrix_file<T, S2>& rhs) {
#if MATMUL_ORDER == 0 || MATMUL_ORDER == 1
#define I i
#elif MATMUL_ORDER == 2 || MATMUL_ORDER == 3
#define I j
#elif MATMUL_ORDER == 4 || MATMUL_ORDER == 5
#define I k
#endif
#if MATMUL_ORDER == 2 || MATMUL_ORDER == 4
#define J i
#elif MATMUL_ORDER == 0 || MATMUL_ORDER == 5
#define J j
#elif MATMUL_ORDER == 1 || MATMUL_ORDER == 3
#define J k
#endif
#if MATMUL_ORDER == 3 || MATMUL_ORDER == 5
#define K i
#elif MATMUL_ORDER == 1 || MATMUL_ORDER == 4
#define K j
#elif MATMUL_ORDER == 0 || MATMUL_ORDER == 2
#define K k
#endif
		size_t n = lhs.rows(), p = lhs.cols(), m = rhs.cols();
		fill(0);
		for (size_t I = 0; I < n; I++) {
			for (size_t J = 0; J < p; J++) {
				for (size_t K = 0; K < m; K++) {
					(*this)[i, j] += lhs[i, k] * rhs[k, j];
				}
			}
		}
		store_block();
#undef I
#undef J
#undef K
	}

private:
	/**
	 * @brief Get the block index from row and column index.
	 *
	 * @param r Index of row.
	 * @param c Index of column.
	 * @return size_t Index of block.
	 */
	inline size_t get_block(size_t r, size_t c) const {
		return (r * m_cols + c) / block_size;
	}

	/**
	 * @brief Load a specific data block by (r, c) to cache line, if not loaded.
	 *
	 * @param b Index of block.
	 */
	void load_block_on_demand(size_t r, size_t c) const {
		size_t b = get_block(r, c);
		if (b != m_current_block) {
			load_block(b);
		}
	}

	/**
	 * @brief Load a specific data block to cache line. It will store active block if dirty.
	 *
	 * @param b Index of block.
	 */
	void load_block(size_t b) const {
		if (m_dirty) store_block();
#ifndef NO_IO
		m_file.seekg(header_size + cache_size * b);
		m_file.read(reinterpret_cast<char*>(const_cast<T*>(m_cacheline)), cache_size);
#endif
		m_current_block = b;
		count_in++;
	}

public:

	/**
	 * @brief Store active block to disk.
	 *
	 */
	void store_block() const {
		size_t n = cache_size;
		if (m_current_block == m_blocks - 1) {
			n = m_rows * m_cols * sizeof(T) - m_current_block * cache_size;
		}
#ifndef NO_IO
		m_file.seekp(header_size + cache_size * m_current_block);
		m_file.write(reinterpret_cast<const char*>(m_cacheline), n);
#endif
		m_dirty = false;
		count_out++;
	}

public:
	inline static size_t output_row_limit{ 6 }; // Limit of displayed rows in `to_string`.
	inline static size_t output_col_limit{ 4 }; // Limit of displayed cols in `to_string`.
	mutable size_t count_in;
	mutable size_t count_out;

private:
	std::filesystem::path m_path; // Path of this matrix file
	mutable std::fstream m_file; // Opened file of this matrix
	mutable size_t m_current_block; // Index of current block
	mutable bool m_dirty; // Whether cache line is dirty
	size_t m_rows; // Number of rows
	size_t m_cols; // Number of columns
	size_t m_blocks; // Number of blocks
	T m_cacheline[block_size]; // Block of cache
};

/**
 * @brief Stringify the matrix file with display style.
 *
 */
template <class T, size_t S>
std::string to_string(const matrix_file<T, S>& mat) {
	size_t r = std::min(matrix_file<T, S>::output_row_limit, mat.rows());
	size_t c = std::min(matrix_file<T, S>::output_col_limit, mat.cols());
	std::string s;
	for (size_t i = 0; i < r; i++) {
		s += " ["[i == 0];
		s += '[';
		for (size_t j = 0; j < c; j++) {
			s += std::to_string(mat[i, j]);
			s += '\t';
		}
		if (c < mat.cols()) s += "...";
		s += ']';
		if (i != r - 1) s += '\n';
	}
	if (r < mat.rows()) {
		s += "\n [";
		for (size_t j = 0; j < c; j++) {
			s += "...";
			s += '\t';
		}
		if (c < mat.cols()) s += "...";
		s += ']';
	}
	s += "]";
	return s;
}