#include "multi_dropdown.hpp"
#include <imgui.h>

namespace gui {

void MultiDropdown::render() {
    if (!should_display) return;
    const std::string label = m_name + "##md" + std::to_string(id);

    std::string preview;
    int count = 0;
    for (int i = 0; i < int(options.size()); ++i) {
        if (value & (1 << i)) {
            if (count++) preview += ", ";
            preview += options[i];
        }
    }
    if (preview.empty()) preview = "(none)";

    if (ImGui::BeginCombo(label.c_str(), preview.c_str())) {
        for (int i = 0; i < int(options.size()); ++i) {
            bool sel = (value & (1 << i)) != 0;
            if (ImGui::Checkbox(options[i].c_str(), &sel)) {
                if (sel) value |= (1 << i);
                else     value &= ~(1 << i);
            }
        }
        ImGui::EndCombo();
    }
    for (auto& c : m_children) c->render();
}

}
