#pragma once
#include <algorithm>
#include <array>
#include <cassert>
#include <filesystem>
#include <fstream>
#include <iostream>

#define CFILE
#include "bufio/unique_file.hpp"

namespace qy {

namespace fs = std::filesystem;

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
	requires requires(T a, T b, T c) {
				 requires cache_size % sizeof(T) == 0; // To ensure memory alignment.
				 a += b * c;
			 }
class matrix_file {
	using header_size_t = uint32_t;

	// Size of header(rows, cols) in bytes
	static constexpr size_t header_size = sizeof(header_size_t) * 2;

	// Number of elements in cache line
	static constexpr size_t block_size = cache_size / sizeof(T);

public:
	/**
	 * @brief Construct a new matrix file object from existent file.
	 *
	 * @param path File of matrix.
	 * @param cache_size Size of cache line in bytes.
	 */
	matrix_file(const fs::path& path) : m_path(path) {
		m_file.open(path, false);
		header_size_t tmp[2];
		m_file.read_at(tmp, 0);
		m_rows = tmp[0];
		m_cols = tmp[1];
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
	matrix_file(const fs::path& path, size_t rows, size_t cols) :
		m_path(path), m_rows(rows), m_cols(cols) {
		m_file.open(path, true);
		// assert(!m_file.bad());
		m_file.write_at(m_rows, 0);
		m_file.write_at(m_cols, sizeof(header_size_t));
		m_blocks = (m_rows * m_cols + block_size - 1) / block_size;
		fill(0);
		count_in = count_out = 0;
	}

	matrix_file(const matrix_file& other) = delete;

	~matrix_file() {
		if (m_dirty)
			store_block();
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
	inline const T operator[](size_t r, size_t c) const {
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
	inline T& operator[](size_t r, size_t c) {
		load_block_on_demand(r, c);
		m_dirty = true;
		return m_cacheline[(r * m_cols + c) % block_size];
	}

	template <size_t S1, size_t S2>
	friend bool operator==(const matrix_file<T, S1>& lhs, const matrix_file<T, S2>& rhs) {
		if (lhs.rows() != rhs.rows() || lhs.cols() != rhs.cols())
			return false;
		size_t n = lhs.rows(), m = lhs.cols();
		for (size_t i = 0; i < n; i++) {
			for (size_t j = 0; j < m; j++) {
				if (lhs[i, j] != rhs[i, j])
					return false;
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
		std::ranges::fill(m_cacheline, val);
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
	inline size_t get_block(size_t r, size_t c) const { return (r * m_cols + c) / block_size; }

	/**
	 * @brief Load a specific data block by (r, c) to cache line, if not loaded.
	 *
	 * @param b Index of block.
	 */
	inline void load_block_on_demand(size_t r, size_t c) const {
		if (size_t b = get_block(r, c); b != m_current_block)
			load_block(b);
	}

	/**
	 * @brief Load a specific data block to cache line. It will store active block if dirty.
	 *
	 * @param b Index of block.
	 */
	inline void load_block(size_t b) const {
		if (m_dirty)
			store_block();
#ifndef NO_IO
		m_file.read_at(m_cacheline, header_size + cache_size * b);
#endif
		m_current_block = b;
		count_in++;
	}

public:
	/**
	 * @brief Store active block to disk.
	 *
	 */
	inline void store_block() const {
#ifndef NO_IO
		size_t n = block_size;
		if (m_current_block == m_blocks - 1)
			n = m_rows * m_cols - m_current_block * block_size;
		m_file.write_at(m_cacheline, n, header_size + cache_size * m_current_block);
#endif
		m_dirty = false;
		count_out++;
	}

public:
	inline static size_t output_row_limit{6}; // Limit of displayed rows in `to_string`.
	inline static size_t output_col_limit{4}; // Limit of displayed cols in `to_string`.
	mutable size_t count_in;
	mutable size_t count_out;

private:
	std::filesystem::path m_path;	// Path of this matrix file
	mutable unique_file m_file;		// Opened file of this matrix
	mutable size_t m_current_block; // Index of current block
	mutable bool m_dirty;			// Whether cache line is dirty
	size_t m_rows;					// Number of rows
	size_t m_cols;					// Number of columns
	size_t m_blocks;				// Number of blocks
	mutable std::array<T, block_size> m_cacheline; // Block of cache
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
		if (c < mat.cols())
			s += "...";
		s += ']';
		if (i != r - 1)
			s += '\n';
	}
	if (r < mat.rows()) {
		s += "\n [";
		for (size_t j = 0; j < c; j++) {
			s += "...";
			s += '\t';
		}
		if (c < mat.cols())
			s += "...";
		s += ']';
	}
	s += "]";
	return s;
}

} // namespace qy
