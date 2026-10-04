#pragma once

typedef struct Matrix3x4
{
    union
    {
        struct
        {
            float _11, _12, _13, _14;
            float _21, _22, _23, _24;
            float _31, _32, _33, _34;
            float _41, _42, _43, _44;
        };

        float m[4][4];
    };

    [[nodiscard]] Matrix3x4 operator*(const Matrix3x4 &m2) const noexcept
    {
        Matrix3x4 out;
        for (std::uint8_t r = 0; r < 4; r++)
        {
            for (std::uint8_t c = 0; c < 4; c++)
            {
                float sum = 0.f;

                for (std::uint8_t i = 0; i < 4; i++)
                    sum += m[r][i] * m2.m[i][c];

                out.m[r][c] = sum;
            }
        }

        return out;
    }
};