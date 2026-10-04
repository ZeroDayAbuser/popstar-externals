#include "../caching/cache.h"
#include "../settings/settings.h"
#include <string>
#include "aimbot/aimbot.h"
#include "exploits/chams/chams.h"


namespace game {





	void loop ( ) {

		aim_loc_2d = Vector2 ( );

		aim_loc = Vector3 ( );

		update_aimbot_target ( );

		std::vector<CachedPlayerData>* current = active_render_cache.load ( );
		ImDrawList* draw_list = ImGui::GetBackgroundDrawList ( );
		const bool primary_down = g_aimbot::aimbot_key.is_down.load ( std::memory_order_relaxed );
		const bool secondary_down = g_aimbot::secondary_aimbot_key.is_down.load ( std::memory_order_relaxed );

		for ( const auto& actor : *current ) {

			auto font_size = ImGui::GetFontSize ( );
			auto top_text_offset = font_size + 2.0;
			auto bottom_text_offset = 1.0;
		

			Vector2 head_2d = actor.head_2d;

			if ( is_in_screen ( Vector3 ( head_2d.x , head_2d.y , 1.0 ) , width_sdk , height_sdk ) ) {


				const auto& player_bounds = actor.m_bounds;

				const auto box_width = player_bounds.max_x - player_bounds.min_x;

				const auto box_height = player_bounds.max_y - player_bounds.min_y;

				if ( box_width <= 1.0 || box_height <= 1.0 )
					continue;

				const ImVec2 top_middle ( static_cast< float >( player_bounds.min_x + ( box_width * 0.5 ) ) , static_cast< float >( player_bounds.min_y ) );
				auto theme_color = get_theme_color ( actor.dbno , actor.is_partner , actor.is_visible );

				auto to_imcolor = [ ] ( const float col [ 4 ] ) -> ImColor { return ImColor ( col [ 0 ] , col [ 1 ] , col [ 2 ] , col [ 3 ] ); };

				theme_color.m_outline_color = to_imcolor ( g_settings::color_outline );

				theme_color.m_text_color = to_imcolor ( g_settings::color_text );
				auto vis_pick = [ & ] ( const float visible [ 4 ] , const float hidden [ 4 ] ) -> ImColor {
					if ( actor.dbno )
						return to_imcolor ( g_settings::color_downed );
					if ( actor.is_partner )
						return to_imcolor ( g_settings::color_teammate );
					return actor.is_visible ? to_imcolor ( visible ) : to_imcolor ( hidden );
					};

				const ImColor box_color = vis_pick ( g_settings::color_box_visible , g_settings::color_box_hidden );
				const ImColor skeleton_color = vis_pick ( g_settings::color_skeleton_visible , g_settings::color_skeleton_hidden );
				const ImColor china_hat_color = vis_pick ( g_settings::color_china_hat_visible , g_settings::color_china_hat_hidden );
				const ImColor name_text_color = vis_pick ( g_settings::color_name_text_visible , g_settings::color_name_text_hidden );
				const ImColor fov_arrow_color = vis_pick ( g_settings::color_fov_arrow_visible , g_settings::color_fov_arrow_hidden );
				auto dim_if_hidden = [ ] ( ImColor c , bool is_visible ) -> ImColor {
					if ( is_visible )
						return c;
					ImVec4 v = c.Value;
					v.x *= 0.68f;
					v.y *= 0.68f;
					v.z *= 0.68f;
					return ImColor ( v );
					};

				if ( g_settings::box ) {
					draw_list->AddRect ( ImVec2 ( player_bounds.min_x , player_bounds.min_y ) , ImVec2 ( player_bounds.min_x + box_width , player_bounds.min_y + box_height ) , IM_COL32 ( 0 , 0 , 0 , 255 ) , 0.0f , 0 , 1.5f + 1.0f );
					draw_list->AddRect ( ImVec2 ( player_bounds.min_x , player_bounds.min_y ) , ImVec2 ( player_bounds.min_x + box_width , player_bounds.min_y + box_height ) , box_color , 0.0f , 0 , 1.5f );
				}

				if ( g_settings::skeleton ) {
					for ( int i = 0; i + 1 < actor.skeleton_line_point_count; i += 2 ) {
						draw_list->AddLine ( actor.skeleton_line_points [ i ] , actor.skeleton_line_points [ i + 1 ] , skeleton_color , 1.35f );
					}
				}


				if ( g_settings::rank ) {
					auto text_format = get_rank_name ( actor.rank );
					auto text_color = dim_if_hidden ( get_rank_color ( actor.rank ) , actor.is_visible );

					if ( !text_format.empty ( ) ) {
						auto text_size = ImGui::CalcTextSize ( text_format.c_str ( ) );

						auto text_pos = ImVec2 ( top_middle.x - ( text_size.x / 2.0f ) , top_middle.y - static_cast< float >( top_text_offset ) );

						for ( auto dx = -1.0f; dx <= 1.0f; dx++ ) {

							for ( auto dy = -1.0f; dy <= 1.0f; dy++ ) {

								if ( dx == 0.0f && dy == 0.0f )
									continue;

								draw_list->AddText ( ImVec2 ( text_pos.x + dx , text_pos.y + dy ) , theme_color.m_outline_color , text_format.c_str ( ) );
							}
						}

						draw_list->AddText ( text_pos , text_color , text_format.c_str ( ) );
						top_text_offset += text_size.y;
					}
				}


				if ( g_settings::player_name ) {
					if ( !actor.username.empty ( ) ) {
						std::string text_format = actor.username;

						if ( actor.player_id != 0 ) {
							text_format += " (" + std::to_string ( actor.player_id ) + ")";
						}
						auto text_size = ImGui::CalcTextSize ( text_format.c_str ( ) );

						auto text_pos = ImVec2 ( top_middle.x - ( text_size.x / 2.0f ) , top_middle.y - static_cast< float >( top_text_offset ) );

						for ( auto dx = -1.0f; dx <= 1.0f; dx++ ) {

							for ( auto dy = -1.0f; dy <= 1.0f; dy++ ) {

								if ( dx == 0.0f && dy == 0.0f )
									continue;

								draw_list->AddText ( ImVec2 ( text_pos.x + dx , text_pos.y + dy ) , theme_color.m_outline_color , text_format.c_str ( ) );
							}
						}

						draw_list->AddText ( text_pos , name_text_color , text_format.c_str ( ) );

						top_text_offset += text_size.y;
					}
				}

				if ( g_settings::china_hat ) {
					auto head_location = actor.head_3d;

					Vector3 tip_3d = { head_location.x, head_location.y, head_location.z + 22.f };
					auto screen_tip_proj = Custom::K2_Project ( tip_3d );

					std::vector<ImVec2> base_points_2d;
					base_points_2d.reserve ( 23 );

					const float angle_step = 2.0f * M_PI / 23;
					for ( int i = 0; i < 23; ++i ) {
						const float angle = angle_step * i;
						Vector3 base_point_3d = {
							head_location.x + std::cos ( angle ) * 17.f,
							head_location.y + std::sin ( angle ) * 17.f,
							head_location.z + 9.f
						};

						auto screen_base = Custom::K2_Project ( base_point_3d );
						base_points_2d.push_back ( screen_base.is_valid ( ) ? screen_base.vec ( ) : ImVec2 ( -FLT_MAX , -FLT_MAX ) );
					}

					for ( size_t i = 0; i < 23; ++i ) {
						const auto& p1 = base_points_2d [ i ];
						const auto& p2 = base_points_2d [ ( i + 1 ) % 23 ];
						if ( p1.x != -FLT_MAX && p2.x != -FLT_MAX ) {
							draw_list->AddLine ( p1 , p2 , china_hat_color , 1.5f );
						}
					}

					if ( screen_tip_proj.is_valid ( ) ) {
						ImVec2 tip_2d = screen_tip_proj.vec ( );
						for ( const auto& base_point : base_points_2d ) {
							if ( base_point.x != -FLT_MAX ) {
								draw_list->AddLine ( tip_2d , base_point , china_hat_color , 1.5f );
							}
						}
					}
				}

				if ( g_settings::platform ) {
					auto text_format = get_platform_name_1 ( actor.platform_text );
					auto text_size = ImGui::CalcTextSize ( text_format.c_str ( ) );

					if ( !actor.platform_text.empty ( ) ) {

						auto text_color = dim_if_hidden ( get_platform_color ( actor.platform_text ) , actor.is_visible );
						auto text_pos = ImVec2 (
							top_middle.x - ( text_size.x / 2.0f ) ,
							top_middle.y - top_text_offset
						);

						for ( auto dx = -1.0f; dx <= 1.0f; dx++ ) {

							for ( auto dy = -1.0f; dy <= 1.0f; dy++ ) {

								if ( dx == 0.0f && dy == 0.0f )
									continue;

								draw_list->AddText ( ImVec2 ( text_pos.x + dx , text_pos.y + dy ) , theme_color.m_outline_color , text_format.c_str ( ) );
							}
						}

						draw_list->AddText ( text_pos , text_color , text_format.c_str ( ) );

						top_text_offset += text_size.y;
					}
				}


				if ( g_settings::fov_arrows && g_aimbot::show_fov ) {
					float fov_radius = g_aimbot::fov * ( 90.0f / ViewPoint::FieldOfView );
					float cx = width_sdk / 2.0f;
					float cy = height_sdk / 2.0f;
					float dx = head_2d.x - cx;
					float dy = head_2d.y - cy;
					float dist_to_center = sqrtf ( dx * dx + dy * dy );

					if ( dist_to_center > fov_radius ) {
						float angle = atan2f ( dy , dx );

						ImVec2 arrow_pos ( cx + ( fov_radius + 12.0f ) * cosf ( angle ) , cy + ( fov_radius + 12.0f ) * sinf ( angle ) );
						ImVec2 arrow_v [ 3 ] = {
							ImVec2 ( arrow_pos.x + cosf ( angle ) * 15.0f , arrow_pos.y + sinf ( angle ) * 15.0f ) ,
							ImVec2 ( arrow_pos.x + cosf ( angle - 1.5f ) * 12.0f , arrow_pos.y + sinf ( angle - 1.5f ) * 12.0f ) ,
							ImVec2 ( arrow_pos.x + cosf ( angle + 1.5f ) * 12.0f , arrow_pos.y + sinf ( angle + 1.5f ) * 12.0f )
						};

						draw_list->AddTriangleFilled ( arrow_v [ 0 ] , arrow_v [ 1 ] , arrow_v [ 2 ] , fov_arrow_color );
						draw_list->AddTriangle ( arrow_v [ 0 ] , arrow_v [ 1 ] , arrow_v [ 2 ] , theme_color.m_outline_color , 1.5f );
					}
				}

				if ( g_aimbot::enable && has_current_locked_cache ) {

					if ( g_aimbot::visible_check && !current_locked_cache.is_visible )
						continue;


					if ( primary_down || secondary_down ) {
						Vector3 aim_location = aim_loc;
						FRotator aim_rotation = get_aim_rotation ( aim_location );
						g_memory->move_mouse ( aim_rotation , g_aimbot::smoothing );
					}
				}

			}
		}

	}

}
