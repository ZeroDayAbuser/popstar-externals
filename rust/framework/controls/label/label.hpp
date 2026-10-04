#pragma once
#include "../../base/object/object.hpp"

namespace gui {

class Label : public Object {
public:
    Label() = default;
    explicit Label(std::string text) : Object(std::move(text)) {}
    void render() override;
};

}
