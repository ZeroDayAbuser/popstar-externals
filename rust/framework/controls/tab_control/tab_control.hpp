#pragma once
#include "../../base/hash/fnv.hpp"
#include "../tab/tab.hpp"
#include <string>

namespace gui {

class TabControl : public Object {
public:
    int current = 0;

    TabControl() : Object("__tab_control") {
        // app::find_widget_recursive checks for this ID to traverse
        // through tab-control boundaries when path-walking widgets.
        id = fnv::hash_const("TabControl");
    }

    Tab* add_tab(std::string name, std::string /*icon*/ = "") {
        return add_object<Tab>(std::move(name));
    }

    void render() override;
};

}
