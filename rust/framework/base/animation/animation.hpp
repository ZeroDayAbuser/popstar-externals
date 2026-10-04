#pragma once
#include <algorithm>

namespace gui {

class Animation {
public:
    float value = 0.0f;
    float speed = 0.15f;

    Animation() = default;
    explicit Animation(float initial) : value(initial) {}

    void update(float target, float s = -1.0f) {
        const float k = (s < 0.0f) ? speed : s;
        value += (target - value) * std::clamp(k, 0.0f, 1.0f);
    }

    operator float() const { return value; }
};

}
