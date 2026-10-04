#include "player_settings.hpp"

#include <settings/settings.hpp>

namespace player_settings {

static Snapshot g_snap{};

void refresh() {
    Snapshot s{};

    const auto& p = settings.visuals.players;

    s.players_enabled      = p.enabled;
    s.players_max_distance = p.max_distance;
    s.show_sleepers        = p.show_sleepers;

    s.vischeck_on                 = p.vischeck;
    s.vischeck_dim_instead_hide   = p.vis_dim_instead_of_hide;
    s.vischeck_visible_color_on   = p.vis_visible_color_enabled;
    s.vischeck_visible_color      = p.vis_visible_color;
    s.vischeck_invisible_color_on = p.vis_invisible_color_enabled;
    s.vischeck_invisible_color    = p.vis_invisible_color;
    s.per_bone_vis_on             = p.vis_per_bone_skeleton;

    s.skeleton           = p.skeleton;
    s.skeleton_color     = p.skeleton_color;
    s.skeleton_rounded   = p.skeleton_rounded;
    s.skeleton_vis_colors= p.skeleton_vis_colors;

    s.head_circle       = p.head_circle;
    s.head_circle_color = p.head_circle_color;

    s.view_line       = p.view_line;
    s.view_line_color = p.view_line_color;

    s.bbox                = p.bounding_box;
    s.bbox_color          = p.bounding_box_color;
    s.bbox_fill           = p.bounding_box_fill;
    s.bbox_fill_top       = p.bounding_box_fill_color_top;
    s.bbox_fill_bottom    = p.bounding_box_fill_color_bottom;
    s.bbox_gradient       = p.bounding_box_gradient;
    s.bbox_gradient_color = p.bounding_box_gradient_color;

    s.name        = p.name;
    s.name_color  = p.name_color;
    s.show_avatar = p.name_show_avatar;

    int flags = 0;
    for (int i = 0; i < 5; ++i)
        if (p.extra_player_flags[i]) flags |= (1 << i);
    flags |= settings.visuals.flags;
    flags |= p.flags;
    s.flag_bits = flags;

    s.item_name       = p.item_name;
    s.item_name_color = p.item_name_color;
    s.item_icon       = p.item_icon;
    s.item_icon_color = p.item_icon_color;

    s.snap_lines           = p.snap_lines;
    s.snap_lines_color     = p.snap_lines_color;
    s.snap_lines_alignment = p.snap_lines_alignment;

    s.off_screen        = p.out_of_view_arrows;
    s.off_screen_color  = p.out_of_view_arrows_color;
    s.off_screen_size   = p.out_of_view_arrows_size;
    s.off_screen_radius = p.out_of_view_arrows_radius;

    g_snap = s;
}

const Snapshot& get() { return g_snap; }

} // namespace player_settings

namespace entity_settings {

static Snapshot g_entity_snap{};

static Settings::VisualsSettings::EntityTypeSettings*
lookup_entity_field(int cat_idx, std::string_view name)
{
    using Ents = Settings::VisualsSettings;
    auto& e = settings.visuals.entities;

    auto eq = [&](const char* lit) { return name == lit; };

    switch (cat_idx) {
        case 0: { // Ores & Collectibles
            auto& c = e.ores_collectibles;
            if (eq("stone"))          return &c.stone;
            if (eq("sulfur"))         return &c.sulfur;
            if (eq("metal"))          return &c.metal;
            if (eq("hemp"))           return &c.hemp;
            if (eq("wood"))           return &c.wood;
            if (eq("diesel_fuel"))    return &c.diesel_fuel;
            if (eq("green_keycard"))  return &c.green_keycard;
            if (eq("blue_keycard"))   return &c.blue_keycard;
            if (eq("red_keycard"))    return &c.red_keycard;
            break;
        }
        case 1: { // Crates & Barrels
            auto& c = e.crates_barrels;
            if (eq("elite_crate"))      return &c.elite_crate;
            if (eq("military_crate"))   return &c.military_crate;
            if (eq("locked_crate"))     return &c.locked_crate;
            if (eq("air_drop"))         return &c.air_drop;
            if (eq("loot_barrel"))      return &c.loot_barrel;
            if (eq("oil_barrel"))       return &c.oil_barrel;
            if (eq("food_crate"))       return &c.food_crate;
            if (eq("tool_crate"))       return &c.tool_crate;
            if (eq("small_crate"))      return &c.small_crate;
            if (eq("health_crate"))     return &c.health_crate;
            if (eq("small_food_crate")) return &c.small_food_crate;
            if (eq("vehicle_parts"))    return &c.vehicle_parts;
            if (eq("crate"))            return &c.crate;
            break;
        }
        case 2: { // Deployables
            auto& c = e.deployables;
            if (eq("wooden_box"))       return &c.wooden_box;
            if (eq("large_wood_box"))   return &c.large_wood_box;
            if (eq("tool_cupboard"))    return &c.tool_cupboard;
            if (eq("tier_1_workbench")) return &c.tier_1_workbench;
            if (eq("tier_2_workbench")) return &c.tier_2_workbench;
            if (eq("tier_3_workbench")) return &c.tier_3_workbench;
            if (eq("repair_bench"))     return &c.repair_bench;
            if (eq("research_table"))   return &c.research_table;
            if (eq("small_stash"))      return &c.small_stash;
            if (eq("sleeping_bag"))     return &c.sleeping_bag;
            if (eq("furnace"))          return &c.furnace;
            if (eq("locker"))           return &c.locker;
            break;
        }
        case 3: { // Traps & Turrets
            auto& c = e.traps_turrets;
            if (eq("auto_turret"))  return &c.auto_turret;
            if (eq("flame_turret")) return &c.flame_turret;
            if (eq("shotgun_trap")) return &c.shotgun_trap;
            break;
        }
        case 4: { // NPCs & Animals
            auto& c = e.npcs_animals;
            if (eq("scientist")) return &c.scientist;
            if (eq("dweller"))   return &c.dweller;
            if (eq("bear"))      return &c.bear;
            if (eq("wolf"))      return &c.wolf;
            if (eq("stag"))      return &c.stag;
            if (eq("boar"))      return &c.boar;
            if (eq("horse"))     return &c.horse;
            if (eq("chicken"))   return &c.chicken;
            if (eq("shark"))     return &c.shark;
            if (eq("scarecrow")) return &c.scarecrow;
            if (eq("corpse"))    return &c.corpse;
            break;
        }
        case 5: { // Static & Monuments
            auto& c = e.static_monuments;
            if (eq("recycler"))         return &c.recycler;
            if (eq("phone_booth"))      return &c.phone_booth;
            if (eq("fuse_box"))         return &c.fuse_box;
            if (eq("refinery"))         return &c.refinery;
            if (eq("elevator"))         return &c.elevator;
            if (eq("car_lift"))         return &c.car_lift;
            if (eq("computer_station")) return &c.computer_station;
            break;
        }
        case 6: { // Vehicles
            auto& c = e.vehicles;
            if (eq("motorbike"))         return &c.motorbike;
            if (eq("sidecar_motorbike")) return &c.sidecar_motorbike;
            if (eq("pedal_bike"))        return &c.pedal_bike;
            if (eq("snowmobile"))        return &c.snowmobile;
            if (eq("tomaha_snowmobile")) return &c.tomaha_snowmobile;
            if (eq("tugboat"))           return &c.tugboat;
            if (eq("rowboat"))           return &c.rowboat;
            if (eq("rhib"))              return &c.rhib;
            if (eq("kyak"))              return &c.kyak;
            if (eq("solo_submarine"))    return &c.solo_submarine;
            if (eq("duo_submarine"))     return &c.duo_submarine;
            if (eq("minicopter"))        return &c.minicopter;
            if (eq("scrap_heli"))        return &c.scrap_heli;
            if (eq("attack_heli"))       return &c.attack_heli;
            break;
        }
        case 7: { // Dropped weapons
            auto& c = e.dropped_weapons;
            if (eq("dropped_rifles"))    return &c.dropped_rifles;
            if (eq("dropped_snipers"))   return &c.dropped_snipers;
            if (eq("dropped_smgs"))      return &c.dropped_smgs;
            if (eq("dropped_shotguns"))  return &c.dropped_shotguns;
            if (eq("dropped_pistols"))   return &c.dropped_pistols;
            if (eq("dropped_lmgs"))      return &c.dropped_lmgs;
            if (eq("dropped_launchers")) return &c.dropped_launchers;
            if (eq("dropped_bows"))      return &c.dropped_bows;
            if (eq("dropped_melee"))     return &c.dropped_melee;
            break;
        }
        case 8: { // Dropped items
            auto& c = e.dropped_items;
            if (eq("throwables")) return &c.throwables;
            if (eq("tools"))      return &c.tools;
            if (eq("medical"))    return &c.medical;
            if (eq("ammo"))       return &c.ammo;
            if (eq("misc"))       return &c.misc;
            break;
        }
        default: break;
    }
    (void)Ents{};
    return nullptr;
}

struct DisplayPair { const char* cat; const char* type; };
static constexpr DisplayPair kDisplayPairs[] = {
    {"Ores & Collectibles","Stone"},{"Ores & Collectibles","Sulfur"},
    {"Ores & Collectibles","Metal"},{"Ores & Collectibles","Hemp"},
    {"Ores & Collectibles","Wood"}, {"Ores & Collectibles","Diesel fuel"},
    {"Ores & Collectibles","Green keycard"},{"Ores & Collectibles","Blue keycard"},
    {"Ores & Collectibles","Red keycard"},
    {"Crates & Barrels","Elite crate"},{"Crates & Barrels","Military crate"},
    {"Crates & Barrels","Locked crate"},{"Crates & Barrels","Air drop"},
    {"Crates & Barrels","Loot barrel"},{"Crates & Barrels","Oil barrel"},
    {"Crates & Barrels","Food crate"},{"Crates & Barrels","Tool crate"},
    {"Crates & Barrels","Small crate"},{"Crates & Barrels","Health crate"},
    {"Crates & Barrels","Small food crate"},{"Crates & Barrels","Vehicle parts"},
    {"Crates & Barrels","Crate"},
    {"Deployables","Wooden box"},{"Deployables","Large wood box"},
    {"Deployables","Tool cupboard"},{"Deployables","Tier 1 workbench"},
    {"Deployables","Tier 2 workbench"},{"Deployables","Tier 3 workbench"},
    {"Deployables","Repair bench"},{"Deployables","Research table"},
    {"Deployables","Small stash"},{"Deployables","Sleeping bag"},
    {"Deployables","Furnace"},{"Deployables","Locker"},
    {"Traps & Turrets","Auto turret"},{"Traps & Turrets","Flame turret"},
    {"Traps & Turrets","Shotgun trap"},
    {"NPCs & Animals","Scientist"},{"NPCs & Animals","Dweller"},
    {"NPCs & Animals","Bear"},{"NPCs & Animals","Wolf"},
    {"NPCs & Animals","Stag"},{"NPCs & Animals","Boar"},
    {"NPCs & Animals","Horse"},{"NPCs & Animals","Chicken"},
    {"NPCs & Animals","Shark"},{"NPCs & Animals","Scarecrow"},
    {"NPCs & Animals","Corpse"},
    {"Static & Monuments","Recycler"},{"Static & Monuments","Phone booth"},
    {"Static & Monuments","Fuse box"},{"Static & Monuments","Refinery"},
    {"Static & Monuments","Elevator"},{"Static & Monuments","Car lift"},
    {"Static & Monuments","Computer station"},
    {"Vehicles","Motorbike"},{"Vehicles","Sidecar motorbike"},
    {"Vehicles","Pedal bike"},{"Vehicles","Snowmobile"},
    {"Vehicles","Tomaha snowmobile"},{"Vehicles","Tugboat"},
    {"Vehicles","Rowboat"},{"Vehicles","RHIB"},
    {"Vehicles","Kyak"},{"Vehicles","Solo submarine"},
    {"Vehicles","Duo submarine"},{"Vehicles","Minicopter"},
    {"Vehicles","Scrap heli"},{"Vehicles","Attack heli"},
    {"Dropped weapons","Dropped rifles"},{"Dropped weapons","Dropped snipers"},
    {"Dropped weapons","Dropped SMGs"},{"Dropped weapons","Dropped shotguns"},
    {"Dropped weapons","Dropped pistols"},{"Dropped weapons","Dropped LMGs"},
    {"Dropped weapons","Dropped launchers"},{"Dropped weapons","Dropped bows"},
    {"Dropped weapons","Dropped melee"},
    {"Dropped items","Throwables"},{"Dropped items","Tools"},
    {"Dropped items","Medical"},{"Dropped items","Ammo"},
    {"Dropped items","Misc"},
};

void refresh() {
    Snapshot s{};

    const auto& e = settings.visuals.entities;

    s.entities_enabled            = e.enabled;
    s.category_enabled_Ores       = e.ores_collectibles.enabled;
    s.category_enabled_Crates     = e.crates_barrels.enabled;
    s.category_enabled_Deployable = e.deployables.enabled;
    s.category_enabled_Traps      = e.traps_turrets.enabled;
    s.category_enabled_NPCs       = e.npcs_animals.enabled;
    s.category_enabled_Static     = e.static_monuments.enabled;
    s.category_enabled_Vehicles   = e.vehicles.enabled;
    s.category_enabled_DroppedW   = e.dropped_weapons.enabled;
    s.category_enabled_DroppedI   = e.dropped_items.enabled;

    static_assert(sizeof(kDisplayPairs) / sizeof(kDisplayPairs[0])
                  == sizeof(settings_meta::kEntityPairs) / sizeof(settings_meta::kEntityPairs[0]),
                  "kDisplayPairs must match settings_meta::kEntityPairs row-for-row");

    const std::size_t n = sizeof(settings_meta::kEntityPairs) / sizeof(settings_meta::kEntityPairs[0]);
    s.types.reserve(n);

    for (std::size_t i = 0; i < n; ++i) {
        const auto& row = settings_meta::kEntityPairs[i];
        const auto& disp = kDisplayPairs[i];

        auto* leaf = lookup_entity_field(row.category_index, row.field_name);
        if (!leaf) continue;

        PerType pt{};
        pt.enabled          = leaf->enabled;
        pt.color            = leaf->color;
        pt.max_distance     = leaf->max_distance;
        pt.include_in_radar = leaf->include_in_radar;

        switch (row.category_index) {
            case 0: pt.category_enabled = s.category_enabled_Ores;       break;
            case 1: pt.category_enabled = s.category_enabled_Crates;     break;
            case 2: pt.category_enabled = s.category_enabled_Deployable; break;
            case 3: pt.category_enabled = s.category_enabled_Traps;      break;
            case 4: pt.category_enabled = s.category_enabled_NPCs;       break;
            case 5: pt.category_enabled = s.category_enabled_Static;     break;
            case 6: pt.category_enabled = s.category_enabled_Vehicles;   break;
            case 7: pt.category_enabled = s.category_enabled_DroppedW;   break;
            case 8: pt.category_enabled = s.category_enabled_DroppedI;   break;
            default: break;
        }

        std::string key;
        key.reserve(std::string_view(disp.cat).size() + 1 + std::string_view(disp.type).size());
        key.append(disp.cat); key.push_back('|'); key.append(disp.type);
        s.types.emplace(std::move(key), pt);
    }

    g_entity_snap = std::move(s);
}

const Snapshot& get() { return g_entity_snap; }

} // namespace entity_settings
