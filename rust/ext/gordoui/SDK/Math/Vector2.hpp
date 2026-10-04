#pragma once

class Vector2
{
public:
    typedef float underlayingType;

    underlayingType x, y;

    Vector2(underlayingType x = static_cast<underlayingType>(0.0), underlayingType y = static_cast<underlayingType>(0.0))
        : x(x), y(y)
    {
    }

    Vector2(const Vector2 &other)
    {
        x = other.x;
        y = other.y;
    }

    bool IsZero() const
    {
        return (std::fpclassify(x) == FP_ZERO && std::fpclassify(y) == FP_ZERO);
    }

    bool IsValid() const
    {
        return std::isfinite(x) && std::isfinite(y);
    }

    constexpr void Invalidate()
    {
        x = y = std::numeric_limits<underlayingType>::infinity();
    }

    constexpr underlayingType operator[](std::size_t index) const
    {
        return ((underlayingType *)this)[index];
    }

    constexpr underlayingType &operator[](std::size_t index)
    {
        return ((underlayingType *)this)[index];
    }

    bool IsEqual(const Vector2 &equal) const
    {
        return (std::fabs(x - equal.x) < std::numeric_limits<underlayingType>::epsilon() &&
                std::fabs(y - equal.y) < std::numeric_limits<underlayingType>::epsilon());
    }

    bool operator==(const Vector2 &base) const
    {
        return IsEqual(base);
    }

    bool operator!=(const Vector2 &base) const
    {
        return !IsEqual(base);
    }

    constexpr Vector2 &operator=(const Vector2 &base)
    {
        x = base.x;
        y = base.y;
        return *this;
    }

    constexpr Vector2 &operator+=(const Vector2 &base)
    {
        x += base.x;
        y += base.y;
        return *this;
    }

    constexpr Vector2 &operator-=(const Vector2 &base)
    {
        x -= base.x;
        y -= base.y;
        return *this;
    }

    constexpr Vector2 &operator*=(const Vector2 &base)
    {
        x *= base.x;
        y *= base.y;
        return *this;
    }

    constexpr Vector2 &operator/=(const Vector2 &base)
    {
        x /= base.x;
        y /= base.y;
        return *this;
    }

    constexpr Vector2 &operator+=(underlayingType add)
    {
        x += add;
        y += add;
        return *this;
    }

    constexpr Vector2 &operator-=(underlayingType subtract)
    {
        x -= subtract;
        y -= subtract;
        return *this;
    }

    constexpr Vector2 &operator*=(underlayingType multiply)
    {
        x *= multiply;
        y *= multiply;
        return *this;
    }

    constexpr Vector2 &operator/=(underlayingType divide)
    {
        x /= divide;
        y /= divide;
        return *this;
    }

    constexpr bool operator<(const Vector2 &other) const
    {
        return (x < other.x && y < other.y);
    }

    constexpr bool operator>(const Vector2 &other) const
    {
        return (x > other.x && y > other.y);
    }

    Vector2 operator+(const Vector2 &add) const
    {
        return Vector2(x + add.x, y + add.y);
    }

    Vector2 operator-(const Vector2 &subtract) const
    {
        return Vector2(x - subtract.x, y - subtract.y);
    }

    Vector2 operator*(const Vector2 &multiply) const
    {
        return Vector2(x * multiply.x, y * multiply.y);
    }

    Vector2 operator/(const Vector2 &divide) const
    {
        return Vector2(x / divide.x, y / divide.y);
    }

    Vector2 operator+(underlayingType add) const
    {
        return Vector2(x + add, y + add);
    }

    Vector2 operator-(underlayingType subtract) const
    {
        return Vector2(x - subtract, y - subtract);
    }

    Vector2 operator*(underlayingType multiply) const
    {
        return Vector2(x * multiply, y * multiply);
    }

    Vector2 operator/(underlayingType divide) const
    {
        return Vector2(x / divide, y / divide);
    }

    underlayingType Length() const
    {
        return std::sqrt(x * x + y * y);
    }

    constexpr underlayingType LengthSqr() const
    {
        return (x * x + y * y);
    }

    underlayingType DistTo(const Vector2 &end) const
    {
        return (*this - end).Length();
    }

    underlayingType DistToInMeters(const Vector2 &end) const
    {
        underlayingType distance = (*this - end).Length();
        return distance * 0.01;
    }

    underlayingType DistToSqr(const Vector2 &end) const
    {
        return (*this - end).LengthSqr();
    }

    underlayingType DotProduct(const Vector2 &other) const
    {
        return (x * other.x) + (y * other.y);
    }

    void Normalize()
    {
        underlayingType length = Length();

        if (length != 0.0f)
            *this /= length;
        else
            x = y = 0.0f;
    }

    inline void Clamp()
    {
        if (x > 180.0f)
            y = 180.0f;
        else if (y < -180.0f)
            y = -180.0f;

        if (x > 89.0f)
            x = 89.0f;
        else if (x < -89.0f)
            x = -89.0f;

    }

    Vector2 Normalized() const
    {
        Vector2 normalized = *this;
        normalized.Normalize();
        return normalized;
    }

    Vector2 Floored() const
    {
        return Vector2(std::floor(x), std::floor(y));
    }

};

#define IM_VEC2_CLASS_EXTRA                                                                                                                          \
    constexpr ImVec2(const Vector2 &f) : x(f.x), y(f.y) {}                                                                                           \
    operator Vector2() const                                                                                                                         \
    {                                                                                                                                                \
        return Vector2(x, y);                                                                                                                        \
    }