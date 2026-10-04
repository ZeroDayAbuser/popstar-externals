#pragma once
#include "../../base/object/object.hpp"

namespace gui {

class Popup : public Object {
public:
    bool opened = false;

    Popup() = default;
    explicit Popup(std::string name) : Object(std::move(name)) {}
    void render() override;
};

}
