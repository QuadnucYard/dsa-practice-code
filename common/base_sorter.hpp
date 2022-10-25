#pragma once
#include "fbufstream.hpp"

class base_sorter {

public:
	base_sorter(size_t buffer_size) : buffer_size(buffer_size) {}

	size_t get_buffersize() const { return buffer_size; }
	virtual std::string get_log() const { return {}; }

protected:
	/// @brief Size of all buffers.
	size_t buffer_size;
};