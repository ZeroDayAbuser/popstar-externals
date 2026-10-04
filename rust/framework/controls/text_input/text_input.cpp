#include "text_input.hpp"
#include <imgui.h>

namespace gui {

void TextInput::render() {
    if (!should_display) return;
    const std::string label = m_name + "##ti" + std::to_string(id);
    char buf[512] = {};
    const std::size_t n = std::min(storage.size(), sizeof(buf) - 1);
    std::memcpy(buf, storage.data(), n);
    if (ImGui::InputTextWithHint(label.c_str(),
            placeholder.c_str(), buf, sizeof(buf))) {
        storage = buf;
    }
    for (auto& c : m_children) c->render();
}

}
