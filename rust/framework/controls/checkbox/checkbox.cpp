#include "checkbox.hpp"
#include <imgui.h>

namespace gui {

void Checkbox::render() {
    if (!should_display) return;
    const std::string label = m_name + "##cb" + std::to_string(id);
    ImGui::Checkbox(label.c_str(), &value);
    if (!m_children.empty()) {
        ImGui::Indent(12.0f);
        for (auto& c : m_children) c->render();
        ImGui::Unindent(12.0f);
    }
}

}
