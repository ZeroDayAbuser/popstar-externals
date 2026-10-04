#include "button.hpp"
#include <imgui.h>

namespace gui {

void Button::render() {
    if (!should_display) return;
    const std::string label = m_name + "##bt" + std::to_string(id);
    if (ImGui::Button(label.c_str())) {
        if (on_press) on_press();
    }
    for (auto& c : m_children) c->render();
}

}
