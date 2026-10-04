#pragma once
#include "../../base/object/object.hpp"

namespace gui {

class Checkbox : public Object {
public:
    bool value = false;
    Checkbox() = default;
    explicit Checkbox(std::string name, bool default_value = false)
        : Object(std::move(name)) { value = default_value; }
    void render() override;
};

}
