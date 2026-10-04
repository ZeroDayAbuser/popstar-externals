#include "container.hpp"
#include <imgui.h>

namespace gui {

void Container::render() {
    if (!should_display) return;

    const std::string label = m_name + "##c" + std::to_string(id);
    if (show_label && !m_name.empty()) {
        ImGui::SeparatorText(m_name.c_str());
    } else {
        ImGui::Spacing();
    }

    ImGui::PushID(int(id));
    for (auto& c : m_children) c->render();
    ImGui::PopID();
}

}
