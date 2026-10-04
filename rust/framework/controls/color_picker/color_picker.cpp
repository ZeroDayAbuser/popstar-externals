#include "color_picker.hpp"
#include <imgui.h>

namespace gui {

void ColorPicker::render() {
    if (!should_display) return;
    float col[4] = { value.r / 255.0f, value.g / 255.0f, value.b / 255.0f, value.a / 255.0f };
    const std::string label = m_name + "##cp" + std::to_string(id);
    if (ImGui::ColorEdit4(label.c_str(), col,
            ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar |
            ImGuiColorEditFlags_AlphaPreviewHalf)) {
        value = Color{ int(col[0] * 255.0f), int(col[1] * 255.0f),
                       int(col[2] * 255.0f), int(col[3] * 255.0f) };
    }
    for (auto& c : m_children) c->render();
}

}
