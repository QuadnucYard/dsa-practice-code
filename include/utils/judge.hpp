#pragma once
#include "./futils.hpp"
#include "./timer.hpp"
#include "sort/base_sorter.hpp"
#include <expected>
#include <fmt/chrono.h>
#include <fmt/color.h>
#include <fmt/ostream.h>
#include <future>
#include <nameof.hpp>
#include <ranges>
#include <regex>

namespace qy {

// clang-format off

template <class T> struct type_tag {};
template <> struct type_tag<int8_t > { inline static const char* value{"i8" }; };
template <> struct type_tag<int16_t> { inline static const char* value{"i16"}; };
template <> struct type_tag<int32_t> { inline static const char* value{"i32"}; };
template <> struct type_tag<int64_t> { inline static const char* value{"i64"}; };
template <> struct type_tag<float  > { inline static const char* value{"f32"}; };
template <> struct type_tag<double > { inline static const char* value{"f64"}; };

// clang-format on

class judge {
public:
	enum class test_result { AC, WA, TLE, RE, UKE };

	static void init() {
		fmt::print("Tester Init\n");
		// fmt::print("Current path: {}\n", fs::current_path().string());
		// fs::current_path(fs::absolute(__FILE__).parent_path().parent_path());
		// fmt::print("Change cwd: {}\n", fs::current_path().string());
	}

	template <class Sorter>
	void test_sort(Sorter&& sorter) {
		using namespace std::chrono_literals;
		fmt::print(fmt::fg(fmt::color::yellow), "Test {}\n", nameof::nameof_type<Sorter>());
		const auto time_limit = 10000ms;
		int passed = 0, total = 0;

		auto tag = type_tag<typename Sorter::value_type>::value;
		for (auto&& f : filter_files(tag)) {
			if (f.path().extension() != ".in")
				continue;
			fs::path pin = f.path();
			fs::path pout = pin;
			pout.replace_extension(".out");
			fs::path pans = pin;
			pans.replace_extension(".ans");
			fmt::print(fmt::fg(fmt::color::light_blue), "  Run {}\n", pin.stem().string());
			auto result = guarded_run(time_limit, [&]() { return func_timer(sorter, pin, pout); });
			std::string result_str;
			int64_t tt;
			fmt::color print_color;
			if (result) {
				auto t = result.value();
				tt = t.count();
				bool ac = file_compare(pout, pans);
				if (ac) {
					result_str = "AC";
					print_color = fmt::color::lime_green;
					passed++;
				} else {
					result_str = "WA";
					print_color = fmt::color::red;
				}
			} else {
				if (result.error() == test_result::TLE) {
					result_str = "TLE";
					tt = std::chrono::duration_cast<std::chrono::nanoseconds>(time_limit).count();
					print_color = fmt::color::steel_blue;
				} else {
					result_str = "RE";
					tt = -1;
					print_color = fmt::color::violet;
				}
			}
			fmt::print(fmt::fg(print_color), "    Result: {}, {:.3f}ms\n", result_str, tt / 1e6f);
			total++;
			std::string log = sorter.get_log_str();
			std::erase(log, '\n');
			log = std::regex_replace(log, std::regex("\""), "\"\"");
			csv_str += fmt::format("{},{},{},{},{},{},\"{}\"\n", nameof::nameof_type<Sorter>(),
								   f.path().stem().string(),
								   fs::file_size(f.path()) / sizeof(typename Sorter::value_type),
								   sorter.get_buffersize(), result_str, tt, log);
		}
		if (passed == total) {
			fmt::print(fmt::fg(fmt::color::lime_green) | fmt::emphasis::bold, "All passed {}/{}\n",
					   passed, total);
		} else {
			fmt::print(fmt::fg(fmt::color::orange_red) | fmt::emphasis::bold, "NOT passed {}/{}\n",
					   passed, total);
		}
	}

	void dump_result(const fs::path& output_path) {
		std::ofstream fout(output_path);
		fmt::print(fout, "{}", csv_str);
	}

private:
	auto filter_files(const char* tag) {
		fmt::print(fmt::fg(fmt::color::light_yellow), "Value tag: {}\n", tag);
		return fs::directory_iterator(fs::current_path() / "test" / "data") |
			   std::views::filter([=](auto&& t) { return t.path().string().contains(tag); });
	}

	template <class Rep, class Period, typename _Fn, typename... _Args>
	std::expected<std::invoke_result_t<_Fn, _Args...>, test_result> guarded_run(
		const std::chrono::duration<Rep, Period>& timeout_duration, _Fn&& __fn, _Args&&... __args) {
		try {
			auto future = std::async(std::launch::async, std::forward<_Fn>(__fn),
									 std::forward<_Args>(__args)...);
			auto status = future.wait_for(timeout_duration);
			if (status == std::future_status::ready) {
				return future.get();
			} else {
				return std::unexpected(test_result::TLE);
			}
		} catch (std::exception& e) {
			fmt::print(fmt::fg(fmt::color::orange) | fmt::emphasis::underline, "Exception: {}\n",
					   e.what());
			return std::unexpected(test_result::RE);
		}
		return std::unexpected(test_result::UKE);
	}

private:
	std::string csv_str{"method,data,data_size,buffer_size,status,time,log\n"};
};

} // namespace qy