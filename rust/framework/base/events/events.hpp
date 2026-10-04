#pragma once
#include <cstdint>
#include <glm/glm.hpp>

namespace gui {

enum class MouseButton : std::int32_t { Left, Right, Middle, X1, X2 };

class Event {
public:
    virtual ~Event() = default;
    bool handled = false;
};

class MouseMoveEvent : public Event {
public:
    glm::vec2 pos;
    explicit MouseMoveEvent(glm::vec2 p) : pos(p) {}
};

class MouseButtonEvent : public Event {
public:
    MouseButton button;
    bool down;
    glm::vec2 pos;
    MouseButtonEvent(MouseButton b, bool d, glm::vec2 p) : button(b), down(d), pos(p) {}
};

class MouseScrollEvent : public Event {
public:
    float delta;
    explicit MouseScrollEvent(float d) : delta(d) {}
};

class KeyPressEvent : public Event {
public:
    std::int32_t key;
    explicit KeyPressEvent(std::int32_t k) : key(k) {}
};

class KeyReleaseEvent : public Event {
public:
    std::int32_t key;
    explicit KeyReleaseEvent(std::int32_t k) : key(k) {}
};

class KeyCharEvent : public Event {
public:
    std::int32_t ch;
    explicit KeyCharEvent(std::int32_t c) : ch(c) {}
};

}
