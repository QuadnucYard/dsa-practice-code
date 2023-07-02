#pragma once
#include <chrono>
#include <iostream>

/// @brief Timer the function.
/// @param __fn The function.
/// @param ...__args Arguments to be passed to the function.
/// @return The running time in micro seconds.
template <typename _Fn, typename... _Args>
std::chrono::nanoseconds func_timer(_Fn&& __fn, _Args&&... __args) {
	auto start_tp = std::chrono::high_resolution_clock::now();
	__fn(std::forward<_Args>(__args)...);
	auto end_tp = std::chrono::high_resolution_clock::now();
	return end_tp - start_tp;
}

/// @brief Timer the function, and print duration to stdout.
/// @param __fn The function.
template <typename _Fn, typename... _Args>
void func_timer_print(_Fn&& __fn, _Args&&... __args) {
	std::cout << "Start timer ...\n";
	auto start_tp = std::chrono::high_resolution_clock::now();
	__fn(std::forward<_Args>(__args)...);
	auto end_tp = std::chrono::high_resolution_clock::now();
	std::cout << "Duration: " << std::chrono::duration_cast<std::chrono::duration<int, std::milli>>(end_tp - start_tp).count() << "ms\n";
}