#pragma once
#include "../common/utils.hpp"
#include "../common/futils.hpp"
#include <fmt/core.h>
#include <fmt/chrono.h>
#include <fmt/color.h>
#include <iostream>
#include <ranges>
#include <future>
#include <typeinfo>
#include <expected>

namespace fs = std::filesystem;

template <class T> struct type_tag {};
template <> struct type_tag<int8_t> { inline static const char* value{ "i8" }; };
template <> struct type_tag<int16_t> { inline static const char* value{ "i16" }; };
template <> struct type_tag<int32_t> { inline static const char* value{ "i32" }; };
template <> struct type_tag<int64_t> { inline static const char* value{ "i64" }; };
template <> struct type_tag<float> { inline static const char* value{ "f32" }; };
template <> struct type_tag<double> { inline static const char* value{ "f64" }; };

class tester {
public:
	enum class test_result {
		AC, WA, TLE, RE, UKE
	};

	static void init() {
		fmt::print("Tester Init\n");
		fmt::print("Current path: {}\n", fs::current_path().string());
		fs::current_path(fs::absolute(__FILE__).parent_path().parent_path());
		fmt::print("Change cwd: {}\n", fs::current_path().string());
	}

	template <class Sorter>
	void test_sort(Sorter&& sorter) {
		using namespace std::chrono_literals;
		fmt::print(fmt::fg(fmt::color::yellow), "Test {}\n", typeid(Sorter).name());
		const auto time_limit = 10000ms;
		int passed = 0, total = 0;
		for (auto&& f : filter_files<Sorter>()) {
			if (f.path().extension() != ".in") continue;
			fs::path pin = f.path();
			fs::path pout = pin; pout.replace_extension(".out");
			fs::path pans = pin; pans.replace_extension(".ans");
			fmt::print(fmt::fg(fmt::color::light_blue), "  Run {}\n", pin.stem().string());
			auto result = guarded_run(time_limit, [&]() {return func_timer(sorter, pin, pout);});
			if (result) {
				auto t = result.value();
				int64_t tt = t.count();
				bool ac = file_compare(pout, pans);
				if (ac) {
					fmt::print(fmt::fg(fmt::color::lime_green), "    Result: AC, {:.3f}ms\n", tt / 1000000.0);
					passed++;
				} else {
					fmt::print(fmt::fg(fmt::color::red), "    Result: WA, {:.3f}ms\n", tt / 1000000.0);
				}
			} else {
				if (result.error() == test_result::TLE) {
					fmt::print(fmt::fg(fmt::color::steel_blue), "    Result: TLE, {}\n", time_limit);
				} else {
					fmt::print(fmt::fg(fmt::color::violet), "    Result: RE\n");
				}
			}
			total++;
		}
		if (passed == total) {
			fmt::print(fmt::fg(fmt::color::lime_green) | fmt::emphasis::bold, "All passed {}/{}\n", passed, total);
		} else {
			fmt::print(fmt::fg(fmt::color::orange_red) | fmt::emphasis::bold, "NOT passed {}/{}\n", passed, total);
		}
	}

private:

	template <class Sorter>
	auto filter_files() {
		auto tag = type_tag<typename Sorter::value_type>::value;
		fmt::print(fmt::fg(fmt::color::light_yellow), "Value tag: {}\n", tag);
		return fs::directory_iterator(fs::current_path() / "test" / "data")
			| std::views::filter([=](auto&& t) { return t.path().string().contains(tag); });
	}

	template <class Rep, class Period, typename _Fn, typename... _Args>
	std::expected<std::invoke_result_t<_Fn, _Args...>, test_result>
		guarded_run(const std::chrono::duration<Rep, Period>& timeout_duration, _Fn&& __fn, _Args&&... __args) {
		try {
			auto future = std::async(std::launch::async, std::forward<_Fn>(__fn), std::forward<_Args>(__args)...);
			auto status = future.wait_for(timeout_duration);
			if (status == std::future_status::ready) {
				return future.get();
			} else {
				return std::unexpected(test_result::TLE);
			}
		}
		catch (std::exception& e) {
			fmt::print(fmt::fg(fmt::color::orange) | fmt::emphasis::underline, "Exception: {}\n", e.what());
			return std::unexpected(test_result::RE);
		}
		return std::unexpected(test_result::UKE);
	}
};

