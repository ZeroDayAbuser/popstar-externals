#pragma once
#include "../../base/object/object.hpp"
#include <string>

namespace gui {

class TextInput : public Object {
public:
    std::string storage;       // app.cpp reads .storage
    std::string placeholder;

    TextInput() = default;
    explicit TextInput(std::string name, std::string placeholder_ = "")
        : Object(std::move(name)), placeholder(std::move(placeholder_)) {}
    void render() override;
};

}
