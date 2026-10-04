#include <pawjob_umbrella.hpp>
#include <settings/settings.hpp>
#include <menu/menu.hpp>
#include <sdk/globals.hpp>

void Menu::InitMiscTab(CTab* tab)
{
    static const auto writesGate = []() { return settings.settings.ui.enable_memory_writes; };

    if (auto General = tab->AddObject<CSubtab>(X("General")))
    {
        if (auto Container = General->AddObject<CContainer>(X("General")))
        {
            Container->SetVisibleCondition(writesGate);
            Container->AddObject<CCheckbox>(X("Fast loot"), &settings.misc.general.fast_loot);
            Container->AddObject<CCheckbox>(X("Instant interactions"), &settings.misc.general.instant_interactions);

            auto revive = Container->AddObject<CCheckbox>(X("Instant revive"), &settings.misc.general.instant_revive);
            revive->AddObject<CKeybind>(X("Key"), &settings.misc.general.instant_revive_key);

            Container->AddObject<CCheckbox>(X("Quick underwater untie"), &settings.misc.general.instant_untie_crate);

            auto debugEsp = Container->AddObject<CCheckbox>(X("Debug ESP"), &settings.misc.general.debug_esp);
            debugEsp->AddObject<CColorPicker>(X("Color"), &settings.misc.general.debug_esp_color);
        }
    }

    if (auto Movement = tab->AddObject<CSubtab>(X("Movement")))
    {
        if (auto Container = Movement->AddObject<CContainer>(X("Movement")))
        {
            Container->SetVisibleCondition(writesGate);
            auto spider = Container->AddObject<CCheckbox>(X("Spiderman"), &settings.misc.movement.spider_man);
            spider->AddObject<CKeybind>(X("Key"), &settings.misc.movement.spider_man_key);

            auto fly = Container->AddObject<CCheckbox>(X("Fly"), &settings.misc.movement.fly);
            fly->AddObject<CKeybind>(X("Key"), &settings.misc.movement.fly_key);

            Container->AddObject<CCheckbox>(X("Silent walk"), &settings.misc.movement.silent_walk);
            Container->AddObject<CCheckbox>(X("Speed hack"), &settings.misc.movement.speed_hack);
            Container->AddObject<CCheckbox>(X("Omni sprint"), &settings.misc.movement.omni_sprint);
            Container->AddObject<CCheckbox>(X("NoFall"), &settings.misc.movement.no_fall);
            Container->AddObject<CCheckbox>(X("Remove Water drag"), &settings.misc.movement.remove_water_drag);
            Container->AddObject<CCheckbox>(X("Walk on Water"), &settings.misc.movement.walk_on_water);
            Container->AddObject<CCheckbox>(X("Anti Aim"), &settings.misc.movement.anti_aim);
        }
    }

    if (auto Camera = tab->AddObject<CSubtab>(X("Camera")))
    {
        if (auto Container = Camera->AddObject<CContainer>(X("Camera")))
        {
            Container->SetVisibleCondition(writesGate);
            auto fov = Container->AddObject<CCheckbox>(X("FOV changer"), &settings.visuals.local.custom_fov);
            if (auto popup = fov->AddObject<CPopup>())
            {
                popup->AddObject<CSliderFloat>(X("Field of view"), &settings.visuals.local.field_of_view, 60.0f, 120.0f, X("{:.0f}"));
                popup->AddObject<CSliderFloat>(X("Zoom FOV"),      &settings.visuals.local.zoom_fov,      20.0f,  90.0f, X("{:.0f}"));
                popup->AddObject<CKeybind>(X("Zoom key"),          &settings.visuals.local.zoom_key);
            }

            Container->AddObject<CCheckbox>(X("Third Person"), &settings.visuals.local.third_person);

            Container->AddObject<CDropdown>(X("Weapon chams"), &settings.visuals.local.weapon_chams, g_material_names);
            Container->AddObject<CDropdown>(X("Arm chams"),    &settings.visuals.local.arm_chams,    g_material_names);

            auto tracers = Container->AddObject<CCheckbox>(X("Bullet tracers"), &settings.visuals.local.bullet_tracers);
            tracers->AddObject<CColorPicker>(X("Color"), &settings.visuals.local.bullet_tracers_color);

            auto debugCam = Container->AddObject<CCheckbox>(X("Debug Camera"), &settings.visuals.local.debug_camera);
            debugCam->AddObject<CKeybind>(X("Toggle key"), &settings.visuals.local.debug_camera_key);
            if (auto popup = debugCam->AddObject<CPopup>())
            {
                popup->AddObject<CSliderFloat>(X("Move speed"), &settings.visuals.local.debug_camera_speed, 1.0f, 50.0f, X("{:.1f}"));
                popup->AddObject<CSliderFloat>(X("Mouse sensitivity"), &settings.visuals.local.mouse_sensitivity, 10.0f, 200.0f, X("{:.0f}"));
            }

            auto layers = Container->AddObject<CCheckbox>(X("Layer toggle"), &settings.misc.layer_toggle.enabled);
            layers->AddObject<CKeybind>(X("Toggle key"), &settings.misc.layer_toggle.key);
            if (auto popup = layers->AddObject<CPopup>())
            {
                popup->AddObject<CCheckbox>(X("Construction"), &settings.misc.layer_toggle.hide_construction);
                popup->AddObject<CCheckbox>(X("Transparent"),  &settings.misc.layer_toggle.hide_transparent);
                popup->AddObject<CCheckbox>(X("Debris"),       &settings.misc.layer_toggle.hide_debris);
                popup->AddObject<CCheckbox>(X("Default"),      &settings.misc.layer_toggle.hide_default);
                popup->AddObject<CCheckbox>(X("Deployed"),     &settings.misc.layer_toggle.hide_deployed);
                popup->AddObject<CCheckbox>(X("Ragdoll"),      &settings.misc.layer_toggle.hide_ragdoll);
                popup->AddObject<CCheckbox>(X("Terrain"),      &settings.misc.layer_toggle.hide_terrain);
                popup->AddObject<CCheckbox>(X("Tree"),         &settings.misc.layer_toggle.hide_tree);
                popup->AddObject<CCheckbox>(X("World"),        &settings.misc.layer_toggle.hide_world);
                popup->AddObject<CCheckbox>(X("Water"),        &settings.misc.layer_toggle.hide_water);
                popup->AddObject<CCheckbox>(X("Clutter"),      &settings.misc.layer_toggle.hide_clutter);
            }
        }
    }

    if (auto World = tab->AddObject<CSubtab>(X("World")))
    {
        if (auto Container = World->AddObject<CContainer>(X("World")))
        {
            Container->SetVisibleCondition(writesGate);
            auto time = Container->AddObject<CCheckbox>(X("Time changer"), &settings.visuals.world.time_changer);
            if (auto popup = time->AddObject<CPopup>())
            {
                popup->AddObject<CSliderFloat>(X("Time"), &settings.visuals.world.time, 0.0f, 24.0f, X("{:.1f}h"));
            }

            auto mod = Container->AddObject<CCheckbox>(X("Full World Modulation"), &settings.visuals.world.full_world_modulation);
            if (auto popup = mod->AddObject<CPopup>())
            {
                static const char* kLabels[7] = {
                    "Sun", "Lights", "Rays", "Sky", "Clouds", "Fog", "Ambience"
                };
                for (int i = 0; i < 7; ++i)
                {
                    auto& m = settings.visuals.world.modulates[i];
                    auto row = popup->AddObject<CCheckbox>(X(kLabels[i]), &m.enabled);
                    row->AddObject<CColorPicker>(X("Color"), &m.color);
                }
            }

            Container->AddObject<CCheckbox>(X("Bright night"), &settings.visuals.world.bright_night);
        }
    }
}
