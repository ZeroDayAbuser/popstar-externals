#pragma once

#include <gordoui/OverlayBridge.hpp>

namespace overlay_geom
{
    struct MenuShape
    {
        float x = 0.0f;
        float y = 0.0f;
        float w = 0.0f;
        float h = 0.0f;
        float radius = 8.0f;
        bool  valid = false;
    };

    inline bool is_menu_open()
    {
        return OverlayBridge::IsMenuOpen();
    }

    inline MenuShape current_menu_rect()
    {
        MenuShape s{};
        if (!OverlayBridge::IsMenuOpen())
            return s;
        const auto r = OverlayBridge::CurrentShape();
        s.x = r.x;
        s.y = r.y;
        s.w = r.w;
        s.h = r.h;
        s.radius = r.r;
        s.valid = (s.w >= 1.0f && s.h >= 1.0f);
        return s;
    }
}
