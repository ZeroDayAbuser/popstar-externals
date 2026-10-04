#pragma once
#include "../../base/object/object.hpp"
#include <string>
#include <vector>

namespace gui {

class MultiDropdown : public Object {
public:
    int value = 0;                    // bitmask of selected indices
    std::vector<std::string> options;

    MultiDropdown() = default;
    MultiDropdown(std::string name, int default_mask = 0,
                  std::vector<std::string> opts = {})
        : Object(std::move(name)), value(default_mask),
          options(std::move(opts)) {}
    void render() override;
};

}
