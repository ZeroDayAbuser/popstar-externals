#pragma once

class Vector4
{
public:
    typedef double underlayingType;

    underlayingType x, y, z, w = static_cast<underlayingType>(0.0);
};