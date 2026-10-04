#include <Cheat.hpp>

HSV Color::ToHSV() const
{
    float h, s, v;
    ImGui::ColorConvertRGBtoHSV(r / 255.0f, g / 255.0f, b / 255.0f, h, s, v);
    return {h, s, v};
}
