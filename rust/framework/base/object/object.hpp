#pragma once
#include "../events/events.hpp"
#include "../hash/fnv.hpp"
#include "../animation/animation.hpp"
#include "../../utils/color.hpp"
#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace gui {

class Object;

class Object {
public:
    virtual ~Object() = default;

    std::string  m_name;
    std::uint64_t id = 0;
    Object*      m_parent = nullptr;
    std::vector<std::shared_ptr<Object>> m_children;

    bool should_display    = true;
    bool should_save       = true;
    bool show_label        = true;
    bool hide_subtab_labels = false;

    Object() = default;
    explicit Object(std::string name) : m_name(std::move(name)), id(fnv::hash(m_name)) {}

    virtual void render() {}
    virtual void update() {}
    virtual void update_layout() {}

    virtual void dispatch_event(Event& e) {
        for (auto& c : m_children) {
            if (e.handled) return;
            c->dispatch_event(e);
        }
    }

    virtual void for_each_logical_child(const std::function<void(Object*)>& fn) {
        for (auto& c : m_children) fn(c.get());
    }

    template <typename T, typename... Args>
    T* add_object(Args&&... args) {
        auto p = std::make_shared<T>(std::forward<Args>(args)...);
        p->m_parent = this;
        T* raw = p.get();
        m_children.push_back(std::move(p));
        return raw;
    }
};

}
