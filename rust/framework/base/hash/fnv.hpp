#pragma once
#include <cstdint>
#include <string_view>

namespace fnv {

inline constexpr std::uint64_t fnv_offset = 0xcbf29ce484222325ULL;
inline constexpr std::uint64_t fnv_prime  = 0x100000001b3ULL;

inline constexpr std::uint64_t hash(std::string_view s) noexcept {
    std::uint64_t h = fnv_offset;
    for (char c : s) {
        h ^= static_cast<std::uint8_t>(c);
        h *= fnv_prime;
    }
    return h;
}

inline constexpr std::uint64_t hash(const char* s) noexcept {
    std::uint64_t h = fnv_offset;
    while (s && *s) {
        h ^= static_cast<std::uint8_t>(*s++);
        h *= fnv_prime;
    }
    return h;
}

inline constexpr std::uint64_t hash_const(const char* s) noexcept {
    return hash(s);
}

}
