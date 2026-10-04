#pragma once


#include <string>
#include <vector>
#include <array>
#include <cstdint>

#include <menu/framework/color.hpp>
#include <menu/framework/widgets/keybind/keybind.hpp>

struct Settings
{
    struct AimbotSettings
    {
        struct Weapons
        {
            bool  automatic_weapons               = false;
            bool  instant_bow                     = false;
            bool  instant_eoka                    = false;
            int   instant_eoka_strikechance       = 100;    // popup slider (1-100%)
            bool  rapid_fire                      = false;
            bool  no_sway                         = false;
            bool  no_animation                    = false;
            bool  thick_bullet                    = false;
            bool  hitbox_override                 = false;
            int   hitbox_override_bone            = 0;      // dropdown index
            bool  fast_shoot                      = false;
            bool  hit_material_override           = false;
            bool  override_weapon_spread          = false;
            int   override_weapon_spread_amount   = 0;
            bool  override_weapon_recoil          = false;
            int   override_weapon_recoil_amount   = 0;
            bool  override_melee_range            = false;
            int   override_melee_range_amount     = 100;
            bool  no_viewmodel_lower              = false;
            bool  hide_viewmodel                  = false;
        } weapons;

        struct Visualization
        {
            bool  fov_circle                      = false;
            Color fov_circle_color                = Color::White();
            int   fov_circle_size                 = 125;
            bool  fov_circle_target_line          = false;
            Color fov_circle_target_line_color    = Color::White();
            bool  prediction_path                 = false;
            Color prediction_path_color           = Color(100, 200, 255);
        } visualization;

        struct WeaponGroupSettings
        {
            struct Main
            {
                bool  enabled                     = false;  // "Enabled" (General) / "Override general" (tiers)
                int   max_distance                = 350;
                bool  silent_aim                  = false;
                int   silent_aim_hit_chance       = 100;
                int   target_bone                 = 0;

                bool  smart_target                = false;
                bool  target_360                  = false;
                bool  auto_shoot                  = false;
                bool  prediction                  = true;
                float prediction_strength         = 1.0f;
                float prediction_drop             = 1.0f;
                int   prediction_ping_comp        = 80;
            } main;

            struct Prerequisites
            {
                bool  scale_by_distance           = false;  // was orphan read
            } prerequisites;

            struct Filters
            {
                bool  skip_teammates              = true;
                bool  skip_scientists             = true;
                bool  skip_dwellers               = true;
                bool  skip_sleepers               = true;
                bool  skip_wounded                = true;
                bool  skip_animals                = true;
            } filters;

            struct Humanization
            {
                float smoothing                   = 10.0f;
            } humanization;
        };

        WeaponGroupSettings general;
        std::array<WeaponGroupSettings, 7> groups;

        Keybind aim_key{};
    } aimbot;

    struct VisualsSettings
    {
        int flags = 0;

        struct Players
        {
            bool  enabled                         = true;
            int   max_distance                    = 350;
            bool  include_in_radar                = false;

            bool  show_sleepers                   = false;

            bool  vischeck                        = false;
            bool  vis_visible_color_enabled       = false;
            Color vis_visible_color               = Color::White();
            bool  vis_invisible_color_enabled     = true;
            Color vis_invisible_color             = Color(255, 80, 80, 220);
            bool  vis_dim_instead_of_hide         = false;
            bool  vis_per_bone_skeleton           = false;

            bool  out_of_view_arrows              = false;
            Color out_of_view_arrows_color        = Color::White();
            float out_of_view_arrows_size         = 10.0f;
            float out_of_view_arrows_radius       = 0.5f;
            bool  arrow_glow                      = false;

            bool  name                            = true;
            Color name_color                      = Color::White();
            bool  name_show_avatar                = false;

            bool  bounding_box                    = false;
            Color bounding_box_color              = Color::White();
            bool  bounding_box_outline            = true;
            bool  bounding_box_corner             = false;
            bool  bounding_box_gradient           = false;
            Color bounding_box_gradient_color     = Color(255, 180, 255);
            bool  bounding_box_fill               = false;
            Color bounding_box_fill_color_top     = Color(0, 255, 255, 100);
            Color bounding_box_fill_color_bottom  = Color(255, 100, 255, 100);

            bool  skeleton                        = true;
            Color skeleton_color                  = Color::White();
            bool  skeleton_outline                = true;
            bool  skeleton_rounded                = false;
            bool  skeleton_vis_colors             = false;

            bool  health_bar                      = true;
            Color health_bar_color                = Color(50, 220, 50);

            bool  distance                        = true;
            Color distance_color                  = Color::White();

            bool  skip_crouching                  = false;
            bool  skip_scoped                     = false;

            bool  head_circle                     = false;
            Color head_circle_color               = Color::White();

            bool  view_line                       = false;
            Color view_line_color                 = Color::White();

            bool  snap_lines                      = false;
            Color snap_lines_color                = Color::White();
            int   snap_lines_alignment            = 1;      // "Middle"

            bool  item_name                       = false;
            Color item_name_color                 = Color::White();
            bool  item_icon                       = false;
            Color item_icon_color                 = Color::White();

            int   flags                           = 0;     // MultiDropdown bitmask (OR'd with parent flags)

            int   chams                           = 0;     // Dropdown index into material list

            std::array<bool, 5> extra_player_flags{};
        } players;

        struct EntityTypeSettings
        {
            bool  enabled           = false;
            Color color             = Color::White();
            int   max_distance      = 250;
            bool  include_in_radar  = false;
        };


        struct OresCollectibles
        {
            bool enabled = false;   // category master
            EntityTypeSettings stone;
            EntityTypeSettings sulfur;
            EntityTypeSettings metal;
            EntityTypeSettings hemp;
            EntityTypeSettings wood;
            EntityTypeSettings diesel_fuel;
            EntityTypeSettings green_keycard;
            EntityTypeSettings blue_keycard;
            EntityTypeSettings red_keycard;
        };

        struct CratesBarrels
        {
            bool enabled = false;
            EntityTypeSettings elite_crate;
            EntityTypeSettings military_crate;
            EntityTypeSettings locked_crate;
            EntityTypeSettings air_drop;
            EntityTypeSettings loot_barrel;
            EntityTypeSettings oil_barrel;
            EntityTypeSettings food_crate;
            EntityTypeSettings tool_crate;
            EntityTypeSettings small_crate;
            EntityTypeSettings health_crate;
            EntityTypeSettings small_food_crate;
            EntityTypeSettings vehicle_parts;
            EntityTypeSettings crate;
        };

        struct Deployables
        {
            bool enabled = false;
            EntityTypeSettings wooden_box;
            EntityTypeSettings large_wood_box;
            EntityTypeSettings tool_cupboard;
            EntityTypeSettings tier_1_workbench;
            EntityTypeSettings tier_2_workbench;
            EntityTypeSettings tier_3_workbench;
            EntityTypeSettings repair_bench;
            EntityTypeSettings research_table;
            EntityTypeSettings small_stash;
            EntityTypeSettings sleeping_bag;
            EntityTypeSettings furnace;
            EntityTypeSettings locker;
        };

        struct TrapsTurrets
        {
            bool enabled = false;
            EntityTypeSettings auto_turret;
            EntityTypeSettings flame_turret;
            EntityTypeSettings shotgun_trap;
        };

        struct NpcsAnimals
        {
            bool enabled = false;
            EntityTypeSettings scientist;
            EntityTypeSettings dweller;
            EntityTypeSettings bear;
            EntityTypeSettings wolf;
            EntityTypeSettings stag;
            EntityTypeSettings boar;
            EntityTypeSettings horse;
            EntityTypeSettings chicken;
            EntityTypeSettings shark;
            EntityTypeSettings scarecrow;
            EntityTypeSettings corpse;
        };

        struct StaticMonuments
        {
            bool enabled = false;
            EntityTypeSettings recycler;
            EntityTypeSettings phone_booth;
            EntityTypeSettings fuse_box;
            EntityTypeSettings refinery;
            EntityTypeSettings elevator;
            EntityTypeSettings car_lift;
            EntityTypeSettings computer_station;
        };

        struct Vehicles
        {
            bool enabled = false;
            EntityTypeSettings motorbike;
            EntityTypeSettings sidecar_motorbike;
            EntityTypeSettings pedal_bike;
            EntityTypeSettings snowmobile;
            EntityTypeSettings tomaha_snowmobile;
            EntityTypeSettings tugboat;
            EntityTypeSettings rowboat;
            EntityTypeSettings rhib;
            EntityTypeSettings kyak;
            EntityTypeSettings solo_submarine;
            EntityTypeSettings duo_submarine;
            EntityTypeSettings minicopter;
            EntityTypeSettings scrap_heli;
            EntityTypeSettings attack_heli;
        };

        struct DroppedWeapons
        {
            bool enabled = false;
            EntityTypeSettings dropped_rifles;
            EntityTypeSettings dropped_snipers;
            EntityTypeSettings dropped_smgs;
            EntityTypeSettings dropped_shotguns;
            EntityTypeSettings dropped_pistols;
            EntityTypeSettings dropped_lmgs;
            EntityTypeSettings dropped_launchers;
            EntityTypeSettings dropped_bows;
            EntityTypeSettings dropped_melee;
        };

        struct DroppedItems
        {
            bool enabled = false;
            EntityTypeSettings throwables;
            EntityTypeSettings tools;
            EntityTypeSettings medical;
            EntityTypeSettings ammo;
            EntityTypeSettings misc;
        };

        struct Entities
        {
            bool enabled = true;                  // master
            OresCollectibles  ores_collectibles;
            CratesBarrels     crates_barrels;
            Deployables       deployables;
            TrapsTurrets      traps_turrets;
            NpcsAnimals       npcs_animals;
            StaticMonuments   static_monuments;
            Vehicles          vehicles;
            DroppedWeapons    dropped_weapons;
            DroppedItems      dropped_items;
        } entities;

        struct Screen
        {
            bool  crosshair         = false;
            Color crosshair_color   = Color(255, 85, 200);
            bool  crosshair_spin    = false;      // spinning triangle style
            bool  player_inventory  = false;
        } screen;

        struct Local
        {
            bool  custom_fov            = false;
            float field_of_view         = 90.0f;
            float zoom_fov              = 35.0f;
            bool  third_person          = false;
            bool  debug_camera          = false;
            float debug_camera_speed    = 10.0f;
            float mouse_sensitivity     = 40.0f;
            bool  bullet_tracers        = false;
            Color bullet_tracers_color  = Color(255, 200, 80);
            int   weapon_chams          = 0;
            int   arm_chams             = 0;
            Keybind zoom_key{};
            Keybind debug_camera_key{};
        } local;

        struct World
        {
            bool  time_changer          = false;
            float time                  = 18.0f;
            bool  bright_night          = false;
            bool  full_world_modulation = false;

            struct ModulateSetting
            {
                bool  enabled = false;
                Color color   = Color(125, 125, 205);
            };
            std::array<ModulateSetting, 7> modulates;
        } world;

        struct Radar
        {
            bool  enabled      = false;
            float size         = 120.0f;
            float range        = 150.0f;
            bool  players_only = false;
        } radar;
    } visuals;

    struct MiscSettings
    {
        struct General
        {
            bool    instant_untie_crate   = false;
            bool    fast_loot             = false;
            bool    instant_interactions  = false;
            bool    instant_revive        = false;
            Keybind instant_revive_key{};
            bool    debug_esp             = false;
            Color   debug_esp_color       = Color::White();
        } general;

        struct Movement
        {
            bool    spider_man          = false;
            Keybind spider_man_key{};
            bool    fly                 = false;
            Keybind fly_key{};
            bool    silent_walk         = false;
            bool    no_fall             = false;
            bool    speed_hack          = false;
            bool    omni_sprint         = false;
            bool    remove_water_drag   = false;
            bool    walk_on_water       = false;
            bool    anti_aim            = false;
        } movement;

        struct LayerToggle
        {
            bool    enabled          = false;
            Keybind key{};

            bool    active           = false;

            bool    hide_construction = false;
            bool    hide_transparent  = false;
            bool    hide_debris       = false;
            bool    hide_default      = false;
            bool    hide_deployed     = false;
            bool    hide_ragdoll      = false;
            bool    hide_terrain      = false;
            bool    hide_tree         = false;
            bool    hide_world        = false;
            bool    hide_water        = false;
            bool    hide_clutter      = false;
        } layer_toggle;
    } misc;

    struct PawjobSettings
    {
        struct UI
        {
            bool  vertical_sync         = false;
            bool  override_accent       = false;
            Color accent_color          = Color::White();
            // MASTER SAFETY SWITCH — DEFAULT FALSE.
            bool  enable_memory_writes  = false;
        } ui;

        struct Configs
        {
            std::vector<std::string> file_list;
            int         selected_index = 0;
            std::string new_config_name;
        } configs;
    } settings;
};

inline Settings settings = {};

namespace settings_io
{
    void Init();
    void SaveByName(const std::string& name);
    void LoadByName(const std::string& name);
    void LoadDefault();
    void CreateNewFile(const std::string& name);
    void Reset();
    const std::vector<std::string>& GetConfigFiles();
}

namespace settings_meta
{
    struct EntityPair
    {
        int         category_index;  // 0-8, index into Entities::categories() layout above
        const char* display_name;    // human-readable name shown in the menu
        const char* field_name;      // snake_case struct field name inside the category struct
    };

    inline constexpr EntityPair kEntityPairs[] = {
        { 0, "Stone",            "stone" },
        { 0, "Sulfur",           "sulfur" },
        { 0, "Metal",            "metal" },
        { 0, "Hemp",             "hemp" },
        { 0, "Wood",             "wood" },
        { 0, "Diesel fuel",      "diesel_fuel" },
        { 0, "Green keycard",    "green_keycard" },
        { 0, "Blue keycard",     "blue_keycard" },
        { 0, "Red keycard",      "red_keycard" },

        { 1, "Elite crate",      "elite_crate" },
        { 1, "Military crate",   "military_crate" },
        { 1, "Locked crate",     "locked_crate" },
        { 1, "Air drop",         "air_drop" },
        { 1, "Loot barrel",      "loot_barrel" },
        { 1, "Oil barrel",       "oil_barrel" },
        { 1, "Food crate",       "food_crate" },
        { 1, "Tool crate",       "tool_crate" },
        { 1, "Small crate",      "small_crate" },
        { 1, "Health crate",     "health_crate" },
        { 1, "Small food crate", "small_food_crate" },
        { 1, "Vehicle parts",    "vehicle_parts" },
        { 1, "Crate",            "crate" },

        { 2, "Wooden box",       "wooden_box" },
        { 2, "Large wood box",   "large_wood_box" },
        { 2, "Tool cupboard",    "tool_cupboard" },
        { 2, "Tier 1 workbench", "tier_1_workbench" },
        { 2, "Tier 2 workbench", "tier_2_workbench" },
        { 2, "Tier 3 workbench", "tier_3_workbench" },
        { 2, "Repair bench",     "repair_bench" },
        { 2, "Research table",   "research_table" },
        { 2, "Small stash",      "small_stash" },
        { 2, "Sleeping bag",     "sleeping_bag" },
        { 2, "Furnace",          "furnace" },
        { 2, "Locker",           "locker" },

        { 3, "Auto turret",      "auto_turret" },
        { 3, "Flame turret",     "flame_turret" },
        { 3, "Shotgun trap",     "shotgun_trap" },

        { 4, "Scientist",        "scientist" },
        { 4, "Dweller",          "dweller" },
        { 4, "Bear",             "bear" },
        { 4, "Wolf",             "wolf" },
        { 4, "Stag",             "stag" },
        { 4, "Boar",             "boar" },
        { 4, "Horse",            "horse" },
        { 4, "Chicken",          "chicken" },
        { 4, "Shark",            "shark" },
        { 4, "Scarecrow",        "scarecrow" },
        { 4, "Corpse",           "corpse" },

        { 5, "Recycler",         "recycler" },
        { 5, "Phone booth",      "phone_booth" },
        { 5, "Fuse box",         "fuse_box" },
        { 5, "Refinery",         "refinery" },
        { 5, "Elevator",         "elevator" },
        { 5, "Car lift",         "car_lift" },
        { 5, "Computer station", "computer_station" },

        { 6, "Motorbike",         "motorbike" },
        { 6, "Sidecar motorbike", "sidecar_motorbike" },
        { 6, "Pedal bike",        "pedal_bike" },
        { 6, "Snowmobile",        "snowmobile" },
        { 6, "Tomaha snowmobile", "tomaha_snowmobile" },
        { 6, "Tugboat",           "tugboat" },
        { 6, "Rowboat",           "rowboat" },
        { 6, "RHIB",              "rhib" },
        { 6, "Kyak",              "kyak" },
        { 6, "Solo submarine",    "solo_submarine" },
        { 6, "Duo submarine",     "duo_submarine" },
        { 6, "Minicopter",        "minicopter" },
        { 6, "Scrap heli",        "scrap_heli" },
        { 6, "Attack heli",       "attack_heli" },

        { 7, "Dropped rifles",    "dropped_rifles" },
        { 7, "Dropped snipers",   "dropped_snipers" },
        { 7, "Dropped SMGs",      "dropped_smgs" },
        { 7, "Dropped shotguns",  "dropped_shotguns" },
        { 7, "Dropped pistols",   "dropped_pistols" },
        { 7, "Dropped LMGs",      "dropped_lmgs" },
        { 7, "Dropped launchers", "dropped_launchers" },
        { 7, "Dropped bows",      "dropped_bows" },
        { 7, "Dropped melee",     "dropped_melee" },

        { 8, "Throwables",        "throwables" },
        { 8, "Tools",             "tools" },
        { 8, "Medical",           "medical" },
        { 8, "Ammo",              "ammo" },
        { 8, "Misc",              "misc" },
    };

    inline constexpr const char* kEntityCategoryNames[9] = {
        "Ores & Collectibles",
        "Crates & Barrels",
        "Deployables",
        "Traps & Turrets",
        "NPCs & Animals",
        "Static & Monuments",
        "Vehicles",
        "Dropped weapons",
        "Dropped items",
    };
}
