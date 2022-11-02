#pragma once
#include "fbufstream.hpp"
#include "json_log.hpp"

class base_sorter : public json_log {

public:
	base_sorter(size_t buffer_size) : buffer_size(buffer_size) {}

	size_t get_buffersize() const { return buffer_size; }

protected:
	/// @brief Size of all buffers.
	size_t buffer_size;
};