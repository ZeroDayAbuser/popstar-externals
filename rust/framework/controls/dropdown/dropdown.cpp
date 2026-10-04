#include "dropdown.hpp"
#include <imgui.h>

namespace gui {

void Dropdown::render() {
    if (!should_display) return;
    const std::string label = m_name + "##dd" + std::to_string(id);
    const char* current = (value >= 0 && value < int(options.size())) ?
                         options[value].c_str() : "";
    if (ImGui::BeginCombo(label.c_str(), current)) {
        for (int i = 0; i < int(options.size()); ++i) {
            const bool sel = (i == value);
            if (ImGui::Selectable(options[i].c_str(), sel)) value = i;
            if (sel) ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }
    for (auto& c : m_children) c->render();
}

}
