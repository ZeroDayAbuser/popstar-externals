#include "listbox.hpp"
#include <imgui.h>

namespace gui {

void Listbox::render() {
    if (!should_display) return;
    const std::string label = m_name + "##lb" + std::to_string(id);
    const float row_h = ImGui::GetTextLineHeightWithSpacing();
    const ImVec2 sz(0.0f, row_h * float(visible_rows));
    if (show_label && !m_name.empty()) ImGui::TextUnformatted(m_name.c_str());
    if (ImGui::BeginListBox(label.c_str(), sz)) {
        for (int i = 0; i < int(options.size()); ++i) {
            const bool sel = (i == value);
            if (ImGui::Selectable(options[i].c_str(), sel)) value = i;
            if (sel) ImGui::SetItemDefaultFocus();
        }
        ImGui::EndListBox();
    }
    for (auto& c : m_children) c->render();
}

}
