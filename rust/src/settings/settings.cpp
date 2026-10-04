
#include <pawjob_umbrella.hpp>
#include <settings/settings.hpp>
#include <settings/config.hpp>

#if __has_include(<nlohmann/json.hpp>)
    #include <nlohmann/json.hpp>
#endif

using json = nlohmann::json;

namespace settings_io
{
    void SaveByName(const std::string& name);
    void LoadByName(const std::string& name);

    static std::string g_config_dir;                 // <USERPROFILE>\Documents\pawjob\configs\  (trailing sep)
    static std::vector<std::string> g_config_files;  // list of *.cfg stems, sorted, "Default" first
    static bool g_registered = false;                // guard so Init() only registers ConfigRefs once

    static constexpr const char* kHackName        = "pawjob";
    static constexpr const char* kConfigExtension = ".cfg";
    static constexpr const char* kDefaultName     = "Default";
    static constexpr const char* kXorKey          = "SabrinaCarpenterEnjoyer67420";

    static std::string GetBasePath()
    {
        static std::string path;
        if (!path.empty())
            return path;

        char* user_profile = nullptr;
        size_t len = 0;
        if (_dupenv_s(&user_profile, &len, X("USERPROFILE")) == 0 && user_profile != nullptr)
        {
            path = std::string(user_profile) + X("\\Documents\\") + kHackName;
            free(user_profile);
        }
        return path;
    }

    static void ProcessXor(std::string& data)
    {
        const std::string key = kXorKey;
        for (std::size_t i = 0; i < data.size(); ++i)
            data[i] ^= key[i % key.size()];
    }

    static void UpdateConfigList()
    {
        g_config_files.clear();
        if (g_config_dir.empty())
            return;

        std::error_code ec;
        for (const auto& entry : std::filesystem::directory_iterator(g_config_dir, ec))
        {
            if (entry.is_regular_file(ec) && entry.path().extension() == kConfigExtension)
                g_config_files.push_back(entry.path().stem().string());
        }
        std::sort(g_config_files.begin(), g_config_files.end());

        auto it = std::find(g_config_files.begin(), g_config_files.end(), kDefaultName);
        if (it != g_config_files.end() && it != g_config_files.begin())
        {
            std::string name = *it;
            g_config_files.erase(it);
            g_config_files.insert(g_config_files.begin(), std::move(name));
        }

        ::settings.settings.configs.file_list = g_config_files;
    }

    static void RegBool(const char* cat, const char* name, bool* p)
    {
        GetConfigVars().push_back(new ConfigRef<bool>(cat, name, p));
    }
    static void RegInt(const char* cat, const char* name, int* p)
    {
        GetConfigVars().push_back(new ConfigRef<int>(cat, name, p));
    }
    static void RegFloat(const char* cat, const char* name, float* p)
    {
        GetConfigVars().push_back(new ConfigRef<float>(cat, name, p));
    }
    static void RegColor(const char* cat, const char* name, Color* p)
    {
        GetConfigVars().push_back(new ConfigRef<Color>(cat, name, p));
    }
    static void RegKey(const char* cat, const char* name, Keybind* p)
    {
        GetConfigVars().push_back(new ConfigRef<Keybind>(cat, name, p));
    }

    static void RegEntity(const char* cat, const std::string& prefix, Settings::VisualsSettings::EntityTypeSettings& e)
    {
        RegBool (cat, (prefix + X("_enabled"))         .c_str(), &e.enabled);
        RegColor(cat, (prefix + X("_color"))           .c_str(), &e.color);
        RegInt  (cat, (prefix + X("_max_distance"))    .c_str(), &e.max_distance);
        RegBool (cat, (prefix + X("_include_in_radar")).c_str(), &e.include_in_radar);
    }

    static void RegWeaponGroup(const std::string& group_name, Settings::AimbotSettings::WeaponGroupSettings& g)
    {
        const std::string cat_main    = X("aimbot->groups->") + group_name + X("->main");
        const std::string cat_prereq  = X("aimbot->groups->") + group_name + X("->prerequisites");
        const std::string cat_filters = X("aimbot->groups->") + group_name + X("->filters");
        const std::string cat_human   = X("aimbot->groups->") + group_name + X("->humanization");

        RegBool (cat_main.c_str(), X("enabled"),                   &g.main.enabled);
        RegInt  (cat_main.c_str(), X("max_distance"),              &g.main.max_distance);
        RegBool (cat_main.c_str(), X("silent_aim"),                &g.main.silent_aim);
        RegInt  (cat_main.c_str(), X("silent_aim_hit_chance"),     &g.main.silent_aim_hit_chance);
        RegInt  (cat_main.c_str(), X("target_bone"),               &g.main.target_bone);
        RegBool (cat_main.c_str(), X("smart_target"),              &g.main.smart_target);
        RegBool (cat_main.c_str(), X("target_360"),                &g.main.target_360);
        RegBool (cat_main.c_str(), X("auto_shoot"),                &g.main.auto_shoot);
        RegBool (cat_main.c_str(), X("prediction"),                &g.main.prediction);
        RegFloat(cat_main.c_str(), X("prediction_strength"),       &g.main.prediction_strength);
        RegFloat(cat_main.c_str(), X("prediction_drop"),           &g.main.prediction_drop);
        RegInt  (cat_main.c_str(), X("prediction_ping_comp"),      &g.main.prediction_ping_comp);

        RegBool (cat_prereq.c_str(), X("scale_by_distance"), &g.prerequisites.scale_by_distance);

        RegBool (cat_filters.c_str(), X("skip_teammates"),  &g.filters.skip_teammates);
        RegBool (cat_filters.c_str(), X("skip_scientists"), &g.filters.skip_scientists);
        RegBool (cat_filters.c_str(), X("skip_dwellers"),   &g.filters.skip_dwellers);
        RegBool (cat_filters.c_str(), X("skip_sleepers"),   &g.filters.skip_sleepers);
        RegBool (cat_filters.c_str(), X("skip_wounded"),    &g.filters.skip_wounded);
        RegBool (cat_filters.c_str(), X("skip_animals"),    &g.filters.skip_animals);

        RegFloat(cat_human.c_str(), X("smoothing"), &g.humanization.smoothing);
    }

    static void RegisterAll()
    {
        auto& S = ::settings;

        {
            constexpr const char* c = "aimbot->weapons";
            RegBool (c, X("automatic_weapons"),             &S.aimbot.weapons.automatic_weapons);
            RegBool (c, X("instant_bow"),                   &S.aimbot.weapons.instant_bow);
            RegBool (c, X("instant_eoka"),                  &S.aimbot.weapons.instant_eoka);
            RegBool (c, X("rapid_fire"),                    &S.aimbot.weapons.rapid_fire);
            RegBool (c, X("no_sway"),                       &S.aimbot.weapons.no_sway);
            RegBool (c, X("no_animation"),                  &S.aimbot.weapons.no_animation);
            RegBool (c, X("hitbox_override"),               &S.aimbot.weapons.hitbox_override);
            RegInt  (c, X("hitbox_override_bone"),          &S.aimbot.weapons.hitbox_override_bone);
            RegBool (c, X("override_weapon_spread"),        &S.aimbot.weapons.override_weapon_spread);
            RegInt  (c, X("override_weapon_spread_amount"), &S.aimbot.weapons.override_weapon_spread_amount);
            RegBool (c, X("override_weapon_recoil"),        &S.aimbot.weapons.override_weapon_recoil);
            RegInt  (c, X("override_weapon_recoil_amount"), &S.aimbot.weapons.override_weapon_recoil_amount);
            RegBool (c, X("override_melee_range"),          &S.aimbot.weapons.override_melee_range);
            RegInt  (c, X("override_melee_range_amount"),   &S.aimbot.weapons.override_melee_range_amount);
            RegBool (c, X("no_viewmodel_lower"),            &S.aimbot.weapons.no_viewmodel_lower);
            RegBool (c, X("hide_viewmodel"),                &S.aimbot.weapons.hide_viewmodel);
        }

        {
            constexpr const char* c = "aimbot->visualization";
            RegBool (c, X("fov_circle"),                   &S.aimbot.visualization.fov_circle);
            RegColor(c, X("fov_circle_color"),             &S.aimbot.visualization.fov_circle_color);
            RegInt  (c, X("fov_circle_size"),              &S.aimbot.visualization.fov_circle_size);
            RegBool (c, X("fov_circle_target_line"),       &S.aimbot.visualization.fov_circle_target_line);
            RegColor(c, X("fov_circle_target_line_color"), &S.aimbot.visualization.fov_circle_target_line_color);
            RegBool (c, X("prediction_path"),              &S.aimbot.visualization.prediction_path);
            RegColor(c, X("prediction_path_color"),        &S.aimbot.visualization.prediction_path_color);
        }

        RegWeaponGroup(X("general"),  S.aimbot.general);
        RegWeaponGroup(X("rifles"),   S.aimbot.groups[0]);
        RegWeaponGroup(X("snipers"),  S.aimbot.groups[1]);
        RegWeaponGroup(X("shotguns"), S.aimbot.groups[2]);
        RegWeaponGroup(X("pistols"),  S.aimbot.groups[3]);
        RegWeaponGroup(X("bows"),     S.aimbot.groups[4]);
        RegWeaponGroup(X("lmgs"),     S.aimbot.groups[5]);
        RegWeaponGroup(X("smgs"),     S.aimbot.groups[6]);

        RegKey("aimbot->keys", X("aim_key"), &S.aimbot.aim_key);

        RegInt("visuals", X("flags"), &S.visuals.flags);

        {
            constexpr const char* c = "visuals->players";
            auto& P = S.visuals.players;
            RegBool (c, X("enabled"),                        &P.enabled);
            RegInt  (c, X("max_distance"),                   &P.max_distance);
            RegBool (c, X("include_in_radar"),               &P.include_in_radar);
            RegBool (c, X("show_sleepers"),                  &P.show_sleepers);
            RegBool (c, X("vischeck"),                       &P.vischeck);
            RegBool (c, X("vis_visible_color_enabled"),      &P.vis_visible_color_enabled);
            RegColor(c, X("vis_visible_color"),              &P.vis_visible_color);
            RegBool (c, X("vis_invisible_color_enabled"),    &P.vis_invisible_color_enabled);
            RegColor(c, X("vis_invisible_color"),            &P.vis_invisible_color);
            RegBool (c, X("vis_dim_instead_of_hide"),        &P.vis_dim_instead_of_hide);
            RegBool (c, X("vis_per_bone_skeleton"),          &P.vis_per_bone_skeleton);
            RegBool (c, X("out_of_view_arrows"),             &P.out_of_view_arrows);
            RegColor(c, X("out_of_view_arrows_color"),       &P.out_of_view_arrows_color);
            RegFloat(c, X("out_of_view_arrows_size"),        &P.out_of_view_arrows_size);
            RegFloat(c, X("out_of_view_arrows_radius"),      &P.out_of_view_arrows_radius);
            RegBool (c, X("name"),                           &P.name);
            RegColor(c, X("name_color"),                     &P.name_color);
            RegBool (c, X("name_show_avatar"),               &P.name_show_avatar);
            RegBool (c, X("bounding_box"),                   &P.bounding_box);
            RegColor(c, X("bounding_box_color"),             &P.bounding_box_color);
            RegBool (c, X("bounding_box_outline"),           &P.bounding_box_outline);
            RegBool (c, X("bounding_box_corner"),            &P.bounding_box_corner);
            RegBool (c, X("bounding_box_gradient"),          &P.bounding_box_gradient);
            RegColor(c, X("bounding_box_gradient_color"),    &P.bounding_box_gradient_color);
            RegBool (c, X("bounding_box_fill"),              &P.bounding_box_fill);
            RegColor(c, X("bounding_box_fill_color_top"),    &P.bounding_box_fill_color_top);
            RegColor(c, X("bounding_box_fill_color_bottom"), &P.bounding_box_fill_color_bottom);
            RegBool (c, X("skeleton"),                       &P.skeleton);
            RegColor(c, X("skeleton_color"),                 &P.skeleton_color);
            RegBool (c, X("skeleton_outline"),               &P.skeleton_outline);
            RegBool (c, X("skeleton_rounded"),               &P.skeleton_rounded);
            RegBool (c, X("skeleton_vis_colors"),            &P.skeleton_vis_colors);
            RegBool (c, X("health_bar"),                     &P.health_bar);
            RegColor(c, X("health_bar_color"),               &P.health_bar_color);
            RegBool (c, X("distance"),                       &P.distance);
            RegColor(c, X("distance_color"),                 &P.distance_color);
            RegBool (c, X("skip_crouching"),                 &P.skip_crouching);
            RegBool (c, X("skip_scoped"),                    &P.skip_scoped);
            RegBool (c, X("head_circle"),                    &P.head_circle);
            RegColor(c, X("head_circle_color"),              &P.head_circle_color);
            RegBool (c, X("view_line"),                      &P.view_line);
            RegColor(c, X("view_line_color"),                &P.view_line_color);
            RegBool (c, X("snap_lines"),                     &P.snap_lines);
            RegColor(c, X("snap_lines_color"),               &P.snap_lines_color);
            RegInt  (c, X("snap_lines_alignment"),           &P.snap_lines_alignment);
            RegBool (c, X("item_name"),                      &P.item_name);
            RegColor(c, X("item_name_color"),                &P.item_name_color);
            RegBool (c, X("item_icon"),                      &P.item_icon);
            RegColor(c, X("item_icon_color"),                &P.item_icon_color);
            RegInt  (c, X("flags"),                          &P.flags);
            RegInt  (c, X("chams"),                          &P.chams);
            for (int i = 0; i < 5; ++i)
            {
                std::string name = X("extra_player_flag_") + std::to_string(i);
                RegBool(c, name.c_str(), &P.extra_player_flags[i]);
            }
        }

        {
            constexpr const char* c_master = "visuals->entities";
            RegBool(c_master, X("enabled"), &S.visuals.entities.enabled);
        }
        {
            using V = Settings::VisualsSettings;
            auto& E = S.visuals.entities;

            RegBool("visuals->entities->ores_collectibles", X("enabled"), &E.ores_collectibles.enabled);
            RegBool("visuals->entities->crates_barrels",    X("enabled"), &E.crates_barrels.enabled);
            RegBool("visuals->entities->deployables",       X("enabled"), &E.deployables.enabled);
            RegBool("visuals->entities->traps_turrets",     X("enabled"), &E.traps_turrets.enabled);
            RegBool("visuals->entities->npcs_animals",      X("enabled"), &E.npcs_animals.enabled);
            RegBool("visuals->entities->static_monuments",  X("enabled"), &E.static_monuments.enabled);
            RegBool("visuals->entities->vehicles",          X("enabled"), &E.vehicles.enabled);
            RegBool("visuals->entities->dropped_weapons",   X("enabled"), &E.dropped_weapons.enabled);
            RegBool("visuals->entities->dropped_items",     X("enabled"), &E.dropped_items.enabled);

            #define REG_ETY(cat_name, field) RegEntity("visuals->entities->" cat_name, #field, E.field.field)

            RegEntity("visuals->entities->ores_collectibles", X("stone"),         E.ores_collectibles.stone);
            RegEntity("visuals->entities->ores_collectibles", X("sulfur"),        E.ores_collectibles.sulfur);
            RegEntity("visuals->entities->ores_collectibles", X("metal"),         E.ores_collectibles.metal);
            RegEntity("visuals->entities->ores_collectibles", X("hemp"),          E.ores_collectibles.hemp);
            RegEntity("visuals->entities->ores_collectibles", X("wood"),          E.ores_collectibles.wood);
            RegEntity("visuals->entities->ores_collectibles", X("diesel_fuel"),   E.ores_collectibles.diesel_fuel);
            RegEntity("visuals->entities->ores_collectibles", X("green_keycard"), E.ores_collectibles.green_keycard);
            RegEntity("visuals->entities->ores_collectibles", X("blue_keycard"),  E.ores_collectibles.blue_keycard);
            RegEntity("visuals->entities->ores_collectibles", X("red_keycard"),   E.ores_collectibles.red_keycard);

            RegEntity("visuals->entities->crates_barrels", X("elite_crate"),      E.crates_barrels.elite_crate);
            RegEntity("visuals->entities->crates_barrels", X("military_crate"),   E.crates_barrels.military_crate);
            RegEntity("visuals->entities->crates_barrels", X("locked_crate"),     E.crates_barrels.locked_crate);
            RegEntity("visuals->entities->crates_barrels", X("air_drop"),         E.crates_barrels.air_drop);
            RegEntity("visuals->entities->crates_barrels", X("loot_barrel"),      E.crates_barrels.loot_barrel);
            RegEntity("visuals->entities->crates_barrels", X("oil_barrel"),       E.crates_barrels.oil_barrel);
            RegEntity("visuals->entities->crates_barrels", X("food_crate"),       E.crates_barrels.food_crate);
            RegEntity("visuals->entities->crates_barrels", X("tool_crate"),       E.crates_barrels.tool_crate);
            RegEntity("visuals->entities->crates_barrels", X("small_crate"),      E.crates_barrels.small_crate);
            RegEntity("visuals->entities->crates_barrels", X("health_crate"),     E.crates_barrels.health_crate);
            RegEntity("visuals->entities->crates_barrels", X("small_food_crate"), E.crates_barrels.small_food_crate);
            RegEntity("visuals->entities->crates_barrels", X("vehicle_parts"),    E.crates_barrels.vehicle_parts);
            RegEntity("visuals->entities->crates_barrels", X("crate"),            E.crates_barrels.crate);

            RegEntity("visuals->entities->deployables", X("wooden_box"),       E.deployables.wooden_box);
            RegEntity("visuals->entities->deployables", X("large_wood_box"),   E.deployables.large_wood_box);
            RegEntity("visuals->entities->deployables", X("tool_cupboard"),    E.deployables.tool_cupboard);
            RegEntity("visuals->entities->deployables", X("tier_1_workbench"), E.deployables.tier_1_workbench);
            RegEntity("visuals->entities->deployables", X("tier_2_workbench"), E.deployables.tier_2_workbench);
            RegEntity("visuals->entities->deployables", X("tier_3_workbench"), E.deployables.tier_3_workbench);
            RegEntity("visuals->entities->deployables", X("repair_bench"),     E.deployables.repair_bench);
            RegEntity("visuals->entities->deployables", X("research_table"),   E.deployables.research_table);
            RegEntity("visuals->entities->deployables", X("small_stash"),      E.deployables.small_stash);
            RegEntity("visuals->entities->deployables", X("sleeping_bag"),     E.deployables.sleeping_bag);
            RegEntity("visuals->entities->deployables", X("furnace"),          E.deployables.furnace);
            RegEntity("visuals->entities->deployables", X("locker"),           E.deployables.locker);

            RegEntity("visuals->entities->traps_turrets", X("auto_turret"),  E.traps_turrets.auto_turret);
            RegEntity("visuals->entities->traps_turrets", X("flame_turret"), E.traps_turrets.flame_turret);
            RegEntity("visuals->entities->traps_turrets", X("shotgun_trap"), E.traps_turrets.shotgun_trap);

            RegEntity("visuals->entities->npcs_animals", X("scientist"), E.npcs_animals.scientist);
            RegEntity("visuals->entities->npcs_animals", X("dweller"),   E.npcs_animals.dweller);
            RegEntity("visuals->entities->npcs_animals", X("bear"),      E.npcs_animals.bear);
            RegEntity("visuals->entities->npcs_animals", X("wolf"),      E.npcs_animals.wolf);
            RegEntity("visuals->entities->npcs_animals", X("stag"),      E.npcs_animals.stag);
            RegEntity("visuals->entities->npcs_animals", X("boar"),      E.npcs_animals.boar);
            RegEntity("visuals->entities->npcs_animals", X("horse"),     E.npcs_animals.horse);
            RegEntity("visuals->entities->npcs_animals", X("chicken"),   E.npcs_animals.chicken);
            RegEntity("visuals->entities->npcs_animals", X("shark"),     E.npcs_animals.shark);
            RegEntity("visuals->entities->npcs_animals", X("scarecrow"), E.npcs_animals.scarecrow);
            RegEntity("visuals->entities->npcs_animals", X("corpse"),    E.npcs_animals.corpse);

            RegEntity("visuals->entities->static_monuments", X("recycler"),         E.static_monuments.recycler);
            RegEntity("visuals->entities->static_monuments", X("phone_booth"),      E.static_monuments.phone_booth);
            RegEntity("visuals->entities->static_monuments", X("fuse_box"),         E.static_monuments.fuse_box);
            RegEntity("visuals->entities->static_monuments", X("refinery"),         E.static_monuments.refinery);
            RegEntity("visuals->entities->static_monuments", X("elevator"),         E.static_monuments.elevator);
            RegEntity("visuals->entities->static_monuments", X("car_lift"),         E.static_monuments.car_lift);
            RegEntity("visuals->entities->static_monuments", X("computer_station"), E.static_monuments.computer_station);

            RegEntity("visuals->entities->vehicles", X("motorbike"),         E.vehicles.motorbike);
            RegEntity("visuals->entities->vehicles", X("sidecar_motorbike"), E.vehicles.sidecar_motorbike);
            RegEntity("visuals->entities->vehicles", X("pedal_bike"),        E.vehicles.pedal_bike);
            RegEntity("visuals->entities->vehicles", X("snowmobile"),        E.vehicles.snowmobile);
            RegEntity("visuals->entities->vehicles", X("tomaha_snowmobile"), E.vehicles.tomaha_snowmobile);
            RegEntity("visuals->entities->vehicles", X("tugboat"),           E.vehicles.tugboat);
            RegEntity("visuals->entities->vehicles", X("rowboat"),           E.vehicles.rowboat);
            RegEntity("visuals->entities->vehicles", X("rhib"),              E.vehicles.rhib);
            RegEntity("visuals->entities->vehicles", X("kyak"),              E.vehicles.kyak);
            RegEntity("visuals->entities->vehicles", X("solo_submarine"),    E.vehicles.solo_submarine);
            RegEntity("visuals->entities->vehicles", X("duo_submarine"),     E.vehicles.duo_submarine);
            RegEntity("visuals->entities->vehicles", X("minicopter"),        E.vehicles.minicopter);
            RegEntity("visuals->entities->vehicles", X("scrap_heli"),        E.vehicles.scrap_heli);
            RegEntity("visuals->entities->vehicles", X("attack_heli"),       E.vehicles.attack_heli);

            RegEntity("visuals->entities->dropped_weapons", X("dropped_rifles"),    E.dropped_weapons.dropped_rifles);
            RegEntity("visuals->entities->dropped_weapons", X("dropped_snipers"),   E.dropped_weapons.dropped_snipers);
            RegEntity("visuals->entities->dropped_weapons", X("dropped_smgs"),      E.dropped_weapons.dropped_smgs);
            RegEntity("visuals->entities->dropped_weapons", X("dropped_shotguns"),  E.dropped_weapons.dropped_shotguns);
            RegEntity("visuals->entities->dropped_weapons", X("dropped_pistols"),   E.dropped_weapons.dropped_pistols);
            RegEntity("visuals->entities->dropped_weapons", X("dropped_lmgs"),      E.dropped_weapons.dropped_lmgs);
            RegEntity("visuals->entities->dropped_weapons", X("dropped_launchers"), E.dropped_weapons.dropped_launchers);
            RegEntity("visuals->entities->dropped_weapons", X("dropped_bows"),      E.dropped_weapons.dropped_bows);
            RegEntity("visuals->entities->dropped_weapons", X("dropped_melee"),     E.dropped_weapons.dropped_melee);

            RegEntity("visuals->entities->dropped_items", X("throwables"), E.dropped_items.throwables);
            RegEntity("visuals->entities->dropped_items", X("tools"),      E.dropped_items.tools);
            RegEntity("visuals->entities->dropped_items", X("medical"),    E.dropped_items.medical);
            RegEntity("visuals->entities->dropped_items", X("ammo"),       E.dropped_items.ammo);
            RegEntity("visuals->entities->dropped_items", X("misc"),       E.dropped_items.misc);

            #undef REG_ETY
        }

        {
            constexpr const char* c = "visuals->screen";
            RegBool (c, X("crosshair"),        &S.visuals.screen.crosshair);
            RegColor(c, X("crosshair_color"),  &S.visuals.screen.crosshair_color);
            RegBool (c, X("crosshair_spin"),   &S.visuals.screen.crosshair_spin);
            RegBool (c, X("player_inventory"), &S.visuals.screen.player_inventory);
        }

        {
            constexpr const char* c = "visuals->local";
            auto& L = S.visuals.local;
            RegBool (c, X("custom_fov"),           &L.custom_fov);
            RegFloat(c, X("field_of_view"),        &L.field_of_view);
            RegFloat(c, X("zoom_fov"),             &L.zoom_fov);
            RegBool (c, X("third_person"),         &L.third_person);
            RegBool (c, X("debug_camera"),         &L.debug_camera);
            RegFloat(c, X("debug_camera_speed"),   &L.debug_camera_speed);
            RegFloat(c, X("mouse_sensitivity"),    &L.mouse_sensitivity);
            RegBool (c, X("bullet_tracers"),       &L.bullet_tracers);
            RegColor(c, X("bullet_tracers_color"), &L.bullet_tracers_color);
            RegInt  (c, X("weapon_chams"),         &L.weapon_chams);
            RegInt  (c, X("arm_chams"),            &L.arm_chams);
            RegKey  (c, X("zoom_key"),             &L.zoom_key);
            RegKey  (c, X("debug_camera_key"),     &L.debug_camera_key);
        }

        {
            constexpr const char* c = "visuals->world";
            auto& W = S.visuals.world;
            RegBool (c, X("time_changer"), &W.time_changer);
            RegFloat(c, X("time"),         &W.time);
            RegBool (c, X("bright_night"), &W.bright_night);
            static const char* modulate_names[7] = {
                X("modulate_sun"), X("modulate_lights"), X("modulate_rays"), X("modulate_sky"),
                X("modulate_clouds"), X("modulate_fog"), X("modulate_ambience"),
            };
            for (int i = 0; i < 7; ++i)
            {
                std::string enabled_name = std::string(modulate_names[i]) + X("_enabled");
                std::string color_name   = std::string(modulate_names[i]) + X("_color");
                RegBool (c, enabled_name.c_str(), &W.modulates[i].enabled);
                RegColor(c, color_name.c_str(),   &W.modulates[i].color);
            }
        }

        {
            constexpr const char* c = "visuals->radar";
            RegBool (c, X("enabled"),      &S.visuals.radar.enabled);
            RegFloat(c, X("size"),         &S.visuals.radar.size);
            RegFloat(c, X("range"),        &S.visuals.radar.range);
            RegBool (c, X("players_only"), &S.visuals.radar.players_only);
        }

        {
            constexpr const char* c = "misc->general";
            auto& G = S.misc.general;
            RegBool (c, X("instant_untie_crate"),  &G.instant_untie_crate);
            RegBool (c, X("fast_loot"),            &G.fast_loot);
            RegBool (c, X("instant_interactions"), &G.instant_interactions);
            RegBool (c, X("instant_revive"),       &G.instant_revive);
            RegKey  (c, X("instant_revive_key"),   &G.instant_revive_key);
            RegBool (c, X("debug_esp"),            &G.debug_esp);
            RegColor(c, X("debug_esp_color"),      &G.debug_esp_color);
        }

        {
            constexpr const char* c = "misc->movement";
            auto& M = S.misc.movement;
            RegBool(c, X("spider_man"),        &M.spider_man);
            RegKey (c, X("spider_man_key"),    &M.spider_man_key);
            RegBool(c, X("fly"),               &M.fly);
            RegKey (c, X("fly_key"),           &M.fly_key);
            RegBool(c, X("silent_walk"),       &M.silent_walk);
            RegBool(c, X("no_fall"),           &M.no_fall);
            RegBool(c, X("speed_hack"),        &M.speed_hack);
            RegBool(c, X("omni_sprint"),       &M.omni_sprint);
            RegBool(c, X("remove_water_drag"), &M.remove_water_drag);
            RegBool(c, X("walk_on_water"),     &M.walk_on_water);
            RegBool(c, X("anti_aim"),          &M.anti_aim);
        }

        {
            constexpr const char* c = "settings->ui";
            auto& U = S.settings.ui;
            RegBool (c, X("vertical_sync"),        &U.vertical_sync);
            RegBool (c, X("override_accent"),      &U.override_accent);
            RegColor(c, X("accent_color"),         &U.accent_color);
            RegBool (c, X("enable_memory_writes"), &U.enable_memory_writes);
        }

    }

    void Init()
    {
        std::string base = GetBasePath();
        if (base.empty())
            return;

        std::error_code ec;
        std::filesystem::create_directories(base, ec);
        g_config_dir = base + X("\\configs\\");
        std::filesystem::create_directories(g_config_dir, ec);

        if (!g_registered)
        {
            RegisterAll();
            g_registered = true;
        }

        UpdateConfigList();

        auto it = std::find(g_config_files.begin(), g_config_files.end(), kDefaultName);
        if (it == g_config_files.end())
        {
            SaveByName(kDefaultName);
            UpdateConfigList();
        }
        LoadByName(kDefaultName);

        LOG(X("settings_io::Init - Started (config dir: {}, {} vars registered)"),
            g_config_dir, GetConfigVars().size());
    }

    void SaveByName(const std::string& name)
    {
        if (name.empty() || g_config_dir.empty())
            return;

        json j;
        for (auto* var : GetConfigVars())
            var->Save(j);

        std::string dump = j.dump(4);
        ProcessXor(dump);

        std::ofstream o(g_config_dir + name + kConfigExtension, std::ios::binary);
        o.write(dump.data(), static_cast<std::streamsize>(dump.size()));
    }

    void LoadByName(const std::string& name)
    {
        if (name.empty() || g_config_dir.empty())
            return;

        std::ifstream i(g_config_dir + name + kConfigExtension, std::ios::binary | std::ios::ate);
        if (!i.good())
            return;

        std::streamsize size = i.tellg();
        if (size <= 0)
            return;

        std::string buffer(size, '\0');
        i.seekg(0);
        i.read(buffer.data(), size);

        ProcessXor(buffer);

        json j = json::parse(buffer, nullptr, false);
        if (j.is_discarded())
            return;

        for (auto* var : GetConfigVars())
            var->Load(j);
    }

    void CreateNewFile(const std::string& name)
    {
        if (name.empty() || name.find_first_not_of(' ') == std::string::npos)
            return;
        if (std::find(g_config_files.cbegin(), g_config_files.cend(), name) != g_config_files.cend())
            return;

        SaveByName(name);
        UpdateConfigList();
    }

    void DeleteFile(const std::string& name)
    {
        if (name.empty() || g_config_dir.empty())
            return;

        std::error_code ec;
        std::filesystem::remove(g_config_dir + name + kConfigExtension, ec);
        UpdateConfigList();
    }

    void Reset()
    {
        ::settings = Settings{};
        UpdateConfigList();
    }

    void LoadDefault()
    {
        LoadByName(kDefaultName);
    }

    const std::vector<std::string>& GetConfigFiles()
    {
        return g_config_files;
    }
}
