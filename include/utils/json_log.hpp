#pragma once
#include <string>

#ifdef LOGGING
	#include <nlohmann/json.hpp>
#endif

namespace qy {

using json = nlohmann::json;

// 一个坑：ctor和dtor没有RTTI

class json_log {
#ifdef LOGGING

public:
	json_log() {}

	void clear_log() { m_log.clear(); }

	json get_log() const { return m_log; }

	std::string get_log_str() const { return m_log.dump(); }

protected:
	template <class T = int>
	void jinc(const char* key, T val = 1) {
		// assert(m_log.contains(key));
		m_log[key] = m_log[key].get<T>() + val;
	}

	/// @brief Log in json form.
	json m_log;

#else

public:
	std::string get_log_str() const { return {}; }

#endif
};

} // namespace qy