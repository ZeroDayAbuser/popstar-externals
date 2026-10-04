#pragma once

namespace Style
{
    void Update();

    inline bool overrideAccent = false;

    inline bool overrideStyle = false;

    inline Color copiedColor = Color::White();

    inline Color accentColor = Color(10, 170, 255);
    inline Color checkmark = Color(0, 0, 0);
    inline Color shadow = Color(0, 0, 0);
    inline Color background = Color(0, 0, 0);
    inline Color outline = Color(0, 0, 0);
    inline Color header = Color(0, 0, 0);
    inline Color container = Color(0, 0, 0);
    inline Color containerHeader = Color(0, 0, 0);
    inline Color headerText = Color(0, 0, 0);
    inline Color dimmedText = Color(0, 0, 0);
    inline Color text = Color(0, 0, 0);
    inline Color widgetBackground = Color(0, 0, 0);
    inline Color widgetBackgroundHovered = Color(0, 0, 0);
    inline Color warning = Color(0, 0, 0);

    inline float rounding = 4.0f;
    inline float padding = 10.0f;

    inline float headerHeight = 35.0f;
    inline float containerHeaderHeight = 30.0f;
}
