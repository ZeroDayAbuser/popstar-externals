#pragma once
#include "../../base/object/object.hpp"
#include "../../utils/color.hpp"

namespace gui {

class ColorPicker : public Object {
public:
    Color value = Color::white();
    ColorPicker() = default;
    explicit ColorPicker(std::string name, Color default_color = Color::white())
        : Object(std::move(name)), value(default_color) {}
    void render() override;
};

}
