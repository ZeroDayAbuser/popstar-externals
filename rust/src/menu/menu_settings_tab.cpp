#include <pawjob_umbrella.hpp>
#include <settings/settings.hpp>
#include <menu/menu.hpp>

void Menu::InitSettingsTab(CTab* tab)
{
    if (auto UISub = tab->AddObject<CSubtab>(X("UI")))
    {
        if (auto UI = UISub->AddObject<CContainer>(X("UI")))
        {
            UI->AddObject<CCheckbox>(X("Vertical sync"), &settings.settings.ui.vertical_sync);

            auto accent = UI->AddObject<CCheckbox>(X("Override accent"), &settings.settings.ui.override_accent);
            if (auto popup = accent->AddObject<CPopup>())
            {
                popup->AddObject<CColorPicker>(X("Color"), &settings.settings.ui.accent_color);
            }

            UI->AddObject<CCheckbox>(X("Enable memory writes"), &settings.settings.ui.enable_memory_writes);
        }
    }

    if (auto ConfigsSub = tab->AddObject<CSubtab>(X("Configs")))
    {
        if (auto Configs = ConfigsSub->AddObject<CContainer>(X("Configs")))
        {
            Menu::configsListbox = Configs->AddObject<CListbox>(
                X("Files"),
                &settings.settings.configs.selected_index,
                settings.settings.configs.file_list);

            Configs->AddObject<CTextInput>(X("Config name"), &settings.settings.configs.new_config_name);

            Configs->AddObject<CButton>(X("Create"), []()
            {
                settings_io::CreateNewFile(settings.settings.configs.new_config_name);
            });

            Configs->AddObject<CButton>(X("Save"), []()
            {
                if (settings.settings.configs.selected_index >= 0 &&
                    settings.settings.configs.selected_index < static_cast<int>(settings.settings.configs.file_list.size()))
                {
                    settings_io::SaveByName(settings.settings.configs.file_list.at(settings.settings.configs.selected_index));
                }
            });

            Configs->AddObject<CButton>(X("Load"), []()
            {
                if (settings.settings.configs.selected_index >= 0 &&
                    settings.settings.configs.selected_index < static_cast<int>(settings.settings.configs.file_list.size()))
                {
                    settings_io::LoadByName(settings.settings.configs.file_list.at(settings.settings.configs.selected_index));
                }
            });

            Configs->AddObject<CButton>(X("Reset"), []()
            {
                settings_io::Reset();
            });
        }
    }
}
