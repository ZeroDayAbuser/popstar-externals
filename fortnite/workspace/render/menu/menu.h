#pragma once
#include <blur/directx_blur.h>
#include <gui.h>
#include "../../../dependencies/oxorany/oxorany.h"

#ifndef IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_DEFINE_MATH_OPERATORS
#endif
#include <gui_colors.h>


namespace menu
{

	void show ( ) {
        ImGui::SetNextWindowSize ( ui::size );
        ImGui::Begin ( oxorany ( "popstar" ), nullptr , ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoBringToFrontOnFocus );
        {

            ui::UpdateMenuColors ( );

            ui::render_background ( );

            ui::render_title_cheat ( oxorany ( "popstar.rocks" ) );

            ui::render_build_date ( );

            ui::render_tabs_content ( );

            ui::render_tabs ( ImGui::GetIO ( ).DeltaTime );

            ui::render_outline ( );

            ui::render_notification ( );
        }
        ImGui::End ( );
    }

}
