#pragma once

struct Transform
{
    Vector4 rotation = Vector4();
    Vector3 translation = Vector3();
    std::uint8_t pad_0[0x4];
    Vector3 scale3D = Vector3();
    std::uint8_t pad_1[0x4];

    Matrix3x4 ScaledMatrix() const
    {
        Matrix3x4 m;
        m.m[3][0] = translation.x;
        m.m[3][1] = translation.y;
        m.m[3][2] = translation.z;

        float x2 = rotation.x + rotation.x;
        float y2 = rotation.y + rotation.y;
        float z2 = rotation.z + rotation.z;

        float xx2 = rotation.x * x2;
        float yy2 = rotation.y * y2;
        float zz2 = rotation.z * z2;
        m.m[0][0] = (1.0f - (yy2 + zz2)) * scale3D.x;
        m.m[1][1] = (1.0f - (xx2 + zz2)) * scale3D.y;
        m.m[2][2] = (1.0f - (xx2 + yy2)) * scale3D.z;

        float yz2 = rotation.y * z2;
        float wx2 = rotation.w * x2;
        m.m[2][1] = (yz2 - wx2) * scale3D.z;
        m.m[1][2] = (yz2 + wx2) * scale3D.y;

        float xy2 = rotation.x * y2;
        float wz2 = rotation.w * z2;
        m.m[1][0] = (xy2 - wz2) * scale3D.y;
        m.m[0][1] = (xy2 + wz2) * scale3D.x;

        float xz2 = rotation.x * z2;
        float wy2 = rotation.w * y2;
        m.m[2][0] = (xz2 + wy2) * scale3D.z;
        m.m[0][2] = (xz2 - wy2) * scale3D.x;

        m.m[0][3] = 0.0f;
        m.m[1][3] = 0.0f;
        m.m[2][3] = 0.0f;
        m.m[3][3] = 1.0f;

        return m;
    }
};