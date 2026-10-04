#include "gui.h"
#include "gui_colors.h"
#include "../../../core/unreal-engine/settings/settings.h"
#include "../../../../dependencies/oxorany/oxorany.h"

void ui::tabs::aimbot(const TabCategory tab)
{
    if (tab.name == "aimbot")
    {
        const int full_height = static_cast<int>(ui::size.y - 95.0f);
        if ( ui::begin_child_left ( oxorany ( "targetting" ) , full_height ) )
        {

            ImGui::Checkbox ( oxorany ( "enable" ) , &g_aimbot::enable );


            if ( g_aimbot::enable ) {
                const bool was_controller_support = g_aimbot::controller_support;
                if ( was_controller_support != g_aimbot::controller_support ) {
                    const bool switched_to_controller = g_aimbot::controller_support;
                    auto clear_if_incompatible = [ switched_to_controller ] ( CustomWidgets::Keybind& bind ) {
                        const bool key_is_controller = CustomWidgets::IsControllerBind ( bind.key );
                        const bool should_clear =
                            ( switched_to_controller && !key_is_controller ) ||
                            ( !switched_to_controller && key_is_controller );

                        if ( should_clear ) {
                            bind.key = 0;
                            bind.is_down.store ( false , std::memory_order_relaxed );
                        }
                        };

                    clear_if_incompatible ( g_aimbot::aimbot_key );
                    clear_if_incompatible ( g_aimbot::secondary_aimbot_key );
                }


                ImGui::Checkbox ( oxorany ( "show fov" ) , &g_aimbot::show_fov );
                if ( g_aimbot::show_fov ) {
                    ImGui::Checkbox ( oxorany ( "fov arrows" ) , &g_settings::fov_arrows );
                    ImGui::SliderInt ( oxorany ( "fov" ) , &g_aimbot::fov , 0 , 350 );
                }

                ImGui::Checkbox ( oxorany ( "visible check" ) , &g_aimbot::visible_check );
                ImGui::SliderFloat ( oxorany ( "smoothing" ) , &g_aimbot::smoothing , 1.0f , 20.0f );

                if ( g_aimbot::controller_support ) {
                    CustomWidgets::RenderControllerKeybind ( oxorany ( "primary controller key" ) , &g_aimbot::aimbot_key );
                    CustomWidgets::RenderControllerKeybind ( oxorany ( "secondary controller key" ) , &g_aimbot::secondary_aimbot_key );
                }
                else {
                    CustomWidgets::RenderKeybind ( oxorany ( "primary aim key" ) , &g_aimbot::aimbot_key );
                    CustomWidgets::RenderKeybind ( oxorany ( "secondary aim key" ) , &g_aimbot::secondary_aimbot_key );
                }

            }
     
        }
        ImGui::EndChild();
    }
}
