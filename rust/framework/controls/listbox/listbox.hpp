#pragma once
#include "../../base/object/object.hpp"
#include <string>
#include <vector>

namespace gui {

class Listbox : public Object {
public:
    int value = -1;
    std::vector<std::string> options;
    int visible_rows = 8;

    Listbox() = default;
    Listbox(std::string name, std::vector<std::string> opts = {}, int rows = 8)
        : Object(std::move(name)), options(std::move(opts)), visible_rows(rows) {}
    void render() override;
};

using ListBox = Listbox;

}
