#pragma once

#include "globals.hpp"
#include <array>
#include <cstdint>

#if __has_include("build_seed_generated.hpp")
    #include "build_seed_generated.hpp"
#endif

#ifdef _MSC_VER
    #define BUILD_OBF_FORCEINLINE __forceinline
#else
    #define BUILD_OBF_FORCEINLINE __attribute__((always_inline)) inline
#endif

namespace build_obf {

    consteval std::uint32_t fnv1a(const char* s, std::uint32_t h = 0x811c9dc5u) {
        while (*s) { h = (h ^ static_cast<std::uint8_t>(*s++)) * 0x01000193u; }
        return h;
    }

    #ifdef PAWJAWB_BUILD_SEED
        inline constexpr std::uint32_t BUILD_SEED = PAWJAWB_BUILD_SEED;
    #else
        inline constexpr std::uint32_t BUILD_SEED =
            fnv1a(__DATE__) ^ 0xA5A5A5A5u;
    #endif

    consteval std::array<wchar_t, 13> compute_class_name() {
        std::array<wchar_t, 13> out{};
        std::uint32_t s = BUILD_SEED;
        for (int i = 0; i < 12; ++i) {
            s = s * 1103515245u + 12345u;
            int v = (s >> 16) & 0x7fff;
            out[i] = static_cast<wchar_t>(L'a' + (v % 26));
        }
        out[12] = L'\0';
        return out;
    }

    inline constexpr auto class_name_array = compute_class_name();
    inline constexpr const wchar_t* WINDOW_CLASS_NAME = class_name_array.data();

    extern volatile std::uint32_t runtime_seed;
    void                          init_runtime_seed();

    template <std::uint32_t Encoded>
    BUILD_OBF_FORCEINLINE std::uint32_t obf_load() {
        return Encoded ^ runtime_seed;
    }

} 

#define OB(x) ::build_obf::obf_load<((x) ^ ::build_obf::BUILD_SEED)>()
