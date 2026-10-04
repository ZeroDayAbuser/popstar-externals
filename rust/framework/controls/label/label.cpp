#include "label.hpp"
#include <imgui.h>

namespace gui {

void Label::render() {
    if (!should_display) return;
    if (show_label && !m_name.empty()) ImGui::TextUnformatted(m_name.c_str());
    for (auto& c : m_children) c->render();
}

}
