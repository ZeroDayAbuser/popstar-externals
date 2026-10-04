#pragma once
#include "../../base/object/object.hpp"
#include <string>

namespace gui {

class Container : public Object {
public:
    Container() = default;
    explicit Container(std::string name) : Object(std::move(name)) {}

    Container* add_container(const std::string& name) {
        return add_object<Container>(name);
    }

    Container* add_subtab(const std::string& name, const std::string& /*icon*/ = "") {
        return add_object<Container>(name);
    }

    void render() override;
};

}
