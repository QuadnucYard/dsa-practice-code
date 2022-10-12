#pragma once
#include "fbufstream.hpp"
#include <iterator>
#include <thread>

class ifbufstream_sentinel {};

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

	ifbufstream_iterator() : stream(nullptr), end_marker(false) {}
	ifbufstream_iterator(stream_type& s) : stream(&s) { read(); }

	inline reference operator* () const { return value; }
	inline reference operator-> () const { return &*this; }

	inline self& operator++ () { read(); return *this; }
	inline self operator++ (int) { auto tmp = *this; read(); return tmp; }

	inline bool operator== (const ifbufstream_sentinel& o) { return !end_marker; }
	inline bool operator== (const ifbufstream_iterator& o) { return !end_marker; }

private:
	stream_type* stream;
	value_type value;
	bool end_marker;

	inline void read() {
		end_marker = (bool)*stream;
		if (end_marker) *stream >> value;
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