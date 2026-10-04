#pragma once
#include "../../base/object/object.hpp"
#include <functional>

namespace gui {

class Button : public Object {
public:
    std::function<void()> on_press;
    Button() = default;
    explicit Button(std::string name) : Object(std::move(name)) {}
    void render() override;
};

}
