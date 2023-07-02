#pragma once
#include "bufio/fbufstream.hpp"
#include "utils/json_log.hpp"

namespace qy {

class base_sorter : public json_log {
public:
	base_sorter(size_t buffer_size) : buffer_size(buffer_size) {}

	size_t get_buffersize() const { return buffer_size; }

protected:
	/// @brief Size of all buffers.
	size_t buffer_size;
};

} // namespace qy