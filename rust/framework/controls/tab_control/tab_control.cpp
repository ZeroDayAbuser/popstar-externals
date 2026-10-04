#include "tab_control.hpp"
#include <imgui.h>

namespace gui {

void TabControl::render() {
    if (!should_display || m_children.empty()) return;
    const std::string id_str = "tabbar##" + std::to_string(id);
    if (ImGui::BeginTabBar(id_str.c_str(), ImGuiTabBarFlags_None)) {
        for (int i = 0; i < int(m_children.size()); ++i) {
            auto& page = m_children[i];
            const std::string tab_name = page->m_name + "##tc" + std::to_string(page->id);
            if (ImGui::BeginTabItem(tab_name.c_str())) {
                current = i;
                page->render();
                ImGui::EndTabItem();
            }
        }
        ImGui::EndTabBar();
    }
}

}
