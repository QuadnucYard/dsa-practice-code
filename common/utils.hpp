#include <chrono>

template <typename _Fn, typename... _Args>
void func_timer(_Fn&& __fn, _Args&&... __args) {
	using time_point_t = std::chrono::time_point<std::chrono::high_resolution_clock>;
	std::cout << "Start timer ...\n";
	auto start_tp = std::chrono::high_resolution_clock::now();
	__fn(std::forward<_Args>(__args)...);
	auto end_tp = std::chrono::high_resolution_clock::now();
	std::cout << "Duration: " << std::chrono::duration_cast<std::chrono::duration<int, std::milli>>(end_tp - start_tp).count() << "ms\n";
}