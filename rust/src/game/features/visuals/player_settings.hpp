#pragma once
#include <algorithm>
#include <cstdint>
#include <cstdlib>   // for _byteswap_ulong on MSVC
#include <functional>
#include <type_traits>
#include <menu/framework/color.hpp>
#include <string>
#include <string_view>
#include <unordered_map>

namespace player_settings {

struct Snapshot {
    bool   players_enabled            = false;
    int    players_max_distance       = 500;
    bool   show_sleepers              = false;

    bool   vischeck_on                = false;
    bool   vischeck_dim_instead_hide  = false;
    bool   vischeck_visible_color_on  = false;
    Color  vischeck_visible_color     {};
    bool   vischeck_invisible_color_on= false;
    Color  vischeck_invisible_color   {};
    bool   per_bone_vis_on            = false;

    bool   skeleton                   = false;
    Color  skeleton_color             {};
    bool   skeleton_rounded           = false;
    bool   skeleton_vis_colors        = false;

    bool   head_circle                = false;
    Color  head_circle_color          {};

    bool   view_line                  = false;
    Color  view_line_color            {};

    bool   bbox                       = false;
    Color  bbox_color                 {};
    bool   bbox_fill                  = false;
    Color  bbox_fill_top              {};
    Color  bbox_fill_bottom           {};
    bool   bbox_gradient              = false;
    Color  bbox_gradient_color        {};

    bool   name                       = false;
    Color  name_color                 {};
    bool   show_avatar                = false;
    int    flag_bits                  = 0;

    bool   item_name                  = false;
    Color  item_name_color            {};
    bool   item_icon                  = false;
    Color  item_icon_color            {};

    bool   snap_lines                 = false;
    Color  snap_lines_color           {};
    int    snap_lines_alignment       = 1;

    bool   off_screen                 = false;
    Color  off_screen_color           {};
    float  off_screen_size            = 10.0f;
    float  off_screen_radius          = 1.0f;
};

void           refresh();
const Snapshot& get();

}

namespace entity_settings {

struct PerType {
    bool   enabled         = false;
    bool   category_enabled = false;
    Color  color           {};
    int    max_distance    = 250;
    bool   include_in_radar= false;
};

struct Snapshot {
    bool                        entities_enabled = false;
    bool   category_enabled_Ores       = false;
    bool   category_enabled_Crates     = false;
    bool   category_enabled_Deployable = false;
    bool   category_enabled_Traps      = false;
    bool   category_enabled_NPCs       = false;
    bool   category_enabled_Static     = false;
    bool   category_enabled_Vehicles   = false;
    bool   category_enabled_DroppedW   = false;
    bool   category_enabled_DroppedI   = false;
    std::unordered_map<std::string, PerType> types;
};

void           refresh();
const Snapshot& get();

inline const PerType* lookup(std::string_view category, std::string_view type) {
    const auto& s = get();
    std::string key;
    key.reserve(category.size() + 1 + type.size());
    key.append(category); key.push_back('|'); key.append(type);
    auto it = s.types.find(key);
    return it == s.types.end() ? nullptr : &it->second;
}

}
