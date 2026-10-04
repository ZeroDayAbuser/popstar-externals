#include <pawjob_umbrella.hpp>
#include <settings/settings.hpp>
#include <menu/menu.hpp>
#include <sdk/globals.hpp>

#include <cstring>

namespace
{
    using EntityLeaf = Settings::VisualsSettings::EntityTypeSettings;

    bool* GetCategoryMasterPtr(int category_index)
    {
        auto& E = settings.visuals.entities;
        switch (category_index)
        {
        case 0: return &E.ores_collectibles.enabled;
        case 1: return &E.crates_barrels.enabled;
        case 2: return &E.deployables.enabled;
        case 3: return &E.traps_turrets.enabled;
        case 4: return &E.npcs_animals.enabled;
        case 5: return &E.static_monuments.enabled;
        case 6: return &E.vehicles.enabled;
        case 7: return &E.dropped_weapons.enabled;
        case 8: return &E.dropped_items.enabled;
        }
        return nullptr;
    }

    EntityLeaf* GetEntityLeafPtr(int category_index, const char* field)
    {
        auto& E = settings.visuals.entities;
        auto eq = [&](const char* n) { return std::strcmp(field, n) == 0; };

        switch (category_index)
        {
        case 0: {
            auto& C = E.ores_collectibles;
            if (eq("stone"))          return &C.stone;
            if (eq("sulfur"))         return &C.sulfur;
            if (eq("metal"))          return &C.metal;
            if (eq("hemp"))           return &C.hemp;
            if (eq("wood"))           return &C.wood;
            if (eq("diesel_fuel"))    return &C.diesel_fuel;
            if (eq("green_keycard")) return &C.green_keycard;
            if (eq("blue_keycard"))  return &C.blue_keycard;
            if (eq("red_keycard"))   return &C.red_keycard;
        } break;
        case 1: {
            auto& C = E.crates_barrels;
            if (eq("elite_crate"))     return &C.elite_crate;
            if (eq("military_crate"))  return &C.military_crate;
            if (eq("locked_crate"))    return &C.locked_crate;
            if (eq("air_drop"))        return &C.air_drop;
            if (eq("loot_barrel"))     return &C.loot_barrel;
            if (eq("oil_barrel"))      return &C.oil_barrel;
            if (eq("food_crate"))      return &C.food_crate;
            if (eq("tool_crate"))      return &C.tool_crate;
            if (eq("small_crate"))     return &C.small_crate;
            if (eq("health_crate"))    return &C.health_crate;
            if (eq("small_food_crate"))return &C.small_food_crate;
            if (eq("vehicle_parts"))   return &C.vehicle_parts;
            if (eq("crate"))           return &C.crate;
        } break;
        case 2: {
            auto& C = E.deployables;
            if (eq("wooden_box"))       return &C.wooden_box;
            if (eq("large_wood_box"))   return &C.large_wood_box;
            if (eq("tool_cupboard"))    return &C.tool_cupboard;
            if (eq("tier_1_workbench")) return &C.tier_1_workbench;
            if (eq("tier_2_workbench")) return &C.tier_2_workbench;
            if (eq("tier_3_workbench")) return &C.tier_3_workbench;
            if (eq("repair_bench"))     return &C.repair_bench;
            if (eq("research_table"))   return &C.research_table;
            if (eq("small_stash"))      return &C.small_stash;
            if (eq("sleeping_bag"))     return &C.sleeping_bag;
            if (eq("furnace"))          return &C.furnace;
            if (eq("locker"))           return &C.locker;
        } break;
        case 3: {
            auto& C = E.traps_turrets;
            if (eq("auto_turret"))  return &C.auto_turret;
            if (eq("flame_turret")) return &C.flame_turret;
            if (eq("shotgun_trap")) return &C.shotgun_trap;
        } break;
        case 4: {
            auto& C = E.npcs_animals;
            if (eq("scientist")) return &C.scientist;
            if (eq("dweller"))   return &C.dweller;
            if (eq("bear"))      return &C.bear;
            if (eq("wolf"))      return &C.wolf;
            if (eq("stag"))      return &C.stag;
            if (eq("boar"))      return &C.boar;
            if (eq("horse"))     return &C.horse;
            if (eq("chicken"))   return &C.chicken;
            if (eq("shark"))     return &C.shark;
            if (eq("scarecrow")) return &C.scarecrow;
            if (eq("corpse"))    return &C.corpse;
        } break;
        case 5: {
            auto& C = E.static_monuments;
            if (eq("recycler"))         return &C.recycler;
            if (eq("phone_booth"))      return &C.phone_booth;
            if (eq("fuse_box"))         return &C.fuse_box;
            if (eq("refinery"))         return &C.refinery;
            if (eq("elevator"))         return &C.elevator;
            if (eq("car_lift"))         return &C.car_lift;
            if (eq("computer_station")) return &C.computer_station;
        } break;
        case 6: {
            auto& C = E.vehicles;
            if (eq("motorbike"))         return &C.motorbike;
            if (eq("sidecar_motorbike")) return &C.sidecar_motorbike;
            if (eq("pedal_bike"))        return &C.pedal_bike;
            if (eq("snowmobile"))        return &C.snowmobile;
            if (eq("tomaha_snowmobile")) return &C.tomaha_snowmobile;
            if (eq("tugboat"))           return &C.tugboat;
            if (eq("rowboat"))           return &C.rowboat;
            if (eq("rhib"))              return &C.rhib;
            if (eq("kyak"))              return &C.kyak;
            if (eq("solo_submarine"))    return &C.solo_submarine;
            if (eq("duo_submarine"))     return &C.duo_submarine;
            if (eq("minicopter"))        return &C.minicopter;
            if (eq("scrap_heli"))        return &C.scrap_heli;
            if (eq("attack_heli"))       return &C.attack_heli;
        } break;
        case 7: {
            auto& C = E.dropped_weapons;
            if (eq("dropped_rifles"))    return &C.dropped_rifles;
            if (eq("dropped_snipers"))   return &C.dropped_snipers;
            if (eq("dropped_smgs"))      return &C.dropped_smgs;
            if (eq("dropped_shotguns"))  return &C.dropped_shotguns;
            if (eq("dropped_pistols"))   return &C.dropped_pistols;
            if (eq("dropped_lmgs"))      return &C.dropped_lmgs;
            if (eq("dropped_launchers")) return &C.dropped_launchers;
            if (eq("dropped_bows"))      return &C.dropped_bows;
            if (eq("dropped_melee"))     return &C.dropped_melee;
        } break;
        case 8: {
            auto& C = E.dropped_items;
            if (eq("throwables")) return &C.throwables;
            if (eq("tools"))      return &C.tools;
            if (eq("medical"))    return &C.medical;
            if (eq("ammo"))       return &C.ammo;
            if (eq("misc"))       return &C.misc;
        } break;
        }
        return nullptr;
    }
} // namespace

void Menu::InitVisualsTab(CTab* tab)
{
    if (auto Players = tab->AddObject<CSubtab>(X("Players")))
    {
        if (auto Container = Players->AddObject<CContainer>(X("Players")))
        {
            auto master = Container->AddObject<CCheckbox>(X("Enabled"), &settings.visuals.players.enabled);
            if (auto popup = master->AddObject<CPopup>())
            {
                popup->AddObject<CSliderInt>(X("Max distance"), &settings.visuals.players.max_distance, 0, 1000, X("{}m"));
            }

            Container->AddObject<CCheckbox>(X("Show sleepers"), &settings.visuals.players.show_sleepers);
            Container->AddObject<CCheckbox>(X("Skip crouching"), &settings.visuals.players.skip_crouching);
            Container->AddObject<CCheckbox>(X("Skip scoped"), &settings.visuals.players.skip_scoped);

            auto vis = Container->AddObject<CCheckbox>(X("Vischeck"), &settings.visuals.players.vischeck);
            if (auto popup = vis->AddObject<CPopup>())
            {
                auto visCol = popup->AddObject<CCheckbox>(X("Visible color"), &settings.visuals.players.vis_visible_color_enabled);
                visCol->AddObject<CColorPicker>(X("Color"), &settings.visuals.players.vis_visible_color);
                auto invCol = popup->AddObject<CCheckbox>(X("Invisible color"), &settings.visuals.players.vis_invisible_color_enabled);
                invCol->AddObject<CColorPicker>(X("Color"), &settings.visuals.players.vis_invisible_color);
                popup->AddObject<CCheckbox>(X("Dim occluded"), &settings.visuals.players.vis_dim_instead_of_hide);
                popup->AddObject<CCheckbox>(X("Per-bone skeleton"), &settings.visuals.players.vis_per_bone_skeleton);
            }

            auto name = Container->AddObject<CCheckbox>(X("Name"), &settings.visuals.players.name);
            name->AddObject<CColorPicker>(X("Color"), &settings.visuals.players.name_color);

            Container->AddObject<CCheckbox>(X("Steam Avatar"), &settings.visuals.players.name_show_avatar);

            {
                static const std::vector<std::string> kFlagItems = {
                    X("Distance"), X("Sleeper"), X("Wounded"), X("Team"), X("ADS")
                };
                Container->AddObject<CMultiDropdown>(X("Flags"),
                    &settings.visuals.players.flags, kFlagItems);
            }

            auto box = Container->AddObject<CCheckbox>(X("Bounding Box"), &settings.visuals.players.bounding_box);
            box->AddObject<CColorPicker>(X("Color"), &settings.visuals.players.bounding_box_color);
            if (auto popup = box->AddObject<CPopup>())
            {
                popup->AddObject<CCheckbox>(X("Outline"), &settings.visuals.players.bounding_box_outline);
                popup->AddObject<CCheckbox>(X("Corner box"), &settings.visuals.players.bounding_box_corner);
                auto gradient = popup->AddObject<CCheckbox>(X("Gradient"), &settings.visuals.players.bounding_box_gradient);
                gradient->AddObject<CColorPicker>(X("Color"), &settings.visuals.players.bounding_box_gradient_color);
                auto fill = popup->AddObject<CCheckbox>(X("Fill"), &settings.visuals.players.bounding_box_fill);
                fill->AddObject<CColorPicker>(X("Top"), &settings.visuals.players.bounding_box_fill_color_top);
                fill->AddObject<CColorPicker>(X("Bottom"), &settings.visuals.players.bounding_box_fill_color_bottom);
            }

            auto skeleton = Container->AddObject<CCheckbox>(X("Skeleton"), &settings.visuals.players.skeleton);
            skeleton->AddObject<CColorPicker>(X("Color"), &settings.visuals.players.skeleton_color);
            if (auto popup = skeleton->AddObject<CPopup>())
            {
                popup->AddObject<CCheckbox>(X("Outline"), &settings.visuals.players.skeleton_outline);
                popup->AddObject<CCheckbox>(X("Rounded"), &settings.visuals.players.skeleton_rounded);
                popup->AddObject<CCheckbox>(X("Vis colors"), &settings.visuals.players.skeleton_vis_colors);
            }

            auto hp = Container->AddObject<CCheckbox>(X("Health bar"), &settings.visuals.players.health_bar);
            hp->AddObject<CColorPicker>(X("Color"), &settings.visuals.players.health_bar_color);

            auto dist = Container->AddObject<CCheckbox>(X("Distance"), &settings.visuals.players.distance);
            dist->AddObject<CColorPicker>(X("Color"), &settings.visuals.players.distance_color);

            auto head = Container->AddObject<CCheckbox>(X("Head circle"), &settings.visuals.players.head_circle);
            head->AddObject<CColorPicker>(X("Color"), &settings.visuals.players.head_circle_color);

            auto view = Container->AddObject<CCheckbox>(X("View Line"), &settings.visuals.players.view_line);
            view->AddObject<CColorPicker>(X("Color"), &settings.visuals.players.view_line_color);

            auto snap = Container->AddObject<CCheckbox>(X("Snap lines"), &settings.visuals.players.snap_lines);
            snap->AddObject<CColorPicker>(X("Color"), &settings.visuals.players.snap_lines_color);
            if (auto popup = snap->AddObject<CPopup>())
            {
                static const std::vector<std::string> kAlign = { X("Top"), X("Middle"), X("Bottom") };
                popup->AddObject<CDropdown>(X("Alignment"), &settings.visuals.players.snap_lines_alignment, kAlign);
            }

            Container->AddObject<CDropdown>(X("Chams"), &settings.visuals.players.chams, g_material_names);

            Container->AddObject<CCheckbox>(X("Show On Radar"), &settings.visuals.players.include_in_radar);

            auto radar = Container->AddObject<CCheckbox>(X("Radar"), &settings.visuals.radar.enabled);
            if (auto popup = radar->AddObject<CPopup>())
            {
                popup->AddObject<CCheckbox>(X("Players only"),    &settings.visuals.radar.players_only);
                popup->AddObject<CSliderFloat>(X("Size"),  &settings.visuals.radar.size,  60.0f, 300.0f, X("{:.0f}"));
                popup->AddObject<CSliderFloat>(X("Range"), &settings.visuals.radar.range, 50.0f, 500.0f, X("{:.0f}m"));
            }

            auto arrows = Container->AddObject<CCheckbox>(X("OOF Arrows"), &settings.visuals.players.out_of_view_arrows);
            arrows->AddObject<CColorPicker>(X("Color"), &settings.visuals.players.out_of_view_arrows_color);
            if (auto popup = arrows->AddObject<CPopup>())
            {
                popup->AddObject<CCheckbox>(X("Glow"), &settings.visuals.players.arrow_glow);
                popup->AddObject<CSliderFloat>(X("Size"), &settings.visuals.players.out_of_view_arrows_size, 0.0f, 30.0f, X("{:.1f}"));
                popup->AddObject<CSliderFloat>(X("Radius"), &settings.visuals.players.out_of_view_arrows_radius, 0.0f, 2.0f, X("{:.2f}"));
            }

            auto crosshair = Container->AddObject<CCheckbox>(X("Crosshair"), &settings.visuals.screen.crosshair);
            crosshair->AddObject<CColorPicker>(X("Color"), &settings.visuals.screen.crosshair_color);
            if (auto popup = crosshair->AddObject<CPopup>())
            {
                popup->AddObject<CCheckbox>(X("Spinning triangle"), &settings.visuals.screen.crosshair_spin);
            }

            Container->AddObject<CCheckbox>(X("Player inventory"), &settings.visuals.screen.player_inventory);

            auto itemName = Container->AddObject<CCheckbox>(X("Item name"), &settings.visuals.players.item_name);
            itemName->AddObject<CColorPicker>(X("Color"), &settings.visuals.players.item_name_color);

            auto itemIcon = Container->AddObject<CCheckbox>(X("Item Icon"), &settings.visuals.players.item_icon);
            itemIcon->AddObject<CColorPicker>(X("Color"), &settings.visuals.players.item_icon_color);
        }
    }

    if (auto Entities = tab->AddObject<CSubtab>(X("Ents")))
    {
        if (auto Master = Entities->AddObject<CContainer>(X("Master")))
        {
            Master->AddObject<CCheckbox>(X("Enabled"), &settings.visuals.entities.enabled);
        }

        constexpr int kCategoryOrder[9] = {
            4, // NPCs & Animals          -> col 1
            0, // Ores & Collectibles     -> col 0
            3, // Traps & Turrets         -> col 1
            1, // Crates & Barrels        -> col 0
            2, // Deployables             -> col 1
            7, // Dropped weapons         -> col 0
            5, // Static & Monuments      -> col 1
            8, // Dropped items           -> col 0
            6, // Vehicles                -> col 1
        };

        for (int cat : kCategoryOrder)
        {
            const char* cat_name = settings_meta::kEntityCategoryNames[cat];
            if (auto CatContainer = Entities->AddObject<CContainer>(X(cat_name)))
            {
                if (bool* master_ptr = GetCategoryMasterPtr(cat))
                    CatContainer->AddObject<CCheckbox>(X("Enabled"), master_ptr);

                for (const auto& pair : settings_meta::kEntityPairs)
                {
                    if (pair.category_index != cat)
                        continue;

                    EntityLeaf* leaf = GetEntityLeafPtr(pair.category_index, pair.field_name);
                    if (!leaf)
                        continue;

                    auto row = CatContainer->AddObject<CCheckbox>(X(pair.display_name), &leaf->enabled);
                    row->AddObject<CColorPicker>(X("Color"), &leaf->color);
                    if (auto popup = row->AddObject<CPopup>())
                    {
                        popup->AddObject<CSliderInt>(X("Max distance"), &leaf->max_distance, 0, 1000, X("{}m"));
                        popup->AddObject<CCheckbox>(X("Include in radar"), &leaf->include_in_radar);
                    }
                }
            }
        }
    }
}
