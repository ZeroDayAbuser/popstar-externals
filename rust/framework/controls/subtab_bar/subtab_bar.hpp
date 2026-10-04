#pragma once
#include "../container/container.hpp"

namespace gui {

class SubTabBar : public Object {
public:
    class TabPage : public Container {
    public:
        using Container::Container;
    };

    int current = 0;

    SubTabBar() : Object("__subtab_bar") {}
    explicit SubTabBar(std::string name) : Object(std::move(name)) {}

    TabPage* add_tab(std::string name) { return add_object<TabPage>(std::move(name)); }
    void render() override;
};

}
