#pragma once
#include "fbufstream.hpp"
#include <iterator>


/// @brief Iterator for ifbufstream
/// @tparam T Value type
template <class T>
class ifbufstream_iterator {
public:
	using iterator_category = std::input_iterator_tag;
	using value_type = T;
	using difference_type = ptrdiff_t;
	using pointer = const value_type*;
	using reference = const value_type&;

	using stream_type = base_ifbufstream<T>;
	using self = ifbufstream_iterator<T>;

	ifbufstream_iterator() : end_marker(false), has_value(false) {}
	ifbufstream_iterator(stream_type& s) : self() { stream = &s; }

	inline reference operator* () { if (!has_value) read(); return value; }
	inline reference operator-> () { return &*this; }

	inline self& operator++ () { has_value = false; return *this; }

	inline bool operator== (const self& o) { return end_marker == o.end_marker; }

private:
	stream_type* stream;
	value_type value;
	bool has_value;
	bool end_marker;
	void read() {
		end_marker = (bool)*stream;
		if (end_marker) *stream >> value, has_value = true;
	}
};


/// @brief Iterator for ofbufstream
/// @tparam T Value type
template <class T>
class ofbufstream_iterator {
public:
	using iterator_category = std::output_iterator_tag;
	using value_type = void;
	using difference_type = void;
	using pointer = void;
	using reference = void;

	using stream_type = base_ofbufstream<T>;
	using self = ofbufstream_iterator<T>;

	ofbufstream_iterator(stream_type& s) : stream(&s) {}

	inline self& operator= (const T& value) {
		*stream << value;
		return *this;
	}

	inline self& operator* () { return *this; }
	inline self& operator++ () { return *this; }
	inline self& operator++ (int) { return *this; }

private:
	stream_type* stream;
};