#include "gui.h"
#include "gui_colors.h"
#include "../../../../dependencies/oxorany/oxorany.h"
#include "../../../core/unreal-engine/settings/settings.h"

void ui::tabs::settings(const TabCategory tab)
{
    if (tab.name == "settings")
    {
        const int full_height = static_cast<int>(ui::size.y - 95.0f);
        if (ui::begin_child_left( ( "settings" ), full_height))
        {

            ImGui::Checkbox ( oxorany ( "controller support" ) , &g_aimbot::controller_support );


            const float button_height = 26.0f;
            const ImVec2 full_button_size ( ImGui::GetContentRegionAvail ( ).x , button_height );

            if ( ImGui::Button ( oxorany ( "load config" ) , full_button_size ) ) {
                ui::add_notification ( oxorany ( "A" ) , oxorany ( "loaded config!" ) , ImVec4 ( 0.20f , 0.85f , 0.35f , 1.0f ) );

            }

            if ( ImGui::Button ( oxorany ( "save config" ) , full_button_size ) ) {
                ui::add_notification ( oxorany ( "A" ) , oxorany ( "saved config!" ) , ImVec4 ( 0.20f , 0.85f , 0.35f , 1.0f ) );

            }


            ImGui::ColorEdit4( oxorany ( "accent color" ), (float*)&ui::colors::main, ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_NoInputs |  ImGuiColorEditFlags_AlphaPreview);
            ImGui::Checkbox ( oxorany ( "vsync" ), &g_settings::vsync );
        }
        ImGui::EndChild();
    }
}
