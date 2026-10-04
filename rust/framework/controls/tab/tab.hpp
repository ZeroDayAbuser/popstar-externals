#pragma once
#include "../container/container.hpp"
#include "../subtab_bar/subtab_bar.hpp"

namespace gui {

// A Tab page within a TabControl. Behaves like a Container plus the
// add_full_subtab_bar() helper that drops a SubTabBar in as its child.
class Tab : public Container {
public:
    using Container::Container;

    SubTabBar* add_full_subtab_bar() {
        return add_object<SubTabBar>("__full_subtab_bar");
    }
};

}
