#pragma once
#include "../../base/object/object.hpp"
#include <string>
#include <vector>

namespace gui {

class Dropdown : public Object {
public:
    int value = 0;
    std::vector<std::string> options;

    Dropdown() = default;
    Dropdown(std::string name, int default_idx = 0,
             std::vector<std::string> opts = {})
        : Object(std::move(name)), value(default_idx), options(std::move(opts)) {}
    void render() override;
};

}
