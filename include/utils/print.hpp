#include <format>

template <typename... _Args>
void print(std::format_string<_Args...> __fmt, _Args&&... __args) {
	fputs(std::format(__fmt, std::forward<_Args>(__args)...).c_str(), stdout);
}