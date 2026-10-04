#pragma once

#include <chrono>
#include <cstdio>
#include <ctime>
#include <memory>
#include <print>
#include <utility>
#include <windows.h>
#include "titles.hpp"

class logger_c
{
public:
	auto setup(const char* title, bool visible = false) -> void
	{
		if (!::GetConsoleWindow())
			::AllocConsole();

		FILE* con = nullptr;
		freopen_s(&con, "CONOUT$", "w", stdout);
		freopen_s(&con, "CONOUT$", "w", stderr);
		freopen_s(&con, "CONIN$", "r", stdin);
		std::setvbuf(stdout, nullptr, _IONBF, 0);
		std::setvbuf(stderr, nullptr, _IONBF, 0);
		::SetConsoleOutputCP(CP_UTF8);
		char gen[64]{};
		titles::ascii(gen, (int)sizeof(gen));
		::SetConsoleTitleA(gen[0] ? gen : title);

		HANDLE std_handle = ::GetStdHandle(STD_OUTPUT_HANDLE);
		DWORD mode = 0;
		if (::GetConsoleMode(std_handle, &mode))
		{
			mode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
			::SetConsoleMode(std_handle, mode);
		}

		CONSOLE_FONT_INFOEX cfi{};
		cfi.cbSize = sizeof(cfi);
		cfi.nFont = 0;
		cfi.dwFontSize.X = 8;
		cfi.dwFontSize.Y = 15;
		cfi.FontFamily = FF_DONTCARE;
		cfi.FontWeight = FW_NORMAL;
		wcscpy_s(cfi.FaceName, L"Raster Fonts");
		::SetCurrentConsoleFontEx(std_handle, FALSE, &cfi);

		if (HWND hwnd = ::GetConsoleWindow())
			::ShowWindow(hwnd, visible ? SW_SHOW : SW_HIDE);
	}

	template<typename... args_t>
	auto print(const char* format, args_t... args) -> void
	{
		auto now = std::chrono::system_clock::now();
		std::time_t time = std::chrono::system_clock::to_time_t(now);
		tm local_tm{};
		localtime_s(&local_tm, &time);

		std::printf("\x1b[38;2;130;90;180m[%02d/%02d/%04d %02d:%02d:%02d]\x1b[0m ",
			local_tm.tm_mon + 1,
			local_tm.tm_mday,
			local_tm.tm_year + 1900,
			local_tm.tm_hour,
			local_tm.tm_min,
			local_tm.tm_sec);

		std::printf("\x1b[38;2;200;130;255m>\x1b[0m ");
		std::printf("\x1b[38;2;230;215;255m");
		std::printf(format, args...);
		std::printf("\x1b[0m\n");
	}
};

inline auto logger = std::make_unique<logger_c>();

namespace logging
{
	inline FILE* g_file = nullptr;

	inline void set_up()
	{
		char gen[64]{};
		titles::ascii(gen, (int)sizeof(gen));
		logger->setup(gen[0] ? gen : "RuntimeBroker", false);
	}

	inline void show()
	{
		if (!::GetConsoleWindow())
			set_up();
		char gen[64]{};
		titles::ascii(gen, (int)sizeof(gen));
		if (gen[0]) ::SetConsoleTitleA(gen);
		if (HWND hwnd = ::GetConsoleWindow())
		{
			::ShowWindow(hwnd, SW_SHOW);
			::SetForegroundWindow(hwnd);
		}
	}

	inline void hide()
	{
		if (HWND hwnd = ::GetConsoleWindow())
			::ShowWindow(hwnd, SW_HIDE);
	}

	template<typename... args_t>
	inline void print(const char* format, args_t... args)
	{
		if (!::GetConsoleWindow())
			return;
		logger->print(format, args...);
	}

	template <typename... Args>
	inline void println(std::format_string<Args...> fmt, Args&&... args)
	{
		if (!::GetConsoleWindow())
			return;
		try
		{
			const auto line = std::format(fmt, std::forward<Args>(args)...);
			logger->print("%s", line.c_str());
		}
		catch (...)
		{
		}
	}
}
