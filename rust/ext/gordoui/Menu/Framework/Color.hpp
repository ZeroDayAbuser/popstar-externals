struct HSV
{
    float h, s, v;
};

union Color
{
    Color() : r(255), g(255), b(255), a(255) {}

    Color(std::uint32_t col) : packed(_byteswap_ulong(col)) {}

    template<typename T>
        requires std::is_integral_v<T>
    constexpr Color(const T& r, const T& g, const T& b, const T& a = 255) : r(r), g(g), b(b), a(a)
    {
    }

    template<typename T, typename T2>
        requires std::is_integral_v<T2>
    constexpr Color(const T& col, const T2& a = 255) : r(col.r), g(col.g), b(col.b), a(a)
    {
    }

    template<typename T, typename T2, typename T3, typename T4>
        requires std::is_integral_v<T>&& std::is_integral_v<T2>&& std::is_integral_v<T3>&& std::is_integral_v<T4>
    constexpr Color(const T& r, const T2& g, const T3& b, const T4& a = 255) : r(r), g(g), b(b), a(a)
    {
    }

    template<typename T, typename T2>
        requires std::is_integral_v<T>&& std::is_floating_point_v<T2>
    constexpr Color(const T& r, const T& g, const T& b, const T2& a = 1.f) : r(r), g(g), b(b), a(static_cast<uint8_t>(a * 255.f))
    {
    }

    template<typename T, typename T2>
        requires std::is_floating_point_v<T2>
    constexpr Color(const T& col, const T2& a = 1.f) : r(col.r), g(col.g), b(col.b), a(static_cast<uint8_t>(a * 255.f))
    {
    }

    constexpr operator unsigned int() const
    {
        return packed;
    }

    Color& operator=(const Color& other)
    {
        packed = other.packed;
        return *this;
    }

    Color Lerp(const Color& to, float fraction) const
    {
        fraction = std::clamp(fraction, 0.0f, 1.0f);
        // std::lerp has bound checks we don't need here, this is faster
        return Color(static_cast<int>((to.r - r) * fraction + r), static_cast<int>((to.g - g) * fraction + g),
            static_cast<int>((to.b - b) * fraction + b), static_cast<int>((to.a - a) * fraction + a));
    }

    Color ScaleAlpha(float alpha) const
    {
        return Color(r, g, b, static_cast<uint8_t>(std::clamp((a / 255.0f) * alpha * 255.0f, 0.0f, 255.0f)));
    }

    Color OverrideAlpha(float alpha) const
    {
        return Color(r, g, b, static_cast<uint8_t>(std::clamp(alpha * 255.0f, 0.0f, 255.0f)));
    }

    float ScalableAlpha() const
    {
        return static_cast<float>(a) / 255.0f;
    }

    Color ScaleColor(int other) const
    {
        return Color(std::clamp(static_cast<int>(r + other), 0, 255), std::clamp(static_cast<int>(g + other), 0, 255), std::clamp(static_cast<int>(b + other), 0, 255), a);
    }

    std::uint32_t packed;

    struct {
        std::uint8_t r;
        std::uint8_t g;
        std::uint8_t b;
        std::uint8_t a;
    };

    constexpr static Color White()
    {
        return { 255, 255, 255 };
    }

    constexpr static Color Black()
    {
        return { 0, 0, 0 };
    }

    constexpr static Color Red()
    {
        return { 255, 0, 0 };
    }

    constexpr static Color Green()
    {
        return { 0, 255, 0 };
    }

    constexpr static Color Blue()
    {
        return { 0, 0, 255 };
    }

    constexpr static Color Yellow()
    {
        return { 255, 255, 0 };
    }

    constexpr static Color Pink()
    {
        return { 255, 0, 255 };
    }

    constexpr static Color Cyan()
    {
        return { 0, 255, 255 };
    }

    HSV ToHSV() const;
};
