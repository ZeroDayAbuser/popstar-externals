#include <Cheat.hpp>

void Style::Update() {
    if (!overrideAccent)
        accentColor = Color(72, 151, 255);

    if (!overrideStyle)
    {
        shadow = Color(0, 0, 5);
        background = Color(30, 30, 35);
        outline = Color(40, 40, 45);
        header = Color(33, 33, 38);
        container = Color(32, 32, 37);
        containerHeader = Color(35, 35, 40);
        headerText = Color(220, 220, 225);
        text = Color(220, 220, 225);
        dimmedText = Color(100, 100, 105);
        widgetBackground = Color(35, 35, 40);
        widgetBackgroundHovered = Color(37, 37, 42);
        warning = Color(255, 200, 50);
    }

    if (!GImGui)
        return;

    static float interp = 0.0f;
    float h, s, v;
    ImGui::ColorConvertRGBtoHSV(accentColor.r / 255.0f, accentColor.g / 255.0f, accentColor.b / 255.0f, h, s, v);
    interp = std::clamp(interp + (5.0f * ImGui::GetIO().DeltaTime * (s < 0.5f ? 1.0f : -1.0f)), 0.0f, 1.0f);
    checkmark = Color::White().Lerp(Color::Black(), interp);
}
