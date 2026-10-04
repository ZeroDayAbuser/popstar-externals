#include <Cheat.hpp>

Matrix3x4 CreateMatrix(Vector3 rotation, Vector3 origin)
{
    constexpr float DEG_TO_RAD = static_cast<float>(std::numbers::pi) / 180.f;
    const float radPitch = rotation.x * DEG_TO_RAD;
    const float radYaw = rotation.y * DEG_TO_RAD;
    const float radRoll = rotation.z * DEG_TO_RAD;

    const float SP = std::sinf(radPitch);
    const float CP = std::cosf(radPitch);
    const float SY = std::sinf(radYaw);
    const float CY = std::cosf(radYaw);
    const float SR = std::sinf(radRoll);
    const float CR = std::cosf(radRoll);

    Matrix3x4 matrix;
    matrix.m[0][0] = CP * CY;
    matrix.m[0][1] = CP * SY;
    matrix.m[0][2] = SP;
    matrix.m[0][3] = 0.f;

    matrix.m[1][0] = SR * SP * CY - CR * SY;
    matrix.m[1][1] = SR * SP * SY + CR * CY;
    matrix.m[1][2] = -SR * CP;
    matrix.m[1][3] = 0.f;

    matrix.m[2][0] = -(CR * SP * CY + SR * SY);
    matrix.m[2][1] = CY * SR - CR * SP * SY;
    matrix.m[2][2] = CR * CP;
    matrix.m[2][3] = 0.f;

    matrix.m[3][0] = origin.x;
    matrix.m[3][1] = origin.y;
    matrix.m[3][2] = origin.z;
    matrix.m[3][3] = 1.f;

    return matrix;
}

Vector2 Vector3::ToScreen() const
{
    return Vector2();
}

Vector3 Vector3::RotateVector(Vector3 point) const
{
    float radYaw = y * (std::numbers::pi_v<float> / 180.0f);

    float s = sinf(radYaw);
    float c = cosf(radYaw);

    Vector3 res;

    res.x = point.x * c - point.y * s;
    res.y = point.x * s + point.y * c;
    res.z = point.z;

    return res;
}

Vector3 Math::CalculateAngle(const Vector3 &source, const Vector3 &destination, const Vector3 &view_angles, float smooth)
{
    Vector3 delta = destination - source;
    Vector3 angles;

    angles.x = RAD2DEG(atan2f(delta.z, sqrtf(delta.x * delta.x + delta.y * delta.y))) - view_angles.x;
    angles.y = RAD2DEG(atan2f(delta.y, delta.x)) - view_angles.y;
    angles.z = 0.0f;

    if (angles.y > 180.0f)
        angles.y -= 360.0f;
    if (angles.y < -180.0f)
        angles.y += 360.0f;

    if (smooth > 1.0f)
    {
        angles.x /= smooth;
        angles.y /= smooth;
    }

    return angles;
}

std::vector<Vector2> Math::SubdivideArc(const std::vector<Vector2> &poly, float cornerFraction, int segmentsPerArc)
{
    const int n = static_cast<int>(poly.size());
    if (n < 3)
        return poly;

    std::vector<Vector2> vecOut;

    for (int i = 0; i < n; ++i)
    {
        const Vector2 &p = poly[(i - 1 + n) % n];
        const Vector2 &v = poly[i];
        const Vector2 &q = poly[(i + 1) % n];

        Vector2 vecPreviousDir = {v.x - p.x, v.y - p.y};
        Vector2 vecNextDir = {q.x - v.x, q.y - v.y};

        float previousLen = std::sqrt(vecPreviousDir.x * vecPreviousDir.x + vecPreviousDir.y * vecPreviousDir.y);
        float nextLen = std::sqrt(vecNextDir.x * vecNextDir.x + vecNextDir.y * vecNextDir.y);

        // Handle zero-length segments to avoid division by zero
        if (previousLen < std::numeric_limits<float>::epsilon() || nextLen < std::numeric_limits<float>::epsilon())
        {
            vecOut.push_back(v);
            continue;
        }

        vecPreviousDir.x /= previousLen;
        vecPreviousDir.y /= previousLen;
        vecNextDir.x /= nextLen;
        vecNextDir.y /= nextLen;

        float minLen = std::min(previousLen, nextLen);
        float previous = minLen * cornerFraction;
        float next = minLen * cornerFraction;

        Vector2 vecA = {v.x - vecPreviousDir.x * previous, v.y - vecPreviousDir.y * previous};
        Vector2 vecB = {v.x + vecNextDir.x * next, v.y + vecNextDir.y * next};

        for (int j = 0; j <= segmentsPerArc; ++j)
        {
            float t = static_cast<float>(j) / static_cast<float>(segmentsPerArc);
            float it = 1.0f - t;

            Vector2 vecBezier;
            vecBezier.x = it * it * vecA.x + 2.0f * it * t * v.x + t * t * vecB.x;
            vecBezier.y = it * it * vecA.y + 2.0f * it * t * v.y + t * t * vecB.y;
            vecOut.push_back(vecBezier);
        }
    }

    return vecOut;
}