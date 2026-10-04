#pragma once
#include "../base/object/object.hpp"
#include <glm/glm.hpp>
#include <string>

namespace gui {

class Window : public Object {
public:
    glm::vec2 m_position{100.f, 100.f};
    glm::vec2 m_size    {680.f, 550.f};
    bool      m_opened = true;
    bool      m_dragging = false;
    glm::vec2 m_drag_offset{0.f, 0.f};

    int       m_toggle_key = 0x2D;
    bool      m_toggle_key_was_down = false;

    std::string window_title_show;
    std::string window_tld;

    Window(std::string name, glm::vec2 position, glm::vec2 size)
        : Object(std::move(name)) {
        m_position = position;
        m_size     = size;
    }

    void render() override;
    void dispatch_event(Event& e) override;
};

}
