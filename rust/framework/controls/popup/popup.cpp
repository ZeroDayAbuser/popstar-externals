#include "popup.hpp"
#include <imgui.h>

namespace gui {

void Popup::render() {
    if (!should_display) return;
    const std::string btn = "\xC2\xB7\xC2\xB7\xC2\xB7##pop" + std::to_string(id);
    const std::string popup_id = "popup_" + std::to_string(id);

    ImGui::SameLine();
    if (ImGui::SmallButton(btn.c_str())) ImGui::OpenPopup(popup_id.c_str());

    if (ImGui::BeginPopup(popup_id.c_str())) {
        if (!m_name.empty()) {
            ImGui::TextDisabled("%s", m_name.c_str());
            ImGui::Separator();
        }
        for (auto& c : m_children) c->render();
        ImGui::EndPopup();
    }
}

}
