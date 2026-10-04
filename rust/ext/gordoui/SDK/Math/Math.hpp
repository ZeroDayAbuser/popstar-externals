#pragma once

constexpr float DEG2RAD(float degrees) noexcept
{
    return degrees * (std::numbers::pi_v<float> / 180.0f);
}

constexpr float RAD2DEG(float radians) noexcept
{
    return radians * (180.0f / std::numbers::pi_v<float>);
}

#define METRE2INCH(x) ((x) / 0.0254f)
#define INCH2METRE(x) ((x) * 0.0254f)
#define METRE2FOOT(x) ((x) * 3.28f)
#define FOOT2METRE(x) ((x) / 3.28f)

struct Rect
{
    Vector2 position = Vector2(), size = Vector2();

    Vector2 Min() const
    {
        return position;
    }

    Vector2 Max() const
    {
        return position + size;
    }

    bool IsValid() const
    {
        return position.x > 0.0f && position.y > 0.0f && size.x > 0.0f && size.y > 0.0f;
    }

    bool Intersects(const Vector2 &p, const Vector2 &s) const
    {
        Vector2 _min = p;
        Vector2 _max = p + s;
        return _min.y < Max().y && _max.y > Min().y && _min.x < Max().x && _max.x > Min().x;
    }
};

namespace Math
{

    template <typename T>
    inline T CatmullRom(const T &p0, const T &p1, const T &p2, const T &p3, float t)
    {
        float t2 = t * t;
        float t3 = t2 * t;

        return ((p1 * 2.0f) + (p2 - p0) * t + (p0 * 2.0f - p1 * 5.0f + p2 * 4.0f - p3) * t2 + (p1 * 3.0f - p0 - p2 * 3.0f + p3) * t3) * 0.5f;
    }

    template <class T>
    inline std::vector<T> GenerateSmoothCurveOpen(const std::vector<T> &points, int subdivisions)
    {
        std::vector<T> smooth_points;
        if (points.size() < 4)
            return points;

        smooth_points.push_back(points[0]);

        for (size_t i = 0; i < points.size() - 1; i++)
        {
            T p0, p1, p2, p3;

            if (i == 0)
            {
                p0 = points[0];
                p1 = points[0];
                p2 = points[1];
                p3 = (points.size() > 2) ? points[2] : points[1];
            }
            else if (i == points.size() - 2)
            {
                p0 = points[i - 1];
                p1 = points[i];
                p2 = points[i + 1];
                p3 = points[i + 1];
            }
            else
            {
                p0 = points[i - 1];
                p1 = points[i];
                p2 = points[i + 1];
                p3 = points[i + 2];
            }

            for (int j = 1; j <= subdivisions; ++j)
            {
                float t = j / static_cast<float>(subdivisions);
                smooth_points.push_back(CatmullRom(p0, p1, p2, p3, t));
            }
        }

        return smooth_points;
    }

    Vector3 CalculateAngle(const Vector3 &source, const Vector3 &destination, const Vector3 &view_angles, float smooth = 0.0f);

    std::vector<Vector2> SubdivideArc(const std::vector<Vector2> &poly, float cornerFraction = 0.12f, int segmentsPerArc = 8);

} // namespace Math