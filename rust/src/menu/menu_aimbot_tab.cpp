#include <pawjob_umbrella.hpp>
#include <settings/settings.hpp>
#include <menu/menu.hpp>

void Menu::InitAimbotTab(CTab* tab)
{
    static const auto writesGate = []() { return settings.settings.ui.enable_memory_writes; };

    if (auto General = tab->AddObject<CSubtab>(X("General")))
    {
        if (auto Container = General->AddObject<CContainer>(X("General")))
        {
            Container->SetVisibleCondition(writesGate);
            auto memoryAim = Container->AddObject<CCheckbox>(X("Memory aim"), &settings.aimbot.general.main.enabled);
            if (auto popup = memoryAim->AddObject<CPopup>())
            {
                popup->AddObject<CSliderFloat>(X("Humanized smoothing"), &settings.aimbot.general.humanization.smoothing, 0.0f, 10.0f, X("{:.1f}"));
                popup->AddObject<CSliderInt>(X("Max distance"), &settings.aimbot.general.main.max_distance, 0, 1000, X("{}m"));
            }

            auto silentAim = Container->AddObject<CCheckbox>(X("Silent aim"), &settings.aimbot.general.main.silent_aim);
            if (auto popup = silentAim->AddObject<CPopup>())
            {
                popup->AddObject<CSliderInt>(X("Hit chance"), &settings.aimbot.general.main.silent_aim_hit_chance, 1, 100, X("{}%%"));
            }

            {
                static const std::vector<std::string> kBones = {
                    X("Head"), X("Neck"), X("Chest"), X("Stomach"), X("Pelvis"), X("Knee")
                };
                Container->AddObject<CDropdown>(X("Target bone"), &settings.aimbot.general.main.target_bone, kBones);
            }

            Container->AddObject<CCheckbox>(X("Smart target"), &settings.aimbot.general.main.smart_target);
            Container->AddObject<CCheckbox>(X("Target 360"), &settings.aimbot.general.main.target_360);

            auto autoShoot = Container->AddObject<CCheckbox>(X("Auto shoot"), &settings.aimbot.general.main.auto_shoot);
            if (auto popup = autoShoot->AddObject<CPopup>())
            {
                popup->AddObject<CCheckbox>(X("Also enable Target 360"), &settings.aimbot.general.main.target_360);
                popup->AddObject<CCheckbox>(X("Silent aim"), &settings.aimbot.general.main.silent_aim);
            }

            auto prediction = Container->AddObject<CCheckbox>(X("Prediction"), &settings.aimbot.general.main.prediction);
            if (auto popup = prediction->AddObject<CPopup>())
            {
                popup->AddObject<CSliderFloat>(X("Lead"), &settings.aimbot.general.main.prediction_strength, 0.0f, 2.0f, X("{:.2f}"));
                popup->AddObject<CSliderFloat>(X("Drop"), &settings.aimbot.general.main.prediction_drop, 0.0f, 2.0f, X("{:.2f}"));
                popup->AddObject<CSliderInt>(X("Ping compensation"), &settings.aimbot.general.main.prediction_ping_comp, 0, 250, X("{}ms"));
            }

            auto path = Container->AddObject<CCheckbox>(X("Prediction path"), &settings.aimbot.visualization.prediction_path);
            path->AddObject<CColorPicker>(X("Color"), &settings.aimbot.visualization.prediction_path_color);

            auto fov = Container->AddObject<CCheckbox>(X("Fov"), &settings.aimbot.visualization.fov_circle);
            fov->AddObject<CColorPicker>(X("Color"), &settings.aimbot.visualization.fov_circle_color);
            if (auto popup = fov->AddObject<CPopup>())
            {
                popup->AddObject<CSliderInt>(X("Size"), &settings.aimbot.visualization.fov_circle_size, 25, 500, X("{}px"));
                auto line = popup->AddObject<CCheckbox>(X("Target line"), &settings.aimbot.visualization.fov_circle_target_line);
                line->AddObject<CColorPicker>(X("Color"), &settings.aimbot.visualization.fov_circle_target_line_color);
            }

            auto filters = Container->AddObject<CCheckbox>(X("Filters"), &settings.aimbot.general.prerequisites.scale_by_distance);
            if (auto popup = filters->AddObject<CPopup>())
            {
                popup->AddObject<CCheckbox>(X("Skip teammates"),  &settings.aimbot.general.filters.skip_teammates);
                popup->AddObject<CCheckbox>(X("Skip scientists"), &settings.aimbot.general.filters.skip_scientists);
                popup->AddObject<CCheckbox>(X("Skip dwellers"),   &settings.aimbot.general.filters.skip_dwellers);
                popup->AddObject<CCheckbox>(X("Skip sleepers"),   &settings.aimbot.general.filters.skip_sleepers);
                popup->AddObject<CCheckbox>(X("Skip wounded"),    &settings.aimbot.general.filters.skip_wounded);
                popup->AddObject<CCheckbox>(X("Skip animals"),    &settings.aimbot.general.filters.skip_animals);
            }

            Container->AddObject<CKeybind>(X("Aim key"), &settings.aimbot.aim_key);
        }
    }

    if (auto Combat = tab->AddObject<CSubtab>(X("Combat")))
    {
        if (auto Container = Combat->AddObject<CContainer>(X("Combat")))
        {
            Container->SetVisibleCondition(writesGate);
            Container->AddObject<CCheckbox>(X("Automatic Weapons"), &settings.aimbot.weapons.automatic_weapons);
            Container->AddObject<CCheckbox>(X("Instant bow"), &settings.aimbot.weapons.instant_bow);

            auto instantEoka = Container->AddObject<CCheckbox>(X("Instant Eoka"), &settings.aimbot.weapons.instant_eoka);
            if (auto popup = instantEoka->AddObject<CPopup>())
            {
                popup->AddObject<CSliderInt>(X("Strike chance"), &settings.aimbot.weapons.instant_eoka_strikechance, 1, 100, X("{}%%"));
            }

            Container->AddObject<CCheckbox>(X("Rapid fire"), &settings.aimbot.weapons.rapid_fire);
            Container->AddObject<CCheckbox>(X("No sway"), &settings.aimbot.weapons.no_sway);
            Container->AddObject<CCheckbox>(X("No animation"), &settings.aimbot.weapons.no_animation);
            Container->AddObject<CCheckbox>(X("Thick bullet"), &settings.aimbot.weapons.thick_bullet);

            auto hitbox = Container->AddObject<CCheckbox>(X("Hitbox override"), &settings.aimbot.weapons.hitbox_override);
            if (auto popup = hitbox->AddObject<CPopup>())
            {
                static const std::vector<std::string> kHitboxBones = {
                    X("Head"), X("Neck"), X("Chest"), X("Stomach"), X("Pelvis"), X("Hip")
                };
                popup->AddObject<CDropdown>(X("Bone"), &settings.aimbot.weapons.hitbox_override_bone, kHitboxBones);
            }

            Container->AddObject<CCheckbox>(X("No viewmodel lower"), &settings.aimbot.weapons.no_viewmodel_lower);
            Container->AddObject<CCheckbox>(X("Hide viewmodel"), &settings.aimbot.weapons.hide_viewmodel);

            Container->AddObject<CCheckbox>(X("Fast Shoot"), &settings.aimbot.weapons.fast_shoot);
            Container->AddObject<CCheckbox>(X("Hit Material Override"), &settings.aimbot.weapons.hit_material_override);
        }
    }

    if (auto Overrides = tab->AddObject<CSubtab>(X("Overrides")))
    {
        if (auto Container = Overrides->AddObject<CContainer>(X("Overrides")))
        {
            Container->SetVisibleCondition(writesGate);
            auto spread = Container->AddObject<CCheckbox>(X("Weapon Spread"), &settings.aimbot.weapons.override_weapon_spread);
            if (auto popup = spread->AddObject<CPopup>())
            {
                popup->AddObject<CSliderInt>(X("Amount"), &settings.aimbot.weapons.override_weapon_spread_amount, 0, 100, X("{}%"));
            }

            auto recoil = Container->AddObject<CCheckbox>(X("Override Recoil"), &settings.aimbot.weapons.override_weapon_recoil);
            if (auto popup = recoil->AddObject<CPopup>())
            {
                popup->AddObject<CSliderInt>(X("Amount"), &settings.aimbot.weapons.override_weapon_recoil_amount, 0, 100, X("{}%"));
            }

            auto melee = Container->AddObject<CCheckbox>(X("Override Melee"), &settings.aimbot.weapons.override_melee_range);
            if (auto popup = melee->AddObject<CPopup>())
            {
                popup->AddObject<CSliderInt>(X("Amount"), &settings.aimbot.weapons.override_melee_range_amount, 100, 200, X("{}%%"));
            }
        }
    }

    static const char* kGroupNames[7] = {
        "Rifles", "Snipers", "Shotguns", "Pistols", "Bows", "LMGs", "SMGs"
    };

    for (int gi = 0; gi < 7; ++gi)
    {
        auto& group = settings.aimbot.groups[gi];
        if (auto Page = tab->AddObject<CSubtab>(X(kGroupNames[gi])))
        {
            if (auto Container = Page->AddObject<CContainer>(X(kGroupNames[gi])))
            {
                Container->SetVisibleCondition(writesGate);
                auto enabled = Container->AddObject<CCheckbox>(X("Override general"), &group.main.enabled);
                if (auto popup = enabled->AddObject<CPopup>())
                {
                    popup->AddObject<CSliderInt>(X("Max distance"), &group.main.max_distance, 0, 1000, X("{}m"));
                }

                auto silent = Container->AddObject<CCheckbox>(X("Silent aim"), &group.main.silent_aim);
                if (auto popup = silent->AddObject<CPopup>())
                {
                    popup->AddObject<CSliderInt>(X("Hit chance"), &group.main.silent_aim_hit_chance, 1, 100, X("{}%%"));
                }

                {
                    static const std::vector<std::string> kBones = {
                        X("Head"), X("Neck"), X("Chest"), X("Stomach"), X("Pelvis"), X("Knee")
                    };
                    Container->AddObject<CDropdown>(X("Target bone"), &group.main.target_bone, kBones);
                }

                Container->AddObject<CCheckbox>(X("Smart target"), &group.main.smart_target);
                Container->AddObject<CCheckbox>(X("Target 360"), &group.main.target_360);
                Container->AddObject<CCheckbox>(X("Auto shoot"), &group.main.auto_shoot);

                auto prediction = Container->AddObject<CCheckbox>(X("Prediction"), &group.main.prediction);
                if (auto popup = prediction->AddObject<CPopup>())
                {
                    popup->AddObject<CSliderFloat>(X("Lead"), &group.main.prediction_strength, 0.0f, 2.0f, X("{:.2f}"));
                    popup->AddObject<CSliderFloat>(X("Drop"), &group.main.prediction_drop, 0.0f, 2.0f, X("{:.2f}"));
                    popup->AddObject<CSliderInt>(X("Ping compensation"), &group.main.prediction_ping_comp, 0, 250, X("{}ms"));
                }

                Container->AddObject<CSliderFloat>(X("Smoothing"), &group.humanization.smoothing, 0.0f, 10.0f, X("{:.1f}"));

                auto filters = Container->AddObject<CCheckbox>(X("Filters"), &group.prerequisites.scale_by_distance);
                if (auto popup = filters->AddObject<CPopup>())
                {
                    popup->AddObject<CCheckbox>(X("Skip teammates"),  &group.filters.skip_teammates);
                    popup->AddObject<CCheckbox>(X("Skip scientists"), &group.filters.skip_scientists);
                    popup->AddObject<CCheckbox>(X("Skip dwellers"),   &group.filters.skip_dwellers);
                    popup->AddObject<CCheckbox>(X("Skip sleepers"),   &group.filters.skip_sleepers);
                    popup->AddObject<CCheckbox>(X("Skip wounded"),    &group.filters.skip_wounded);
                    popup->AddObject<CCheckbox>(X("Skip animals"),    &group.filters.skip_animals);
                }
            }
        }
    }
}
