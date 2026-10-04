#pragma once

#include <algorithm>
#include <cstdint>
#include <type_traits>
#include <intrin.h>

struct HSV {
    float h{}, s{}, v{};
};

// Unified Color type: supports both the pawjob framework/render API
// (lowercase Color::white(), .to_imgui(), .scale_alpha()) and the Selene
// menu framework API (Color::White(), .packed, .ScaleAlpha(), .Lerp()).
union Color {
    std::uint32_t packed;
    struct {
        std::uint8_t r;
        std::uint8_t g;
        std::uint8_t b;
        std::uint8_t a;
    };

    Color() : r(255), g(255), b(255), a(255) {}

    Color(std::uint32_t col) : packed(_byteswap_ulong(col)) {}

    template<typename T>
        requires std::is_integral_v<T>
    constexpr Color(const T& r_, const T& g_, const T& b_, const T& a_ = 255)
        : r(static_cast<std::uint8_t>(r_))
        , g(static_cast<std::uint8_t>(g_))
        , b(static_cast<std::uint8_t>(b_))
        , a(static_cast<std::uint8_t>(a_)) {}

    template<typename T, typename T2>
        requires std::is_integral_v<T2>
    constexpr Color(const T& col, const T2& a_ = 255)
        : r(col.r), g(col.g), b(col.b), a(static_cast<std::uint8_t>(a_)) {}

    template<typename T, typename T2, typename T3, typename T4>
        requires std::is_integral_v<T>&& std::is_integral_v<T2>&& std::is_integral_v<T3>&& std::is_integral_v<T4>
    constexpr Color(const T& r_, const T2& g_, const T3& b_, const T4& a_ = 255)
        : r(static_cast<std::uint8_t>(r_))
        , g(static_cast<std::uint8_t>(g_))
        , b(static_cast<std::uint8_t>(b_))
        , a(static_cast<std::uint8_t>(a_)) {}

    template<typename T, typename T2>
        requires std::is_integral_v<T>&& std::is_floating_point_v<T2>
    constexpr Color(const T& r_, const T& g_, const T& b_, const T2& a_ = 1.f)
        : r(static_cast<std::uint8_t>(r_))
        , g(static_cast<std::uint8_t>(g_))
        , b(static_cast<std::uint8_t>(b_))
        , a(static_cast<std::uint8_t>(a_ * 255.f)) {}

    template<typename T, typename T2>
        requires std::is_floating_point_v<T2>
    constexpr Color(const T& col, const T2& a_ = 1.f)
        : r(col.r), g(col.g), b(col.b), a(static_cast<std::uint8_t>(a_ * 255.f)) {}

    constexpr operator unsigned int() const { return packed; }

    Color& operator=(const Color& other) { packed = other.packed; return *this; }

    bool operator==(const Color& o) const { return r == o.r && g == o.g && b == o.b && a == o.a; }
    bool operator!=(const Color& o) const { return !(*this == o); }

    //====== Selene menu API (Uppercase) ======//
    Color Lerp(const Color& to, float fraction) const {
        fraction = std::clamp(fraction, 0.0f, 1.0f);
        return Color(static_cast<int>((to.r - r) * fraction + r),
                     static_cast<int>((to.g - g) * fraction + g),
                     static_cast<int>((to.b - b) * fraction + b),
                     static_cast<int>((to.a - a) * fraction + a));
    }

    Color ScaleAlpha(float alpha) const {
        return Color(r, g, b, static_cast<uint8_t>(std::clamp((a / 255.0f) * alpha * 255.0f, 0.0f, 255.0f)));
    }

    Color OverrideAlpha(float alpha) const {
        return Color(r, g, b, static_cast<uint8_t>(std::clamp(alpha * 255.0f, 0.0f, 255.0f)));
    }

    float ScalableAlpha() const { return static_cast<float>(a) / 255.0f; }

    Color ScaleColor(int other) const {
        return Color(std::clamp(static_cast<int>(r + other), 0, 255),
                     std::clamp(static_cast<int>(g + other), 0, 255),
                     std::clamp(static_cast<int>(b + other), 0, 255), a);
    }

    constexpr static Color White()  { return { 255, 255, 255 }; }
    constexpr static Color Black()  { return { 0,   0,   0 }; }
    constexpr static Color Red()    { return { 255, 0,   0 }; }
    constexpr static Color Green()  { return { 0,   255, 0 }; }
    constexpr static Color Blue()   { return { 0,   0,   255 }; }
    constexpr static Color Yellow() { return { 255, 255, 0 }; }
    constexpr static Color Pink()   { return { 255, 0,   255 }; }
    constexpr static Color Cyan()   { return { 0,   255, 255 }; }

    //====== pawjob framework/render API (lowercase) ======//
    std::uint32_t to_imgui() const {
        return static_cast<std::uint32_t>(r)
             | (static_cast<std::uint32_t>(g) << 8)
             | (static_cast<std::uint32_t>(b) << 16)
             | (static_cast<std::uint32_t>(a) << 24);
    }

    std::uint32_t packed_rgba() const { return to_imgui(); }

    std::uint32_t packed_argb() const {
        return static_cast<std::uint32_t>(b)
             | (static_cast<std::uint32_t>(g) << 8)
             | (static_cast<std::uint32_t>(r) << 16)
             | (static_cast<std::uint32_t>(a) << 24);
    }

    Color scale_alpha(float f) const { return ScaleAlpha(f); }
    Color override_alpha(float f) const { return OverrideAlpha(f); }
    float scalable_alpha() const { return ScalableAlpha(); }

    static Color white(int a = 255)  { return Color(255, 255, 255, a); }
    static Color black(int a = 255)  { return Color(0,   0,   0,   a); }
    static Color red(int a   = 255)  { return Color(255, 0,   0,   a); }
    static Color green(int a = 255)  { return Color(0,   255, 0,   a); }
    static Color blue(int a  = 255)  { return Color(0,   0,   255, a); }
    static Color yellow(int a = 255) { return Color(255, 255, 0,   a); }
    static Color pink(int a  = 255)  { return Color(255, 0,   255, a); }
    static Color cyan(int a  = 255)  { return Color(0,   255, 255, a); }

    HSV ToHSV() const;
};
