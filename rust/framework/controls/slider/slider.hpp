#pragma once
#include "../../base/object/object.hpp"
#include <functional>
#include <imgui.h>
#include <string>
#include <type_traits>

namespace gui {

template <typename T>
class Slider : public Object {
public:
    T value{};
    T min_value{};
    T max_value{};
    T step_value{};
    int precision = 0;
    std::string suffix;
    std::function<std::string(T)> format_value;

    Slider() = default;
    Slider(std::string name, T mn, T mx, T def,
           std::string suff = "", int prec = 0, T step = T(1))
        : Object(std::move(name)),
          value(def), min_value(mn), max_value(mx), step_value(step),
          precision(prec), suffix(std::move(suff)) {}

    void render() override {
        if (!should_display) return;

        std::string display;
        if (format_value) display = format_value(value);
        else {
            char buf[64];
            if constexpr (std::is_integral_v<T>) {
                std::snprintf(buf, sizeof(buf), "%lld", (long long)value);
            } else {
                std::snprintf(buf, sizeof(buf), "%.*f", precision > 0 ? precision : 2, double(value));
            }
            display = buf;
            display += suffix;
        }

        const std::string label = m_name + ": " + display + "##sl" + std::to_string(id);

        if constexpr (std::is_integral_v<T>) {
            int v = int(value);
            if (ImGui::SliderInt(label.c_str(), &v, int(min_value), int(max_value))) {
                value = T(v);
            }
        } else {
            float v = float(value);
            if (ImGui::SliderFloat(label.c_str(), &v, float(min_value), float(max_value), "%.3f")) {
                value = T(v);
            }
        }

        if (!m_children.empty()) {
            ImGui::Indent(12.0f);
            for (auto& c : m_children) c->render();
            ImGui::Unindent(12.0f);
        }
    }
};

}
