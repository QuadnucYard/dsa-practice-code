#pragma once
#ifdef LOGGING
#include <json/json.h>

namespace Json {
	template <class T>
	void set_vector(Json::Value& root, const char* key, const std::vector<T>& vec) {
		for (int i = static_cast<int>(vec.size()) - 1; i >= 0; i--) {
			root[key][i] = vec[i];
		}
	}

	template <class T = int>
	void inc(Json::Value& root, const char* key, T val = 1) {
		root[key] = root[key].as<T>() + val;
	}
}
#endif

class json_log {

#ifdef LOGGING

public:
	json_log() { clear_log(); }
	virtual void clear_log() { m_log.clear(); }
	Json::Value get_log() const { return m_log; }
	std::string get_log_str() const { return Json::FastWriter().write(m_log); }
protected:
	/// @brief Log in json form.
	Json::Value m_log;

#else

public:
	std::string get_log_str() const { return {}; }

#endif

};