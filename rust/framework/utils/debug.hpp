#pragma once
#include <cstdio>
#include <print>
#include <format>
#include <string_view>
#include <utility>

namespace debug {

inline bool enabled = false;

inline void set_enabled(bool e) { enabled = e; }
inline bool is_enabled()        { return enabled; }

template <typename... Args>
inline void println(std::format_string<Args...> fmt, Args&&... args) {
    if (!enabled) return;
    std::println(fmt, std::forward<Args>(args)...);
}

template <typename... Args>
inline void print(std::format_string<Args...> fmt, Args&&... args) {
    if (!enabled) return;
    std::print(fmt, std::forward<Args>(args)...);
}

template <typename... Args>
inline void log(std::string_view tag, std::format_string<Args...> fmt, Args&&... args) {
    if (!enabled) return;
    std::print("[{}] ", tag);
    std::println(fmt, std::forward<Args>(args)...);
}

}

#define DBG(...)          do { if (::debug::enabled) ::debug::println(__VA_ARGS__); } while (0)
#define DBG_PRINT(...)    do { if (::debug::enabled) ::debug::print(__VA_ARGS__); } while (0)
#define DBG_LOG(tag, ...) do { if (::debug::enabled) ::debug::log(tag, __VA_ARGS__); } while (0)
