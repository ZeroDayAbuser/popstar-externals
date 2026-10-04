#pragma once

class Vector3
{
public:
    typedef double underlayingType;

    underlayingType x, y, z;

    Vector3(underlayingType x = static_cast<underlayingType>(0.0), underlayingType y = static_cast<underlayingType>(0.0),
            underlayingType z = static_cast<underlayingType>(0.0))
        : x(x), y(y), z(z)
    {
    }

    Vector3(const Vector3& other)
    {
        x = other.x;
        y = other.y;
        z = other.z;
    }

    bool IsZero() const
    {
        return (std::fpclassify(x) == FP_ZERO && std::fpclassify(y) == FP_ZERO && std::fpclassify(z) == FP_ZERO);
    }

    bool IsValid() const
    {
        return std::isfinite(x) && std::isfinite(y) && std::isfinite(z);
    }

    constexpr void Invalidate()
    {
        x = y = z = std::numeric_limits<underlayingType>::infinity();
    }

    constexpr underlayingType operator[](std::size_t index) const
    {
        return ((underlayingType *)this)[index];
    }

    constexpr underlayingType &operator[](std::size_t index)
    {
        return ((underlayingType *)this)[index];
    }

    bool IsEqual(const Vector3 &equal) const
    {
        return (std::fabs(x - equal.x) < std::numeric_limits<underlayingType>::epsilon() &&
                std::fabs(y - equal.y) < std::numeric_limits<underlayingType>::epsilon() &&
                std::fabs(z - equal.z) < std::numeric_limits<underlayingType>::epsilon());
    }

    bool operator==(const Vector3 &base) const
    {
        return IsEqual(base);
    }

    bool operator!=(const Vector3 &base) const
    {
        return !IsEqual(base);
    }

    constexpr Vector3 &operator=(const Vector3 &base)
    {
        x = base.x;
        y = base.y;
        z = base.z;
        return *this;
    }

    constexpr Vector3 &operator+=(const Vector3 &base)
    {
        x += base.x;
        y += base.y;
        z += base.z;
        return *this;
    }

    constexpr Vector3 &operator-=(const Vector3 &base)
    {
        x -= base.x;
        y -= base.y;
        z -= base.z;
        return *this;
    }

    constexpr Vector3 &operator*=(const Vector3 &base)
    {
        x *= base.x;
        y *= base.y;
        z *= base.z;
        return *this;
    }

    constexpr Vector3 &operator/=(const Vector3 &base)
    {
        x /= base.x;
        y /= base.y;
        z /= base.z;
        return *this;
    }

    constexpr Vector3 &operator+=(underlayingType add)
    {
        x += add;
        y += add;
        z += add;
        return *this;
    }

    constexpr Vector3 &operator-=(underlayingType subtract)
    {
        x -= subtract;
        y -= subtract;
        z -= subtract;
        return *this;
    }

    constexpr Vector3 &operator*=(underlayingType multiply)
    {
        x *= multiply;
        y *= multiply;
        z *= multiply;
        return *this;
    }

    constexpr Vector3 &operator/=(underlayingType divide)
    {
        x /= divide;
        y /= divide;
        z /= divide;
        return *this;
    }

    constexpr bool operator<(const Vector3 &other) const
    {
        return (x < other.x && y < other.y && z < other.z);
    }

    constexpr bool operator>(const Vector3 &other) const
    {
        return (x > other.x && y > other.y && z > other.z);
    }

    Vector3 operator+(const Vector3 &add) const
    {
        return Vector3(x + add.x, y + add.y, z + add.z);
    }

    Vector3 operator-(const Vector3 &subtract) const
    {
        return Vector3(x - subtract.x, y - subtract.y, z - subtract.z);
    }

    Vector3 operator*(const Vector3 &multiply) const
    {
        return Vector3(x * multiply.x, y * multiply.y, z * multiply.z);
    }

    Vector3 operator/(const Vector3 &divide) const
    {
        return Vector3(x / divide.x, y / divide.y, z / divide.z);
    }

    Vector3 operator+(underlayingType add) const
    {
        return Vector3(x + add, y + add, z + add);
    }

    Vector3 operator-(underlayingType subtract) const
    {
        return Vector3(x - subtract, y - subtract, z - subtract);
    }

    Vector3 operator*(underlayingType multiply) const
    {
        return Vector3(x * multiply, y * multiply, z * multiply);
    }

    Vector3 operator/(underlayingType divide) const
    {
        return Vector3(x / divide, y / divide, z / divide);
    }

    underlayingType Length() const
    {
        return std::sqrt(x * x + y * y + z * z);
    }

    constexpr underlayingType LengthSqr() const
    {
        return (x * x + y * y + z * z);
    }

    underlayingType DistTo(const Vector3 &end) const
    {
        return (*this - end).Length();
    }

    underlayingType DistToInMeters(const Vector3 &end) const
    {
        underlayingType distance = (*this - end).Length();
        return distance * 0.01;
    }

    underlayingType DistToSqr(const Vector3 &end) const
    {
        return (*this - end).LengthSqr();
    }

    underlayingType DotProduct(const Vector3 &other) const
    {
        return (x * other.x) + (y * other.y) + (z * other.z);
    }

    void Normalize()
    {
        underlayingType length = Length();

        if (length != 0.0f)
            *this /= length;
        else
            x = y = z = 0.0f;
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

        z = 0;
    }

    Vector3 Normalized() const
    {
        Vector3 normalized = *this;
        normalized.Normalize();
        return normalized;
    }

    Vector3 Floored() const
    {
        return Vector3(std::floor(x), std::floor(y), std::floor(z));
    }

    Vector2 ToScreen() const;

    Vector3 RotateVector(Vector3 point) const;
};