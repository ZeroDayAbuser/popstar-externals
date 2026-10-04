#include <Cheat.hpp>
#include "OverlayBridge.hpp"
#include "OverlayUI.h"

#include <atomic>
#include <config/config.hpp>
namespace OverlayUI {

    constexpr float kTopMargin   = 12.0f;
    constexpr float kPillW       = 72.0f;
    constexpr float kPillH       = 24.0f;
    constexpr float kPillRadius  = 12.0f;
    constexpr float kMenuW       = 720.0f;
    constexpr float kMenuH       = 580.0f;
    constexpr float kMenuRadius  = 14.0f;

    constexpr float kMaxContainerHeight = 460.0f;
    constexpr float kScrollbarWidth     = 5.0f;
    constexpr float kScrollLineSize     = 32.0f;

    constexpr float kEaseSpeed       = 8.0f;
    constexpr DWORD kCollapseDelayMs = 250;

    enum class State { Pill, Menu };
    static State g_state             = State::Pill;
    static float g_anim              = 0.0f;
    static DWORD g_outsideSinceTick  = 0;
    static DWORD g_menuOpenTick      = 0;

    static ShapeRect g_shape{ 0, 0, kPillW, kPillH, kPillRadius };

    static bool g_cursorVisibleCached = false;

    constexpr int kNumMainTabs   = 4;
    constexpr int kMaxSubtabs    = 5;
    constexpr int kMaxContainers = 10;
    constexpr int kMaxWidgets    = 40;

    constexpr DWORD kTabSwitchMs     = 220;
    constexpr float kTabIndicEase    = 14.0f;

    enum PopupKind : int {
        PK_None            = 0,
        PK_BoxFill         = 1,
        PK_Skeleton        = 2,
        PK_OffscreenArrows = 3,
        PK_MaxDistance     = 4,
        PK_Generator       = 5,
        PK_PostSaturation  = 6,
        PK_PostContrast    = 7,
        PK_PostSharpness   = 8,
        PK_PostBloom       = 9,
        PK_FlagsSurvivor   = 10,
        PK_FlagsKiller     = 11,
        PK_AutoPallet      = 12,
        PK_Desync          = 13,
        PK_SpeedBoost      = 14,
        PK_ThickBullet     = 15,
        PK_ColorPicker     = 20,
        PK_Confirm         = 21,
        // Override popups — each binds a "Amount" slider that misc.cpp
        // reads via app::get_value(... "Options", "Amount"). Previously
        // all three checkboxes pointed at PK_MaxDistance, which rendered
        // a "Max distance" 1-300m slider — wrong label + wrong range.
        PK_OverrideSpread  = 22,
        PK_OverrideRecoil  = 23,
        PK_OverrideMelee   = 24,
        PK_FlagsRust       = 25,   // Rust player flags — Distance / Sleeping / Wounded / Team ID / Aiming
        PK_FovZoom         = 26,   // FOV slider gear → "Zoom FOV" (default 35)
        PK_EnableWritesConfirm = 27,  // master writes gate — confirm popup before flipping on
    };

    enum class WT {
        Section, Check, Slider, Flags, Keybind,
        Listbox,
        TextInput,
        ConfirmBtn,
        Dropdown,
    };
    struct Widget {
        WT          type;
        const char* label;
        float       minV;
        float       maxV;
        float       defV;
        const char* suffix;
        bool        hasColor;
        bool        hasGear;
        int         popupKind;
        const char* bind;
        bool        writes;            // true → modifies game memory; hidden until Settings → Enable writes
        bool        highRisk = false;  // true → red "?" badge + "HIGH RISK" hover tooltip
    };
    struct Container {
        const char* title;
        int         col;
        Widget      widgets[kMaxWidgets];
    };
    struct SubtabDef {
        const char* name;
        const char* desc;
        Container   containers[kMaxContainers];
    };
    struct MainTabDef {
        const char* name;
        int         numSubtabs;
        SubtabDef   subtabs[kMaxSubtabs];
    };

    const MainTabDef kTabs[kNumMainTabs] = {
        {
            "Aimbot", 2,
            {
                {
                    "General",
                    "Aim assist — toggles, filters, visualization.",
                    {
                        {
                            "Main", 0,
                            {
                                { WT::Check,    "Enabled",                  0,0,0, "", false, true,  PK_MaxDistance, "Aimbot/General/Main/Enabled",             true },
                                { WT::Check,    "Silent aim",               0,0,0, "", false, true,  PK_MaxDistance, "Aimbot/General/Main/Silent aim",          true },
                                { WT::Check,    "Smart target",             0,0,0, "", false, false, PK_None,        "Aimbot/General/Main/Smart target",        true },
                                { WT::Check,    "Target 360",               0,0,0, "", false, false, PK_None,        "Aimbot/General/Main/Target 360",          true },
                                { WT::Dropdown, "Target bone",              0,0,0, "", false, false, PK_None,        "Aimbot/General/Main/Target bone",         true },
                                { WT::Check,    "Prediction",               0,0,0, "", false, false, PK_None,        "Aimbot/General/Main/Prediction",          true },
                                { WT::Keybind,  "Aim key",                  0,0,0, "", false, false, PK_None,        nullptr,                                   true },
                            }
                        },
                        {
                            "Filters", 1,
                            {
                                { WT::Check,  "Skip scientists",          0,0,0, "", false, false, PK_None,        "Aimbot/General/Filters/Skip scientists",  true },
                                { WT::Check,  "Skip dwellers",            0,0,0, "", false, false, PK_None,        "Aimbot/General/Filters/Skip dwellers",    true },
                                { WT::Check,  "Skip sleepers",            0,0,0, "", false, false, PK_None,        "Aimbot/General/Filters/Skip sleepers",    true },
                                { WT::Check,  "Skip wounded",             0,0,0, "", false, false, PK_None,        "Aimbot/General/Filters/Skip wounded",     true },
                                { WT::Check,  "Skip animals",             0,0,0, "", false, false, PK_None,        "Aimbot/General/Filters/Skip animals",     true },
                            }
                        },
                        {
                            "Humanization", 1,
                            {
                                { WT::Slider, "Smoothing",                1, 50, 10, "", false, false, PK_None,    "f:Aimbot/General/Humanization/Smoothing", true },
                            }
                        },
                        {
                            "Visualization", 1,
                            {
                                { WT::Check,  "Field of view circle",     0,0,0, "", true, true, PK_None,          "Aimbot/Visualization/Field of view circle", true },
                                { WT::Slider, "FOV size",                 10, 500, 125, "px", false, false, PK_None, "i:Aimbot/Visualization/Field of view circle/Options/Size", true },
                                { WT::Check,  "Bullet tracers",           0,0,0, "", true, false, PK_None,         "Aimbot/Visualization/Bullet tracers",       true },
                                { WT::Check,  "Prediction path",          0,0,0, "", true, true,  PK_None,         "Aimbot/Visualization/Prediction path",      true },
                            }
                        },
                    }
                },
                {
                    "Weapons",
                    "Weapon modifications — instant fires, recoil/spread overrides, hitbox.",
                    {
                        {
                            "Modifications", 0,
                            {
                                { WT::Check,    "Automatic weapons",    0,0,0, "", false, false, PK_None,           "Aimbot/Weapons/Automatic weapons",    true },
                                { WT::Check,    "Instant bow",          0,0,0, "", false, false, PK_None,           "Aimbot/Weapons/Instant bow",          true },
                                { WT::Check,    "Instant eoka",         0,0,0, "", false, false, PK_None,           "Aimbot/Weapons/Instant eoka",         true },
                                { WT::Check,    "Thick bullet",         0,0,0, "", false, false, PK_None,           "Aimbot/Weapons/Thick bullet",         true, /*highRisk*/ true },
                                { WT::Check,    "Rapid fire",           0,0,0, "", false, false, PK_None,           "Aimbot/Weapons/Rapid fire",           true, /*highRisk*/ true },
                                { WT::Check,    "No sway",              0,0,0, "", false, false, PK_None,           "Aimbot/Weapons/No sway",              true },
                                { WT::Check,    "No animation",         0,0,0, "", false, false, PK_None,           "Aimbot/Weapons/No animation",         true },
                                { WT::Check,    "Hitbox override",      0,0,0, "", false, false, PK_None,           "Aimbot/Weapons/Hitbox override",      true, /*highRisk*/ true },
                                { WT::Dropdown, "Hit bone",             0,0,0, "", false, false, PK_None,           "Aimbot/Weapons/Hitbox override/Bone", true },
                            }
                        },
                        {
                            "Overrides", 1,
                            {
                                { WT::Check,    "Override weapon spread",  0,0,0, "", false, true, PK_OverrideSpread, "Aimbot/Weapons/Override weapon spread", true },
                                { WT::Check,    "Override weapon recoil",  0,0,0, "", false, true, PK_OverrideRecoil, "Aimbot/Weapons/Override weapon recoil", true },
                                { WT::Check,    "Override melee range",    0,0,0, "", false, true, PK_OverrideMelee,  "Aimbot/Weapons/Override melee range",   true },
                            }
                        },
                    }
                },
            }
        },
        {
            "Visuals", 2,
            {
                {
                    "Players",
                    "Player ESP + screen overlays — all visuals on humans and HUD.",
                    {
                        {
                            "Player ESP", 0,
                            {
                                { WT::Check,    "Enabled",            0,0,0, "", false, true,  PK_MaxDistance,     "Visuals/Players/Enabled"                              },
                                { WT::Flags,    "Flags",              0,0,0, "", false, false, PK_FlagsRust,       nullptr                                                },

                                // Vischeck UI hidden — feature still wired in code (and the
                                // PhysX cache thread still runs) but the user can't toggle it.
                                // Re-add the Vischeck section here when the integration is solid.

                                { WT::Section,  "Identification",     0,0,0, "" },
                                { WT::Check,    "Name",               0,0,0, "", true,  true,  PK_MaxDistance,     "Visuals/Players/Name"                                 },
                                { WT::Check,    "Show avatar",        0,0,0, "", false, false, PK_None,            "Visuals/Players/Name/Options/Show avatar"             },

                                { WT::Section,  "Body",               0,0,0, "" },
                                { WT::Check,    "Bounding box",       0,0,0, "", true,  true,  PK_BoxFill,         "Visuals/Players/Bounding box"                         },
                                { WT::Check,    "Box fill",           0,0,0, "", true,  false, PK_None,            "Visuals/Players/Bounding box/Options/Fill"            },
                                { WT::Check,    "Box gradient",       0,0,0, "", true,  false, PK_None,            "Visuals/Players/Bounding box/Options/Gradient"        },
                                { WT::Check,    "Skeleton",           0,0,0, "", true,  true,  PK_Skeleton,        "Visuals/Players/Skeleton"                             },
                                { WT::Check,    "Head circle",        0,0,0, "", true,  false, PK_None,            "Visuals/Players/Head circle"                          },
                                { WT::Dropdown, "Chams",              0,0,0, "", true,  false, PK_None,            "Visuals/Players/Chams",                               true },
                            }
                        },
                        {
                            "Aim helpers", 1,
                            {
                                { WT::Check,    "View line",          0,0,0, "", true,  false, PK_None,            "Visuals/Players/View line"                            },
                                { WT::Check,    "Snap lines",         0,0,0, "", true,  false, PK_None,            "Visuals/Players/Snap lines"                           },
                            }
                        },
                        {
                            "Off-screen", 1,
                            {
                                { WT::Check,    "Show on radar",      0,0,0, "", false, false, PK_None,            "Visuals/Players/Enabled/Options/Include in radar"     },
                                { WT::Check,    "Out of view arrows", 0,0,0, "", true,  true,  PK_OffscreenArrows, "Visuals/Players/Out of view arrows"                   },
                                { WT::Check,    "Arrow glow",         0,0,0, "", false, false, PK_None,            "Visuals/Players/Out of view arrows/Options/Glow"      },
                            }
                        },
                        {
                            "HUD", 1,
                            {
                                { WT::Check,    "Crosshair",          0,0,0, "", true,  false, PK_None,            "Visuals/Screen/Crosshair"                             },
                                { WT::Check,    "Player inventory",   0,0,0, "", false, false, PK_None,            "Visuals/Screen/Player inventory"                      },
                                { WT::Check,    "Item name",          0,0,0, "", true,  false, PK_None,            "Visuals/Players/Item name"                            },
                                { WT::Check,    "Item icon",          0,0,0, "", true,  false, PK_None,            "Visuals/Players/Item icon"                            },
                            }
                        },
                        {
                            "Radar", 1,
                            {
                                { WT::Check,    "Enabled",            0,0,0, "", false, false, PK_None,            "Visuals/Radar/Enabled"                                },
                                { WT::Slider,   "Size",              50,250,120, "px", false, false, PK_None,      "f:Visuals/Radar/Size"                                  },
                                { WT::Slider,   "Range",             10,500,150, "m",  false, false, PK_None,      "f:Visuals/Radar/Range"                                 },
                            }
                        },
                        {
                            "Filters", 1,
                            {
                                { WT::Check,    "Show sleepers",      0,0,0, "", false, false, PK_None,            "Visuals/Players/Show sleepers"                        },
                            }
                        },
                    }
                },
                {
                    "Entities",
                    "Per-object ESP — every Rust entity category from the framework.",
                    {
                        {
                            "Master", 0,
                            {
                                { WT::Check, "Enabled (all entities)",      0,0,0, "", false, false, PK_None,        "Visuals/Entities/Enabled" },
                            }
                        },
                        {
                            "Ores & Collectibles", 0,
                            {
                                { WT::Check, "Enabled",                     0,0,0, "", true, false, PK_None, "Visuals/Entities/Ores & Collectibles/Enabled"         },
                                { WT::Check, "Stone",                       0,0,0, "", true, true, PK_MaxDistance, "Visuals/Entities/Ores & Collectibles/Stone"           },
                                { WT::Check, "Sulfur",                      0,0,0, "", true, true, PK_MaxDistance, "Visuals/Entities/Ores & Collectibles/Sulfur"          },
                                { WT::Check, "Metal",                       0,0,0, "", true, true, PK_MaxDistance, "Visuals/Entities/Ores & Collectibles/Metal"           },
                                { WT::Check, "Hemp",                        0,0,0, "", true, true, PK_MaxDistance, "Visuals/Entities/Ores & Collectibles/Hemp"            },
                                { WT::Check, "Wood",                        0,0,0, "", true, true, PK_MaxDistance, "Visuals/Entities/Ores & Collectibles/Wood"            },
                                { WT::Check, "Diesel fuel",                 0,0,0, "", true, true, PK_MaxDistance, "Visuals/Entities/Ores & Collectibles/Diesel fuel"     },
                                { WT::Check, "Green keycard",               0,0,0, "", true, true, PK_MaxDistance, "Visuals/Entities/Ores & Collectibles/Green keycard"   },
                                { WT::Check, "Blue keycard",                0,0,0, "", true, true, PK_MaxDistance, "Visuals/Entities/Ores & Collectibles/Blue keycard"    },
                                { WT::Check, "Red keycard",                 0,0,0, "", true, true, PK_MaxDistance, "Visuals/Entities/Ores & Collectibles/Red keycard"     },
                            }
                        },
                        {
                            "Crates & Barrels", 0,
                            {
                                { WT::Check, "Enabled",                     0,0,0, "", true, false, PK_None, "Visuals/Entities/Crates & Barrels/Enabled"            },
                                { WT::Check, "Elite crate",                 0,0,0, "", true, true, PK_MaxDistance, "Visuals/Entities/Crates & Barrels/Elite crate"        },
                                { WT::Check, "Military crate",              0,0,0, "", true, true, PK_MaxDistance, "Visuals/Entities/Crates & Barrels/Military crate"     },
                                { WT::Check, "Locked crate",                0,0,0, "", true, true, PK_MaxDistance, "Visuals/Entities/Crates & Barrels/Locked crate"       },
                                { WT::Check, "Air drop",                    0,0,0, "", true, true, PK_MaxDistance, "Visuals/Entities/Crates & Barrels/Air drop"           },
                                { WT::Check, "Loot barrel",                 0,0,0, "", true, true, PK_MaxDistance, "Visuals/Entities/Crates & Barrels/Loot barrel"        },
                                { WT::Check, "Oil barrel",                  0,0,0, "", true, true, PK_MaxDistance, "Visuals/Entities/Crates & Barrels/Oil barrel"         },
                                { WT::Check, "Food crate",                  0,0,0, "", true, true, PK_MaxDistance, "Visuals/Entities/Crates & Barrels/Food crate"         },
                                { WT::Check, "Tool crate",                  0,0,0, "", true, true, PK_MaxDistance, "Visuals/Entities/Crates & Barrels/Tool crate"         },
                                { WT::Check, "Small crate",                 0,0,0, "", true, true, PK_MaxDistance, "Visuals/Entities/Crates & Barrels/Small crate"        },
                                { WT::Check, "Health crate",                0,0,0, "", true, true, PK_MaxDistance, "Visuals/Entities/Crates & Barrels/Health crate"       },
                                { WT::Check, "Small food crate",            0,0,0, "", true, true, PK_MaxDistance, "Visuals/Entities/Crates & Barrels/Small food crate"   },
                                { WT::Check, "Vehicle parts",               0,0,0, "", true, true, PK_MaxDistance, "Visuals/Entities/Crates & Barrels/Vehicle parts"      },
                                { WT::Check, "Crate",                       0,0,0, "", true, true, PK_MaxDistance, "Visuals/Entities/Crates & Barrels/Crate"              },
                            }
                        },
                        {
                            "Deployables", 0,
                            {
                                { WT::Check, "Enabled",                     0,0,0, "", true, false, PK_None, "Visuals/Entities/Deployables/Enabled"                 },
                                { WT::Check, "Wooden box",                  0,0,0, "", true, true, PK_MaxDistance, "Visuals/Entities/Deployables/Wooden box"              },
                                { WT::Check, "Large wood box",              0,0,0, "", true, true, PK_MaxDistance, "Visuals/Entities/Deployables/Large wood box"          },
                                { WT::Check, "Tool cupboard",               0,0,0, "", true, true, PK_MaxDistance, "Visuals/Entities/Deployables/Tool cupboard"           },
                                { WT::Check, "Tier 1 workbench",            0,0,0, "", true, true, PK_MaxDistance, "Visuals/Entities/Deployables/Tier 1 workbench"        },
                                { WT::Check, "Tier 2 workbench",            0,0,0, "", true, true, PK_MaxDistance, "Visuals/Entities/Deployables/Tier 2 workbench"        },
                                { WT::Check, "Tier 3 workbench",            0,0,0, "", true, true, PK_MaxDistance, "Visuals/Entities/Deployables/Tier 3 workbench"        },
                                { WT::Check, "Repair bench",                0,0,0, "", true, true, PK_MaxDistance, "Visuals/Entities/Deployables/Repair bench"            },
                                { WT::Check, "Research table",              0,0,0, "", true, true, PK_MaxDistance, "Visuals/Entities/Deployables/Research table"          },
                                { WT::Check, "Small stash",                 0,0,0, "", true, true, PK_MaxDistance, "Visuals/Entities/Deployables/Small stash"             },
                                { WT::Check, "Sleeping bag",                0,0,0, "", true, true, PK_MaxDistance, "Visuals/Entities/Deployables/Sleeping bag"            },
                                { WT::Check, "Furnace",                     0,0,0, "", true, true, PK_MaxDistance, "Visuals/Entities/Deployables/Furnace"                 },
                                { WT::Check, "Locker",                      0,0,0, "", true, true, PK_MaxDistance, "Visuals/Entities/Deployables/Locker"                  },
                            }
                        },
                        {
                            "Traps & Turrets", 1,
                            {
                                { WT::Check, "Enabled",                     0,0,0, "", true, false, PK_None, "Visuals/Entities/Traps & Turrets/Enabled"             },
                                { WT::Check, "Auto turret",                 0,0,0, "", true, true, PK_MaxDistance, "Visuals/Entities/Traps & Turrets/Auto turret"         },
                                { WT::Check, "Flame turret",                0,0,0, "", true, true, PK_MaxDistance, "Visuals/Entities/Traps & Turrets/Flame turret"        },
                                { WT::Check, "Shotgun trap",                0,0,0, "", true, true, PK_MaxDistance, "Visuals/Entities/Traps & Turrets/Shotgun trap"        },
                            }
                        },
                        {
                            "NPCs & Animals", 1,
                            {
                                { WT::Check, "Enabled",                     0,0,0, "", true, false, PK_None, "Visuals/Entities/NPCs & Animals/Enabled"              },
                                { WT::Check, "Scientist",                   0,0,0, "", true, true, PK_MaxDistance, "Visuals/Entities/NPCs & Animals/Scientist"            },
                                { WT::Check, "Dweller",                     0,0,0, "", true, true, PK_MaxDistance, "Visuals/Entities/NPCs & Animals/Dweller"              },
                                { WT::Check, "Bear",                        0,0,0, "", true, true, PK_MaxDistance, "Visuals/Entities/NPCs & Animals/Bear"                 },
                                { WT::Check, "Wolf",                        0,0,0, "", true, true, PK_MaxDistance, "Visuals/Entities/NPCs & Animals/Wolf"                 },
                                { WT::Check, "Stag",                        0,0,0, "", true, true, PK_MaxDistance, "Visuals/Entities/NPCs & Animals/Stag"                 },
                                { WT::Check, "Boar",                        0,0,0, "", true, true, PK_MaxDistance, "Visuals/Entities/NPCs & Animals/Boar"                 },
                                { WT::Check, "Horse",                       0,0,0, "", true, true, PK_MaxDistance, "Visuals/Entities/NPCs & Animals/Horse"                },
                                { WT::Check, "Chicken",                     0,0,0, "", true, true, PK_MaxDistance, "Visuals/Entities/NPCs & Animals/Chicken"              },
                                { WT::Check, "Shark",                       0,0,0, "", true, true, PK_MaxDistance, "Visuals/Entities/NPCs & Animals/Shark"                },
                                { WT::Check, "Scarecrow",                   0,0,0, "", true, true, PK_MaxDistance, "Visuals/Entities/NPCs & Animals/Scarecrow"            },
                                { WT::Check, "Corpse",                      0,0,0, "", true, true, PK_MaxDistance, "Visuals/Entities/NPCs & Animals/Corpse"               },
                            }
                        },
                        {
                            "Static & Monuments", 1,
                            {
                                { WT::Check, "Enabled",                     0,0,0, "", true, false, PK_None, "Visuals/Entities/Static & Monuments/Enabled"          },
                                { WT::Check, "Recycler",                    0,0,0, "", true, true, PK_MaxDistance, "Visuals/Entities/Static & Monuments/Recycler"         },
                                { WT::Check, "Phone booth",                 0,0,0, "", true, true, PK_MaxDistance, "Visuals/Entities/Static & Monuments/Phone booth"      },
                                { WT::Check, "Fuse box",                    0,0,0, "", true, true, PK_MaxDistance, "Visuals/Entities/Static & Monuments/Fuse box"         },
                                { WT::Check, "Refinery",                    0,0,0, "", true, true, PK_MaxDistance, "Visuals/Entities/Static & Monuments/Refinery"         },
                                { WT::Check, "Elevator",                    0,0,0, "", true, true, PK_MaxDistance, "Visuals/Entities/Static & Monuments/Elevator"         },
                                { WT::Check, "Car lift",                    0,0,0, "", true, true, PK_MaxDistance, "Visuals/Entities/Static & Monuments/Car lift"         },
                                { WT::Check, "Computer station",            0,0,0, "", true, true, PK_MaxDistance, "Visuals/Entities/Static & Monuments/Computer station" },
                            }
                        },
                        {
                            "Vehicles + Weapons", 1,
                            {
                                { WT::Section, "Vehicles",                  0,0,0, "" },
                                { WT::Check, "Enabled",                     0,0,0, "", true, false, PK_None, "Visuals/Entities/Vehicles/Enabled"                    },
                                { WT::Check, "Motorbike",                   0,0,0, "", true, true, PK_MaxDistance, "Visuals/Entities/Vehicles/Motorbike"                  },
                                { WT::Check, "Sidecar motorbike",           0,0,0, "", true, true, PK_MaxDistance, "Visuals/Entities/Vehicles/Sidecar motorbike"          },
                                { WT::Check, "Pedal bike",                  0,0,0, "", true, true, PK_MaxDistance, "Visuals/Entities/Vehicles/Pedal bike"                 },
                                { WT::Check, "Snowmobile",                  0,0,0, "", true, true, PK_MaxDistance, "Visuals/Entities/Vehicles/Snowmobile"                 },
                                { WT::Check, "Tomaha snowmobile",           0,0,0, "", true, true, PK_MaxDistance, "Visuals/Entities/Vehicles/Tomaha snowmobile"          },
                                { WT::Check, "Tugboat",                     0,0,0, "", true, true, PK_MaxDistance, "Visuals/Entities/Vehicles/Tugboat"                    },
                                { WT::Check, "Rowboat",                     0,0,0, "", true, true, PK_MaxDistance, "Visuals/Entities/Vehicles/Rowboat"                    },
                                { WT::Check, "RHIB",                        0,0,0, "", true, true, PK_MaxDistance, "Visuals/Entities/Vehicles/RHIB"                       },
                                { WT::Check, "Kyak",                        0,0,0, "", true, true, PK_MaxDistance, "Visuals/Entities/Vehicles/Kyak"                       },
                                { WT::Check, "Solo submarine",              0,0,0, "", true, true, PK_MaxDistance, "Visuals/Entities/Vehicles/Solo submarine"             },
                                { WT::Check, "Duo submarine",               0,0,0, "", true, true, PK_MaxDistance, "Visuals/Entities/Vehicles/Duo submarine"              },
                                { WT::Check, "Minicopter",                  0,0,0, "", true, true, PK_MaxDistance, "Visuals/Entities/Vehicles/Minicopter"                 },
                                { WT::Check, "Scrap heli",                  0,0,0, "", true, true, PK_MaxDistance, "Visuals/Entities/Vehicles/Scrap heli"                 },
                                { WT::Check, "Attack heli",                 0,0,0, "", true, true, PK_MaxDistance, "Visuals/Entities/Vehicles/Attack heli"                },

                                { WT::Section, "Weapons",                   0,0,0, "" },
                                { WT::Check, "Enabled",                     0,0,0, "", true, false, PK_None,        "Visuals/Entities/Dropped weapons/Enabled"             },
                                { WT::Check, "Rifles",                      0,0,0, "", true, true,  PK_MaxDistance, "Visuals/Entities/Dropped weapons/Dropped rifles"      },
                                { WT::Check, "Snipers",                     0,0,0, "", true, true,  PK_MaxDistance, "Visuals/Entities/Dropped weapons/Dropped snipers"     },
                                { WT::Check, "SMGs",                        0,0,0, "", true, true,  PK_MaxDistance, "Visuals/Entities/Dropped weapons/Dropped SMGs"        },
                                { WT::Check, "Shotguns",                    0,0,0, "", true, true,  PK_MaxDistance, "Visuals/Entities/Dropped weapons/Dropped shotguns"    },
                                { WT::Check, "Pistols",                     0,0,0, "", true, true,  PK_MaxDistance, "Visuals/Entities/Dropped weapons/Dropped pistols"     },
                                { WT::Check, "LMGs",                        0,0,0, "", true, true,  PK_MaxDistance, "Visuals/Entities/Dropped weapons/Dropped LMGs"        },
                                { WT::Check, "Launchers",                   0,0,0, "", true, true,  PK_MaxDistance, "Visuals/Entities/Dropped weapons/Dropped launchers"   },
                                { WT::Check, "Bows",                        0,0,0, "", true, true,  PK_MaxDistance, "Visuals/Entities/Dropped weapons/Dropped bows"        },
                                { WT::Check, "Melee",                       0,0,0, "", true, true,  PK_MaxDistance, "Visuals/Entities/Dropped weapons/Dropped melee"       },
                            }
                        },
                        {
                            "Dropped items", 0,
                            {
                                { WT::Check, "Enabled",                     0,0,0, "", true, false, PK_None, "Visuals/Entities/Dropped items/Enabled"      },
                                { WT::Check, "Throwables",                  0,0,0, "", true, true,  PK_MaxDistance, "Visuals/Entities/Dropped items/Throwables"   },
                                { WT::Check, "Tools",                       0,0,0, "", true, true,  PK_MaxDistance, "Visuals/Entities/Dropped items/Tools"        },
                                { WT::Check, "Medical",                     0,0,0, "", true, true,  PK_MaxDistance, "Visuals/Entities/Dropped items/Medical"      },
                                { WT::Check, "Ammo",                        0,0,0, "", true, true,  PK_MaxDistance, "Visuals/Entities/Dropped items/Ammo"         },
                                { WT::Check, "Misc",                        0,0,0, "", true, true,  PK_MaxDistance, "Visuals/Entities/Dropped items/Misc"         },
                            }
                        },
                    }
                },
            }
        },
        {
            "Misc", 5,
            {
                {
                    "General",
                    "Gameplay exploits and shortcuts.",
                    {
                        {
                            "Exploits", 0,
                            {
                                { WT::Check,    "Fast loot",            0,0,0, "", false, false, PK_None,           "Misc/General/Fast loot",            true },
                                { WT::Check,    "Instant untie crate",  0,0,0, "", false, false, PK_None,           "Misc/General/Instant untie crate",  true },
                                { WT::Check,    "Instant interactions", 0,0,0, "", false, false, PK_None,           "Misc/General/Instant interactions", true, /*highRisk*/ true },
                                { WT::Check,    "Instant revive",       0,0,0, "", false, false, PK_None,           "Misc/General/Instant revive",       true, /*highRisk*/ true },
                                { WT::Keybind,  "Instant revive key",   0,0,0, "", false, false, PK_None,           nullptr,                             true },
                            }
                        },
                    }
                },
                {
                    "Movement",
                    "Movement exploits.",
                    {
                        {
                            "Movement", 0,
                            {
                                { WT::Check,   "Spider man",       0,0,0, "", false, false, PK_None,              "Misc/Movement/Spider man",        true },
                                { WT::Keybind, "Spider man key",   0,0,0, "", false, false, PK_None,              nullptr,                           true },
                                { WT::Check,   "Omni sprint",      0,0,0, "", false, false, PK_None,              "Misc/Movement/Omni sprint",       true, /*highRisk*/ true },
                                { WT::Check,   "Fly",              0,0,0, "", false, false, PK_None,              "Misc/Movement/Fly",               true },
                                { WT::Keybind, "Fly key",          0,0,0, "", false, false, PK_None,              nullptr,                           true },
                                { WT::Check,   "No fall",          0,0,0, "", false, false, PK_None,              "Misc/Movement/No fall",           true },
                                { WT::Check,  "Remove water drag",0,0,0, "", false, false, PK_None,              "Misc/Movement/Remove water drag",  true },
                                { WT::Check,  "Walk on water",    0,0,0, "", false, false, PK_None,              "Misc/Movement/Walk on water",      true },
                                { WT::Check,  "Anti-aim",         0,0,0, "", false, false, PK_None,              "Misc/Movement/Anti-aim",           true },
                            }
                        },
                    }
                },
                {
                    "Camera",
                    "Local view tweaks.",
                    {
                        {
                            "View", 0,
                            {
                                { WT::Check,   "Custom FOV",      0,0,0,  "", false, false, PK_None,             "Visuals/Local/Custom FOV",          true },
                                { WT::Slider,  "Field of view",  40,150,90, "", false, true,  PK_FovZoom,         "f:Visuals/Local/Field of view",     true },
                                { WT::Keybind, "Zoom key",        0,0,0,  "", false, false, PK_None,             nullptr,                             true },
                                { WT::Check,   "Third person",    0,0,0,  "", false, false, PK_None,             "Visuals/Local/Third person",        true },
                                { WT::Check,   "Debug camera",      0,0,0,  "", false, false, PK_None,             "Visuals/Local/Debug camera",        true, /*highRisk*/ true },
                                { WT::Keybind, "Debug camera key",  0,0,0,  "", false, false, PK_None,             nullptr,                             true },
                                { WT::Slider,  "Debug camera speed", 1, 50, 10, " m/s", false, false, PK_None,    "f:Visuals/Local/Debug camera speed",true },
                                { WT::Slider,  "Mouse sensitivity",  1, 200, 40, "%",   false, false, PK_None,    "f:Visuals/Local/Mouse sensitivity", true },
                            }
                        },
                    }
                },
                {
                    "World",
                    "Time of day, sky modulation, ambience.",
                    {
                        {
                            "Time", 0,
                            {
                                { WT::Check,  "Time changer",        0,0,0, "", false, false, PK_None,            "Visuals/World/Time changer",     true },
                                { WT::Slider, "Time",                0, 24, 18, "h", false, false, PK_None,       "f:Visuals/World/Time",           true },
                            }
                        },
                        {
                            "Modulate", 1,
                            {
                                { WT::Check,  "Modulate sun",        0,0,0, "", true, false, PK_None,             "Visuals/World/Modulate sun",      true },
                                { WT::Check,  "Modulate lights",     0,0,0, "", true, false, PK_None,             "Visuals/World/Modulate lights",   true },
                                { WT::Check,  "Modulate rays",       0,0,0, "", true, false, PK_None,             "Visuals/World/Modulate rays",     true },
                                { WT::Check,  "Modulate sky",        0,0,0, "", true, false, PK_None,             "Visuals/World/Modulate sky",      true },
                                { WT::Check,  "Modulate clouds",     0,0,0, "", true, false, PK_None,             "Visuals/World/Modulate clouds",   true },
                                { WT::Check,  "Modulate fog",        0,0,0, "", true, false, PK_None,             "Visuals/World/Modulate fog",      true },
                                { WT::Check,  "Modulate ambience",   0,0,0, "", true, false, PK_None,             "Visuals/World/Modulate ambience", true },
                            }
                        },
                        {
                            "Lighting", 1,
                            {
                                { WT::Check,  "Bright night",        0,0,0, "", false, false, PK_None,            "Visuals/World/Bright night",     true },
                            }
                        },
                    }
                },
                {
                    "Debug",
                    "Developer diagnostics.",
                    {
                        {
                            "Debug", 0,
                            {
                                { WT::Check,  "Debug ESP",        0,0,0, "", true,  false, PK_None,              "Misc/General/Debug ESP"          },
                            }
                        },
                    }
                },
            }
        },
        {
            "Settings", 2,
            {
                {
                    "Configs",
                    "Save your desired features and customization here.",
                    {
                        {
                            "Memory writes", 0,
                            {
                                { WT::Check,  "Enable memory writes", 0,0,0, "", false, false, PK_EnableWritesConfirm, "Settings/UI/Enable memory writes" },
                            }
                        },
                        {
                            "General", 0,
                            {
                                { WT::Listbox,    "",                   0, 0, 0, "",  false, false, PK_None },
                                { WT::TextInput,  "New config name...", 0, 0, 0, "",  false, false, PK_None },
                                { WT::ConfirmBtn, "Create config",      0, 0, 0, "",  false, false, 1       },
                                { WT::ConfirmBtn, "Save config",        0, 0, 0, "",  false, false, 2       },
                                { WT::ConfirmBtn, "Load config",        0, 0, 0, "",  false, false, 3       },
                                { WT::ConfirmBtn, "Delete config",      0, 0, 0, "",  false, false, 4       },
                            }
                        },
                    }
                },
                {
                    "UI",
                    "Loader UI preferences.",
                    {
                        {
                            "UI", 0,
                            {
                                { WT::Check,  "Override accent",  0,0,0, "", true,  false, PK_None,              "Settings/UI/Override accent"     },
                                { WT::Check,  "Vertical sync",    0,0,0, "", false, false, PK_None,              "Settings/UI/Vertical sync"       },
                            }
                        },
                    }
                },
            }
        },
    };

    static int   g_mainTab          = 0;
    static int   g_subtab[kNumMainTabs] = {};
    static int   g_lastMainTab      = 0;
    static DWORD g_tabSwitchTick    = 0;
    static float g_indicatorX       = -1.0f;
    static float g_indicatorW       = 0.0f;

    static float g_mainTabHover[kNumMainTabs] = {};
    static float g_subTabHover [kNumMainTabs][kMaxSubtabs] = {};

    static bool  g_checkState [kNumMainTabs][kMaxSubtabs][kMaxContainers][kMaxWidgets] = {};
    static float g_checkAnim  [kNumMainTabs][kMaxSubtabs][kMaxContainers][kMaxWidgets] = {};
    static float g_sliderValue[kNumMainTabs][kMaxSubtabs][kMaxContainers][kMaxWidgets] = {};
    static bool  g_sliderInited     = false;

    static int   g_dragMain      = -1;
    static int   g_dragSubtab    = -1;
    static int   g_dragContainer = -1;
    static int   g_dragWidget    = -1;

    static int   g_popupOwnerMain  = -1;
    static int   g_popupOwnerSub   = -1;
    static int   g_popupOwnerCont  = -1;
    static int   g_popupOwnerWid   = -1;
    static int   g_popupKind       = 0;
    static float g_popupAnim       = 0.0f;
    static float g_popupTarget     = 0.0f;
    static ImVec2 g_popupAnchor    = {};
    // Last-frame snapshot of the popup's screen rect. Widget rendering on
    // the next frame checks this — clicks inside this rect get consumed
    // BEFORE per-widget hit-test so clicking inside an open popup doesn't
    // also toggle / drag the widget visually behind it.
    static bool   g_popupBlockActive = false;
    static ImVec2 g_popupBlockMin    = {};
    static ImVec2 g_popupBlockMax    = {};
    static bool  g_popupCheck1  [kNumMainTabs][kMaxSubtabs][kMaxContainers][kMaxWidgets] = {};
    static bool  g_popupCheck2  [kNumMainTabs][kMaxSubtabs][kMaxContainers][kMaxWidgets] = {};
    static float g_popupSlider1 [kNumMainTabs][kMaxSubtabs][kMaxContainers][kMaxWidgets] = {};
    static float g_popupSlider2 [kNumMainTabs][kMaxSubtabs][kMaxContainers][kMaxWidgets] = {};
    static bool  g_popupFlag    [kNumMainTabs][kMaxSubtabs][kMaxContainers][kMaxWidgets][8] = {};
    static bool  g_popupDefaultsInited = false;

    static bool g_player_flag_mirror[8] = {};

    // Pending HIGH-RISK tooltip request — written by DrawCheckboxRow when
    // the user hovers the red "?" badge, read+cleared at the end of Render()
    // so the tooltip box paints on top of all other widgets/popups.
    struct HighRiskTooltip {
        bool        active = false;
        ImVec2      anchor = {};
        const char* text   = nullptr;
        float       alpha  = 0.0f;
    };
    static HighRiskTooltip g_riskTooltip;

    bool GetPlayerFlag(int idx) {
        if (idx < 0 || idx >= 8) return false;
        return g_player_flag_mirror[idx];
    }
    static int   g_popupDragSlot = -1;

    static int   g_pickerDrag    = 0;

    static unsigned g_widgetColor[kNumMainTabs][kMaxSubtabs][kMaxContainers][kMaxWidgets] = {};
    static bool     g_widgetColorInited = false;

    static float    g_containerScroll      [kNumMainTabs][kMaxSubtabs][kMaxContainers] = {};
    static float    g_containerScrollTarget[kNumMainTabs][kMaxSubtabs][kMaxContainers] = {};

    // Page-level scroll: applied to BOTH columns of the current subtab when
    // their stacked container heights exceed the visible content area. The
    // Entities subtab is the main motivator (10 containers in 2 columns =
    // ~5 per column = up to 2300px tall vs. 580px menu height) but applies
    // to any subtab whose containers overflow.
    static float    g_pageScroll      [kNumMainTabs][kMaxSubtabs] = {};
    static float    g_pageScrollTarget[kNumMainTabs][kMaxSubtabs] = {};

    constexpr int   kMaxConfigs    = 16;
    constexpr int   kConfigNameMax = 32;
    static char     g_configNames[kMaxConfigs][kConfigNameMax] = {};
    static int      g_configCount    = 0;
    static int      g_configSelected = -1;
    static char     g_configInput[kConfigNameMax] = {};
    static int      g_configInputLen = 0;
    static bool     g_configInited   = false;
    static float    g_listboxScroll [kNumMainTabs][kMaxSubtabs][kMaxContainers][kMaxWidgets] = {};
    static float    g_listboxScrollT[kNumMainTabs][kMaxSubtabs][kMaxContainers][kMaxWidgets] = {};
    static float    g_listSelY      [kNumMainTabs][kMaxSubtabs][kMaxContainers][kMaxWidgets] = {};
    static float    g_listHovY      [kNumMainTabs][kMaxSubtabs][kMaxContainers][kMaxWidgets] = {};
    static float    g_listHovA      [kNumMainTabs][kMaxSubtabs][kMaxContainers][kMaxWidgets] = {};
    static float    g_widgetAnim    [kNumMainTabs][kMaxSubtabs][kMaxContainers][kMaxWidgets] = {};
    static DWORD    g_widgetPulseTk [kNumMainTabs][kMaxSubtabs][kMaxContainers][kMaxWidgets] = {};

    static int      g_focusedTI[4] = { -1, -1, -1, -1 };
    static bool     g_keyPrevDown[256] = {};

    // ── Search bar (top-left of menu) ────────────────────────────────
    // Filter-as-you-type: when len > 0, widgets whose label doesn't
    // contain the buffer (case-insensitive substring) are skipped in
    // the per-widget render loop. Containers whose every widget filters
    // out are also skipped so the column layout stays tight.
    static char  g_searchBuf[64] = {};
    static int   g_searchLen     = 0;
    static bool  g_searchFocused = false;
    // Keystroke pulse — set to 1.0 on each text change, decays each
    // frame. Drives a brief glow + border thickness bump on the bar.
    static float g_searchPulse   = 0.0f;
    // FNV-1a hash of g_searchBuf, used to detect "search text changed
    // this frame" so auto-nav-to-first-match only fires on edit, not on
    // every render — otherwise the user can never navigate away from
    // an auto-jumped subtab while search is active.
    static std::uint32_t g_searchTokenHash = 0;
    // Snapshot of (mainTab, subtab) right before search began. Restored
    // when the user clears the search so they don't get stranded on the
    // last auto-jumped page with no obvious way back.
    static int   g_searchSavedMainTab = -1;
    static int   g_searchSavedSubtab  = -1;

    // Case-insensitive substring search. Returns true when needle is
    // empty (so an empty filter shows everything) or when needle is
    // found inside hay. ASCII only — fine for the menu's English labels.
    static bool _str_icontains(const char* hay, const char* needle) {
        if (!needle || !*needle) return true;
        if (!hay)                return false;
        const size_t nl = strlen(needle);
        for (const char* p = hay; *p; ++p) {
            size_t i = 0;
            while (i < nl) {
                const char a = (char)tolower((unsigned char)p[i]);
                const char b = (char)tolower((unsigned char)needle[i]);
                if (!p[i] || a != b) break;
                ++i;
            }
            if (i == nl) return true;
        }
        return false;
    }

    // Group synonyms: user-typed term -> menu term. Lets vague searches
    // like "guns" map to the "Weapons" subtab / container, "loot" find
    // crates+barrels, "node" find ores, etc.
    static bool _str_icontains_with_synonyms(const char* hay, const char* needle) {
        if (_str_icontains(hay, needle)) return true;
        struct Syn { const char* user; const char* maps_to; };
        static constexpr Syn kSyns[] = {
            { "gun",       "weapon" },
            { "guns",      "weapon" },
            { "firearm",   "weapon" },
            { "rifle",     "weapon" },
            { "shooter",   "weapon" },
            { "ammo",      "weapon" },
            { "ammunition","weapon" },
            { "animal",    "animal" },     // matches "NPCs & Animals"
            { "animals",   "animal" },
            { "mob",       "npc" },
            { "mobs",      "npc" },
            { "monster",   "npc" },
            { "enemy",     "npc" },
            { "node",      "ore" },
            { "nodes",     "ore" },
            { "rock",      "ore" },
            { "mineral",   "ore" },
            { "resource",  "collectible" },
            { "loot",      "crate" },
            { "loot",      "barrel" },
            { "container", "crate" },
            { "throwable", "explosive" },
            { "boom",      "explosive" },
            { "explosive", "explosive" },
            { "base",      "deployable" },
            { "deploy",    "deployable" },
            { "structure", "deployable" },
            { "build",     "deployable" },
            { "veh",       "horse" },
            { "vehicle",   "horse" },
            { "mount",     "horse" },
            { "drop",      "dropped" },
            { "ground",    "dropped" },
            { "cheat",     "exploit" },
            { "speed",     "movement" },
            { "fly",       "movement" },
            { "wallhack",  "esp" },
            { "wall",      "esp" },
            { "skeleton",  "esp" },
            { "spread",    "weapon" },
            { "recoil",    "weapon" },
        };
        for (const auto& s : kSyns) {
            const size_t ul = strlen(s.user);
            const size_t nl = strlen(needle);
            if (nl != ul) continue;
            bool eq = true;
            for (size_t i = 0; i < nl; ++i) {
                if (tolower((unsigned char)needle[i]) != (unsigned char)s.user[i]) { eq = false; break; }
            }
            if (eq && _str_icontains(hay, s.maps_to)) return true;
        }
        return false;
    }

    static bool WidgetMatchesSearch(const Widget& w) {
        if (g_searchLen == 0) return true;
        return _str_icontains_with_synonyms(w.label, g_searchBuf);
    }

    // Returns true if the widget should pass the search filter *given*
    // its parent container also matched. Container/subtab-level matches
    // pull all children along so a vague search like "animal" shows
    // every entry under "NPCs & Animals".
    static bool WidgetVisibleViaParent(const Widget& w, const Container& parent) {
        if (g_searchLen == 0)                                 return true;
        if (WidgetMatchesSearch(w))                            return true;
        if (_str_icontains_with_synonyms(parent.title, g_searchBuf)) return true;
        return false;
    }

    static bool ContainerHasAnyMatch(const Container& c) {
        if (g_searchLen == 0) return true;
        if (_str_icontains_with_synonyms(c.title, g_searchBuf)) return true;
        for (int wi = 0; wi < kMaxWidgets; ++wi) {
            const Widget& w = c.widgets[wi];
            if (!w.label) break;
            if (_str_icontains_with_synonyms(w.label, g_searchBuf)) return true;
        }
        return false;
    }

    static bool SubtabHasAnyMatch(const SubtabDef& s) {
        if (g_searchLen == 0) return true;
        for (int ci = 0; ci < kMaxContainers; ++ci) {
            const Container& cont = s.containers[ci];
            if (!cont.title || !*cont.title) break;
            if (ContainerHasAnyMatch(cont)) return true;
        }
        return false;
    }

    static std::uint32_t HashSearchBuf() {
        std::uint32_t h = 2166136261u;
        for (int i = 0; i < g_searchLen; ++i) {
            h ^= (std::uint8_t)g_searchBuf[i];
            h *= 16777619u;
        }
        return h;
    }

    static int      g_confirmAction = 0;

    static std::atomic<bool> g_writes_enabled{ false };

    // Flip to true to lock the writes toggle (no-op click + "Temporarily disabled" tooltip).
    static constexpr bool kWritesToggleLocked = false;

    bool IsWritesEnabled() {
        return g_writes_enabled.load(std::memory_order_acquire);
    }
    void SetWritesEnabled(bool v) {
        g_writes_enabled.store(v, std::memory_order_release);
    }

    static float Lerp(float a, float b, float t) { return a + (b - a) * t; }

    static float EaseOutCubic(float u) {
        const float k = 1.0f - u;
        return 1.0f - k * k * k;
    }

    static float EaseTo(float cur, float target, float speed) {
        float k = ImGui::GetIO().DeltaTime * speed;
        if (k > 1.0f) k = 1.0f;
        return cur + (target - cur) * k;
    }

    static ImVec2 PixelSnap(ImVec2 p) {
        return ImVec2(floorf(p.x + 0.5f), floorf(p.y + 0.5f));
    }

    static ImU32 LerpColorU32(ImU32 a, ImU32 b, float t) {
        if (t <= 0.0f) return a;
        if (t >= 1.0f) return b;
        const int ar =  a        & 0xFF, br =  b        & 0xFF;
        const int ag = (a >>  8) & 0xFF, bg = (b >>  8) & 0xFF;
        const int ab = (a >> 16) & 0xFF, bb = (b >> 16) & 0xFF;
        const int aa = (a >> 24) & 0xFF, ba = (b >> 24) & 0xFF;
        const int rr = (int)(ar + (br - ar) * t);
        const int gg = (int)(ag + (bg - ag) * t);
        const int bbv= (int)(ab + (bb - ab) * t);
        const int aaa= (int)(aa + (ba - aa) * t);
        return IM_COL32(rr, gg, bbv, aaa);
    }

    ShapeRect CurrentShape() {
        if (!g_cursorVisibleCached) {
            return ShapeRect{ 0, 0, 0, 0, 0 };
        }
        return g_shape;
    }

    static ImVec2 MenuCenter() {
        if (g_shape.w >= 100.0f && g_shape.h >= 100.0f) {
            return ImVec2(g_shape.x + g_shape.w * 0.5f,
                          g_shape.y + g_shape.h * 0.5f);
        }
        const ImVec2 d = ImGui::GetIO().DisplaySize;
        return ImVec2(d.x * 0.5f, d.y * 0.5f);
    }

    void Init() {
        ImGuiStyle& style = ImGui::GetStyle();
        style.CircleTessellationMaxError = 0.02f;
        style.AntiAliasedFill            = true;
        style.AntiAliasedLines           = true;

        if (!g_configInited) {
            auto configs = config::list_files();
            g_configCount = 0;
            for (const auto& cfg : configs) {
                if (g_configCount >= kMaxConfigs) break;
                std::snprintf(g_configNames[g_configCount], kConfigNameMax, "%s", cfg.c_str());
                g_configCount++;
            }
            if (g_configCount == 0) {
                std::snprintf(g_configNames[0], kConfigNameMax, "Default");
                g_configCount = 1;
                config::save_by_name("Default");
            }
            g_configSelected = 0;
            config::load_by_name(g_configNames[0]);
            g_configInited   = true;
        }

        g_state = State::Pill;
        g_anim  = 0.0f;
        g_outsideSinceTick = 0;

        if (!g_sliderInited) {
            for (int t = 0; t < kNumMainTabs; ++t) {
                const int subCount = (kTabs[t].numSubtabs == 0) ? 1 : kTabs[t].numSubtabs;
                for (int s = 0; s < subCount; ++s) {
                    const SubtabDef& sub = kTabs[t].subtabs[s];
                    for (int c = 0; c < kMaxContainers; ++c) {
                        const Container& cont = sub.containers[c];
                        if (!cont.title || !*cont.title) break;
                        for (int w = 0; w < kMaxWidgets; ++w) {
                            const Widget& wd = cont.widgets[w];
                            if (!wd.label) break;
                            if (wd.type == WT::Slider) {
                                g_sliderValue[t][s][c][w] = wd.defV;
                            }
                        }
                    }
                }
            }
            g_sliderInited = true;
        }

        if (!g_widgetColorInited) {
            static const unsigned kPalette[] = {
                IM_COL32(92, 200, 232, 255),
                IM_COL32(232, 96, 96, 255),
                IM_COL32(232, 196, 96, 255),
                IM_COL32(96, 232, 140, 255),
                IM_COL32(192, 130, 232, 255),
                IM_COL32(232, 140, 200, 255),
            };
            int idx = 0;
            for (int t = 0; t < kNumMainTabs; ++t) {
                const int subCount = (kTabs[t].numSubtabs == 0) ? 1 : kTabs[t].numSubtabs;
                for (int s = 0; s < subCount; ++s) {
                    const SubtabDef& sub = kTabs[t].subtabs[s];
                    for (int c = 0; c < kMaxContainers; ++c) {
                        const Container& cont = sub.containers[c];
                        if (!cont.title || !*cont.title) break;
                        for (int w = 0; w < kMaxWidgets; ++w) {
                            const Widget& wd = cont.widgets[w];
                            if (!wd.label) break;
                            if (wd.hasColor) {
                                g_widgetColor[t][s][c][w] = kPalette[idx % (sizeof(kPalette) / sizeof(kPalette[0]))];
                                ++idx;
                            }
                        }
                    }
                }
            }
            for (int t = 0; t < kNumMainTabs; ++t) {
                const int subCount = (kTabs[t].numSubtabs == 0) ? 1 : kTabs[t].numSubtabs;
                for (int s = 0; s < subCount; ++s) {
                    const SubtabDef& sub = kTabs[t].subtabs[s];
                    for (int c = 0; c < kMaxContainers; ++c) {
                        const Container& cont = sub.containers[c];
                        if (!cont.title || !*cont.title) break;
                        for (int w = 0; w < kMaxWidgets; ++w) {
                            const Widget& wd = cont.widgets[w];
                            if (!wd.label) break;
                            switch (wd.popupKind) {
                                case PK_Skeleton:
                                    g_popupSlider1[t][s][c][w] = 1.5f; break;
                                case PK_OffscreenArrows:
                                    g_popupSlider1[t][s][c][w] = 250.0f;
                                    g_popupSlider2[t][s][c][w] = 16.0f;
                                    break;
                                case PK_MaxDistance:
                                    g_popupSlider1[t][s][c][w] = 150.0f; break;
                                case PK_Generator:
                                    g_popupSlider1[t][s][c][w] = 150.0f; break;
                                case PK_PostSaturation:
                                case PK_PostContrast:
                                    g_popupSlider1[t][s][c][w] = 1.0f; break;
                                case PK_PostSharpness:
                                    g_popupSlider1[t][s][c][w] = 1.0f; break;
                                case PK_PostBloom:
                                    g_popupSlider1[t][s][c][w] = 50.0f;
                                    g_popupSlider2[t][s][c][w] = 0.5f;
                                    break;
                                case PK_AutoPallet:
                                    g_popupSlider1[t][s][c][w] = 80.0f; break;
                                case PK_SpeedBoost:
                                    g_popupSlider1[t][s][c][w] = 25.0f; break;
                                case PK_ThickBullet:
                                    g_popupSlider1[t][s][c][w] = 1.5f; break;
                                case PK_FovZoom:
                                    g_popupSlider1[t][s][c][w] = 35.0f; break;
                                default: break;
                            }
                        }
                    }
                }
            }
            g_widgetColorInited = true;
            g_popupDefaultsInited = true;
        }
    }

    void Shutdown() {}

    static void UpdateState(const ImVec2& disp) {
        CURSORINFO ci = { sizeof(ci) };
        const bool gotInfo      = ::GetCursorInfo(&ci) != 0;
        const bool cursorVisible = gotInfo && (ci.flags & CURSOR_SHOWING) != 0;
        g_cursorVisibleCached = cursorVisible;

        const ImVec2 mp = ImGui::GetIO().MousePos;

        const float cx = disp.x * 0.5f;
        const bool insideShape =
            mp.x >= g_shape.x && mp.x <= g_shape.x + g_shape.w &&
            mp.y >= g_shape.y && mp.y <= g_shape.y + g_shape.h;

        const DWORD now = ::GetTickCount();

        if (g_state == State::Pill) {
            if (cursorVisible && insideShape) {
                g_state = State::Menu;
                g_outsideSinceTick = 0;
                g_menuOpenTick = now;
            }
        } else {
            if (!cursorVisible) {
                g_state = State::Pill;
                g_outsideSinceTick = 0;
            } else if (insideShape) {
                g_outsideSinceTick = 0;
            } else {
                if (g_outsideSinceTick == 0) {
                    g_outsideSinceTick = now;
                } else if (now - g_outsideSinceTick >= kCollapseDelayMs) {
                    g_state = State::Pill;
                    g_outsideSinceTick = 0;
                }
            }
        }

        const float target = (g_state == State::Menu) ? 1.0f : 0.0f;
        g_anim = EaseTo(g_anim, target, kEaseSpeed);

        const float t  = EaseOutCubic(g_anim);
        const float w  = Lerp(kPillW,      kMenuW,      t);
        const float h  = Lerp(kPillH,      kMenuH,      t);
        const float r  = Lerp(kPillRadius, kMenuRadius, t);

        g_shape.x      = cx - w * 0.5f;
        g_shape.y      = kTopMargin;
        g_shape.w      = w;
        g_shape.h      = h;
        g_shape.radius = r;
    }

    constexpr float kTitleFontSize = 16.0f;
    constexpr float kBodyFontSize  = 14.0f;

    static void DrawPillContent(ImDrawList* dl, const ShapeRect& r, float pillAlpha) {
        const ImU32 ink = IM_COL32(228, 230, 244, (BYTE)(255 * pillAlpha));

        ImFont* font     = Render::Fonts::montserrat14px;
        const char* text = "Gordo";
        const ImVec2 sz  = font
            ? font->CalcTextSizeA(kBodyFontSize, FLT_MAX, 0.0f, text)
            : ImGui::CalcTextSize(text);

        const ImVec2 pos = PixelSnap(ImVec2(
            r.x + (r.w - sz.x) * 0.5f,
            r.y + (r.h - sz.y) * 0.5f - 1.0f));
        if (font) dl->AddText(font, kBodyFontSize, pos, ink, text);
        else      dl->AddText(pos, ink, text);

        constexpr DWORD kCycleMs = 4200;
        constexpr DWORD kSweepMs = 750;
        const DWORD nowTk = ::GetTickCount();
        const DWORD tt    = nowTk % kCycleMs;
        if (tt >= kSweepMs) return;

        const float u     = (float)tt / (float)kSweepMs;
        const float eased = 1.0f - (1.0f - u) * (1.0f - u);
        const float edge  = sinf(u * 3.14159265f);

        const float startX = r.x - r.h * 0.6f;
        const float endX   = r.x + r.w + r.h * 0.6f;
        const float sx     = startX + (endX - startX) * eased;

        dl->PushClipRect(
            ImVec2(r.x + 0.5f,        r.y + 0.5f),
            ImVec2(r.x + r.w - 0.5f,  r.y + r.h - 0.5f),
            true);

        const int   bands     = 11;
        const float bandTotal = 14.0f;
        const float tilt      = r.h * 0.55f;

        for (int i = 0; i < bands; ++i) {
            const float f      = (float)i / (float)(bands - 1);
            const float xBase  = sx - bandTotal * 0.5f + bandTotal * f;
            const float fa     = 1.0f - fabsf(f - 0.5f) * 2.0f;
            const float aFloat = fa * fa * fa * edge * 110.0f * pillAlpha;
            if (aFloat < 0.5f) continue;
            dl->AddLine(
                ImVec2(xBase + tilt * 0.5f, r.y),
                ImVec2(xBase - tilt * 0.5f, r.y + r.h),
                IM_COL32(255, 255, 255, (BYTE)aFloat),
                1.2f);
        }

        dl->PopClipRect();
    }

    static bool DrawSliderRow(ImDrawList* dl, ImFont* font,
                               int mt, int st, int ci, int wi,
                               const ImVec2& rowMin, const ImVec2& rowMax,
                               const Widget& def, float& value,
                               float alpha, bool allowClick) {
        const ImVec2 mouse = ImGui::GetIO().MousePos;
        const float rowH   = rowMax.y - rowMin.y;
        const float rowW   = rowMax.x - rowMin.x;
        const ImU32 ink    = IM_COL32(235, 238, 245, (BYTE)(255 * alpha));
        const ImU32 inkDim = IM_COL32(170, 175, 188, (BYTE)(220 * alpha));

        if (font) {
            const ImVec2 lsz = font->CalcTextSizeA(kBodyFontSize, FLT_MAX, 0.0f, def.label);
            const ImVec2 lpos = PixelSnap(ImVec2(rowMin.x + 4.0f,
                                                   rowMin.y + 2.0f));
            dl->AddText(font, kBodyFontSize, lpos, ink, def.label);

            char vbuf[32];
            if (def.maxV - def.minV <= 4.0f) {
                std::snprintf(vbuf, sizeof(vbuf), "%.2f%s", value, def.suffix);
            } else {
                std::snprintf(vbuf, sizeof(vbuf), "%.0f%s", value, def.suffix);
            }
            const ImVec2 vsz = font->CalcTextSizeA(kBodyFontSize, FLT_MAX, 0.0f, vbuf);
            const ImVec2 vpos = PixelSnap(ImVec2(rowMax.x - 4.0f - vsz.x,
                                                   rowMin.y + 2.0f));
            dl->AddText(font, kBodyFontSize, vpos, inkDim, vbuf);
            (void)lsz;
        }

        const float trackH = 3.0f;
        const float trackX = rowMin.x + 4.0f;
        const float trackW = rowW - 8.0f;
        const float trackY = rowMax.y - 6.0f - trackH;

        const ImVec2 hitMin{ rowMin.x, rowMin.y };
        const ImVec2 hitMax{ rowMax.x, rowMax.y };
        const bool hovered = mouse.x >= hitMin.x && mouse.x <= hitMax.x &&
                              mouse.y >= hitMin.y && mouse.y <= hitMax.y;

        const bool isDragging = (g_dragMain == mt && g_dragSubtab == st &&
                                  g_dragContainer == ci && g_dragWidget == wi);
        bool consumed = false;
        if (allowClick && hovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
            g_dragMain = mt; g_dragSubtab = st;
            g_dragContainer = ci; g_dragWidget = wi;
            consumed = true;
        }
        if (isDragging) {
            if (ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
                const float t = (mouse.x - trackX) / trackW;
                const float tc = (t < 0.0f) ? 0.0f : (t > 1.0f) ? 1.0f : t;
                value = def.minV + tc * (def.maxV - def.minV);
                consumed = true;
            } else {
                g_dragMain = g_dragSubtab = g_dragContainer = g_dragWidget = -1;
            }
        }
        if (value < def.minV) value = def.minV;
        if (value > def.maxV) value = def.maxV;
        const float t     = (value - def.minV) / (def.maxV - def.minV);
        const float fillW = trackW * t;

        dl->AddRectFilled(
            ImVec2(trackX,          trackY),
            ImVec2(trackX + trackW, trackY + trackH),
            IM_COL32(28, 28, 36, (BYTE)(255 * alpha)),
            trackH * 0.5f);
        if (fillW > 1.0f) {
            dl->AddRectFilled(
                ImVec2(trackX,         trackY),
                ImVec2(trackX + fillW, trackY + trackH),
                IM_COL32(72, 151, 255, (BYTE)(255 * alpha)),
                trackH * 0.5f);
        }
        return consumed;
    }

    static void DrawGearIcon(ImDrawList* dl, ImVec2 c, float r,
                              ImU32 col, float rotation = 0.0f) {
        ImFont* fa = Render::Fonts::fontAwesome14px;
        if (!fa) return;
        static constexpr ImWchar kGearCodepoint = 0xf013;

        const float renderSize = r * 2.0f;
        ImFontBaked* baked = fa->GetFontBaked(renderSize);
        if (!baked) return;
        const ImFontGlyph* g = baked->FindGlyph(kGearCodepoint);
        if (!g) return;

        const float qX0 = g->X0;
        const float qY0 = g->Y0;
        const float qX1 = g->X1;
        const float qY1 = g->Y1;
        const float midX = (qX0 + qX1) * 0.5f;
        const float midY = (qY0 + qY1) * 0.5f;

        const ImVec2 cornersLocal[4] = {
            ImVec2(qX0 - midX, qY0 - midY),
            ImVec2(qX1 - midX, qY0 - midY),
            ImVec2(qX1 - midX, qY1 - midY),
            ImVec2(qX0 - midX, qY1 - midY),
        };

        const float cs = cosf(rotation);
        const float sn = sinf(rotation);

        const float pivotX = (rotation == 0.0f) ? floorf(c.x + 0.5f) : c.x;
        const float pivotY = (rotation == 0.0f) ? floorf(c.y + 0.5f) : c.y;

        ImVec2 p[4];
        for (int i = 0; i < 4; ++i) {
            p[i] = ImVec2(
                pivotX + cornersLocal[i].x * cs - cornersLocal[i].y * sn,
                pivotY + cornersLocal[i].x * sn + cornersLocal[i].y * cs);
        }

        const ImVec2 uv[4] = {
            ImVec2(g->U0, g->V0),
            ImVec2(g->U1, g->V0),
            ImVec2(g->U1, g->V1),
            ImVec2(g->U0, g->V1),
        };

        dl->AddImageQuad(ImGui::GetIO().Fonts->TexRef,
                          p[0], p[1], p[2], p[3],
                          uv[0], uv[1], uv[2], uv[3],
                          col);
    }

    static float GearRotationFor(int mt, int st, int ci, int wi) {
        const bool sameOwner = (g_popupOwnerMain == mt && g_popupOwnerSub == st &&
                                 g_popupOwnerCont == ci && g_popupOwnerWid == wi);
        return sameOwner ? (g_popupAnim * 0.55f) : 0.0f;
    }

    static void OpenPopup(int mt, int st, int ci, int wi, int kind, ImVec2 anchor);
    static void RequestClosePopup();

    static bool DrawColorSwatch(ImDrawList* dl,
                                 ImVec2 topLeft, float sz,
                                 unsigned color, float alpha,
                                 bool allowClick, bool hovered,
                                 int mt, int st, int ci, int wi) {
        const BYTE r =  color        & 0xFF;
        const BYTE g = (color >>  8) & 0xFF;
        const BYTE b = (color >> 16) & 0xFF;
        const ImU32 fill   = IM_COL32(r, g, b, (BYTE)(255 * alpha));
        const ImU32 border = IM_COL32(255, 255, 255, (BYTE)(60 * alpha));
        dl->AddRectFilled(topLeft, ImVec2(topLeft.x + sz, topLeft.y + sz),
                           fill, 3.0f);
        dl->AddRect(topLeft, ImVec2(topLeft.x + sz, topLeft.y + sz),
                    border, 3.0f, 0, 1.0f);

        bool consumed = false;
        if (allowClick && hovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
            const bool isMineOpen = (g_popupOwnerMain == mt && g_popupOwnerSub == st &&
                                      g_popupOwnerCont == ci && g_popupOwnerWid == wi &&
                                      g_popupKind == PK_ColorPicker &&
                                      g_popupTarget > 0.5f);
            if (isMineOpen) {
                RequestClosePopup();
            } else {
                OpenPopup(mt, st, ci, wi, PK_ColorPicker,
                          ImVec2(topLeft.x, topLeft.y + sz + 2.0f));
            }
            consumed = true;
        }
        return consumed;
    }

    static void OpenPopup(int mt, int st, int ci, int wi, int kind,
                           ImVec2 anchor) {
        g_popupOwnerMain = mt; g_popupOwnerSub = st;
        g_popupOwnerCont = ci; g_popupOwnerWid = wi;
        g_popupKind  = kind;
        g_popupAnchor = anchor;
        g_popupTarget = 1.0f;
    }
    static void RequestClosePopup() {
        g_popupTarget = 0.0f;
    }
    static bool IsPopupOwnedBy(int mt, int st, int ci, int wi) {
        return g_popupOwnerMain == mt && g_popupOwnerSub == st &&
               g_popupOwnerCont == ci && g_popupOwnerWid == wi &&
               g_popupTarget > 0.5f;
    }

    static bool DrawWritesToggleBig(ImDrawList* dl, ImFont* font,
                                     const ImVec2& rowMin, const ImVec2& rowMax,
                                     const Widget& def,
                                     int mt, int st, int ci, int wi,
                                     bool& state, float& anim,
                                     float alpha, bool allowClick) {
        const ImVec2 mouse = ImGui::GetIO().MousePos;
        const float  rowH  = rowMax.y - rowMin.y;
        bool consumed = false;

        const bool hoverRow =
            mouse.x >= rowMin.x && mouse.x <= rowMax.x &&
            mouse.y >= rowMin.y && mouse.y <= rowMax.y;

        float& ha = g_widgetAnim[mt][st][ci][wi];
        ha = EaseTo(ha, hoverRow ? 1.0f : 0.0f, 16.0f);

        if (allowClick && hoverRow && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
            if (!kWritesToggleLocked) state = !state;
            consumed = true;
        }
        anim = EaseTo(anim, state ? 1.0f : 0.0f, 20.0f);

        const float t = anim;
        const float h = ha;

        const ImU32 fillOff  = IM_COL32(24, 30, 46, (BYTE)((220 + 20 * h) * alpha));
        const ImU32 fillOn   = IM_COL32(34, 70, 130, (BYTE)((230 + 20 * h) * alpha));
        const ImU32 fillNow  = LerpColorU32(fillOff, fillOn, t);
        dl->AddRectFilled(rowMin, rowMax, fillNow, 6.0f);

        dl->PushClipRect(
            ImVec2(rowMin.x + 1.0f, rowMin.y + 1.0f),
            ImVec2(rowMax.x - 1.0f, rowMax.y - 1.0f),
            true);
        const ImU32 sheenT = IM_COL32(255, 255, 255, (BYTE)((20 + 18 * h) * alpha));
        const ImU32 sheen0 = IM_COL32(255, 255, 255, 0);
        dl->AddRectFilledMultiColor(
            ImVec2(rowMin.x + 1.0f, rowMin.y + 1.0f),
            ImVec2(rowMax.x - 1.0f, rowMin.y + 8.0f),
            sheenT, sheenT, sheen0, sheen0);
        dl->PopClipRect();

        const ImU32 brdOff = IM_COL32(72, 151, 255, (BYTE)(( 80 + 50 * h) * alpha));
        const ImU32 brdOn  = IM_COL32(72, 151, 255, (BYTE)(220 * alpha));
        const ImU32 brd    = LerpColorU32(brdOff, brdOn, t);
        dl->AddRect(rowMin, rowMax, brd, 6.0f, 0, 1.0f + 0.6f * t);

        if (t > 0.02f) {
            const float fullW = (rowMax.x - rowMin.x) - 16.0f;
            const float bw    = fullW * t;
            const float bx    = (rowMin.x + rowMax.x) * 0.5f;
            const float by    = rowMax.y - 2.0f;
            dl->AddRectFilled(
                ImVec2(bx - bw * 0.5f, by),
                ImVec2(bx + bw * 0.5f, by + 1.0f),
                IM_COL32(72, 151, 255, (BYTE)(220 * t * alpha)));
        }

        const float boxSz = 18.0f;
        const float boxX  = rowMin.x + 12.0f;
        const float boxY  = rowMin.y + (rowH - boxSz) * 0.5f;
        const int offR = 18, offG = 18, offB = 26;
        const int onR  = 96, onG  = 170, onB = 255;
        const int fR = (int)(offR + (onR - offR) * t);
        const int fG = (int)(offG + (onG - offG) * t);
        const int fB = (int)(offB + (onB - offB) * t);
        dl->AddRectFilled(ImVec2(boxX, boxY),
                          ImVec2(boxX + boxSz, boxY + boxSz),
                          IM_COL32(fR, fG, fB, (BYTE)(255 * alpha)),
                          3.0f);
        dl->AddRect(ImVec2(boxX, boxY),
                    ImVec2(boxX + boxSz, boxY + boxSz),
                    IM_COL32(255, 255, 255, (BYTE)((38 + 95 * t) * alpha)),
                    3.0f, 0, 1.0f);

        if (t > 0.05f) {
            const ImU32 chk = IM_COL32(255, 255, 255, (BYTE)(255 * t * alpha));
            const ImVec2 a{ boxX + boxSz * 0.22f, boxY + boxSz * 0.52f };
            const ImVec2 b{ boxX + boxSz * 0.43f, boxY + boxSz * 0.72f };
            const ImVec2 c{ boxX + boxSz * 0.80f, boxY + boxSz * 0.30f };
            dl->AddLine(a, b, chk, 1.8f);
            dl->AddLine(b, c, chk, 1.8f);
        }

        if (font) {
            const float labelX = boxX + boxSz + 10.0f;
            const float padR   = 10.0f;

            dl->PushClipRect(
                ImVec2(rowMin.x + 4.0f, rowMin.y + 2.0f),
                ImVec2(rowMax.x - 4.0f, rowMax.y - 2.0f),
                true);

            const ImU32 ink = LerpColorU32(
                IM_COL32(220, 220, 232, (BYTE)(255 * alpha)),
                IM_COL32(244, 248, 255, (BYTE)(255 * alpha)),
                t);
            const ImVec2 labelSz = font->CalcTextSizeA(kBodyFontSize, FLT_MAX, 0.0f, def.label);

            const char* sub = state
                ? "Memory writes enabled."
                : "Click to enable memory writes.";
            const ImU32 subC = LerpColorU32(
                IM_COL32(140, 140, 158, (BYTE)(200 * alpha)),
                IM_COL32(180, 210, 255, (BYTE)(200 * alpha)),
                t);
            const ImVec2 subSz = font->CalcTextSizeA(kBodyFontSize, FLT_MAX, 0.0f, sub);

            const float gap    = 3.0f;
            const float stackH = labelSz.y + gap + subSz.y;
            const float topY   = rowMin.y + (rowH - stackH) * 0.5f - 1.0f;
            const float maxX   = rowMax.x - padR;

            dl->AddText(font, kBodyFontSize,
                         PixelSnap(ImVec2(labelX, topY)),
                         ink, def.label);
            dl->AddText(font, kBodyFontSize,
                         PixelSnap(ImVec2(labelX, topY + labelSz.y + gap)),
                         subC, sub);
            (void)maxX;
            dl->PopClipRect();
        }

        if (kWritesToggleLocked && font) {
            const float qR   = 8.0f;
            const float qCX  = rowMax.x - 14.0f;
            const float qCY  = rowMin.y + rowH * 0.5f;
            const ImU32 ring = IM_COL32(190, 190, 210, (BYTE)(180 * alpha));
            const ImU32 fill = IM_COL32( 18,  22,  34, (BYTE)(220 * alpha));
            dl->AddCircleFilled(ImVec2(qCX, qCY), qR,       fill, 16);
            dl->AddCircle      (ImVec2(qCX, qCY), qR,       ring, 16, 1.2f);
            const ImVec2 qSz = font->CalcTextSizeA(kBodyFontSize, FLT_MAX, 0.0f, "?");
            dl->AddText(font, kBodyFontSize,
                         PixelSnap(ImVec2(qCX - qSz.x * 0.5f, qCY - qSz.y * 0.5f - 1.0f)),
                         IM_COL32(220, 220, 235, (BYTE)(230 * alpha)),
                         "?");

            const bool hoverQ =
                (mouse.x >= qCX - qR - 2.0f) && (mouse.x <= qCX + qR + 2.0f) &&
                (mouse.y >= qCY - qR - 2.0f) && (mouse.y <= qCY + qR + 2.0f);
            if (hoverQ) {
                const char* tip = "Temporarily disabled";
                const ImVec2 tSz = font->CalcTextSizeA(kBodyFontSize, FLT_MAX, 0.0f, tip);
                const float pad  = 6.0f;
                const float tipW = tSz.x + pad * 2.0f;
                const float tipH = tSz.y + pad * 2.0f;
                // Anchor below the "?" so it doesn't cover it.
                float tipX = qCX + qR + 4.0f;
                float tipY = qCY - tipH * 0.5f;
                // Keep on-row vertically if there's room above; otherwise
                // fall back to mouse-anchored placement so it never spills
                // out of the menu when the row is near the screen edge.
                const ImU32 bg  = IM_COL32(14, 16, 24, 230);
                const ImU32 brd = IM_COL32(72, 151, 255, 180);
                const ImU32 ink = IM_COL32(232, 236, 250, 255);
                dl->AddRectFilled(ImVec2(tipX, tipY),
                                   ImVec2(tipX + tipW, tipY + tipH),
                                   bg, 4.0f);
                dl->AddRect      (ImVec2(tipX, tipY),
                                   ImVec2(tipX + tipW, tipY + tipH),
                                   brd, 4.0f, 0, 1.0f);
                dl->AddText(font, kBodyFontSize,
                             PixelSnap(ImVec2(tipX + pad, tipY + pad)),
                             ink, tip);
            }
        }

        return consumed;
    }

    static bool DrawCheckboxRow(ImDrawList* dl, ImFont* font,
                                 const ImVec2& rowMin, const ImVec2& rowMax,
                                 const Widget& def,
                                 int mt, int st, int ci, int wi,
                                 bool& state, float& anim,
                                 unsigned& swatchColor,
                                 float alpha, bool allowClick) {
        const ImVec2 mouse = ImGui::GetIO().MousePos;
        const float  rowH  = rowMax.y - rowMin.y;
        bool consumed = false;

        const float boxSz   = 14.0f;
        const float gearR   = 7.0f;
        const float swSz    = 12.0f;

        const float boxX        = rowMin.x + 4.0f;
        const float boxY        = rowMin.y + (rowH - boxSz) * 0.5f;
        const float labelStartX = boxX + boxSz + 8.0f;

        // Right side: gear (if any) then swatch (if any), walking left.
        float cursorX = rowMax.x - 4.0f;
        float gearCX = 0.0f, gearCY = rowMin.y + rowH * 0.5f;
        ImVec2 gearHitMin{}, gearHitMax{};
        if (def.hasGear) {
            gearCX = cursorX - gearR;
            gearHitMin = ImVec2(gearCX - gearR - 2.0f, gearCY - gearR - 2.0f);
            gearHitMax = ImVec2(gearCX + gearR + 2.0f, gearCY + gearR + 2.0f);
            cursorX = gearCX - gearR - 6.0f;
        }
        float swX = 0.0f, swY = rowMin.y + (rowH - swSz) * 0.5f;
        ImVec2 swHitMin{}, swHitMax{};
        if (def.hasColor) {
            swX = cursorX - swSz;
            swHitMin = ImVec2(swX - 2.0f,        swY - 2.0f);
            swHitMax = ImVec2(swX + swSz + 2.0f, swY + swSz + 2.0f);
            cursorX = swX - 6.0f;
        }

        // HIGH-RISK "?" badge hit-zone — computed early so it can be
        // subtracted from the label toggle zone below.
        // Sized to roughly fit a 14px "?" glyph with a few px of slack.
        const float riskR = 7.0f;
        float       riskCX = 0.0f, riskCY = rowMin.y + rowH * 0.5f;
        ImVec2      riskHitMin{}, riskHitMax{};
        if (def.highRisk && font) {
            const ImVec2 lsz = font->CalcTextSizeA(kBodyFontSize, FLT_MAX, 0.0f, def.label);
            riskCX = labelStartX + lsz.x + 8.0f + riskR;
            riskHitMin = ImVec2(riskCX - riskR - 2.0f, riskCY - riskR - 2.0f);
            riskHitMax = ImVec2(riskCX + riskR + 2.0f, riskCY + riskR + 2.0f);
        }

        // Hover bookkeeping for each interactive sub-zone.
        const bool hoverGear  = def.hasGear  &&
            mouse.x >= gearHitMin.x && mouse.x <= gearHitMax.x &&
            mouse.y >= gearHitMin.y && mouse.y <= gearHitMax.y;
        const bool hoverSw    = def.hasColor &&
            mouse.x >= swHitMin.x && mouse.x <= swHitMax.x &&
            mouse.y >= swHitMin.y && mouse.y <= swHitMax.y;
        const bool hoverRisk  = def.highRisk &&
            mouse.x >= riskHitMin.x && mouse.x <= riskHitMax.x &&
            mouse.y >= riskHitMin.y && mouse.y <= riskHitMax.y;
        // Label/checkbox hit zone = whole row MINUS the right-side controls
        // AND the risk badge (so clicking ? doesn't toggle the checkbox).
        const bool hoverLabel =
            mouse.x >= rowMin.x && mouse.x <= rowMax.x &&
            mouse.y >= rowMin.y && mouse.y <= rowMax.y &&
            !hoverGear && !hoverSw && !hoverRisk;

        // Gear click → open or close this row's popup.
        if (def.hasGear && allowClick && hoverGear &&
            ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
            if (IsPopupOwnedBy(mt, st, ci, wi)) {
                RequestClosePopup();
            } else {
                // Anchor popup at the right edge of the gear icon, vertically
                // aligned to the row centre.
                OpenPopup(mt, st, ci, wi, def.popupKind,
                          ImVec2(gearCX + gearR + 4.0f, gearCY));
            }
            consumed = true;
        }

        if (def.hasColor) {
            const bool sc = DrawColorSwatch(dl, ImVec2(swX, swY), swSz,
                                              swatchColor, alpha,
                                              allowClick && !consumed, hoverSw,
                                              mt, st, ci, wi);
            if (sc) consumed = true;
        }

        if (def.hasGear) {
            const float rot  = GearRotationFor(mt, st, ci, wi);
            const ImU32 gcol = hoverGear || IsPopupOwnedBy(mt, st, ci, wi)
                                ? IM_COL32(228, 228, 240, (BYTE)(255 * alpha))
                                : IM_COL32(150, 150, 165, (BYTE)(255 * alpha));
            DrawGearIcon(dl, ImVec2(gearCX, gearCY), gearR, gcol, rot);
        }

        if (allowClick && hoverLabel && !consumed &&
            ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
            state = !state;
            consumed = true;
        }
        anim = EaseTo(anim, state ? 1.0f : 0.0f, 20.0f);

        const ImU32 ink = IM_COL32(220, 220, 232, (BYTE)(255 * alpha));
        if (font) {
            const ImVec2 sz  = font->CalcTextSizeA(kBodyFontSize, FLT_MAX, 0.0f, def.label);
            const ImVec2 pos = PixelSnap(ImVec2(labelStartX,
                                                  rowMin.y + (rowH - sz.y) * 0.5f - 1.0f));
            dl->AddText(font, kBodyFontSize, pos, ink, def.label);
        }

        // ── HIGH-RISK "?" badge ──
        // Red "?" glyph with a subtle pill behind it. Brightens on hover.
        // Hover queues the tooltip to be drawn last in Render() so it
        // paints above other widgets and popups.
        if (def.highRisk && font) {
            const BYTE   ringA = (BYTE)((hoverRisk ? 255 : 180) * alpha);
            const ImU32  red   = IM_COL32(235, 64, 64, (BYTE)(255 * alpha));
            const ImU32  redB  = IM_COL32(235, 64, 64, ringA);
            const ImU32  bgCol = IM_COL32(40, 14, 14, (BYTE)((hoverRisk ? 180 : 120) * alpha));

            dl->AddCircleFilled(ImVec2(riskCX, riskCY), riskR, bgCol, 18);
            dl->AddCircle      (ImVec2(riskCX, riskCY), riskR, redB,  18, 1.0f);

            // Centered "?" glyph
            const ImVec2 qsz = font->CalcTextSizeA(kBodyFontSize, FLT_MAX, 0.0f, "?");
            const ImVec2 qp  = PixelSnap(ImVec2(riskCX - qsz.x * 0.5f,
                                                riskCY - qsz.y * 0.5f - 1.0f));
            dl->AddText(font, kBodyFontSize, qp, red, "?");

            if (hoverRisk) {
                g_riskTooltip.active = true;
                g_riskTooltip.anchor = ImVec2(riskCX + riskR + 6.0f, riskCY);
                g_riskTooltip.text   = "This feature is flagged as HIGH RISK";
                g_riskTooltip.alpha  = alpha;
            }
        }

        const float t = anim;
        const int offR = 22, offG = 22, offB = 30;
        const int onR  = 72, onG  = 151, onB = 255;
        const int fR = (int)(offR + (onR - offR) * t);
        const int fG = (int)(offG + (onG - offG) * t);
        const int fB = (int)(offB + (onB - offB) * t);
        dl->AddRectFilled(ImVec2(boxX, boxY),
                          ImVec2(boxX + boxSz, boxY + boxSz),
                          IM_COL32(fR, fG, fB, (BYTE)(255 * alpha)),
                          3.0f);
        const BYTE borderA = (BYTE)((38 + 60 * t) * alpha);
        dl->AddRect(ImVec2(boxX, boxY),
                    ImVec2(boxX + boxSz, boxY + boxSz),
                    IM_COL32(255, 255, 255, borderA),
                    3.0f, 0, 1.0f);

        return consumed;
    }

    // g_keyIndex now stores Win32 VK_* directly (was: index into a
    // preset list). 0 = "None / no binding". This lets the user bind
    // any keyboard or mouse button by clicking the pill and pressing
    // the key — no more preset cycling.
    static int g_keyIndex[kNumMainTabs][kMaxSubtabs][kMaxContainers][kMaxWidgets] = {};

    // Single global "the user is currently picking a key for this
    // widget" state. Only one keybind can be in capture mode at a
    // time — clicking another pill cancels the previous capture.
    static int g_captureMt = -1, g_captureSt = -1, g_captureCi = -1, g_captureWi = -1;

    // Human-readable name for a VK_* code. Mouse buttons get explicit
    // names because GetKeyNameTextA doesn't know about them. Falls back
    // to GetKeyNameTextA for the keyboard side, then "VK_XX" hex if
    // even that fails. Returned pointer is a thread_local static buffer.
    static const char* VkToName(int vk) {
        thread_local char buf[64];
        if (vk == 0)             { return "None"; }
        if (vk == VK_LBUTTON)    { return "Mouse1"; }
        if (vk == VK_RBUTTON)    { return "Mouse2"; }
        if (vk == VK_MBUTTON)    { return "Mouse3"; }
        if (vk == VK_XBUTTON1)   { return "Mouse4"; }
        if (vk == VK_XBUTTON2)   { return "Mouse5"; }

        UINT scan = MapVirtualKeyA((UINT)vk, MAPVK_VK_TO_VSC);
        // Extended-key flag for the keys whose scancodes collide
        // with the keypad versions.
        switch (vk) {
            case VK_LEFT: case VK_UP: case VK_RIGHT: case VK_DOWN:
            case VK_PRIOR: case VK_NEXT: case VK_END: case VK_HOME:
            case VK_INSERT: case VK_DELETE: case VK_DIVIDE:
            case VK_NUMLOCK: case VK_RCONTROL: case VK_RMENU:
                scan |= 0x100;
                break;
        }
        LONG lparam = (LONG)(scan << 16);
        if (GetKeyNameTextA(lparam, buf, sizeof(buf)) > 0) return buf;

        std::snprintf(buf, sizeof(buf), "VK_%02X", vk);
        return buf;
    }

    // Walk pressed-state for every plausible VK in a single sweep.
    // Skips left mouse so the user can click the pill without
    // immediately binding LMB. Returns 0 if nothing is held.
    static int PollAnyKey() {
        for (int vk = 0x01; vk <= 0xFE; ++vk) {
            if (vk == VK_LBUTTON) continue; // reserved for "click pill"
            if (GetAsyncKeyState(vk) & 0x8000) return vk;
        }
        return 0;
    }

    int GetKeybindVk(const char* widget_label) {
        if (!widget_label) return 0;
        for (int t = 0; t < kNumMainTabs; ++t) {
            const int subCount = (kTabs[t].numSubtabs == 0) ? 1 : kTabs[t].numSubtabs;
            for (int s = 0; s < subCount; ++s) {
                const SubtabDef& sub = kTabs[t].subtabs[s];
                for (int c = 0; c < kMaxContainers; ++c) {
                    const Container& cont = sub.containers[c];
                    if (!cont.title || !*cont.title) break;
                    for (int w = 0; w < kMaxWidgets; ++w) {
                        const Widget& wd = cont.widgets[w];
                        if (!wd.label) break;
                        if (wd.type != WT::Keybind) continue;
                        if (std::string_view(wd.label) != widget_label) continue;
                        // g_keyIndex IS the VK now — no preset lookup.
                        return g_keyIndex[t][s][c][w];
                    }
                }
            }
        }
        return 0;
    }

    static bool DrawKeybindRow(ImDrawList* dl, ImFont* font,
                                const ImVec2& rowMin, const ImVec2& rowMax,
                                const Widget& def,
                                int mt, int st, int ci, int wi,
                                float alpha, bool allowClick) {
        const ImVec2 mouse = ImGui::GetIO().MousePos;
        const float  rowH  = rowMax.y - rowMin.y;
        bool consumed = false;

        const float pillW = 64.0f;
        const float pillH = 16.0f;
        float pillX = rowMax.x - 4.0f - pillW;
        const float pillY = rowMin.y + (rowH - pillH) * 0.5f;

        const float gearR = 7.0f;
        float gearCX = 0.0f, gearCY = rowMin.y + rowH * 0.5f;
        ImVec2 gearHitMin{}, gearHitMax{};
        float cursorX = pillX - 8.0f;
        if (def.hasGear) {
            gearCX = cursorX - gearR;
            gearHitMin = ImVec2(gearCX - gearR - 2.0f, gearCY - gearR - 2.0f);
            gearHitMax = ImVec2(gearCX + gearR + 2.0f, gearCY + gearR + 2.0f);
            cursorX = gearCX - gearR - 6.0f;
        }
        const float swSz = 12.0f;
        float swX = 0.0f, swY = rowMin.y + (rowH - swSz) * 0.5f;
        ImVec2 swHitMin{}, swHitMax{};
        if (def.hasColor) {
            swX = cursorX - swSz;
            swHitMin = ImVec2(swX - 2.0f,        swY - 2.0f);
            swHitMax = ImVec2(swX + swSz + 2.0f, swY + swSz + 2.0f);
        }

        const bool hoverGear = def.hasGear &&
            mouse.x >= gearHitMin.x && mouse.x <= gearHitMax.x &&
            mouse.y >= gearHitMin.y && mouse.y <= gearHitMax.y;
        const bool hoverPill =
            mouse.x >= pillX && mouse.x <= pillX + pillW &&
            mouse.y >= pillY && mouse.y <= pillY + pillH;
        const bool hoverSw   = def.hasColor &&
            mouse.x >= swHitMin.x && mouse.x <= swHitMax.x &&
            mouse.y >= swHitMin.y && mouse.y <= swHitMax.y;

        if (def.hasGear && allowClick && hoverGear &&
            ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
            if (IsPopupOwnedBy(mt, st, ci, wi)) RequestClosePopup();
            else OpenPopup(mt, st, ci, wi, def.popupKind,
                            ImVec2(gearCX + gearR + 4.0f, gearCY));
            consumed = true;
        }
        int& vk = g_keyIndex[mt][st][ci][wi];

        const bool capturing =
            (g_captureMt == mt && g_captureSt == st &&
             g_captureCi == ci && g_captureWi == wi);

        if (allowClick && hoverPill && !consumed &&
            ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
            if (capturing) {
                // Click on the same pill while capturing = cancel,
                // leave the existing binding alone.
                g_captureMt = g_captureSt = g_captureCi = g_captureWi = -1;
            } else {
                g_captureMt = mt; g_captureSt = st;
                g_captureCi = ci; g_captureWi = wi;
            }
            consumed = true;
        }

        if (capturing) {
            // While capturing, poll every key once per frame and accept
            // the first one held. ESC clears the binding (sets None).
            // Right click on the pill also cancels without changing.
            if (allowClick && hoverPill &&
                ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
                g_captureMt = g_captureSt = g_captureCi = g_captureWi = -1;
                consumed = true;
            } else {
                const int pressed = PollAnyKey();
                if (pressed == VK_ESCAPE) {
                    vk = 0;
                    g_captureMt = g_captureSt = g_captureCi = g_captureWi = -1;
                } else if (pressed != 0) {
                    vk = pressed;
                    g_captureMt = g_captureSt = g_captureCi = g_captureWi = -1;
                }
            }
        }
        if (def.hasColor) {
            const bool sc = DrawColorSwatch(dl, ImVec2(swX, swY), swSz,
                                              g_widgetColor[mt][st][ci][wi], alpha,
                                              allowClick && !consumed, hoverSw,
                                              mt, st, ci, wi);
            if (sc) consumed = true;
        }
        if (def.hasGear) {
            const float rot  = GearRotationFor(mt, st, ci, wi);
            const ImU32 gcol = hoverGear || IsPopupOwnedBy(mt, st, ci, wi)
                                ? IM_COL32(228, 228, 240, (BYTE)(255 * alpha))
                                : IM_COL32(150, 150, 165, (BYTE)(255 * alpha));
            DrawGearIcon(dl, ImVec2(gearCX, gearCY), gearR, gcol, rot);
        }

        const ImU32 ink = IM_COL32(220, 220, 232, (BYTE)(255 * alpha));
        if (font) {
            const ImVec2 sz  = font->CalcTextSizeA(kBodyFontSize, FLT_MAX, 0.0f, def.label);
            const ImVec2 pos = PixelSnap(ImVec2(rowMin.x + 8.0f,
                                                  rowMin.y + (rowH - sz.y) * 0.5f - 1.0f));
            dl->AddText(font, kBodyFontSize, pos, ink, def.label);
        }

        const ImU32 pillFill = hoverPill
            ? IM_COL32(36, 36, 46, (BYTE)(255 * alpha))
            : IM_COL32(28, 28, 36, (BYTE)(255 * alpha));
        dl->AddRectFilled(ImVec2(pillX, pillY),
                          ImVec2(pillX + pillW, pillY + pillH),
                          pillFill, 4.0f);
        dl->AddRect(ImVec2(pillX, pillY),
                    ImVec2(pillX + pillW, pillY + pillH),
                    IM_COL32(255, 255, 255, (BYTE)(40 * alpha)),
                    4.0f, 0, 1.0f);

        const char* keyText = capturing ? "press key…" : VkToName(vk);
        if (font) {
            const ImVec2 ksz = font->CalcTextSizeA(kBodyFontSize, FLT_MAX, 0.0f, keyText);
            const ImVec2 kpos = PixelSnap(ImVec2(
                pillX + (pillW - ksz.x) * 0.5f,
                pillY + (pillH - ksz.y) * 0.5f - 1.0f));
            // Capturing-state hint: render the prompt text dimmer so
            // the user can see the pill is in input-listen mode.
            const ImU32 textCol = capturing
                ? IM_COL32(180, 200, 255, (BYTE)(255 * alpha))
                : IM_COL32(228, 228, 240, (BYTE)(255 * alpha));
            dl->AddText(font, kBodyFontSize, kpos, textCol, keyText);
        }
        return consumed;
    }

    static const char* g_openDropdownBind = nullptr;

    static bool DrawDropdownRow(ImDrawList* dl, ImFont* font,
                                 const ImVec2& rowMin, const ImVec2& rowMax,
                                 const Widget& def,
                                 float alpha, bool allowClick) {
        const ImVec2 mouse = ImGui::GetIO().MousePos;
        const float  rowH  = rowMax.y - rowMin.y;
        bool consumed = false;

        const int count = def.bind ? OverlayBridge::get_dropdown_count(def.bind) : 0;
        int current     = def.bind ? OverlayBridge::get_dropdown(def.bind) : 0;
        if (count <= 0) {
            return false;
        }
        if (current < 0)       current = 0;
        if (current >= count)  current = count - 1;

        const char* curLabel = OverlayBridge::get_dropdown_label(def.bind, current);
        if (!curLabel || !*curLabel) curLabel = "—";

        const float boxH = 18.0f;
        const float boxW = 170.0f;
        const float boxX = rowMax.x - 4.0f - boxW;
        const float boxY = rowMin.y + (rowH - boxH) * 0.5f;

        const bool hoverBox =
            mouse.x >= boxX && mouse.x <= boxX + boxW &&
            mouse.y >= boxY && mouse.y <= boxY + boxH;

        const bool isOpen = (g_openDropdownBind == def.bind);

        if (allowClick && hoverBox &&
            ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
            g_openDropdownBind = isOpen ? nullptr : def.bind;
            consumed = true;
        }

        const ImU32 ink = IM_COL32(220, 220, 232, (BYTE)(255 * alpha));
        if (font) {
            const ImVec2 sz  = font->CalcTextSizeA(kBodyFontSize, FLT_MAX, 0.0f, def.label);
            const ImVec2 pos = PixelSnap(ImVec2(rowMin.x + 4.0f,
                                                  rowMin.y + (rowH - sz.y) * 0.5f - 1.0f));
            dl->AddText(font, kBodyFontSize, pos, ink, def.label);
        }

        const ImU32 boxFill = (hoverBox || isOpen)
            ? IM_COL32(36, 36, 46, (BYTE)(255 * alpha))
            : IM_COL32(22, 22, 30, (BYTE)(255 * alpha));
        dl->AddRectFilled(ImVec2(boxX, boxY),
                          ImVec2(boxX + boxW, boxY + boxH),
                          boxFill, 3.0f);
        dl->AddRect(ImVec2(boxX, boxY),
                    ImVec2(boxX + boxW, boxY + boxH),
                    IM_COL32(255, 255, 255, (BYTE)(40 * alpha)),
                    3.0f, 0, 1.0f);

        if (font) {
            const float chevSpace = 16.0f;
            const float maxTextW  = boxW - 12.0f - chevSpace;
            char buf[160];
            std::snprintf(buf, sizeof(buf), "%s", curLabel);
            ImVec2 sz = font->CalcTextSizeA(kBodyFontSize, FLT_MAX, 0.0f, buf);
            if (sz.x > maxTextW) {
                const float ellipW = font->CalcTextSizeA(kBodyFontSize, FLT_MAX, 0.0f, "...").x;
                int len = (int)strlen(buf);
                while (len > 0) {
                    buf[--len] = '\0';
                    sz = font->CalcTextSizeA(kBodyFontSize, FLT_MAX, 0.0f, buf);
                    if (sz.x + ellipW <= maxTextW) break;
                }
                const size_t left = sizeof(buf) - strlen(buf) - 1;
                if (left >= 3) strncat(buf, "...", left);
            }
            const ImU32 textCol = (current > 0)
                ? IM_COL32(225, 230, 242, (BYTE)(255 * alpha))
                : IM_COL32(140, 148, 162, (BYTE)(220 * alpha));
            sz = font->CalcTextSizeA(kBodyFontSize, FLT_MAX, 0.0f, buf);
            const ImVec2 tp = PixelSnap(ImVec2(boxX + 6.0f,
                                                 boxY + (boxH - sz.y) * 0.5f - 1.0f));
            dl->AddText(font, kBodyFontSize, tp, textCol, buf);
        }

        {
            const float chevCX = boxX + boxW - 9.0f;
            const float chevCY = boxY + boxH * 0.5f;
            const float chevS  = 3.5f;
            const float rot    = isOpen ? 3.14159265f : 0.0f;
            ImVec2 p[3] = {
                ImVec2(-chevS, -chevS * 0.45f),
                ImVec2( chevS, -chevS * 0.45f),
                ImVec2( 0.0f,   chevS *  0.70f),
            };
            const float cr = cosf(rot), sr = sinf(rot);
            for (int i = 0; i < 3; ++i) {
                p[i] = ImVec2(chevCX + p[i].x * cr - p[i].y * sr,
                               chevCY + p[i].x * sr + p[i].y * cr);
            }
            dl->AddTriangleFilled(p[0], p[1], p[2],
                IM_COL32(200, 206, 218, (BYTE)(255 * alpha)));
        }

        if (isOpen) {
            constexpr int   kMaxVisible = 8;
            const float itemH      = 18.0f;
            const int   visible    = (count < kMaxVisible) ? count : kMaxVisible;
            const bool  scrollable = (count > kMaxVisible);
            const float scrollW    = scrollable ? 6.0f : 0.0f;
            const float listW      = boxW;
            const float listX      = boxX;
            const float listY      = boxY + boxH + 2.0f;
            const float listH      = itemH * static_cast<float>(visible);

            // Scroll offset cached per-bind so opening a different dropdown
            // doesn't carry over the previous one's position.
            static const char* scrollBind   = nullptr;
            static float       scrollOffset = 0.0f;
            if (scrollBind != def.bind) {
                scrollBind   = def.bind;
                // Center the current selection in the visible window.
                scrollOffset = (float)current - (float)(visible / 2);
                if (scrollOffset < 0.0f) scrollOffset = 0.0f;
                const float maxScroll = (float)(count - visible);
                if (scrollOffset > maxScroll) scrollOffset = maxScroll;
            }

            // Mouse wheel scrolling when hovering inside the list.
            const bool hoverList =
                mouse.x >= listX && mouse.x <= listX + listW &&
                mouse.y >= listY && mouse.y <= listY + listH;
            if (scrollable && hoverList) {
                const float wheel = ImGui::GetIO().MouseWheel;
                if (wheel != 0.0f) scrollOffset -= wheel * 2.0f;
                const float maxScroll = (float)(count - visible);
                if (scrollOffset < 0.0f)         scrollOffset = 0.0f;
                if (scrollOffset > maxScroll)    scrollOffset = maxScroll;
            }

            ImDrawList* fg = ImGui::GetForegroundDrawList();

            fg->AddRectFilled(ImVec2(listX, listY),
                              ImVec2(listX + listW, listY + listH),
                              IM_COL32(20, 20, 26, (BYTE)(248 * alpha)), 3.0f);
            fg->AddRect(ImVec2(listX, listY),
                        ImVec2(listX + listW, listY + listH),
                        IM_COL32(255, 255, 255, (BYTE)(45 * alpha)),
                        3.0f, 0, 1.0f);

            const int firstVisible = (int)scrollOffset;
            const float textRightLimit = listX + listW - scrollW - 2.0f;

            bool clickedInside = false;
            for (int row = 0; row < visible; ++row) {
                const int i = firstVisible + row;
                if (i < 0 || i >= count) break;
                const float itemTop    = listY + itemH * static_cast<float>(row);
                const float itemBottom = itemTop + itemH;
                const bool  hover =
                    mouse.x >= listX && mouse.x <= textRightLimit &&
                    mouse.y >= itemTop && mouse.y <= itemBottom;
                const bool  isCurrent = (i == current);

                if (hover) {
                    fg->AddRectFilled(ImVec2(listX + 1.0f, itemTop),
                                      ImVec2(textRightLimit, itemBottom),
                                      IM_COL32(72, 151, 255, (BYTE)(48 * alpha)));
                }

                if (isCurrent) {
                    fg->AddRectFilled(
                        ImVec2(listX + 4.0f, itemTop + 4.0f),
                        ImVec2(listX + 6.0f, itemBottom - 4.0f),
                        IM_COL32(72, 151, 255, (BYTE)(255 * alpha)),
                        1.0f);
                }

                const char* lbl = OverlayBridge::get_dropdown_label(def.bind, i);
                if (lbl && font) {
                    const ImVec2 ts = font->CalcTextSizeA(kBodyFontSize, FLT_MAX, 0.0f, lbl);
                    const ImU32  c  = isCurrent
                        ? IM_COL32(232, 240, 252, (BYTE)(255 * alpha))
                        : IM_COL32(140, 148, 162, (BYTE)(230 * alpha));
                    fg->AddText(font, kBodyFontSize,
                                PixelSnap(ImVec2(listX + 12.0f,
                                                 itemTop + (itemH - ts.y) * 0.5f - 1.0f)),
                                c, lbl);
                }

                if (allowClick && hover &&
                    ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
                    OverlayBridge::set_dropdown(def.bind, i);
                    g_openDropdownBind = nullptr;
                    clickedInside = true;
                    consumed = true;
                    break;
                }
            }

            // Scrollbar thumb on the right edge.
            if (scrollable) {
                const float trackX = listX + listW - scrollW - 1.0f;
                const float trackW = scrollW;
                const float thumbH = listH * (float)visible / (float)count;
                const float thumbY = listY + (listH - thumbH) *
                    (scrollOffset / (float)(count - visible));
                fg->AddRectFilled(ImVec2(trackX, listY),
                                  ImVec2(trackX + trackW, listY + listH),
                                  IM_COL32(34, 34, 42, (BYTE)(200 * alpha)),
                                  trackW * 0.5f);
                fg->AddRectFilled(ImVec2(trackX, thumbY),
                                  ImVec2(trackX + trackW, thumbY + thumbH),
                                  IM_COL32(180, 188, 204, (BYTE)(220 * alpha)),
                                  trackW * 0.5f);
            }

            if (!clickedInside && allowClick &&
                ImGui::IsMouseClicked(ImGuiMouseButton_Left))
            {
                const bool outside_list =
                    mouse.x < listX || mouse.x > listX + listW ||
                    mouse.y < listY || mouse.y > listY + listH;
                const bool outside_box = !hoverBox;
                if (outside_list && outside_box)
                    g_openDropdownBind = nullptr;
            }

            // Clicks anywhere over the list are consumed so widgets
            // visually under the open list don't react.
            if (hoverList) consumed = true;
        }

        return consumed;
    }

    static void DrawSubsectionHeader(ImDrawList* dl, ImFont* font,
                                      const ImVec2& rMin, const ImVec2& rMax,
                                      const char* label, float alpha);

    static const char** FlagsList(int popupKind, int* outCount) {
        static const char* kSurv[] = { "Distance", "Latency", "Platform", "Character", "Interaction" };
        static const char* kKill[] = { "Distance", "Latency", "Hook state", "Platform", "Character", "Interaction" };
        // Rust player flags — order MUST match the bit positions used
        // in visuals.cpp flags() (bit 0 = Distance, bit 1 = Sleeping, etc.)
        // and the OverlayBridge::get_player_flag(idx) accessor.
        static const char* kRust[] = { "Distance", "Sleeping", "Wounded", "Team ID", "Aiming" };
        if (popupKind == PK_FlagsSurvivor) { *outCount = 5; return kSurv; }
        if (popupKind == PK_FlagsKiller)   { *outCount = 6; return kKill; }
        if (popupKind == PK_FlagsRust)     { *outCount = 5; return kRust; }
        *outCount = 0; return nullptr;
    }

    static bool DrawListboxRow(ImDrawList* dl, ImFont* font,
                                const ImVec2& wMin, const ImVec2& wMax,
                                int mt, int st, int ci, int wi,
                                float alpha, bool allowClick) {
        const ImVec2 mouse = ImGui::GetIO().MousePos;
        bool consumed = false;

        dl->AddRectFilled(wMin, wMax,
                          IM_COL32(10, 10, 14, (BYTE)(225 * alpha)),
                          5.0f);
        dl->AddRect(wMin, wMax,
                    IM_COL32(255, 255, 255, (BYTE)(38 * alpha)),
                    5.0f, 0, 1.0f);

        const float rowH = 24.0f;
        const ImVec2 inMin{ wMin.x + 4.0f, wMin.y + 4.0f };
        const ImVec2 inMax{ wMax.x - 4.0f, wMax.y - 4.0f };

        const float contentH = (float)g_configCount * rowH;
        const float visH     = inMax.y - inMin.y;
        const bool  scrolls  = contentH > visH;
        const float maxS     = scrolls ? (contentH - visH) : 0.0f;
        float& s   = g_listboxScroll [mt][st][ci][wi];
        float& sT  = g_listboxScrollT[mt][st][ci][wi];

        const bool hov = mouse.x >= wMin.x && mouse.x <= wMax.x &&
                          mouse.y >= wMin.y && mouse.y <= wMax.y;
        if (hov && scrolls) {
            const float wheel = ImGui::GetIO().MouseWheel;
            if (wheel != 0.0f) sT -= wheel * kScrollLineSize;
        }
        if (sT < 0)    sT = 0;
        if (sT > maxS) sT = maxS;
        s = EaseTo(s, sT, 22.0f);
        if (fabsf(s - sT) < 0.5f) s = sT;
        if (s < 0)    s = 0;
        if (s > maxS) s = maxS;

        int hoveredRow = -1;
        if (hov && allowClick) {
            for (int i = 0; i < g_configCount; ++i) {
                const float rowYTop = inMin.y - s + (float)i * rowH;
                if (mouse.y >= rowYTop && mouse.y < rowYTop + rowH &&
                    mouse.x >= inMin.x && mouse.x <= inMax.x) {
                    hoveredRow = i;
                    break;
                }
            }
        }
        if (hoveredRow >= 0 && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
            g_configSelected = hoveredRow;
            consumed = true;
        }

        const float targetSelY = (g_configSelected >= 0 && g_configSelected < g_configCount)
                                  ? (float)g_configSelected * rowH : 0.0f;
        float& selY = g_listSelY[mt][st][ci][wi];
        selY = EaseTo(selY, targetSelY, 22.0f);

        float& hovY = g_listHovY[mt][st][ci][wi];
        float& hovA = g_listHovA[mt][st][ci][wi];
        if (hoveredRow >= 0) {
            const float targetHovY = (float)hoveredRow * rowH;
            hovY = EaseTo(hovY, targetHovY, 28.0f);
            hovA = EaseTo(hovA, 1.0f, 14.0f);
        } else {
            hovA = EaseTo(hovA, 0.0f, 14.0f);
        }

        dl->PushClipRect(inMin, inMax, true);

        if (hovA > 0.02f) {
            const float yH = inMin.y - s + hovY;
            dl->AddRectFilled(
                ImVec2(inMin.x + 2.0f, yH + 1.0f),
                ImVec2(inMax.x - 2.0f, yH + rowH - 1.0f),
                IM_COL32(255, 255, 255, (BYTE)(20 * hovA * alpha)),
                3.0f);
        }

        if (g_configCount > 0 && g_configSelected >= 0) {
            const float yS = inMin.y - s + selY;
            dl->AddRectFilled(
                ImVec2(inMin.x + 2.0f, yS + 1.0f),
                ImVec2(inMax.x - 2.0f, yS + rowH - 1.0f),
                IM_COL32(72, 151, 255, (BYTE)(40 * alpha)),
                3.0f);
            dl->AddRectFilled(
                ImVec2(inMin.x + 1.0f, yS + 3.0f),
                ImVec2(inMin.x + 3.0f, yS + rowH - 3.0f),
                IM_COL32(72, 151, 255, (BYTE)(230 * alpha)),
                1.0f);
        }

        float rowY = inMin.y - s;
        for (int i = 0; i < g_configCount; ++i) {
            const ImVec2 rMin{ inMin.x, rowY };
            const ImVec2 rMax{ inMax.x, rowY + rowH };
            if (rMax.y >= inMin.y && rMin.y <= inMax.y) {
                ImU32 textCol;
                if (i == g_configSelected) {
                    textCol = IM_COL32(244, 244, 250, (BYTE)(255 * alpha));
                } else if (i == hoveredRow) {
                    textCol = IM_COL32(220, 220, 232, (BYTE)(245 * alpha));
                } else {
                    textCol = IM_COL32(160, 160, 178, (BYTE)(225 * alpha));
                }
                if (font) {
                    const ImVec2 sz = font->CalcTextSizeA(kBodyFontSize, FLT_MAX, 0.0f, g_configNames[i]);
                    const ImVec2 pos = PixelSnap(ImVec2(
                        rMin.x + 12.0f,
                        rMin.y + (rowH - sz.y) * 0.5f - 1.0f));
                    dl->AddText(font, kBodyFontSize, pos, textCol, g_configNames[i]);
                }
            }
            rowY += rowH;
        }

        const ImU32 shA = IM_COL32(0, 0, 0, (BYTE)(70 * alpha));
        const ImU32 sh0 = IM_COL32(0, 0, 0, 0);
        dl->AddRectFilledMultiColor(
            ImVec2(inMin.x, inMin.y),
            ImVec2(inMax.x, inMin.y + 7.0f),
            shA, shA, sh0, sh0);
        dl->AddRectFilledMultiColor(
            ImVec2(inMin.x, inMax.y - 7.0f),
            ImVec2(inMax.x, inMax.y),
            sh0, sh0, shA, shA);

        dl->PopClipRect();

        if (scrolls) {
            const float sbX  = inMax.x - 4.0f;
            const float sbY0 = inMin.y + 2.0f;
            const float sbY1 = inMax.y - 2.0f;
            const float trackH = sbY1 - sbY0;
            const float thumbH = trackH * (visH / contentH);
            const float thumbY = sbY0 + (trackH - thumbH) * (maxS > 0 ? (s / maxS) : 0);
            dl->AddRectFilled(
                ImVec2(sbX, sbY0),
                ImVec2(sbX + 3.0f, sbY1),
                IM_COL32(255, 255, 255, (BYTE)(14 * alpha)), 1.5f);
            dl->AddRectFilled(
                ImVec2(sbX, thumbY),
                ImVec2(sbX + 3.0f, thumbY + thumbH),
                IM_COL32(72, 151, 255, (BYTE)(180 * alpha)), 1.5f);
        }
        return consumed;
    }

    static bool DrawTextInputRow(ImDrawList* dl, ImFont* font,
                                  const ImVec2& wMin, const ImVec2& wMax,
                                  const Widget& def,
                                  int mt, int st, int ci, int wi,
                                  float alpha, bool allowClick) {
        const ImVec2 mouse = ImGui::GetIO().MousePos;
        bool consumed = false;
        const bool focused =
            g_focusedTI[0] == mt && g_focusedTI[1] == st &&
            g_focusedTI[2] == ci && g_focusedTI[3] == wi;

        const bool hov = mouse.x >= wMin.x && mouse.x <= wMax.x &&
                          mouse.y >= wMin.y && mouse.y <= wMax.y;
        if (allowClick && hov && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
            g_focusedTI[0] = mt; g_focusedTI[1] = st;
            g_focusedTI[2] = ci; g_focusedTI[3] = wi;
            consumed = true;
        } else if (allowClick && !hov && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
            if (focused) {
                g_focusedTI[0] = g_focusedTI[1] = g_focusedTI[2] = g_focusedTI[3] = -1;
            }
        }

        float& fa = g_widgetAnim[mt][st][ci][wi];
        fa = EaseTo(fa, focused ? 1.0f : (hov ? 0.35f : 0.0f), 18.0f);

        const ImU32 frameFill = IM_COL32(
            16 + (int)(8 * fa),
            16 + (int)(8 * fa),
            22 + (int)(10 * fa),
            (BYTE)((220 + 35 * fa) * alpha));
        dl->AddRectFilled(wMin, wMax, frameFill, 4.0f);

        const ImU32 borderIdle  = IM_COL32(255, 255, 255, (BYTE)(40  * alpha));
        const ImU32 borderFocus = IM_COL32( 72, 151, 255, (BYTE)(190 * alpha));
        const ImU32 borderCol   = LerpColorU32(borderIdle, borderFocus, fa);
        dl->AddRect(wMin, wMax, borderCol, 4.0f, 0, 1.0f + 0.6f * fa);

        dl->PushClipRect(
            ImVec2(wMin.x + 1.0f, wMin.y + 1.0f),
            ImVec2(wMax.x - 1.0f, wMax.y - 1.0f),
            true);
        const ImU32 shA = IM_COL32(0, 0, 0, (BYTE)(45 * alpha));
        const ImU32 sh0 = IM_COL32(0, 0, 0, 0);
        dl->AddRectFilledMultiColor(
            ImVec2(wMin.x + 1.0f, wMin.y + 1.0f),
            ImVec2(wMax.x - 1.0f, wMin.y + 5.0f),
            shA, shA, sh0, sh0);
        dl->PopClipRect();

        const bool hasText = (g_configInputLen > 0);
        const char* show   = hasText ? g_configInput : def.label;
        const ImU32 col    = hasText
            ? IM_COL32(244, 244, 250, (BYTE)(255 * alpha))
            : IM_COL32(140, 140, 158, (BYTE)(220 * alpha));
        if (font) {
            const ImVec2 sz  = font->CalcTextSizeA(kBodyFontSize, FLT_MAX, 0.0f, show);
            const ImVec2 pos = PixelSnap(ImVec2(
                wMin.x + 10.0f,
                wMin.y + ((wMax.y - wMin.y) - sz.y) * 0.5f - 1.0f));
            dl->AddText(font, kBodyFontSize, pos, col, show);

            if (focused) {
                const float tk    = (float)::GetTickCount() / 1000.0f;
                const float pulse = cosf(tk * 4.0f) * 0.5f + 0.5f;
                const BYTE  caretA = (BYTE)((90.0f + 130.0f * pulse) * alpha);
                const float caretX = pos.x + sz.x + 1.5f;
                dl->AddRectFilled(
                    ImVec2(caretX, pos.y + 1.0f),
                    ImVec2(caretX + 1.5f, pos.y + sz.y - 1.0f),
                    IM_COL32(244, 244, 250, caretA));
            }
        }

        if (fa > 0.02f) {
            const float fullW = (wMax.x - wMin.x) - 12.0f;
            const float w     = fullW * fa;
            const float cx    = (wMin.x + wMax.x) * 0.5f;
            const float underY = wMax.y - 2.0f;
            dl->AddRectFilled(
                ImVec2(cx - w * 0.5f, underY),
                ImVec2(cx + w * 0.5f, underY + 1.0f),
                IM_COL32(72, 151, 255, (BYTE)(200 * fa * alpha)));
        }

        if (focused) {
            for (int vk = 0; vk < 256; ++vk) {
                const bool down = (::GetAsyncKeyState(vk) & 0x8000) != 0;
                if (down && !g_keyPrevDown[vk]) {
                    if (vk >= 'A' && vk <= 'Z') {
                        const bool shift = (::GetAsyncKeyState(VK_SHIFT) & 0x8000) != 0;
                        if (g_configInputLen < kConfigNameMax - 1) {
                            g_configInput[g_configInputLen++] = (char)(shift ? vk : (vk + 32));
                            g_configInput[g_configInputLen]   = 0;
                        }
                    } else if (vk >= '0' && vk <= '9') {
                        if (g_configInputLen < kConfigNameMax - 1) {
                            g_configInput[g_configInputLen++] = (char)vk;
                            g_configInput[g_configInputLen]   = 0;
                        }
                    } else if (vk == VK_SPACE) {
                        if (g_configInputLen < kConfigNameMax - 1) {
                            g_configInput[g_configInputLen++] = ' ';
                            g_configInput[g_configInputLen]   = 0;
                        }
                    } else if (vk == VK_BACK) {
                        if (g_configInputLen > 0) {
                            g_configInput[--g_configInputLen] = 0;
                        }
                    } else if (vk == VK_ESCAPE) {
                        g_focusedTI[0] = g_focusedTI[1] = g_focusedTI[2] = g_focusedTI[3] = -1;
                    }
                }
                g_keyPrevDown[vk] = down;
            }
        }
        return consumed;
    }

    static bool DrawConfirmBtnRow(ImDrawList* dl, ImFont* font,
                                   const ImVec2& wMin, const ImVec2& wMax,
                                   const Widget& def,
                                   int mt, int st, int ci, int wi,
                                   float alpha, bool allowClick) {
        const ImVec2 mouse = ImGui::GetIO().MousePos;
        const bool hov = mouse.x >= wMin.x && mouse.x <= wMax.x &&
                          mouse.y >= wMin.y && mouse.y <= wMax.y;
        bool consumed = false;

        float& ha = g_widgetAnim[mt][st][ci][wi];
        ha = EaseTo(ha, hov ? 1.0f : 0.0f, 16.0f);

        const DWORD now = ::GetTickCount();
        DWORD& pulseTk = g_widgetPulseTk[mt][st][ci][wi];

        if (allowClick && hov && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
            g_confirmAction = def.popupKind;
            pulseTk         = now;
            OpenPopup(mt, st, ci, wi, PK_Confirm, MenuCenter());
            consumed = true;
        }

        const DWORD pulseDur = 320;
        float pulse = 0.0f;
        if (pulseTk != 0 && (now - pulseTk) < pulseDur) {
            pulse = 1.0f - ((float)(now - pulseTk) / (float)pulseDur);
            pulse = pulse * pulse;
        }

        ImU32 fillIdle, fillHov, bordIdle, bordHov, textIdle, accent;
        bool wantAccentBar = false;
        const int action = def.popupKind;
        if (action == 1) {
            fillIdle = IM_COL32(24, 30, 46, (BYTE)(240 * alpha));
            fillHov  = IM_COL32(34, 50, 90, (BYTE)(255 * alpha));
            bordIdle = IM_COL32(72, 151, 255, (BYTE)(95  * alpha));
            bordHov  = IM_COL32(72, 151, 255, (BYTE)(220 * alpha));
            textIdle = IM_COL32(228, 234, 250, (BYTE)(255 * alpha));
            accent   = IM_COL32(72, 151, 255, 255);
            wantAccentBar = true;
        } else if (action == 4) {
            fillIdle = IM_COL32(22, 22, 30, (BYTE)(220 * alpha));
            fillHov  = IM_COL32(54, 28, 34, (BYTE)(255 * alpha));
            bordIdle = IM_COL32(255, 255, 255, (BYTE)(40  * alpha));
            bordHov  = IM_COL32(232,  92,  92, (BYTE)(220 * alpha));
            textIdle = IM_COL32(220, 220, 232, (BYTE)(255 * alpha));
            accent   = IM_COL32(232, 92, 92, 255);
            wantAccentBar = true;
        } else {
            fillIdle = IM_COL32(22, 22, 30, (BYTE)(220 * alpha));
            fillHov  = IM_COL32(36, 36, 46, (BYTE)(255 * alpha));
            bordIdle = IM_COL32(255, 255, 255, (BYTE)(40  * alpha));
            bordHov  = IM_COL32(255, 255, 255, (BYTE)(110 * alpha));
            textIdle = IM_COL32(220, 220, 232, (BYTE)(255 * alpha));
            accent   = IM_COL32(255, 255, 255, 255);
            wantAccentBar = false;
        }

        const float bright = ha + pulse * 0.4f;
        ImU32 fill = LerpColorU32(fillIdle, fillHov, bright > 1.0f ? 1.0f : bright);
        ImU32 bord = LerpColorU32(bordIdle, bordHov, ha);
        ImU32 text = textIdle;
        if (action == 4 && ha > 0.0f) {
            text = LerpColorU32(textIdle,
                                IM_COL32(255, 210, 210, (BYTE)(255 * alpha)),
                                ha);
        }

        dl->AddRectFilled(wMin, wMax, fill, 4.0f);
        dl->AddRect(wMin, wMax, bord, 4.0f, 0, 1.0f + 0.5f * ha);

        const ImU32 sheenT = IM_COL32(255, 255, 255, (BYTE)((10 + 18 * ha) * alpha));
        const ImU32 sheen0 = IM_COL32(255, 255, 255, 0);
        dl->PushClipRect(
            ImVec2(wMin.x + 1.0f, wMin.y + 1.0f),
            ImVec2(wMax.x - 1.0f, wMax.y - 1.0f),
            true);
        dl->AddRectFilledMultiColor(
            ImVec2(wMin.x + 1.0f, wMin.y + 1.0f),
            ImVec2(wMax.x - 1.0f, wMin.y + 7.0f),
            sheenT, sheenT, sheen0, sheen0);
        dl->PopClipRect();

        if (wantAccentBar && ha > 0.02f) {
            const float full = (wMax.y - wMin.y) - 8.0f;
            const float bh   = full * ha;
            const float cy   = (wMin.y + wMax.y) * 0.5f;
            const BYTE  acA  = (BYTE)(230 * ha * alpha);
            const ImU32 acC  = (accent & 0x00FFFFFFu) | ((ImU32)acA << 24);
            dl->AddRectFilled(
                ImVec2(wMin.x + 1.0f, cy - bh * 0.5f),
                ImVec2(wMin.x + 3.0f, cy + bh * 0.5f),
                acC, 1.0f);
        }

        if (font) {
            const ImVec2 sz  = font->CalcTextSizeA(kBodyFontSize, FLT_MAX, 0.0f, def.label);
            const float liftY = -0.8f * ha;
            const ImVec2 pos = PixelSnap(ImVec2(
                wMin.x + ((wMax.x - wMin.x) - sz.x) * 0.5f,
                wMin.y + ((wMax.y - wMin.y) - sz.y) * 0.5f - 1.0f + liftY));
            dl->AddText(font, kBodyFontSize, pos, text, def.label);
        }

        if (pulse > 0.02f) {
            const BYTE  pA = (BYTE)(140 * pulse * alpha);
            const ImU32 pC = (accent & 0x00FFFFFFu) | ((ImU32)pA << 24);
            for (int i = 0; i < 2; ++i) {
                const float infl = 1.0f + (float)i * 2.0f;
                dl->AddRect(
                    ImVec2(wMin.x - infl, wMin.y - infl),
                    ImVec2(wMax.x + infl, wMax.y + infl),
                    pC, 4.0f + infl, 0, 1.0f);
            }
        }
        return consumed;
    }

    static bool DrawFlagsDropdown(ImDrawList* dl, ImFont* font,
                                   const ImVec2& wMin, const ImVec2& wMax,
                                   const Widget& def,
                                   int mt, int st, int ci, int wi,
                                   float alpha, bool allowClick) {
        const ImVec2 mouse = ImGui::GetIO().MousePos;
        const float  rowH  = wMax.y - wMin.y;
        bool consumed = false;

        const ImU32 ink = IM_COL32(220, 220, 232, (BYTE)(255 * alpha));
        if (font) {
            const ImVec2 sz  = font->CalcTextSizeA(kBodyFontSize, FLT_MAX, 0.0f, def.label);
            const ImVec2 pos = PixelSnap(ImVec2(wMin.x + 4.0f,
                                                  wMin.y + (rowH - sz.y) * 0.5f - 1.0f));
            dl->AddText(font, kBodyFontSize, pos, ink, def.label);
        }

        const float boxH = 18.0f;
        const float boxW = 170.0f;
        const float boxX = wMax.x - 4.0f - boxW;
        const float boxY = wMin.y + (rowH - boxH) * 0.5f;

        const bool hoverBox =
            mouse.x >= boxX && mouse.x <= boxX + boxW &&
            mouse.y >= boxY && mouse.y <= boxY + boxH;
        const bool isOpen = IsPopupOwnedBy(mt, st, ci, wi);

        if (allowClick && hoverBox && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
            if (isOpen) RequestClosePopup();
            else        OpenPopup(mt, st, ci, wi, def.popupKind,
                                   ImVec2(boxX, boxY + boxH));
            consumed = true;
        }

        const ImU32 boxFill = (hoverBox || isOpen)
            ? IM_COL32(36, 36, 46, (BYTE)(255 * alpha))
            : IM_COL32(22, 22, 30, (BYTE)(255 * alpha));
        dl->AddRectFilled(ImVec2(boxX, boxY),
                          ImVec2(boxX + boxW, boxY + boxH),
                          boxFill, 3.0f);
        dl->AddRect(ImVec2(boxX, boxY),
                    ImVec2(boxX + boxW, boxY + boxH),
                    IM_COL32(255, 255, 255, (BYTE)(40 * alpha)),
                    3.0f, 0, 1.0f);

        int n = 0;
        const char** flags = FlagsList(def.popupKind, &n);
        char sel[160]; sel[0] = '\0';
        int selCount = 0;
        if (flags) {
            for (int i = 0; i < n; ++i) {
                if (!g_popupFlag[mt][st][ci][wi][i]) continue;
                if (selCount > 0) {
                    const size_t left = sizeof(sel) - strlen(sel) - 1;
                    if (left > 0) strncat(sel, ", ", left);
                }
                const size_t left = sizeof(sel) - strlen(sel) - 1;
                if (left > 0) strncat(sel, flags[i], left);
                ++selCount;
            }
        }
        const char* shown = (selCount > 0) ? sel : "None";

        const float chevSpace = 16.0f;
        const float maxTextW  = boxW - 12.0f - chevSpace;
        char buf[160];
        std::snprintf(buf, sizeof(buf), "%s", shown);
        if (font) {
            ImVec2 sz = font->CalcTextSizeA(kBodyFontSize, FLT_MAX, 0.0f, buf);
            if (sz.x > maxTextW) {
                const float ellipW = font->CalcTextSizeA(kBodyFontSize, FLT_MAX, 0.0f, "...").x;
                int len = (int)strlen(buf);
                while (len > 0) {
                    buf[--len] = '\0';
                    sz = font->CalcTextSizeA(kBodyFontSize, FLT_MAX, 0.0f, buf);
                    if (sz.x + ellipW <= maxTextW) break;
                }
                const size_t left = sizeof(buf) - strlen(buf) - 1;
                if (left >= 3) strncat(buf, "...", left);
            }

            const ImU32 textCol = (selCount > 0)
                ? IM_COL32(225, 230, 242, (BYTE)(255 * alpha))
                : IM_COL32(140, 148, 162, (BYTE)(220 * alpha));
            sz = font->CalcTextSizeA(kBodyFontSize, FLT_MAX, 0.0f, buf);
            const ImVec2 tpos = PixelSnap(ImVec2(boxX + 6.0f,
                                                   boxY + (boxH - sz.y) * 0.5f - 1.0f));
            dl->AddText(font, kBodyFontSize, tpos, textCol, buf);
        }

        const float chevCX = boxX + boxW - 9.0f;
        const float chevCY = boxY + boxH * 0.5f;
        const float chevS  = 3.5f;
        const float rot    = isOpen ? 3.14159265f : 0.0f;
        ImVec2 p[3] = {
            ImVec2(-chevS, -chevS * 0.45f),
            ImVec2( chevS, -chevS * 0.45f),
            ImVec2( 0.0f,   chevS *  0.70f),
        };
        const float cr = cosf(rot), sr = sinf(rot);
        for (int i = 0; i < 3; ++i) {
            p[i] = ImVec2(chevCX + p[i].x * cr - p[i].y * sr,
                           chevCY + p[i].x * sr + p[i].y * cr);
        }
        dl->AddTriangleFilled(p[0], p[1], p[2],
            IM_COL32(200, 206, 218, (BYTE)(255 * alpha)));

        if (def.popupKind == PK_FlagsSurvivor || def.popupKind == PK_FlagsRust) {
            for (int i = 0; i < 8; ++i)
                g_player_flag_mirror[i] = g_popupFlag[mt][st][ci][wi][i];
        }

        return consumed;
    }

    static void DrawContainerBox(ImDrawList* dl, ImFont* font,
                                  const ImVec2& boxMin, const ImVec2& boxMax,
                                  const char* title, float alpha) {
        const ImU32 fill   = IM_COL32(20, 21, 30, (BYTE)(170 * alpha));
        const ImU32 border = IM_COL32(255, 255, 255, (BYTE)(20 * alpha));
        const ImU32 titleC = IM_COL32(220, 220, 232, (BYTE)(255 * alpha));

        dl->AddRectFilled(boxMin, boxMax, fill, 6.0f);
        dl->AddRect(boxMin, boxMax, border, 6.0f, 0, 1.0f);

        if (font && title && *title) {
            const ImVec2 sz  = font->CalcTextSizeA(kBodyFontSize, FLT_MAX, 0.0f, title);
            const ImVec2 pos = PixelSnap(ImVec2(boxMin.x + 14.0f, boxMin.y + 10.0f));
            dl->AddText(font, kBodyFontSize, pos, titleC, title);
            (void)sz;
        }
    }

    static int FlagsCount(int popupKind) {
        switch (popupKind) {
            case PK_FlagsSurvivor: return 5;
            case PK_FlagsKiller:   return 6;
            case PK_FlagsRust:     return 5;
            default:               return 0;
        }
    }

    static float WidgetHeightFor(const Widget& w) {
        if (w.type == WT::Check && w.popupKind == PK_EnableWritesConfirm)
            return 50.0f;
        switch (w.type) {
            case WT::Section:    return 22.0f;
            case WT::Check:      return 22.0f;
            case WT::Dropdown:   return 26.0f;
            case WT::Slider:     return 38.0f;
            case WT::Keybind:    return 22.0f;
            case WT::Flags:      return 22.0f;
            case WT::Listbox:    return 130.0f;
            case WT::TextInput:  return 26.0f;
            case WT::ConfirmBtn: return 26.0f;
        }
        (void)w;
        return 22.0f;
    }

    static bool IsWidgetHidden(const Widget& w) {
        if (w.popupKind == PK_EnableWritesConfirm) return false;
        return w.writes && !IsWritesEnabled();
    }

    static int ContainerWidgetCount(const Container& c) {
        int n = 0;
        for (int i = 0; i < kMaxWidgets; ++i) {
            if (!c.widgets[i].label) break;
            ++n;
        }
        return n;
    }

    static bool IsContainerEffectivelyEmpty(const Container& c) {
        for (int i = 0; i < kMaxWidgets; ++i) {
            const Widget& w = c.widgets[i];
            if (!w.label) break;
            if (w.type == WT::Section) continue;  // bare headers don't count
            if (!IsWidgetHidden(w)) return false;
        }
        return true;
    }

    static bool IsSubtabEffectivelyEmpty(const SubtabDef& sub) {
        for (int c = 0; c < kMaxContainers; ++c) {
            const Container& cont = sub.containers[c];
            if (!cont.title || !*cont.title) break;
            if (!IsContainerEffectivelyEmpty(cont)) return false;
        }
        return true;
    }

    static bool IsMainTabEffectivelyEmpty(const MainTabDef& mt) {
        const int subCount = (mt.numSubtabs == 0) ? 1 : mt.numSubtabs;
        for (int s = 0; s < subCount; ++s) {
            if (!IsSubtabEffectivelyEmpty(mt.subtabs[s])) return false;
        }
        return true;
    }

    static float ContainerHeight(const Container& c) {
        float h = 28.0f + 8.0f;
        const int n = ContainerWidgetCount(c);
        for (int i = 0; i < n; ++i) {
            if (IsWidgetHidden(c.widgets[i])) continue;
            h += WidgetHeightFor(c.widgets[i]);
        }
        return h;
    }

    static void DrawSubsectionHeader(ImDrawList* dl, ImFont* font,
                                      const ImVec2& rMin, const ImVec2& rMax,
                                      const char* label, float alpha) {
        if (!label || !*label) return;
        const float rH      = rMax.y - rMin.y;
        const ImU32 textCol = IM_COL32(190, 195, 208, (BYTE)(255 * alpha));
        if (font) {
            const ImVec2 sz   = font->CalcTextSizeA(kBodyFontSize, FLT_MAX, 0.0f, label);
            const float yMid  = rMin.y + (rH - sz.y) * 0.5f + 4.0f;
            const ImVec2 pos  = PixelSnap(ImVec2(rMin.x + 4.0f, yMid));
            dl->AddText(font, kBodyFontSize, pos, textCol, label);
        }
    }

    static bool DrawPopup(ImDrawList* dlBg, ImFont* font, float menuAlpha) {
        if (g_popupAnim < 0.02f || g_popupOwnerMain < 0) {
            g_popupBlockActive = false;
            return false;
        }

        // Force all popups (gear-icon options, color picker, max-distance,
        // flags dropdown, confirm prompt) onto the foreground draw list.
        // The previous behaviour drew them on the background layer that
        // also carries the menu surface, so anything extending past the
        // menu's rounded bounding rect got visually overlapped by the
        // surface chrome — looked like "popups get clipped at menu
        // edges". The foreground layer is unclipped and rendered above
        // everything (same layer the dropdown's open-list already uses).
        (void)dlBg;
        ImDrawList* dl = ImGui::GetForegroundDrawList();

        const int mt = g_popupOwnerMain, st = g_popupOwnerSub;
        const int ci = g_popupOwnerCont, wi = g_popupOwnerWid;

        int rows = 0;
        const char* titleText = "";
        switch (g_popupKind) {
            case PK_BoxFill:         rows = 1; titleText = "Box";              break;
            case PK_Skeleton:        rows = 2; titleText = "Skeleton";         break;
            case PK_OffscreenArrows: rows = 2; titleText = "Offscreen arrows"; break;
            case PK_MaxDistance:     rows = 1; titleText = "Max distance";     break;
            case PK_Generator:       rows = 2; titleText = "Generator";        break;
            case PK_PostSaturation:  rows = 1; titleText = "Saturation";       break;
            case PK_PostContrast:    rows = 1; titleText = "Contrast";         break;
            case PK_PostSharpness:   rows = 1; titleText = "Sharpness";        break;
            case PK_PostBloom:       rows = 2; titleText = "Bloom";            break;
            case PK_FlagsSurvivor:   rows = 5; titleText = "Survivor flags";   break;
            case PK_FlagsKiller:     rows = 6; titleText = "Killer flags";     break;
            case PK_FlagsRust:       rows = 5; titleText = "Player flags";     break;
            case PK_AutoPallet:      rows = 1; titleText = "Auto pallet stun"; break;
            case PK_Desync:          rows = 1; titleText = "Desync";           break;
            case PK_SpeedBoost:      rows = 1; titleText = "Speed";            break;
            case PK_ThickBullet:     rows = 1; titleText = "Thickness";        break;
            case PK_OverrideSpread:  rows = 1; titleText = "Spread";           break;
            case PK_OverrideRecoil:  rows = 1; titleText = "Recoil";           break;
            case PK_OverrideMelee:   rows = 1; titleText = "Range";            break;
            case PK_FovZoom:         rows = 1; titleText = "Zoom";             break;
            case PK_ColorPicker:     rows = 0; titleText = "";                 break;
            case PK_Confirm:         rows = 0; titleText = "";                 break;
            default: return false;
        }

        const bool flagsKind =
            (g_popupKind == PK_FlagsSurvivor ||
             g_popupKind == PK_FlagsKiller   ||
             g_popupKind == PK_FlagsRust);
        const bool isPicker  = (g_popupKind == PK_ColorPicker);
        const bool isConfirm = (g_popupKind == PK_Confirm);

        constexpr float kPickerSV   = 200.0f;
        constexpr float kPickerHue  = 14.0f;
        constexpr float kPickerPad  = 8.0f;
        constexpr float kConfirmW   = 300.0f;
        constexpr float kConfirmH   = 110.0f;

        const float popupW = isConfirm
            ? kConfirmW
            : (isPicker
                ? (kPickerSV + kPickerPad * 2.0f)
                : (flagsKind ? 170.0f : 190.0f));
        const float titleH = (flagsKind || isPicker || isConfirm) ? 0.0f : 28.0f;
        const float rowH   = 24.0f;
        const float padBot = 10.0f;
        const float popupH = isConfirm
            ? kConfirmH
            : (isPicker
                ? (kPickerSV + kPickerHue + kPickerPad * 3.0f)
                : (titleH + rows * rowH + padBot));

        const float eased = EaseOutCubic(g_popupAnim);
        const float alpha = menuAlpha * eased;

        const ImVec2 disp = ImGui::GetIO().DisplaySize;
        const bool dropDown =
            (g_popupKind == PK_FlagsSurvivor ||
             g_popupKind == PK_FlagsKiller   ||
             g_popupKind == PK_FlagsRust     ||
             g_popupKind == PK_ColorPicker);

        // Clamp box: use the live menu rect when the menu is open (so the
        // popup never extends past the menu's own right/bottom edge), else
        // fall back to the screen. Previously the clamps used disp.x/disp.y
        // alone, so on a 1080p screen with a 720-px wide menu the popup
        // would still spill out into the empty area to the right of the
        // panel rather than flipping back inside it.
        float boundsL = 8.0f;
        float boundsT = 8.0f;
        float boundsR = disp.x - 8.0f;
        float boundsB = disp.y - 8.0f;
        if (g_shape.w >= 100.0f && g_shape.h >= 100.0f) {
            boundsL = g_shape.x + 4.0f;
            boundsT = g_shape.y + 4.0f;
            boundsR = g_shape.x + g_shape.w - 4.0f;
            boundsB = g_shape.y + g_shape.h - 4.0f;
        }

        float pX, pY;
        if (isConfirm) {
            pX = g_popupAnchor.x - popupW * 0.5f;
            pY = g_popupAnchor.y - popupH * 0.5f;
            if (pX < boundsL) pX = boundsL;
            if (pY < boundsT) pY = boundsT;
            if (pX + popupW > boundsR) pX = boundsR - popupW;
            if (pY + popupH > boundsB) pY = boundsB - popupH;
        } else if (dropDown) {
            pX = g_popupAnchor.x;
            pY = g_popupAnchor.y + 4.0f;
            if (pX + popupW > boundsR) pX = boundsR - popupW;
            if (pX < boundsL)          pX = boundsL;
            if (pY + popupH > boundsB) pY = g_popupAnchor.y - 4.0f - popupH;
            if (pY < boundsT)          pY = boundsT;
        } else {
            pX = g_popupAnchor.x + 4.0f;
            pY = g_popupAnchor.y - popupH * 0.5f;
            if (pX + popupW > boundsR) {
                // Flip to LEFT of the anchor (popup pointing the other way)
                pX = g_popupAnchor.x - popupW - 28.0f;
            }
            if (pX < boundsL)          pX = boundsL;
            if (pX + popupW > boundsR) pX = boundsR - popupW;
            if (pY < boundsT)          pY = boundsT;
            if (pY + popupH > boundsB) pY = boundsB - popupH;
        }

        const float scale = 0.88f + 0.12f * eased;
        const float cX = pX + popupW * 0.5f;
        const float cY = pY + popupH * 0.5f;
        const float halfW = popupW * scale * 0.5f;
        const float halfH = popupH * scale * 0.5f;
        const ImVec2 bMin{ cX - halfW, cY - halfH };
        const ImVec2 bMax{ cX + halfW, cY + halfH };

        // Snapshot for next-frame widget click-block. Only when the
        // popup is reasonably opened (anim past half) so a closing
        // popup doesn't block clicks on its way out.
        if (g_popupAnim > 0.5f) {
            g_popupBlockActive = true;
            g_popupBlockMin    = bMin;
            g_popupBlockMax    = bMax;
        } else {
            g_popupBlockActive = false;
        }

        for (int i = 3; i >= 0; --i) {
            const float infl = (i + 1) * 3.0f;
            const BYTE  a    = (BYTE)(60.0f * alpha / (i + 1));
            if (a == 0) continue;
            dl->AddRectFilled(
                ImVec2(bMin.x - infl, bMin.y - infl + 3.0f),
                ImVec2(bMax.x + infl, bMax.y + infl + 3.0f),
                IM_COL32(0, 0, 0, a),
                8.0f + infl);
        }
        dl->AddRectFilled(bMin, bMax,
                          IM_COL32(16, 16, 22, (BYTE)(252 * alpha)),
                          8.0f);
        dl->AddRect(bMin, bMax,
                    IM_COL32(255, 255, 255, (BYTE)(50 * alpha)),
                    8.0f, 0, 1.0f);

        if (titleH > 0.0f) {
            if (font) {
                const ImVec2 pos = PixelSnap(ImVec2(bMin.x + 14.0f, bMin.y + 9.0f));
                dl->AddText(font, kBodyFontSize, pos,
                            IM_COL32(230, 234, 244, (BYTE)(255 * alpha)),
                            titleText);
            }
            dl->AddRectFilled(
                ImVec2(bMin.x + 10.0f, bMin.y + titleH),
                ImVec2(bMax.x - 10.0f, bMin.y + titleH + 1.0f),
                IM_COL32(255, 255, 255, (BYTE)(28 * alpha)));
        }
        (void)titleText;

        const ImVec2 mouse  = ImGui::GetIO().MousePos;
        const bool hoverPopup =
            mouse.x >= bMin.x && mouse.x <= bMax.x &&
            mouse.y >= bMin.y && mouse.y <= bMax.y;

        const bool interact = (eased > 0.7f);
        bool consumed = hoverPopup;

        float rowY = bMin.y + titleH + 4.0f;

        auto popupCheck = [&](const char* label, bool& state) {
            const ImVec2 rMin{ bMin.x + 12.0f, rowY };
            const ImVec2 rMax{ bMax.x - 12.0f, rowY + rowH - 2.0f };
            const float  rH   = rMax.y - rMin.y;
            const bool   hov  = mouse.x >= rMin.x && mouse.x <= rMax.x &&
                                 mouse.y >= rMin.y && mouse.y <= rMax.y;

            if (hov && alpha > 0.05f) {
                dl->AddRectFilled(rMin, rMax,
                    IM_COL32(255, 255, 255, (BYTE)(14 * alpha)), 4.0f);
            }
            const float boxSz = 13.0f;
            const float boxX  = rMin.x + 4.0f;
            const float boxY  = rMin.y + (rH - boxSz) * 0.5f;
            const float labelStartX = boxX + boxSz + 8.0f;
            if (font) {
                const ImVec2 sz  = font->CalcTextSizeA(kBodyFontSize, FLT_MAX, 0.0f, label);
                const ImVec2 pos = PixelSnap(ImVec2(labelStartX,
                                                      rMin.y + (rH - sz.y) * 0.5f - 1.0f));
                dl->AddText(font, kBodyFontSize, pos,
                            IM_COL32(220, 220, 232, (BYTE)(255 * alpha)),
                            label);
            }
            const float t     = state ? 1.0f : 0.0f;
            const int fR = (int)(22 + ( 72 - 22)  * t);
            const int fG = (int)(22 + (151 - 22)  * t);
            const int fB = (int)(30 + (255 - 30)  * t);
            dl->AddRectFilled(ImVec2(boxX, boxY),
                              ImVec2(boxX + boxSz, boxY + boxSz),
                              IM_COL32(fR, fG, fB, (BYTE)(255 * alpha)), 3.0f);
            dl->AddRect(ImVec2(boxX, boxY),
                        ImVec2(boxX + boxSz, boxY + boxSz),
                        IM_COL32(255, 255, 255, (BYTE)((38 + 60 * t) * alpha)),
                        3.0f, 0, 1.0f);
            if (interact && hov && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
                state = !state;
                consumed = true;
            }
            rowY += rowH;
        };

        auto popupFlagItem = [&](const char* label, bool& state) {
            const ImVec2 rMin{ bMin.x + 6.0f, rowY };
            const ImVec2 rMax{ bMax.x - 6.0f, rowY + rowH - 2.0f };
            const float  rH   = rMax.y - rMin.y;
            const bool   hov  =
                mouse.x >= rMin.x && mouse.x <= rMax.x &&
                mouse.y >= rMin.y && mouse.y <= rMax.y;

            if (hov && alpha > 0.05f) {
                dl->AddRectFilled(rMin, rMax,
                    IM_COL32(255, 255, 255, (BYTE)(14 * alpha)), 3.0f);
            }
            if (state) {
                dl->AddRectFilled(
                    ImVec2(rMin.x + 2.0f,  rMin.y + 4.0f),
                    ImVec2(rMin.x + 4.0f,  rMax.y - 4.0f),
                    IM_COL32(72, 151, 255, (BYTE)(255 * alpha)),
                    1.0f);
            }
            if (font) {
                const ImU32 ink = state
                    ? IM_COL32(232, 240, 252, (BYTE)(255 * alpha))
                    : IM_COL32(140, 148, 162, (BYTE)(230 * alpha));
                const ImVec2 sz  = font->CalcTextSizeA(kBodyFontSize, FLT_MAX, 0.0f, label);
                const ImVec2 pos = PixelSnap(ImVec2(rMin.x + 10.0f,
                                                      rMin.y + (rH - sz.y) * 0.5f - 1.0f));
                dl->AddText(font, kBodyFontSize, pos, ink, label);
            }
            if (interact && hov && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
                state = !state;
                consumed = true;
            }
            rowY += rowH;
        };

        auto popupSlider = [&](const char* label, float& value,
                                float minV, float maxV, int slot,
                                const char* suffix) {
            const ImVec2 rMin{ bMin.x + 12.0f, rowY };
            const ImVec2 rMax{ bMax.x - 12.0f, rowY + rowH - 2.0f };
            const float  rH   = rMax.y - rMin.y;
            const bool   hov  = mouse.x >= rMin.x && mouse.x <= rMax.x &&
                                 mouse.y >= rMin.y && mouse.y <= rMax.y;

            if (font) {
                const ImVec2 lpos = PixelSnap(ImVec2(rMin.x + 4.0f, rMin.y));
                dl->AddText(font, kBodyFontSize, lpos,
                            IM_COL32(220, 220, 232, (BYTE)(255 * alpha)),
                            label);
                char vbuf[32];
                if (maxV - minV <= 4.0f) std::snprintf(vbuf, sizeof(vbuf), "%.2f%s", value, suffix);
                else                     std::snprintf(vbuf, sizeof(vbuf), "%.0f%s", value, suffix);
                const ImVec2 vsz  = font->CalcTextSizeA(kBodyFontSize, FLT_MAX, 0.0f, vbuf);
                const ImVec2 vpos = PixelSnap(ImVec2(rMax.x - 4.0f - vsz.x, rMin.y));
                dl->AddText(font, kBodyFontSize, vpos,
                            IM_COL32(170, 175, 188, (BYTE)(220 * alpha)),
                            vbuf);
            }
            const float trackH = 3.0f;
            const float trackX = rMin.x + 4.0f;
            const float trackW = (rMax.x - rMin.x) - 8.0f;
            const float trackY = rMax.y - 5.0f - trackH;

            if (interact && hov && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
                g_popupDragSlot = slot;
                consumed = true;
            }
            if (g_popupDragSlot == slot) {
                if (ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
                    const float ut = (mouse.x - trackX) / trackW;
                    const float tc = (ut < 0.0f) ? 0.0f : (ut > 1.0f) ? 1.0f : ut;
                    value = minV + tc * (maxV - minV);
                    consumed = true;
                } else {
                    g_popupDragSlot = -1;
                }
            }
            if (value < minV) value = minV;
            if (value > maxV) value = maxV;
            const float t     = (value - minV) / (maxV - minV);
            const float fillW = trackW * t;
            dl->AddRectFilled(ImVec2(trackX, trackY),
                              ImVec2(trackX + trackW, trackY + trackH),
                              IM_COL32(28, 28, 36, (BYTE)(255 * alpha)),
                              trackH * 0.5f);
            if (fillW > 1.0f) {
                dl->AddRectFilled(ImVec2(trackX, trackY),
                                  ImVec2(trackX + fillW, trackY + trackH),
                                  IM_COL32(72, 151, 255, (BYTE)(255 * alpha)),
                                  trackH * 0.5f);
            }
            (void)rH;
            rowY += rowH;
        };

        switch (g_popupKind) {
            case PK_BoxFill:
                popupCheck("Fill", g_popupCheck1[mt][st][ci][wi]);
                break;
            case PK_Skeleton: {
                // Rounded — bound to the data model so the skeleton
                // renderer in visuals.cpp picks it up (this checkbox
                // used to live as a separate top-level row; moved into
                // the gear popup so the Player ESP container isn't
                // cluttered with sub-options).
                constexpr const char* kRoundedBind =
                    "Visuals/Players/Skeleton/Options/Rounded";
                bool& rnd = g_popupCheck1[mt][st][ci][wi];
                rnd = OverlayBridge::get_checkbox(kRoundedBind);
                bool rnd_was = rnd;
                popupCheck("Rounded", rnd);
                if (rnd != rnd_was)
                    OverlayBridge::set_checkbox(kRoundedBind, rnd);

                // Vis colors — per-bone raycast from local eye colors
                // each segment green (visible) / red (occluded). Same
                // bind pattern as Rounded so the visuals.cpp reader
                // picks it up via app::get_value.
                constexpr const char* kVisColorsBind =
                    "Visuals/Players/Skeleton/Options/Vis colors";
                bool& vc = g_popupCheck2[mt][st][ci][wi];
                vc = OverlayBridge::get_checkbox(kVisColorsBind);
                bool vc_was = vc;
                popupCheck("Vis colors", vc);
                if (vc != vc_was)
                    OverlayBridge::set_checkbox(kVisColorsBind, vc);

                popupSlider("Thickness", g_popupSlider1[mt][st][ci][wi],
                            0.5f, 5.0f, 0, "px");
                break;
            }
            case PK_OffscreenArrows:
                popupSlider("Radius", g_popupSlider1[mt][st][ci][wi],
                            50.0f, 500.0f, 0, "px");
                popupSlider("Size",   g_popupSlider2[mt][st][ci][wi],
                            8.0f, 32.0f, 1, "px");
                break;
            case PK_FlagsSurvivor: {
                int n = 0;
                const char** kF = FlagsList(PK_FlagsSurvivor, &n);
                for (int i = 0; i < n; ++i)
                    popupFlagItem(kF[i], g_popupFlag[mt][st][ci][wi][i]);
                break;
            }
            case PK_FlagsKiller: {
                int n = 0;
                const char** kF = FlagsList(PK_FlagsKiller, &n);
                for (int i = 0; i < n; ++i)
                    popupFlagItem(kF[i], g_popupFlag[mt][st][ci][wi][i]);
                break;
            }
            case PK_FlagsRust: {
                int n = 0;
                const char** kF = FlagsList(PK_FlagsRust, &n);
                for (int i = 0; i < n; ++i)
                    popupFlagItem(kF[i], g_popupFlag[mt][st][ci][wi][i]);
                break;
            }
            case PK_MaxDistance: {
                // Bind path = <owner widget's bind>/Options/Max distance.
                // Without this round-trip, slider changes never reached the
                // data model — every entity/player rendered at the default
                // 350 m regardless of what the user set.
                const Widget& wd = kTabs[mt].subtabs[st].containers[ci].widgets[wi];
                if (!wd.bind) {
                    popupSlider("Max distance", g_popupSlider1[mt][st][ci][wi],
                                1.0f, 500.0f, 0, "m");
                    break;
                }
                std::string kBind = wd.bind;
                kBind += "/Options/Max distance";
                float& v = g_popupSlider1[mt][st][ci][wi];
                v = (float)OverlayBridge::get_slider_i32(kBind.c_str());
                popupSlider("Max distance", v, 0.0f, 500.0f, 0, "m");
                OverlayBridge::set_slider_i32(kBind.c_str(), (int)v);
                break;
            }
            case PK_Generator:
                popupSlider("Max distance", g_popupSlider1[mt][st][ci][wi],
                            1.0f, 300.0f, 0, "m");
                popupCheck ("Smart Aura",   g_popupCheck1 [mt][st][ci][wi]);
                break;
            case PK_PostSaturation:
                popupSlider("Saturation", g_popupSlider1[mt][st][ci][wi],
                            0.0f, 5.0f, 0, "");
                break;
            case PK_PostContrast:
                popupSlider("Contrast", g_popupSlider1[mt][st][ci][wi],
                            0.0f, 5.0f, 0, "");
                break;
            case PK_PostSharpness:
                popupSlider("Sharpness", g_popupSlider1[mt][st][ci][wi],
                            0.0f, 10.0f, 0, "");
                break;
            case PK_PostBloom:
                popupSlider("Intensity", g_popupSlider1[mt][st][ci][wi],
                            0.0f, 100.0f, 0, "%");
                popupSlider("Threshold", g_popupSlider2[mt][st][ci][wi],
                            0.0f, 1.0f, 1, "");
                break;
            case PK_AutoPallet:
                popupSlider("Prediction time", g_popupSlider1[mt][st][ci][wi],
                            0.0f, 200.0f, 0, "ms");
                break;
            case PK_Desync:
                popupCheck("Indicator", g_popupCheck1[mt][st][ci][wi]);
                break;
            case PK_SpeedBoost:
                popupSlider("Boost", g_popupSlider1[mt][st][ci][wi],
                            0.0f, 150.0f, 0, "%");
                break;
            case PK_ThickBullet: {
                constexpr const char* kBind =
                    "Misc/General/Thick bullet/Options/Amount";
                float& v = g_popupSlider1[mt][st][ci][wi];
                v = OverlayBridge::get_slider_f32(kBind);
                popupSlider("Thickness", v, 0.0f, 3.0f, 0, "");
                OverlayBridge::set_slider_f32(kBind, v);
                break;
            }
            case PK_OverrideSpread: {
                constexpr const char* kBind =
                    "Aimbot/Weapons/Override weapon spread/Options/Amount";
                float& v = g_popupSlider1[mt][st][ci][wi];
                v = (float)OverlayBridge::get_slider_i32(kBind);
                popupSlider("Amount", v, 0.0f, 100.0f, 0, "%");
                OverlayBridge::set_slider_i32(kBind, (int)v);
                break;
            }
            case PK_OverrideRecoil: {
                constexpr const char* kBind =
                    "Aimbot/Weapons/Override weapon recoil/Options/Amount";
                float& v = g_popupSlider1[mt][st][ci][wi];
                v = (float)OverlayBridge::get_slider_i32(kBind);
                popupSlider("Amount", v, 0.0f, 100.0f, 0, "%");
                OverlayBridge::set_slider_i32(kBind, (int)v);
                break;
            }
            case PK_OverrideMelee: {
                constexpr const char* kBind =
                    "Aimbot/Weapons/Override melee range/Options/Amount";
                float& v = g_popupSlider1[mt][st][ci][wi];
                v = (float)OverlayBridge::get_slider_i32(kBind);
                popupSlider("Amount", v, 100.0f, 500.0f, 0, "%");
                OverlayBridge::set_slider_i32(kBind, (int)v);
                break;
            }
            case PK_FovZoom: {
                constexpr const char* kBind =
                    "Visuals/Local/Field of view/Options/Zoom FOV";
                float& v = g_popupSlider1[mt][st][ci][wi];
                v = OverlayBridge::get_slider_f32(kBind);
                popupSlider("Zoom FOV", v, 10.0f, 150.0f, 0, "");
                OverlayBridge::set_slider_f32(kBind, v);
                break;
            }
            case PK_Confirm: {
                const char* title  = "Confirm";
                const char* msg    = "Are you sure?";
                switch (g_confirmAction) {
                    case 1: title = "Create config";
                            msg = "Are you sure you want to create a new config?"; break;
                    case 2: title = "Save config";
                            msg = "Are you sure you want to save this config?"; break;
                    case 3: title = "Load config";
                            msg = "Are you sure you want to load this config?"; break;
                    case 4: title = "Delete config";
                            msg = "Are you sure you want to delete this config?"; break;
                    case 5: title = "Enable memory writes";
                            msg = "This will let the cheat modify game memory. Continue?"; break;
                }
                if (font) {
                    const ImVec2 tpos = PixelSnap(ImVec2(bMin.x + 14.0f, bMin.y + 10.0f));
                    dl->AddText(font, kBodyFontSize, tpos,
                                IM_COL32(244, 244, 250, (BYTE)(255 * alpha)),
                                title);
                    // Divider under title.
                    dl->AddRectFilled(
                        ImVec2(bMin.x + 12.0f, bMin.y + 30.0f),
                        ImVec2(bMax.x - 12.0f, bMin.y + 31.0f),
                        IM_COL32(255, 255, 255, (BYTE)(60 * alpha)));
                    // Message.
                    const ImVec2 msz  = font->CalcTextSizeA(kBodyFontSize, FLT_MAX, 0.0f, msg);
                    const ImVec2 mpos = PixelSnap(ImVec2(
                        bMin.x + ((bMax.x - bMin.x) - msz.x) * 0.5f,
                        bMin.y + 44.0f));
                    dl->AddText(font, kBodyFontSize, mpos,
                                IM_COL32(208, 208, 220, (BYTE)(255 * alpha)),
                                msg);
                }
                // Yes / No buttons.
                const float btnW = 80.0f, btnH = 26.0f;
                const float btnGap = 14.0f;
                const float btnY = bMax.y - btnH - 10.0f;
                const float yesX = bMin.x + (bMax.x - bMin.x) * 0.5f - btnW - btnGap * 0.5f;
                const float noX  = bMin.x + (bMax.x - bMin.x) * 0.5f + btnGap * 0.5f;
                const bool hovYes = interact &&
                    mouse.x >= yesX && mouse.x <= yesX + btnW &&
                    mouse.y >= btnY && mouse.y <= btnY + btnH;
                const bool hovNo  = interact &&
                    mouse.x >= noX  && mouse.x <= noX  + btnW &&
                    mouse.y >= btnY && mouse.y <= btnY + btnH;

                const ImU32 yesFill = hovYes
                    ? IM_COL32(72, 151, 255, (BYTE)(255 * alpha))
                    : IM_COL32(72, 151, 255, (BYTE)(255 * alpha));
                dl->AddRectFilled(ImVec2(yesX, btnY),
                                  ImVec2(yesX + btnW, btnY + btnH),
                                  yesFill, 4.0f);
                if (font) {
                    const ImVec2 sz = font->CalcTextSizeA(kBodyFontSize, FLT_MAX, 0.0f, "Yes");
                    dl->AddText(font, kBodyFontSize,
                                PixelSnap(ImVec2(yesX + (btnW - sz.x) * 0.5f,
                                                  btnY + (btnH - sz.y) * 0.5f - 1.0f)),
                                IM_COL32(255, 255, 255, (BYTE)(255 * alpha)),
                                "Yes");
                }

                const ImU32 noFill = hovNo
                    ? IM_COL32(58, 44, 80, (BYTE)(255 * alpha))
                    : IM_COL32(22, 22, 30, (BYTE)(255 * alpha));
                dl->AddRectFilled(ImVec2(noX, btnY),
                                  ImVec2(noX + btnW, btnY + btnH),
                                  noFill, 4.0f);
                dl->AddRect(ImVec2(noX, btnY),
                            ImVec2(noX + btnW, btnY + btnH),
                            IM_COL32(255, 255, 255, (BYTE)(60 * alpha)),
                            4.0f, 0, 1.0f);
                if (font) {
                    const ImVec2 sz = font->CalcTextSizeA(kBodyFontSize, FLT_MAX, 0.0f, "No");
                    dl->AddText(font, kBodyFontSize,
                                PixelSnap(ImVec2(noX + (btnW - sz.x) * 0.5f,
                                                  btnY + (btnH - sz.y) * 0.5f - 1.0f)),
                                IM_COL32(220, 214, 234, (BYTE)(255 * alpha)),
                                "No");
                }

                // Handle clicks.
                if (interact && hovYes && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
                    consumed = true;
                    switch (g_confirmAction) {
                        case 1: {  // Create
                            if (g_configInputLen > 0 && g_configCount < kMaxConfigs) {
                                std::snprintf(g_configNames[g_configCount],
                                               kConfigNameMax, "%s", g_configInput);
                                config::save_by_name(g_configNames[g_configCount]);
                                g_configSelected = g_configCount;
                                g_configCount++;
                                g_configInput[0] = 0;
                                g_configInputLen = 0;
                            }
                            break;
                        }
                        case 2: {        // Save (overwrite current)
                            if (g_configSelected >= 0 && g_configSelected < g_configCount)
                                config::save_by_name(g_configNames[g_configSelected]);
                            break;
                        }
                        case 3: {        // Load
                            if (g_configSelected >= 0 && g_configSelected < g_configCount)
                                config::load_by_name(g_configNames[g_configSelected]);
                            break;
                        }
                        case 4: {        // Delete
                            if (g_configSelected >= 0 && g_configSelected < g_configCount) {
                                config::remove_by_name(g_configNames[g_configSelected]);
                                for (int i = g_configSelected; i < g_configCount - 1; ++i) {
                                    std::snprintf(g_configNames[i], kConfigNameMax,
                                                   "%s", g_configNames[i + 1]);
                                }
                                g_configCount--;
                                if (g_configSelected >= g_configCount)
                                    g_configSelected = g_configCount - 1;
                            }
                            break;
                        }
                        case 5: {        // Enable memory writes
                            SetWritesEnabled(true);
                            OverlayBridge::set_checkbox("Settings/UI/Enable memory writes", true);
                            break;
                        }
                    }
                    g_confirmAction = 0;
                    RequestClosePopup();
                }
                if (interact && hovNo && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
                    g_confirmAction = 0;
                    consumed = true;
                    RequestClosePopup();
                }
                break;
            }
            case PK_ColorPicker: {
                unsigned& color = g_widgetColor[mt][st][ci][wi];
                // Commit-after-edit: if the owning widget has a bind path,
                // push any color change immediately to the data model so
                // the next-frame WT::Check render's bridge re-read returns
                // OUR new value instead of overwriting the user's pick.
                const char* ownerBind = nullptr;
                {
                    const SubtabDef& sub = kTabs[mt].subtabs[st];
                    const Container& cont = sub.containers[ci];
                    const Widget& wd = cont.widgets[wi];
                    if (wd.hasColor) ownerBind = wd.bind;
                }
                const unsigned colorBefore = color;

                // Unpack to floats for HSV math.
                const float curR = (color & 0xFF)         / 255.0f;
                const float curG = ((color >> 8) & 0xFF)  / 255.0f;
                const float curB = ((color >> 16) & 0xFF) / 255.0f;
                float curH, curS, curV;
                ImGui::ColorConvertRGBtoHSV(curR, curG, curB, curH, curS, curV);

                const float svX = bMin.x + kPickerPad;
                const float svY = bMin.y + kPickerPad;
                const float svW = kPickerSV;
                const float svH = kPickerSV;
                const ImVec2 svMin{ svX,       svY };
                const ImVec2 svMax{ svX + svW, svY + svH };

                const float hueX = svX;
                const float hueY = svY + svH + kPickerPad;
                const float hueW = svW;
                const float hueH = kPickerHue;
                const ImVec2 hueMin{ hueX,        hueY };
                const ImVec2 hueMax{ hueX + hueW, hueY + hueH };

                // Hit-test (only when the open animation has settled
                // and the click hasn't been consumed by something else).
                const bool hovSV  = interact &&
                    mouse.x >= svMin.x && mouse.x <= svMax.x &&
                    mouse.y >= svMin.y && mouse.y <= svMax.y;
                const bool hovHue = interact &&
                    mouse.x >= hueMin.x && mouse.x <= hueMax.x &&
                    mouse.y >= hueMin.y && mouse.y <= hueMax.y;

                if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
                    if      (hovSV)  g_pickerDrag = 1;
                    else if (hovHue) g_pickerDrag = 2;
                }
                if (!ImGui::IsMouseDown(ImGuiMouseButton_Left)) g_pickerDrag = 0;

                // Dragging SV writes new saturation/value (hue unchanged).
                if (g_pickerDrag == 1) {
                    float ns = (mouse.x - svMin.x) / svW;
                    float nv = 1.0f - (mouse.y - svMin.y) / svH;
                    if (ns < 0) ns = 0; if (ns > 1) ns = 1;
                    if (nv < 0) nv = 0; if (nv > 1) nv = 1;
                    curS = ns; curV = nv;
                    float nr, ng, nb;
                    ImGui::ColorConvertHSVtoRGB(curH, curS, curV, nr, ng, nb);
                    color = IM_COL32((int)(nr * 255), (int)(ng * 255), (int)(nb * 255), 255);
                    consumed = true;
                }
                // Dragging hue writes new hue (saturation/value preserved).
                if (g_pickerDrag == 2) {
                    float nh = (mouse.x - hueMin.x) / hueW;
                    if (nh < 0)      nh = 0;
                    if (nh > 0.9999f) nh = 0.9999f;
                    curH = nh;
                    float nr, ng, nb;
                    ImGui::ColorConvertHSVtoRGB(curH, curS, curV, nr, ng, nb);
                    color = IM_COL32((int)(nr * 255), (int)(ng * 255), (int)(nb * 255), 255);
                    consumed = true;
                }

                float pr, pg, pb;
                ImGui::ColorConvertHSVtoRGB(curH, 1.0f, 1.0f, pr, pg, pb);
                const ImU32 hueColor = IM_COL32((int)(pr * 255), (int)(pg * 255), (int)(pb * 255),
                                                  (BYTE)(255 * alpha));
                const ImU32 white    = IM_COL32(255, 255, 255, (BYTE)(255 * alpha));
                dl->AddRectFilledMultiColor(svMin, svMax,
                                             white, hueColor, hueColor, white);
                // Layer 2: transparent → black gradient (value along Y).
                const ImU32 t0 = IM_COL32(0, 0, 0, 0);
                const ImU32 t1 = IM_COL32(0, 0, 0, (BYTE)(255 * alpha));
                dl->AddRectFilledMultiColor(svMin, svMax, t0, t0, t1, t1);
                dl->AddRect(svMin, svMax,
                            IM_COL32(255, 255, 255, (BYTE)(40 * alpha)),
                            2.0f, 0, 1.0f);

                // SV indicator: small ringed dot.
                const float dotX = svMin.x + curS * svW;
                const float dotY = svMin.y + (1.0f - curV) * svH;
                dl->AddCircleFilled(ImVec2(dotX, dotY + 1.0f), 4.5f,
                                     IM_COL32(0, 0, 0, (BYTE)(120 * alpha)), 18);
                dl->AddCircleFilled(ImVec2(dotX, dotY), 4.0f,
                                     IM_COL32(16, 16, 22, (BYTE)(255 * alpha)), 18);
                dl->AddCircleFilled(ImVec2(dotX, dotY), 3.0f,
                                     IM_COL32(255, 255, 255, (BYTE)(255 * alpha)), 18);

                const ImU32 hueStops[7] = {
                    IM_COL32(255,   0,   0, (BYTE)(255 * alpha)),   // R
                    IM_COL32(255, 255,   0, (BYTE)(255 * alpha)),   // Y
                    IM_COL32(  0, 255,   0, (BYTE)(255 * alpha)),   // G
                    IM_COL32(  0, 255, 255, (BYTE)(255 * alpha)),   // C
                    IM_COL32(  0,   0, 255, (BYTE)(255 * alpha)),   // B
                    IM_COL32(255,   0, 255, (BYTE)(255 * alpha)),   // M
                    IM_COL32(255,   0,   0, (BYTE)(255 * alpha)),   // R (loop)
                };
                for (int i = 0; i < 6; ++i) {
                    const float xL = hueMin.x + ((float)i       / 6.0f) * hueW;
                    const float xR = hueMin.x + ((float)(i + 1) / 6.0f) * hueW;
                    dl->AddRectFilledMultiColor(
                        ImVec2(xL, hueMin.y),
                        ImVec2(xR, hueMax.y),
                        hueStops[i], hueStops[i + 1],
                        hueStops[i + 1], hueStops[i]);
                }
                dl->AddRect(hueMin, hueMax,
                            IM_COL32(255, 255, 255, (BYTE)(40 * alpha)),
                            2.0f, 0, 1.0f);

                // Hue grab handle: thin vertical white pill on the bar.
                const float grabX = hueMin.x + curH * hueW;
                const float grabHalfW = 2.0f;
                const ImVec2 grabMin{ grabX - grabHalfW, hueMin.y - 1.0f };
                const ImVec2 grabMax{ grabX + grabHalfW, hueMax.y + 1.0f };
                dl->AddRectFilled(grabMin, grabMax,
                                   IM_COL32(0, 0, 0, (BYTE)(180 * alpha)),
                                   1.5f);
                dl->AddRectFilled(
                    ImVec2(grabMin.x + 0.8f, grabMin.y + 0.8f),
                    ImVec2(grabMax.x - 0.8f, grabMax.y - 0.8f),
                    IM_COL32(255, 255, 255, (BYTE)(255 * alpha)),
                    1.0f);
                // Color picker commit (see ownerBind capture at top of case).
                if (ownerBind && color != colorBefore) {
                    std::string p = ownerBind;
                    p += "/Color";
                    OverlayBridge::set_color_picker(p.c_str(), color);
                }
                break;
            }
        }

        return consumed;
    }

    static void DrawMenuContent(ImDrawList* dl, const ShapeRect& r, float menuAlpha) {
        const ImVec2 mouse   = ImGui::GetIO().MousePos;
        const bool   clicked = ImGui::IsMouseClicked(ImGuiMouseButton_Left);
        const DWORD  now     = ::GetTickCount();

        const ImU32 ink      = IM_COL32(220, 220, 232, (BYTE)(255 * menuAlpha));
        const ImU32 inkDim   = IM_COL32(150, 150, 165, (BYTE)(230 * menuAlpha));
        const ImU32 inkFaint = IM_COL32(140, 140, 158,  (BYTE)(200 * menuAlpha));
        const ImU32 acc      = IM_COL32(72, 151, 255, (BYTE)(255 * menuAlpha));

        ImFont* font = Render::Fonts::montserrat14px;

        const float padX   = 22.0f;
        const float top    = r.y + 16.0f;

        const ImVec2 brandPos = PixelSnap(ImVec2(r.x + padX, top));
        if (font) {
            const char* brandPrefix = "Gordocheats";
            const char* brandSuffix = ".me";
            const ImVec2 sz = font->CalcTextSizeA(kBodyFontSize, FLT_MAX, 0.0f, brandPrefix);
            dl->AddText(font, kBodyFontSize, brandPos, ink, brandPrefix);
            const ImVec2 sufPos = PixelSnap(ImVec2(brandPos.x + sz.x, brandPos.y));
            dl->AddText(font, kBodyFontSize, sufPos, acc, brandSuffix);
        }

        bool clickConsumed = false;

        // Block clicks from reaching widgets that visually sit behind an
        // open popup. The popup's rect was snapshotted at the end of the
        // previous frame's DrawPopup. One-frame lag the very first frame
        // a popup opens — accepted.
        if (g_popupBlockActive &&
            mouse.x >= g_popupBlockMin.x && mouse.x <= g_popupBlockMax.x &&
            mouse.y >= g_popupBlockMin.y && mouse.y <= g_popupBlockMax.y)
        {
            clickConsumed = true;
        }

        int  hoveredMain   = -1;
        struct MainTabRect { float xMin, xMax; };
        MainTabRect mtr[kNumMainTabs] = {};

        // auto-jump off any tab that's now writes-gated empty
        if (g_mainTab >= 0 && g_mainTab < kNumMainTabs &&
            IsMainTabEffectivelyEmpty(kTabs[g_mainTab])) {
            for (int i = 0; i < kNumMainTabs; ++i) {
                if (!IsMainTabEffectivelyEmpty(kTabs[i])) {
                    g_lastMainTab   = g_mainTab;
                    g_mainTab       = i;
                    g_indicatorX    = -1.0f;
                    if (g_popupOwnerMain >= 0) RequestClosePopup();
                    break;
                }
            }
        }

        {
            float cursorR = r.x + r.w - padX;
            for (int i = kNumMainTabs - 1; i >= 0; --i) {
                if (IsMainTabEffectivelyEmpty(kTabs[i])) continue;
                const char* name = kTabs[i].name;
                if (!font) break;
                const ImVec2 sz = font->CalcTextSizeA(kBodyFontSize, FLT_MAX, 0.0f, name);
                mtr[i].xMax = cursorR;
                mtr[i].xMin = cursorR - sz.x;
                cursorR    -= sz.x + 18.0f;
            }
        }
        const float mainTabY = top;
        const float mainTabH = 18.0f;
        for (int i = 0; i < kNumMainTabs; ++i) {
            if (mtr[i].xMin == 0 && mtr[i].xMax == 0) continue;
            const bool hov = mouse.x >= mtr[i].xMin && mouse.x <= mtr[i].xMax &&
                              mouse.y >= mainTabY     && mouse.y <= mainTabY + mainTabH;
            if (hov) hoveredMain = i;
        }
        for (int i = 0; i < kNumMainTabs; ++i) {
            if (mtr[i].xMin == 0 && mtr[i].xMax == 0) continue;
            const bool isActive = (i == g_mainTab);
            const bool hov      = (i == hoveredMain);
            g_mainTabHover[i] = EaseTo(g_mainTabHover[i],
                                        (hov || isActive) ? 1.0f : 0.0f,
                                        14.0f);
            const float h = g_mainTabHover[i];

            if (h > 0.04f) {
                const float padX2 = 8.0f, padY2 = 4.0f;
                dl->AddRectFilled(
                    ImVec2(mtr[i].xMin - padX2, mainTabY - padY2),
                    ImVec2(mtr[i].xMax + padX2, mainTabY + mainTabH + padY2),
                    IM_COL32(255, 255, 255, (BYTE)(18 * h * menuAlpha)),
                    5.0f);
            }
            const ImU32 idleCol   = inkDim;
            const ImU32 brightCol = isActive
                ? ink
                : IM_COL32(212, 218, 230, (BYTE)(255 * menuAlpha));
            const ImU32 col = LerpColorU32(idleCol, brightCol, h);
            if (font) {
                const ImVec2 pos = PixelSnap(ImVec2(mtr[i].xMin, mainTabY));
                dl->AddText(font, kBodyFontSize, pos, col, kTabs[i].name);
            }
        }
        if (clicked && hoveredMain >= 0 && hoveredMain != g_mainTab) {
            g_lastMainTab   = g_mainTab;
            g_mainTab       = hoveredMain;
            g_tabSwitchTick = now;
            g_indicatorX    = -1.0f;
            if (g_popupOwnerMain >= 0) RequestClosePopup();
            clickConsumed   = true;
        }

        // ── Search bar (top-left) ────────────────────────────────────
        // Filter-as-you-type. When focused, captures A-Z / 0-9 / space /
        // backspace / escape. Filter applied in the container + widget
        // render loops below (see ContainerHasAnyMatch / WidgetMatchesSearch).
        //
        // Shares the same Y line as the subtab description (descY=top+20).
        // The description's X is shifted further down so they sit side-
        // by-side: [search][description] horizontally. Before this fix
        // the search box overlapped text like "Developer diagnostics".
        // The description shift lives in the sub.desc render block; if
        // sbW / sbX change here, update the +30 offset there too.
        {
            const float sbW = 160.0f;
            const float sbH = 16.0f;
            const float sbX = r.x + padX;
            const float sbY = mainTabY + mainTabH + 4.0f;
            const ImVec2 sbMin{ sbX, sbY };
            const ImVec2 sbMax{ sbX + sbW, sbY + sbH };

            const bool sbHover =
                mouse.x >= sbMin.x && mouse.x <= sbMax.x &&
                mouse.y >= sbMin.y && mouse.y <= sbMax.y;
            if (!clickConsumed && clicked) {
                if (sbHover) {
                    g_searchFocused = true;
                    clickConsumed   = true;
                } else if (g_searchFocused) {
                    g_searchFocused = false;
                }
            }

            // Frame: subtle fill + accent border when focused. Keystroke
            // pulse layers a glow behind the bar and bumps the border.
            const float fa = g_searchFocused ? 1.0f : (sbHover ? 0.35f : 0.0f);
            const float pulse = g_searchPulse;
            const ImU32 fill = IM_COL32(
                16 + (int)(8 * fa),
                16 + (int)(8 * fa),
                22 + (int)(10 * fa),
                (BYTE)((200 + 35 * fa) * menuAlpha));
            if (pulse > 0.01f) {
                const float halo  = 3.0f * pulse;
                const BYTE  haloA = (BYTE)(140.0f * pulse * menuAlpha);
                dl->AddRectFilled(
                    ImVec2(sbMin.x - halo, sbMin.y - halo),
                    ImVec2(sbMax.x + halo, sbMax.y + halo),
                    IM_COL32(72, 151, 255, haloA), 6.0f);
            }
            dl->AddRectFilled(sbMin, sbMax, fill, 4.0f);
            const ImU32 borderIdle  = IM_COL32(255, 255, 255, (BYTE)(40  * menuAlpha));
            const ImU32 borderFocus = IM_COL32( 72, 151, 255, (BYTE)(190 * menuAlpha));
            const float borderW = 1.0f + 1.5f * pulse;
            dl->AddRect(sbMin, sbMax, LerpColorU32(borderIdle, borderFocus, fa),
                        4.0f, 0, borderW);

            // Magnifier glyph (just a small circle + handle)
            const float mgX = sbMin.x + 8.0f;
            const float mgY = sbMin.y + sbH * 0.5f;
            const ImU32 mgCol = IM_COL32(160, 165, 180, (BYTE)(220 * menuAlpha));
            dl->AddCircle(ImVec2(mgX, mgY), 3.5f, mgCol, 0, 1.2f);
            dl->AddLine(ImVec2(mgX + 2.5f, mgY + 2.5f),
                        ImVec2(mgX + 5.5f, mgY + 5.5f), mgCol, 1.2f);

            if (font) {
                const char* show = (g_searchLen > 0) ? g_searchBuf : "Search...";
                const ImU32 col  = (g_searchLen > 0)
                    ? IM_COL32(244, 244, 250, (BYTE)(255 * menuAlpha))
                    : IM_COL32(140, 140, 158, (BYTE)(180 * menuAlpha));
                const ImVec2 sz  = font->CalcTextSizeA(kBodyFontSize, FLT_MAX, 0.0f, show);
                const ImVec2 pos = PixelSnap(ImVec2(
                    sbMin.x + 20.0f,
                    sbMin.y + (sbH - sz.y) * 0.5f - 1.0f));
                dl->AddText(font, kBodyFontSize, pos, col, show);

                if (g_searchFocused) {
                    const float tk    = (float)::GetTickCount() / 1000.0f;
                    const float pulse = cosf(tk * 4.0f) * 0.5f + 0.5f;
                    const BYTE  ca    = (BYTE)((90.0f + 130.0f * pulse) * menuAlpha);
                    const float caretX = pos.x + sz.x + 1.5f;
                    dl->AddRectFilled(
                        ImVec2(caretX, pos.y + 1.0f),
                        ImVec2(caretX + 1.5f, pos.y + sz.y - 1.0f),
                        IM_COL32(244, 244, 250, ca));
                }
            }

            // Keyboard capture while focused. Mirrors the existing
            // DrawTextInputRow handler (g_configInput) but writes into
            // g_searchBuf and updates g_searchLen.
            if (g_searchFocused) {
                bool textChanged = false;
                for (int vk = 0; vk < 256; ++vk) {
                    const bool down = (::GetAsyncKeyState(vk) & 0x8000) != 0;
                    if (down && !g_keyPrevDown[vk]) {
                        if (vk >= 'A' && vk <= 'Z') {
                            const bool shift = (::GetAsyncKeyState(VK_SHIFT) & 0x8000) != 0;
                            if (g_searchLen < (int)sizeof(g_searchBuf) - 1) {
                                g_searchBuf[g_searchLen++] = (char)(shift ? vk : (vk + 32));
                                g_searchBuf[g_searchLen]   = 0;
                                textChanged = true;
                            }
                        } else if (vk >= '0' && vk <= '9') {
                            if (g_searchLen < (int)sizeof(g_searchBuf) - 1) {
                                g_searchBuf[g_searchLen++] = (char)vk;
                                g_searchBuf[g_searchLen]   = 0;
                                textChanged = true;
                            }
                        } else if (vk == VK_SPACE) {
                            if (g_searchLen < (int)sizeof(g_searchBuf) - 1) {
                                g_searchBuf[g_searchLen++] = ' ';
                                g_searchBuf[g_searchLen]   = 0;
                                textChanged = true;
                            }
                        } else if (vk == VK_BACK) {
                            if (g_searchLen > 0) {
                                g_searchBuf[--g_searchLen] = 0;
                                textChanged = true;
                            }
                        } else if (vk == VK_ESCAPE) {
                            g_searchLen = 0;
                            g_searchBuf[0] = 0;
                            g_searchFocused = false;
                            textChanged = true;
                        }
                    }
                    g_keyPrevDown[vk] = down;
                }
                if (textChanged) g_searchPulse = 1.0f;
            }
        }

        // Pulse decays smoothly even outside the input block so the
        // glow fades after keystrokes stop.
        g_searchPulse = EaseTo(g_searchPulse, 0.0f, 6.0f);

        // ── Global search nav ─────────────────────────────────────
        // When the search text changes AND the active subtab has no
        // matches, jump to the first (mt, st) pair that does. Only on
        // change — once jumped, user can navigate freely without being
        // snapped back. When the user clears the search, restore the
        // pre-search location.
        {
            const std::uint32_t h = HashSearchBuf();
            if (h != g_searchTokenHash) {
                if (g_searchLen > 0) {
                    if (g_searchSavedMainTab < 0) {
                        g_searchSavedMainTab = g_mainTab;
                        g_searchSavedSubtab  = (g_mainTab >= 0 && g_mainTab < kNumMainTabs)
                            ? g_subtab[g_mainTab] : 0;
                    }
                    const MainTabDef& curMt = kTabs[g_mainTab];
                    const int curSub = (curMt.numSubtabs > 0) ? g_subtab[g_mainTab] : 0;
                    bool hereOK = false;
                    if (curSub >= 0 && curSub < curMt.numSubtabs)
                        hereOK = SubtabHasAnyMatch(curMt.subtabs[curSub]);
                    if (!hereOK) {
                        for (int t = 0; t < kNumMainTabs && !hereOK; ++t) {
                            const MainTabDef& mtd = kTabs[t];
                            for (int s = 0; s < mtd.numSubtabs; ++s) {
                                if (SubtabHasAnyMatch(mtd.subtabs[s])) {
                                    g_mainTab    = t;
                                    g_subtab[t]  = s;
                                    g_indicatorX = -1.0f;
                                    if (g_popupOwnerMain >= 0) RequestClosePopup();
                                    hereOK = true;
                                    break;
                                }
                            }
                        }
                    }
                }
                else if (g_searchSavedMainTab >= 0) {
                    g_mainTab    = g_searchSavedMainTab;
                    g_subtab[g_mainTab] = g_searchSavedSubtab;
                    g_searchSavedMainTab = -1;
                    g_searchSavedSubtab  = -1;
                    g_indicatorX = -1.0f;
                }
                g_searchTokenHash = h;
            }
        }

        const MainTabDef& mt = kTabs[g_mainTab];
        const bool hasSubtabs = mt.numSubtabs > 0;
        if (hasSubtabs) {
            int& curSub = g_subtab[g_mainTab];
            if (curSub >= 0 && curSub < mt.numSubtabs &&
                IsSubtabEffectivelyEmpty(mt.subtabs[curSub])) {
                for (int s = 0; s < mt.numSubtabs; ++s) {
                    if (!IsSubtabEffectivelyEmpty(mt.subtabs[s])) {
                        curSub = s;
                        g_indicatorX = -1.0f;
                        if (g_popupOwnerMain >= 0) RequestClosePopup();
                        break;
                    }
                }
            }
        }
        const int  subIdx     = hasSubtabs ? g_subtab[g_mainTab] : 0;
        const SubtabDef& sub  = mt.subtabs[subIdx];

        const float descY = top + 20.0f;

        if (font && sub.desc && *sub.desc) {
            // Shifted +176 (160 search-box width + 16 gap) so the
            // description sits to the RIGHT of the search bar instead
            // of being overlapped by it. Keep this offset in sync with
            // sbW + a small gap in the search-bar block above.
            const ImVec2 pos = PixelSnap(ImVec2(r.x + padX + 176.0f, descY));
            dl->AddText(font, kBodyFontSize, pos, inkFaint, sub.desc);
        }

        int hoveredSub = -1;
        struct SubTabRect { float xMin, xMax; };
        SubTabRect str[kMaxSubtabs] = {};
        if (hasSubtabs) {
            float cursorR = r.x + r.w - padX;
            for (int i = mt.numSubtabs - 1; i >= 0; --i) {
                if (!font) break;
                // Guard against numSubtabs > actual subtab count — zero-init
                // empty slots have null name, which crashes CalcTextSizeA.
                if (!mt.subtabs[i].name || !*mt.subtabs[i].name) continue;
                if (IsSubtabEffectivelyEmpty(mt.subtabs[i])) continue;
                const ImVec2 sz = font->CalcTextSizeA(kBodyFontSize, FLT_MAX, 0.0f, mt.subtabs[i].name);
                str[i].xMax = cursorR;
                str[i].xMin = cursorR - sz.x;
                cursorR    -= sz.x + 16.0f;
            }
            for (int i = 0; i < mt.numSubtabs; ++i) {
                if (!mt.subtabs[i].name || !*mt.subtabs[i].name) continue;
                if (IsSubtabEffectivelyEmpty(mt.subtabs[i])) continue;
                const bool hov = mouse.x >= str[i].xMin && mouse.x <= str[i].xMax &&
                                  mouse.y >= descY        && mouse.y <= descY + 18.0f;
                if (hov) hoveredSub = i;
            }
            for (int i = 0; i < mt.numSubtabs; ++i) {
                if (!mt.subtabs[i].name || !*mt.subtabs[i].name) continue;
                if (IsSubtabEffectivelyEmpty(mt.subtabs[i])) continue;
                const bool isActive = (i == subIdx);
                const bool hov      = (i == hoveredSub);
                g_subTabHover[g_mainTab][i] = EaseTo(
                    g_subTabHover[g_mainTab][i],
                    (hov || isActive) ? 1.0f : 0.0f, 14.0f);
                const float h = g_subTabHover[g_mainTab][i];

                if (h > 0.04f && !isActive) {
                    dl->AddRectFilled(
                        ImVec2(str[i].xMin - 6.0f, descY - 3.0f),
                        ImVec2(str[i].xMax + 6.0f, descY + 18.0f),
                        IM_COL32(255, 255, 255, (BYTE)(14 * h * menuAlpha)),
                        4.0f);
                }
                const ImU32 idleCol   = inkDim;
                const ImU32 brightCol = isActive
                    ? ink
                    : IM_COL32(208, 214, 226, (BYTE)(255 * menuAlpha));
                const ImU32 col = LerpColorU32(idleCol, brightCol, h);
                if (font) {
                    const ImVec2 pos = PixelSnap(ImVec2(str[i].xMin, descY));
                    dl->AddText(font, kBodyFontSize, pos, col, mt.subtabs[i].name);
                }
            }
            if (clicked && hoveredSub >= 0 && hoveredSub != subIdx) {
                g_subtab[g_mainTab] = hoveredSub;
                g_tabSwitchTick     = now;
                if (g_popupOwnerMain >= 0) RequestClosePopup();
                clickConsumed       = true;
            }

            const float activeX  = str[subIdx].xMin;
            const float activeW  = str[subIdx].xMax - str[subIdx].xMin;
            if (g_indicatorX < 0.0f) {
                g_indicatorX = activeX;
                g_indicatorW = activeW;
            } else {
                g_indicatorX = EaseTo(g_indicatorX, activeX, kTabIndicEase);
                g_indicatorW = EaseTo(g_indicatorW, activeW, kTabIndicEase);
            }
            const float underY = descY + 19.0f;
            dl->AddRectFilled(
                ImVec2(g_indicatorX,                  underY),
                ImVec2(g_indicatorX + g_indicatorW,   underY + 1.0f),
                IM_COL32(72, 151, 255, (BYTE)(220 * menuAlpha)));
        }

        const float colGap     = 14.0f;
        const float contentTop = top + 50.0f;
        const float contentBot = r.y + r.h - 18.0f;
        const float contentL   = r.x + padX;
        const float contentR   = r.x + r.w - padX;
        const float colW       = (contentR - contentL - colGap) * 0.5f;
        const float colX[2]    = { contentL, contentL + colW + colGap };
        const ImVec2 mousePos  = ImGui::GetIO().MousePos;

        // ── Page-level scroll bookkeeping ─────────────────────────
        // Walk containers once to compute the per-column natural height
        // so we know whether to scroll the page AND how far. Mirrors the
        // render loop's height calc below — kept in sync by reusing
        // ContainerHeight(cont) + the same kMaxContainerHeight clamp.
        // Also honors the search filter so the page-scroll math matches
        // the visibly-rendered set.
        float pageColH[2] = { 0.0f, 0.0f };
        for (int ci = 0; ci < kMaxContainers; ++ci) {
            const Container& cont = sub.containers[ci];
            if (!cont.title || !*cont.title) break;
            if (IsContainerEffectivelyEmpty(cont)) continue;
            if (!ContainerHasAnyMatch(cont)) continue;
            const int   col  = (cont.col == 1) ? 1 : 0;
            const float natH = ContainerHeight(cont);
            const float visH = (natH > kMaxContainerHeight) ? kMaxContainerHeight : natH;
            pageColH[col] += visH + 10.0f;
        }
        const float pageVisH = contentBot - contentTop;
        const float pageNatH = (pageColH[0] > pageColH[1]) ? pageColH[0] : pageColH[1];
        const bool  pageScrollable = pageNatH > pageVisH + 0.5f;
        const float pageMaxScroll  = pageScrollable ? (pageNatH - pageVisH) : 0.0f;

        float& pageScroll       = g_pageScroll      [g_mainTab][subIdx];
        float& pageScrollTarget = g_pageScrollTarget[g_mainTab][subIdx];

        // Mouse-wheel routing: containers that have internal scroll consume
        // the wheel first; otherwise it scrolls the page. The container loop
        // below handles its own wheel; this section catches the leftover.
        {
            const bool hoverPage =
                mousePos.x >= contentL && mousePos.x <= contentR &&
                mousePos.y >= contentTop && mousePos.y <= contentBot;
            if (pageScrollable && hoverPage) {
                // Is the mouse over a container that's itself scrollable?
                // If so, defer to that container's wheel handler — don't
                // double-scroll. Cheap re-walk; container count is tiny.
                bool consumedByContainer = false;
                float probeY[2] = { contentTop - pageScroll, contentTop - pageScroll };
                for (int ci = 0; ci < kMaxContainers; ++ci) {
                    const Container& cc = sub.containers[ci];
                    if (!cc.title || !*cc.title) break;
                    if (IsContainerEffectivelyEmpty(cc)) continue;
                    const int col2 = (cc.col == 1) ? 1 : 0;
                    const float natH2 = ContainerHeight(cc);
                    const float visH2 = (natH2 > kMaxContainerHeight) ? kMaxContainerHeight : natH2;
                    const bool  cScrl = natH2 > visH2 + 0.5f;
                    const float bx0 = colX[col2];
                    const float by0 = probeY[col2];
                    const float bx1 = bx0 + colW;
                    const float by1 = by0 + visH2;
                    if (cScrl &&
                        mousePos.x >= bx0 && mousePos.x <= bx1 &&
                        mousePos.y >= by0 && mousePos.y <= by1) {
                        consumedByContainer = true;
                        break;
                    }
                    probeY[col2] = by1 + 10.0f;
                }
                if (!consumedByContainer) {
                    const float wheel = ImGui::GetIO().MouseWheel;
                    if (wheel != 0.0f) pageScrollTarget -= wheel * kScrollLineSize;
                }
            }
            if (pageScrollTarget < 0.0f)          pageScrollTarget = 0.0f;
            if (pageScrollTarget > pageMaxScroll) pageScrollTarget = pageMaxScroll;
            pageScroll = EaseTo(pageScroll, pageScrollTarget, 22.0f);
            if (fabsf(pageScroll - pageScrollTarget) < 0.5f) pageScroll = pageScrollTarget;
            if (pageScroll < 0.0f)          pageScroll = 0.0f;
            if (pageScroll > pageMaxScroll) pageScroll = pageMaxScroll;
        }

        // Clip the entire content area so containers scrolled past the
        // top/bottom of the page don't bleed into the header or footer.
        dl->PushClipRect(ImVec2(contentL - 6.0f, contentTop - 4.0f),
                         ImVec2(contentR + 6.0f, contentBot + 4.0f), true);

        float       colY[2]    = { contentTop - pageScroll, contentTop - pageScroll };

        const DWORD baseTick =
            (g_tabSwitchTick > g_menuOpenTick) ? g_tabSwitchTick : g_menuOpenTick;
        const DWORD menuOpenElapsed =
            (g_state == State::Menu && baseTick > 0)
                ? (now - baseTick)
                : 999999u;
        constexpr DWORD kRowStaggerMs = 30;
        constexpr DWORD kRowFadeMs    = 240;

        const bool allowClick = !clickConsumed;

        for (int ci = 0; ci < kMaxContainers; ++ci) {
            const Container& cont = sub.containers[ci];
            if (!cont.title || !*cont.title) break;
            if (IsContainerEffectivelyEmpty(cont)) continue;
            if (!ContainerHasAnyMatch(cont)) continue;

            const int col = (cont.col == 1) ? 1 : 0;
            const float natH = ContainerHeight(cont);
            const float visH = (natH > kMaxContainerHeight) ? kMaxContainerHeight : natH;
            const bool  scrollable = natH > visH + 0.5f;

            const float boxX0 = colX[col];
            const float boxY0 = colY[col];
            const float boxX1 = boxX0 + colW;
            const float boxY1 = boxY0 + visH;

            // Out-of-frustum cull (above/below the visible page) — saves
            // text rasterisation for containers that scrolled off screen
            // while still letting them stay in the column layout so the
            // page scroll math stays consistent.
            const bool culled =
                boxY1 < contentTop - 4.0f || boxY0 > contentBot + 4.0f;
            if (culled) {
                colY[col] = boxY1 + 10.0f;
                continue;
            }

            // Container reveal cascade.
            const DWORD revealDelay = (DWORD)ci * kRowStaggerMs;
            float revealA = 1.0f;
            float revealS = 0.0f;
            if (menuOpenElapsed < revealDelay) {
                revealA = 0.0f; revealS = 10.0f;
            } else if (menuOpenElapsed < revealDelay + kRowFadeMs) {
                const float u = (menuOpenElapsed - revealDelay) / (float)kRowFadeMs;
                const float t = EaseOutCubic(u);
                revealA = t; revealS = (1.0f - t) * 10.0f;
            }
            const float bxA = revealA * menuAlpha;
            if (bxA < 0.02f) {
                colY[col] = boxY1 + 10.0f;
                continue;
            }

            const ImVec2 bMin{ boxX0, boxY0 + revealS };
            const ImVec2 bMax{ boxX1, boxY1 + revealS };

            DrawContainerBox(dl, font, bMin, bMax, cont.title, bxA);

            float& scroll = g_containerScroll      [g_mainTab][subIdx][ci];
            float& target = g_containerScrollTarget[g_mainTab][subIdx][ci];
            const float maxScroll = scrollable ? (natH - visH) : 0.0f;

            const bool hoverBox =
                mousePos.x >= bMin.x && mousePos.x <= bMax.x &&
                mousePos.y >= bMin.y && mousePos.y <= bMax.y;
            if (scrollable && hoverBox) {
                const float wheel = ImGui::GetIO().MouseWheel;
                if (wheel != 0.0f) target -= wheel * kScrollLineSize;
            }
            if (target < 0.0f)      target = 0.0f;
            if (target > maxScroll) target = maxScroll;

            scroll = EaseTo(scroll, target, 22.0f);
            if (fabsf(scroll - target) < 0.5f) scroll = target;
            if (scroll < 0.0f)      scroll = 0.0f;
            if (scroll > maxScroll) scroll = maxScroll;

            const float rightInset = scrollable ? (kScrollbarWidth + 6.0f) : 0.0f;
            const ImVec2 clipMin{ bMin.x + 4.0f,                bMin.y + 28.0f };
            const ImVec2 clipMax{ bMax.x - 4.0f - rightInset,   bMax.y - 6.0f };

            dl->PushClipRect(clipMin, clipMax, true);

            float widgetY = bMin.y + 28.0f - scroll;
            for (int wi = 0; wi < kMaxWidgets; ++wi) {
                const Widget& w = cont.widgets[wi];
                if (!w.label) break;
                if (IsWidgetHidden(w)) continue;
                // Search filter — widgets pass when the widget label OR
                // its container title matches the query (so vague group
                // searches like "animal" / "guns" pull the whole group
                // through "NPCs & Animals" / "Weapons" container titles
                // via the synonym table).
                if (g_searchLen > 0 && !WidgetVisibleViaParent(w, cont)) continue;
                const float wh = WidgetHeightFor(w);

                if (widgetY + wh < clipMin.y - 2.0f ||
                    widgetY      > clipMax.y + 2.0f) {
                    widgetY += wh;
                    continue;
                }

                const ImVec2 wMin{ bMin.x + 10.0f,               widgetY };
                const ImVec2 wMax{ bMax.x - 10.0f - rightInset,  widgetY + wh - 2.0f };

                switch (w.type) {
                    case WT::Section:
                        DrawSubsectionHeader(dl, font, wMin, wMax, w.label, bxA);
                        break;
                    case WT::Check: {
                        bool& localState = g_checkState[g_mainTab][subIdx][ci][wi];
                        bool was = false;
                        // Writes-toggle reads g_writes_enabled, not the bridge —
                        // config load can't bypass the confirm each session.
                        const bool isWritesToggle =
                            (w.popupKind == PK_EnableWritesConfirm);
                        if (isWritesToggle) {
                            localState = IsWritesEnabled();
                            was = localState;
                        } else if (w.bind) {
                            localState = OverlayBridge::get_checkbox(w.bind);
                            was = localState;
                        }

                        // ── Color swatch sync ──
                        // When the checkbox carries a swatch AND has a bind
                        // path, the swatch's color is stored at <bind>/Color
                        // in the data model. Read before render so the swatch
                        // displays the current model color; write after if it
                        // changed (e.g. user dragged the picker popup).
                        std::string colorPath;
                        unsigned& swCol = g_widgetColor[g_mainTab][subIdx][ci][wi];
                        unsigned colorBefore = swCol;
                        if (w.bind && w.hasColor) {
                            colorPath = w.bind;
                            colorPath += "/Color";
                            swCol = OverlayBridge::get_color_picker(colorPath.c_str());
                            colorBefore = swCol;
                        }

                        const bool cons = isWritesToggle
                            ? DrawWritesToggleBig(
                                dl, font, wMin, wMax, w,
                                g_mainTab, subIdx, ci, wi,
                                localState,
                                g_checkAnim  [g_mainTab][subIdx][ci][wi],
                                bxA, allowClick && !clickConsumed)
                            : DrawCheckboxRow(
                                dl, font, wMin, wMax, w,
                                g_mainTab, subIdx, ci, wi,
                                localState,
                                g_checkAnim  [g_mainTab][subIdx][ci][wi],
                                swCol,
                                bxA, allowClick && !clickConsumed);
                        if (isWritesToggle) {
                            if (!was && localState) {
                                localState = false;
                                g_confirmAction = 5;
                                OpenPopup(g_mainTab, subIdx, ci, wi, PK_Confirm,
                                           MenuCenter());
                                clickConsumed = true;
                            }
                            else if (was && !localState) {
                                SetWritesEnabled(false);
                                if (w.bind) OverlayBridge::set_checkbox(w.bind, false);
                            }
                        } else if (w.bind && localState != was) {
                            OverlayBridge::set_checkbox(w.bind, localState);
                        }
                        if (w.bind && w.hasColor && swCol != colorBefore)
                            OverlayBridge::set_color_picker(colorPath.c_str(), swCol);
                        if (cons) clickConsumed = true;
                        break;
                    }
                    case WT::Slider: {
                        // Bind path (when present) routes the slider value
                        // through OverlayBridge so it lands in the typed
                        // data model that features read. Prefix selects type:
                        //   "f:..."  → gui::Slider<float>
                        //   "i:..."  → gui::Slider<int>
                        //   anything else / nullptr → UI-local only
                        Widget effW = w;

                        // ── Smoothing ↔ Hit chance swap ──
                        // The Humanization>Smoothing slider doubles as the
                        // silent-aim Hit chance control: when "Silent aim"
                        // is on, the same UI slot retargets to the Hit
                        // chance binding (and shows %), so the user has one
                        // slider that means whatever's actually active.
                        if (effW.label && std::string_view(effW.label) == "Smoothing" &&
                            OverlayBridge::get_checkbox("Aimbot/General/Main/Silent aim"))
                        {
                            effW.label  = "Hit chance";
                            effW.minV   = 0.0f;
                            effW.maxV   = 100.0f;
                            effW.defV   = 100.0f;
                            effW.suffix = "%";
                            effW.bind   = "i:Aimbot/General/Main/Silent aim/Options/Hit chance";
                        }

                        float& localVal = g_sliderValue[g_mainTab][subIdx][ci][wi];
                        bool boundFloat = false, boundInt = false;
                        const char* path = nullptr;
                        if (effW.bind && effW.bind[0] == 'f' && effW.bind[1] == ':') {
                            boundFloat = true; path = effW.bind + 2;
                            localVal = OverlayBridge::get_slider_f32(path);
                        } else if (effW.bind && effW.bind[0] == 'i' && effW.bind[1] == ':') {
                            boundInt = true;   path = effW.bind + 2;
                            localVal = (float)OverlayBridge::get_slider_i32(path);
                        }
                        const bool cons = DrawSliderRow(
                            dl, font, g_mainTab, subIdx, ci, wi, wMin, wMax,
                            effW, localVal,
                            bxA, allowClick && !clickConsumed);
                        if (boundFloat) OverlayBridge::set_slider_f32(path, localVal);
                        else if (boundInt) OverlayBridge::set_slider_i32(path, (int)localVal);
                        if (cons) clickConsumed = true;
                        break;
                    }
                    case WT::Dropdown: {
                        const bool cons = DrawDropdownRow(
                            dl, font, wMin, wMax, w,
                            bxA, allowClick && !clickConsumed);
                        if (cons) clickConsumed = true;
                        break;
                    }
                    case WT::Flags: {
                        const bool cons = DrawFlagsDropdown(
                            dl, font, wMin, wMax, w,
                            g_mainTab, subIdx, ci, wi,
                            bxA, allowClick && !clickConsumed);
                        if (cons) clickConsumed = true;
                        break;
                    }
                    case WT::Keybind: {
                        const bool cons = DrawKeybindRow(
                            dl, font, wMin, wMax, w,
                            g_mainTab, subIdx, ci, wi,
                            bxA, allowClick && !clickConsumed);
                        if (cons) clickConsumed = true;
                        break;
                    }
                    case WT::Listbox: {
                        const bool cons = DrawListboxRow(
                            dl, font, wMin, wMax,
                            g_mainTab, subIdx, ci, wi,
                            bxA, allowClick && !clickConsumed);
                        if (cons) clickConsumed = true;
                        break;
                    }
                    case WT::TextInput: {
                        const bool cons = DrawTextInputRow(
                            dl, font, wMin, wMax, w,
                            g_mainTab, subIdx, ci, wi,
                            bxA, allowClick && !clickConsumed);
                        if (cons) clickConsumed = true;
                        break;
                    }
                    case WT::ConfirmBtn: {
                        const bool cons = DrawConfirmBtnRow(
                            dl, font, wMin, wMax, w,
                            g_mainTab, subIdx, ci, wi,
                            bxA, allowClick && !clickConsumed);
                        if (cons) clickConsumed = true;
                        break;
                    }
                }
                widgetY += wh;
            }

            dl->PopClipRect();

            if (scrollable) {
                const float sbX  = bMax.x - 4.0f - kScrollbarWidth;
                const float sbY0 = clipMin.y + 2.0f;
                const float sbY1 = clipMax.y - 2.0f;
                const float trackH = sbY1 - sbY0;
                const float thumbH = (trackH * (visH / natH));
                const float thumbY = sbY0 + (trackH - thumbH) *
                                       (maxScroll > 0 ? (scroll / maxScroll) : 0.0f);
                dl->AddRectFilled(
                    ImVec2(sbX, sbY0),
                    ImVec2(sbX + kScrollbarWidth, sbY1),
                    IM_COL32(255, 255, 255, (BYTE)(14 * bxA)),
                    kScrollbarWidth * 0.5f);
                dl->AddRectFilled(
                    ImVec2(sbX, thumbY),
                    ImVec2(sbX + kScrollbarWidth, thumbY + thumbH),
                    IM_COL32(180, 188, 204, (BYTE)(170 * bxA)),
                    kScrollbarWidth * 0.5f);
            }

            colY[col] = boxY1 + revealS + 10.0f;
        }

        dl->PopClipRect();   // matches the content-area clip pushed above

        // ── Page-level scrollbar ──────────────────────────────────
        // Sits flush with the right edge of the page content area. Hidden
        // when the page fits without scroll.
        if (pageScrollable) {
            const float sbX  = contentR + 2.0f;
            const float sbY0 = contentTop;
            const float sbY1 = contentBot;
            const float trackH = sbY1 - sbY0;
            const float thumbH = trackH * (pageVisH / pageNatH);
            const float thumbY = sbY0 + (trackH - thumbH) *
                (pageMaxScroll > 0.0f ? (pageScroll / pageMaxScroll) : 0.0f);
            dl->AddRectFilled(
                ImVec2(sbX, sbY0),
                ImVec2(sbX + kScrollbarWidth, sbY1),
                IM_COL32(255, 255, 255, (BYTE)(18 * menuAlpha)),
                kScrollbarWidth * 0.5f);
            dl->AddRectFilled(
                ImVec2(sbX, thumbY),
                ImVec2(sbX + kScrollbarWidth, thumbY + thumbH),
                IM_COL32(180, 188, 204, (BYTE)(200 * menuAlpha)),
                kScrollbarWidth * 0.5f);
        }

        const bool popupConsumed = DrawPopup(dl, font, menuAlpha);
        if (popupConsumed) clickConsumed = true;
        if (clicked && !clickConsumed && g_popupOwnerMain >= 0 &&
            g_popupTarget > 0.5f) {
            RequestClosePopup();
        }
    }

    static void DrawSurface(ImDrawList* dl, const ShapeRect& r) {
        const float chromeFactor = g_anim;
        const float pillFactor   = 1.0f - g_anim;

        if (pillFactor > 0.01f) {
            for (int i = 2; i >= 0; --i) {
                const float infl = (i + 1) * 2.0f;
                const float yOff = 1.5f + (float)i * 1.0f;
                const float fall = 1.0f - (float)i / 3.0f;
                const BYTE  a    = (BYTE)(60.0f * fall * fall * pillFactor);
                if (a == 0) continue;
                dl->AddRectFilled(
                    ImVec2(r.x - infl,       r.y - infl + yOff),
                    ImVec2(r.x + r.w + infl, r.y + r.h + infl + yOff),
                    IM_COL32(0, 2, 10, a),
                    r.radius + infl);
            }
            for (int i = 2; i >= 0; --i) {
                const float infl = 1.0f + (float)i * 1.5f;
                const BYTE  a    = (BYTE)(22.0f * pillFactor / (i + 1));
                if (a == 0) continue;
                dl->AddRect(
                    ImVec2(r.x - infl,       r.y - infl),
                    ImVec2(r.x + r.w + infl, r.y + r.h + infl),
                    IM_COL32(72, 151, 255, a),
                    r.radius + infl, 0, 1.0f);
            }
        }

        if (chromeFactor > 0.01f) {
            for (int i = 5; i >= 0; --i) {
                const float infl = (i + 1) * 3.5f;
                const float yOff = 4.0f + (float)i * 1.8f;
                const float fall = 1.0f - (float)i / 6.0f;
                const BYTE  a    = (BYTE)(70.0f * fall * fall * chromeFactor);
                if (a == 0) continue;
                dl->AddRectFilled(
                    ImVec2(r.x - infl,       r.y - infl + yOff),
                    ImVec2(r.x + r.w + infl, r.y + r.h + infl + yOff),
                    IM_COL32(0, 0, 0, a),
                    r.radius + infl);
            }
        }

        const BYTE  baseAlpha = (BYTE)(252.0f - 74.0f * chromeFactor);
        const ImU32 baseCol   = IM_COL32(11, 11, 17, baseAlpha);
        if (r.radius >= r.h * 0.5f - 0.5f && r.w >= r.h) {
            const float cr = r.h * 0.5f;
            const int   eseg = 48;
            dl->AddCircleFilled(ImVec2(r.x + cr,       r.y + cr), cr, baseCol, eseg);
            dl->AddCircleFilled(ImVec2(r.x + r.w - cr, r.y + cr), cr, baseCol, eseg);
            dl->AddRectFilled(
                ImVec2(r.x + cr, r.y),
                ImVec2(r.x + r.w - cr, r.y + r.h),
                baseCol);
        } else {
            dl->AddRectFilled(
                ImVec2(r.x,       r.y),
                ImVec2(r.x + r.w, r.y + r.h),
                baseCol, r.radius);
        }

        if (pillFactor > 0.01f) {
            const ImU32 borderCol = IM_COL32(72, 151, 255, (BYTE)(120 * pillFactor));
            const ImU32 sheenCol  = IM_COL32(255, 255, 255, (BYTE)(28 * pillFactor));
            const bool  isPill    = (r.radius >= r.h * 0.5f - 0.5f && r.w >= r.h);

            if (isPill) {
                const float cr    = r.h * 0.5f;
                const int   eseg  = 32;
                dl->PathClear();
                dl->PathArcTo(ImVec2(r.x + cr,       r.y + cr), cr - 0.5f,
                              IM_PI * 0.5f, IM_PI * 1.5f, eseg);
                dl->PathStroke(borderCol, ImDrawFlags_None, 1.0f);
                dl->PathClear();
                dl->PathArcTo(ImVec2(r.x + r.w - cr, r.y + cr), cr - 0.5f,
                              -IM_PI * 0.5f, IM_PI * 0.5f, eseg);
                dl->PathStroke(borderCol, ImDrawFlags_None, 1.0f);
                dl->AddLine(
                    ImVec2(r.x + cr,       r.y + 0.5f),
                    ImVec2(r.x + r.w - cr, r.y + 0.5f),
                    borderCol, 1.0f);
                dl->AddLine(
                    ImVec2(r.x + cr,       r.y + r.h - 0.5f),
                    ImVec2(r.x + r.w - cr, r.y + r.h - 0.5f),
                    borderCol, 1.0f);

                const float ir = cr - 1.5f;
                dl->PathClear();
                dl->PathArcTo(ImVec2(r.x + cr,       r.y + cr), ir,
                              IM_PI * 0.5f, IM_PI * 1.5f, eseg);
                dl->PathStroke(sheenCol, ImDrawFlags_None, 1.0f);
                dl->PathClear();
                dl->PathArcTo(ImVec2(r.x + r.w - cr, r.y + cr), ir,
                              -IM_PI * 0.5f, IM_PI * 0.5f, eseg);
                dl->PathStroke(sheenCol, ImDrawFlags_None, 1.0f);
                dl->AddLine(
                    ImVec2(r.x + cr,       r.y + 1.5f),
                    ImVec2(r.x + r.w - cr, r.y + 1.5f),
                    sheenCol, 1.0f);
                dl->AddLine(
                    ImVec2(r.x + cr,       r.y + r.h - 1.5f),
                    ImVec2(r.x + r.w - cr, r.y + r.h - 1.5f),
                    sheenCol, 1.0f);
            } else {
                dl->AddRect(
                    ImVec2(r.x,       r.y),
                    ImVec2(r.x + r.w, r.y + r.h),
                    borderCol, r.radius, 0, 1.0f);
                dl->AddRect(
                    ImVec2(r.x + 1.5f,         r.y + 1.5f),
                    ImVec2(r.x + r.w - 1.5f,   r.y + r.h - 1.5f),
                    sheenCol, r.radius - 1.5f, 0, 1.0f);
            }
        }

        if (chromeFactor > 0.01f) {
            dl->AddRect(
                ImVec2(r.x,       r.y),
                ImVec2(r.x + r.w, r.y + r.h),
                IM_COL32(255, 255, 255, (BYTE)(46 * chromeFactor)),
                r.radius, 0, 1.0f);

            const float stripY = r.y + 1.5f;
            const float stripL = r.x + r.radius * 0.8f;
            const float stripR = r.x + r.w - r.radius * 0.8f;
            dl->AddRectFilled(
                ImVec2(stripL, stripY),
                ImVec2(stripR, stripY + 1.0f),
                IM_COL32(72, 151, 255, (BYTE)(140 * chromeFactor)));
        }
    }

    void Render() {
        const ImVec2 disp = ImGui::GetIO().DisplaySize;
        UpdateState(disp);

        g_popupAnim = EaseTo(g_popupAnim, g_popupTarget, 18.0f);
        if (g_popupAnim < 0.02f && g_popupTarget == 0.0f && g_popupOwnerMain >= 0) {
            g_popupOwnerMain = g_popupOwnerSub = -1;
            g_popupOwnerCont = g_popupOwnerWid = -1;
            g_popupKind      = PK_None;
            g_popupAnim      = 0.0f;
            g_popupDragSlot  = -1;
            g_pickerDrag     = 0;
        }
        if (g_state == State::Pill && g_popupOwnerMain >= 0) {
            RequestClosePopup();
        }

        ImDrawList* bg = ImGui::GetBackgroundDrawList();
        const ShapeRect& r = g_shape;

        DrawSurface(bg, r);

        const float pillAlpha = 1.0f - ImClamp(g_anim * 2.5f, 0.0f, 1.0f);
        const float menuAlpha = ImClamp((g_anim - 0.4f) * 1.8f, 0.0f, 1.0f);
        if (pillAlpha > 0.02f) DrawPillContent(bg, r, pillAlpha);

        // Reset before menu draw so stale hover state doesn't linger one
        // frame when the cursor leaves the badge.
        g_riskTooltip.active = false;
        if (menuAlpha > 0.02f) DrawMenuContent(bg, r, menuAlpha);

        // HIGH-RISK tooltip painted last → above every other widget/popup.
        if (g_riskTooltip.active && g_riskTooltip.text) {
            ImFont* font = Render::Fonts::montserrat14px;
            if (font) {
                const float padX = 8.0f, padY = 5.0f;
                const ImVec2 sz = font->CalcTextSizeA(
                    kBodyFontSize, FLT_MAX, 0.0f, g_riskTooltip.text);
                ImVec2 tipMin(g_riskTooltip.anchor.x,
                              g_riskTooltip.anchor.y - sz.y * 0.5f - padY);
                ImVec2 tipMax(tipMin.x + sz.x + padX * 2.0f,
                              tipMin.y + sz.y + padY * 2.0f);

                // Clamp into display so the tooltip never bleeds offscreen.
                if (tipMax.x > disp.x - 4.0f) {
                    const float shift = tipMax.x - (disp.x - 4.0f);
                    tipMin.x -= shift; tipMax.x -= shift;
                }
                if (tipMin.y < 4.0f) {
                    const float shift = 4.0f - tipMin.y;
                    tipMin.y += shift; tipMax.y += shift;
                }

                const float a = g_riskTooltip.alpha;
                bg->AddRectFilled(tipMin, tipMax,
                                  IM_COL32(22, 22, 28, (BYTE)(238 * a)), 5.0f);
                bg->AddRect      (tipMin, tipMax,
                                  IM_COL32(235, 64, 64, (BYTE)(220 * a)),
                                  5.0f, 0, 1.0f);
                bg->AddText(font, kBodyFontSize,
                            PixelSnap(ImVec2(tipMin.x + padX, tipMin.y + padY)),
                            IM_COL32(240, 240, 245, (BYTE)(255 * a)),
                            g_riskTooltip.text);
            }
        }
    }

}  // namespace OverlayUI
