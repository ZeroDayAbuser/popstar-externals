#include "window.hpp"
#include "../render/render.hpp"
#include "../utils/style.hpp"
#include <imgui.h>
#include <Windows.h>

namespace gui {

void Window::render() {
    if (m_toggle_key != 0) {
        const bool now_down = (::GetAsyncKeyState(m_toggle_key) & 0x8000) != 0;
        if (now_down && !m_toggle_key_was_down) {
            m_opened = !m_opened;
        }
        m_toggle_key_was_down = now_down;
    }

    if (!m_opened) return;

    ImGui::SetNextWindowPos({m_position.x, m_position.y}, ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize({m_size.x, m_size.y}, ImGuiCond_FirstUseEver);

    const std::string title = window_title_show.empty()
        ? m_name + "##" + std::to_string(id)
        : window_title_show + window_tld + "##" + std::to_string(id);

    ImGui::PushStyleColor(ImGuiCol_WindowBg,
        IM_COL32(gui::style::colors::bg.r, gui::style::colors::bg.g,
                 gui::style::colors::bg.b, gui::style::colors::bg.a));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, gui::style::corner_radius);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(gui::style::padding, gui::style::padding));

    if (ImGui::Begin(title.c_str(), &m_opened,
                     ImGuiWindowFlags_NoCollapse))
    {
        for (auto& c : m_children) {
            c->render();
        }
        const ImVec2 p = ImGui::GetWindowPos();
        const ImVec2 s = ImGui::GetWindowSize();
        m_position = { p.x, p.y };
        m_size     = { s.x, s.y };
    }
    ImGui::End();

    ImGui::PopStyleVar(2);
    ImGui::PopStyleColor();
}

void Window::dispatch_event(Event& e) {
    if (auto* kp = dynamic_cast<KeyPressEvent*>(&e)) {
        if (kp->key == m_toggle_key) {
            e.handled = true;
            return;
        }
    }
    Object::dispatch_event(e);
}

}
