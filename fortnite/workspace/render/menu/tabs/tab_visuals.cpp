#include "gui.h"
#include "gui_colors.h"
#include "../../../core/unreal-engine/settings/settings.h"
#include "../../../../dependencies/oxorany/oxorany.h"

void ui::tabs::visuals ( const TabCategory tab )
{
    if ( tab.name == "visuals" )
    {
        const int full_height = static_cast< int >( ui::size.y - 95.0f );
        if ( ui::begin_child_left ( "main" , full_height ) )
        {

            ImGui::Checkbox ( oxorany ( "box" ) , &g_settings::box );
            ImGui::Checkbox ( oxorany ( "skeleton" ) , &g_settings::skeleton );
            ImGui::Checkbox ( oxorany ( "china hat" ) , &g_settings::china_hat );
            ImGui::Checkbox ( oxorany ( "rank" ) , &g_settings::rank );
            ImGui::Checkbox ( oxorany ( "platform" ) , &g_settings::platform );
            ImGui::Checkbox ( oxorany ( "player name" ) , &g_settings::player_name );
            ImGui::Checkbox ( oxorany ( "fov arrows" ) , &g_settings::fov_arrows );
            ImGui::Checkbox ( oxorany ( "chams" ) , &g_exploits::chams );

        }
        ImGui::EndChild ( );

        if ( ui::begin_child_right ( "esp colors" , full_height ) )
        {

            const ImGuiColorEditFlags color_flags = ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaPreview;

            ImGui::ColorEdit4 ( oxorany ( "teammate" ) , g_settings::color_teammate , color_flags );
            ImGui::ColorEdit4 ( oxorany ( "downed" ) , g_settings::color_downed , color_flags );
            ImGui::ColorEdit4 ( oxorany ( "box visible" ) , g_settings::color_box_visible , color_flags );
            ImGui::ColorEdit4 ( oxorany ( "box hidden" ) , g_settings::color_box_hidden , color_flags );
            ImGui::ColorEdit4 ( oxorany ( "skeleton visible" ) , g_settings::color_skeleton_visible , color_flags );
            ImGui::ColorEdit4 ( oxorany ( "skeleton hidden" ) , g_settings::color_skeleton_hidden , color_flags );
            ImGui::ColorEdit4 ( oxorany ( "hat visible" ) , g_settings::color_china_hat_visible , color_flags );
            ImGui::ColorEdit4 ( oxorany ( "hat hidden" ) , g_settings::color_china_hat_hidden , color_flags );
            ImGui::ColorEdit4 ( oxorany ( "rank visible" ) , g_settings::color_rank_text_visible , color_flags );
            ImGui::ColorEdit4 ( oxorany ( "rank hidden" ) , g_settings::color_rank_text_hidden , color_flags );
            ImGui::ColorEdit4 ( oxorany ( "platform visible" ) , g_settings::color_platform_text_visible , color_flags );
            ImGui::ColorEdit4 ( oxorany ( "platform hidden" ) , g_settings::color_platform_text_hidden , color_flags );
            ImGui::ColorEdit4 ( oxorany ( "name visible" ) , g_settings::color_name_text_visible , color_flags );
            ImGui::ColorEdit4 ( oxorany ( "name hidden" ) , g_settings::color_name_text_hidden , color_flags );
            ImGui::ColorEdit4 ( oxorany ( "arrow visible" ) , g_settings::color_fov_arrow_visible , color_flags );
            ImGui::ColorEdit4 ( oxorany ( "arrow hidden" ) , g_settings::color_fov_arrow_hidden , color_flags );

        }
        ImGui::EndChild ( );
    }
}
