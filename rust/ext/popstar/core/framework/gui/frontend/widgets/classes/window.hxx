#pragma once

#include <cmath>
#include <functional>
#include <memory>
#include <string>
#include <vector>

#include <core/framework/gui/frontend/widgets/classes/child.hxx>
#include <core/framework/gui/frontend/widgets/classes/tab.hxx>
#include <core/framework/gui/frontend/widgets/classes/config.hxx>
#include <core/framework/gui/frontend/widgets/settings.hxx>
#include <core/framework/gui/backend/render/render.hxx>
#include <core/framework/gui/frontend/widgets/classes/context.hxx>
#include <core/framework/gui/backend/blur/blur.hxx>
#include <core/framework/gui/backend/water/water_blob.hxx>
#include <core/framework/gui/backend/manager/textures/textures.hxx>

namespace core::gui
{
	class c_window
	{
	public:
		c_window( std::string title, c_vector_2d pos, c_vector_2d size )
			: m_title( std::move( title ) ), m_pos( pos ), m_size( size )
		{
			if ( g_style )
			{
				m_size.x = g_style->window_w;
				m_size.y = g_style->window_h;
			}
		}

		~c_window( ) = default;

		void paint( )
		{
			if ( !g_render || !g_style || !g_ctx )
				return;
			if ( g_ctx->m_open_anim < 0.001f )
				return;

			const float sw = g_style->sidebar_w;
			const float th = g_style->toolbar_h;
			const float round = g_style->rounding;
			const float e = c_render::ease_in_out_quint( g_ctx->m_open_anim );
			const c_vector_2d pivot( m_pos.x + m_size.x * 0.5f, m_pos.y + m_size.y * 0.5f );
			const float scale = ImLerp( 0.92f, 1.f, e );
			const float y_off = ImLerp( 18.f, 0.f, e );
			auto xform = [&]( float x, float y )
			{
				return c_vector_2d(
					pivot.x + ( x - pivot.x ) * scale,
					pivot.y + ( y - pivot.y ) * scale + y_off );
			};
			const c_vector_2d bmin = xform( m_pos.x, m_pos.y );
			const c_vector_2d bmax = xform( m_pos.x + m_size.x, m_pos.y + m_size.y );

			g_render->set_alpha( e );

			if ( g_blur && g_blur->ready( ) )
			{
				ImDrawList* dl = g_render->draw_list( );
				if ( dl )
					g_blur->draw_region( dl, bmin, bmax, round * scale, c_color( 255, 255, 255, static_cast<int>( 255.f * e ) ) );
			}

			if ( g_water )
			{
				ImDrawList* dl = g_render->draw_list( );
				if ( dl )
				{
					g_water->process( static_cast<float>( ImGui::GetTime( ) ), e, bmin, bmax );
					if ( g_water->ready( ) )
						g_water->draw_region( dl, bmin, bmax, round * scale, e * 0.88f );
				}
			}

			const int vtx = g_render->vtx_count( );

			g_render->rect_filled(
				static_cast<int>( m_pos.x ),
				static_cast<int>( m_pos.y ),
				static_cast<int>( m_size.x ),
				static_cast<int>( m_size.y ),
				g_style->window_bg,
				round );

			paint_sidebar( sw );
			paint_toolbar( sw, th );
			tick_tab_anim( );
			if ( !g_ctx->m_open )
				close_overlays( );
			layout_children( sw, th );

			g_render->apply_scale( vtx, pivot, g_ctx->m_open_anim, 0.92f, 18.f );
			g_render->set_alpha( 1.f );
		}

		void input( )
		{
			if ( !g_ctx || !g_input || !g_style )
				return;
			if ( g_ctx->m_open_anim < 0.95f && !g_ctx->m_open )
				return;
			if ( !g_ctx->m_open && g_ctx->m_open_anim < 0.05f )
				return;

			const float sw = g_style->sidebar_w;
			const float th = g_style->toolbar_h;

			// Config popup owns input while open — menu widgets / nav stay inert.
			if ( m_config.blocks_menu( ) )
			{
				m_config.input( );
				return;
			}

			if ( !g_ctx->m_modal_owner )
			{
				handle_nav_input( sw );
				handle_side_input( );
				m_config.input( );
				handle_drag( th );
			}

			if ( g_ctx->m_open )
			{
				for ( const std::shared_ptr<c_child>& child : m_childrens )
				{
					if ( child && child->visible( ) )
						child->input( );
				}
			}
		}

		std::shared_ptr<c_child> build_child( std::string name, child_width width, float y, std::function<void( c_child* ptr )> callback )
		{
			std::shared_ptr<c_child> child = std::make_shared<c_child>( std::move( name ), width, y );
			if ( callback )
				callback( child.get( ) );
			m_childrens.push_back( child );
			return child;
		}

		std::shared_ptr<c_tab> prebuild_tabs( std::function<void( c_tab* ptr )> callback )
		{
			m_obj_tab = std::make_shared<c_tab>( );
			if ( callback )
				callback( m_obj_tab.get( ) );
			return m_obj_tab;
		}

		void finish_tab_prebuild( )
		{
			if ( !m_obj_tab || !g_ctx )
				return;

			g_ctx->m_tabs.clear( );
			for ( const std::string& name : m_obj_tab->m_names )
			{
				tab_data_t tab {};
				tab.m_name = name;
				g_ctx->m_tabs.push_back( tab );
			}

			if ( !g_ctx->m_tabs.empty( ) )
			{
				g_ctx->m_active_tab = 0;
				g_ctx->m_cur_tab = g_ctx->m_tabs[0].m_name;
			}
		}

		c_vector_2d subtab_padding( ) const
		{
			return c_vector_2d( 8.f, 8.f );
		}

		c_vector_2d child_padding( ) const
		{
			return c_vector_2d( 9.f, 28.f );
		}

		c_vector_2d& pos( ) { return m_pos; }
		c_vector_2d& size( ) { return m_size; }
		const std::string& title( ) const { return m_title; }

	private:
		void paint_sidebar( float sw )
		{
			g_render->rect_filled(
				static_cast<int>( m_pos.x ),
				static_cast<int>( m_pos.y ),
				static_cast<int>( sw ),
				static_cast<int>( m_size.y ),
				g_style->sidebar_bg,
				g_style->rounding,
				draw_flags_round_corners_left );

			g_render->line(
				c_vector_2d( m_pos.x + sw, m_pos.y ),
				c_vector_2d( m_pos.x + sw, m_pos.y + m_size.y ),
				c_color( 30, 33, 43, 160 ) );

			g_render->push_clip(
				static_cast<int>( m_pos.x ),
				static_cast<int>( m_pos.y ),
				static_cast<int>( sw ),
				static_cast<int>( m_size.y ) );

			constexpr float nav_x = 12.f;
			const float nav_w = sw - nav_x - 12.f;
			const float sub_x = nav_x + 14.f;
			const float sub_w = sw - sub_x - 12.f;
			const float dt = ImGui::GetIO( ).DeltaTime;
			const logo_layout_t logo = compute_logo_layout( sw );
			const float nav_y0 = logo.block_h;

			paint_sidebar_logo( logo );

			g_render->line(
				c_vector_2d( m_pos.x + nav_x, m_pos.y + nav_y0 - 12.f ),
				c_vector_2d( m_pos.x + sw - 12.f, m_pos.y + nav_y0 - 12.f ),
				c_color( 38, 42, 54, 160 ) );

			float y = nav_y0;
			paint_section_label( "COMBAT", y, nav_x );
			y += 20.f;
			m_nav_y[0] = y;
			paint_nav_item( "icon_aimbot", "Aimbot", y, 0, nav_x, nav_w );
			y += 42.f;
			y += paint_current_subs( 0, y, sub_x, sub_w, dt );

			paint_section_label( "VISUALS", y, nav_x );
			y += 20.f;
			m_nav_y[1] = y;
			paint_nav_item( "icon_visuals", "Visuals", y, 1, nav_x, nav_w );
			y += 42.f;
			y += paint_current_subs( 1, y, sub_x, sub_w, dt );

			paint_section_label( "CUSTOMIZATION", y, nav_x );
			y += 20.f;
			m_nav_y[2] = y;
			paint_nav_item( "icon_misc", "Misc", y, 2, nav_x, nav_w );
			y += 42.f;
			y += paint_current_subs( 2, y, sub_x, sub_w, dt );

			m_nav_y[3] = y;
			paint_nav_item( "icon_settings", "Settings", y, 3, nav_x, nav_w );

			const float ind_tab = m_nav_y[m_nav_index] + 6.f;
			m_nav_indicator_y += ( ind_tab - m_nav_indicator_y ) * ( 1.f - std::exp( -16.f * dt ) );
			if ( m_nav_indicator_y < 1.f )
				m_nav_indicator_y = ind_tab;

			g_render->rect_filled_f( m_pos.x + nav_x - 9.f, m_pos.y + m_nav_indicator_y, 4.f, 18.f, g_style->accent, 2.f );

			g_render->restore_clip( );
		}

		struct logo_layout_t
		{
			float x { 0.f };
			float y { 0.f };
			float w { 0.f };
			float h { 0.f };
			float block_h { 148.f };
			bool valid { false };
		};

		logo_layout_t compute_logo_layout( float sw ) const
		{
			logo_layout_t out {};
			const float pad = g_style ? g_style->logo_pad : 10.f;
			const float target = g_style ? g_style->logo_size : 132.f;
			const float max_w = ( std::max ) ( 40.f, sw - pad * 2.f );
			const float max_h = ( std::max ) ( 40.f, target );
			auto* tex = g_textures ? g_textures->get_texture( c_textures::k_logo_key ) : nullptr;
			if ( !tex || !tex->srv || tex->width <= 0 || tex->height <= 0 )
			{
				out.block_h = pad + 36.f + 18.f;
				return out;
			}
			const float aspect = static_cast< float >( tex->width ) / static_cast< float >( tex->height );
			float draw_w = max_w;
			float draw_h = draw_w / aspect;
			if ( draw_h > max_h )
			{
				draw_h = max_h;
				draw_w = draw_h * aspect;
			}
			out.w = std::floor( draw_w );
			out.h = std::floor( draw_h );
			out.x = std::floor( m_pos.x + ( sw - out.w ) * 0.5f );
			out.y = std::floor( m_pos.y + pad );
			out.block_h = pad + out.h + 18.f;
			out.valid = true;
			return out;
		}

		void paint_sidebar_logo( const logo_layout_t& logo )
		{
			if ( !g_render || !g_style )
				return;

			auto* tex = g_textures ? g_textures->get_texture( c_textures::k_logo_key ) : nullptr;
			if ( !tex || !tex->srv || tex->width <= 0 || tex->height <= 0 )
			{
				g_render->text( c_fonts::k_title_key, c_vector_2d( m_pos.x + 12.f, m_pos.y + 28.f ), g_style->accent, "NIRVANA", g_style->text_title );
				return;
			}

			if ( !logo.valid || logo.w <= 0.f || logo.h <= 0.f )
				return;

			const ImTextureID id = reinterpret_cast< ImTextureID >( tex->srv );
			if ( !id )
				return;

			g_render->image( static_cast< int >( logo.x ) + 1, static_cast< int >( logo.y ) + 2, static_cast< int >( logo.w ), static_cast< int >( logo.h ), id, c_color( 0, 0, 0, 70 ) );
			g_render->image( static_cast< int >( logo.x ), static_cast< int >( logo.y ), static_cast< int >( logo.w ), static_cast< int >( logo.h ), id, g_style->accent.with_alpha( 255 ) );
		}

		void paint_section_label( const char* label, float y, float ox )
		{
			g_render->text(
				c_fonts::k_caption_key,
				c_vector_2d( m_pos.x + ox + 2.f, m_pos.y + y ),
				c_color( 89, 94, 106 ),
				label,
				g_style->text_caption );
		}

		void paint_tex_icon( const char* key, float x, float y, float size, c_color col )
		{
			auto* tex = g_textures ? g_textures->get_texture( key ) : nullptr;
			if ( tex && tex->srv )
			{
				g_render->image( static_cast<int>( x ), static_cast<int>( y ), static_cast<int>( size ), static_cast<int>( size ),
					reinterpret_cast<ImTextureID>( tex->srv ), col );
				return;
			}
			g_render->text( c_fonts::k_icons_key, c_vector_2d( x, y ), col, "\xef\x80\x91", size );
		}

		int current_sub( ) const
		{
			if ( m_nav_index == 0 ) return m_aim_sub;
			if ( m_nav_index == 1 ) return m_visuals_sub;
			if ( m_nav_index == 2 ) return m_misc_sub;
			return 0;
		}

		int sub_count_for( int nav ) const
		{
			if ( nav == 0 ) return 2;
			if ( nav == 1 ) return 0;
			if ( nav == 2 ) return 1;
			return 0;
		}

		const char* sub_label( int nav, int sub ) const
		{
			if ( nav == 0 ) return sub == 0 ? "General" : "Combat";
			if ( nav == 1 ) return "";
			if ( nav == 2 ) return "Movement";
			return "";
		}

		const char* sub_icon( int nav, int sub ) const
		{
			if ( nav == 0 ) return sub == 0 ? "icon_aimbot" : "icon_combat";
			if ( nav == 1 ) return sub == 0 ? "icon_player" : "icon_entities";
			if ( nav == 2 ) return "icon_movement";
			return "icon_settings";
		}

		float paint_current_subs( int nav, float y, float ox, float w, float dt )
		{
			const float target = ( m_nav_index == nav && sub_count_for( nav ) > 0 ) ? 1.f : 0.f;
			float& reveal = m_sub_reveal_tab[nav];
			reveal += ( target - reveal ) * ( 1.f - std::exp( -14.f * dt ) );
			if ( reveal < 0.01f )
				return 0.f;

			const float e = c_render::smoothstep( reveal );
			const int count = sub_count_for( nav );
			for ( int i = 0; i < count; ++i )
			{
				m_sub_hit_y[nav][i] = y + i * 28.f;
				paint_sub_nav( sub_icon( nav, i ), sub_label( nav, i ), m_sub_hit_y[nav][i], i, ox, w, e, nav );
			}

			const float ind_target = m_sub_hit_y[nav][current_sub( )] + 4.f;
			m_sub_indicator_y += ( ind_target - m_sub_indicator_y ) * ( 1.f - std::exp( -16.f * dt ) );
			if ( m_sub_indicator_y < 1.f )
				m_sub_indicator_y = ind_target;
			g_render->rect_filled_f( m_pos.x + ox - 9.f, m_pos.y + m_sub_indicator_y, 4.f, 18.f, g_style->accent.with_alpha( static_cast<int>( 230.f * e ) ), 2.f );
			return count * 28.f * e + 8.f * e;
		}

		void paint_nav_item( const char* icon, const char* label, float y, int index, float ox, float w )
		{
			const c_vector_2d p( m_pos.x + ox, m_pos.y + y );
			const bool selected = m_nav_index == index;
			float& anim = m_nav_anim[index];
			const float hover = ( g_input && g_input->mouse_in_region( p, c_vector_2d( w, 30.f ) ) ) ? 0.55f : 0.f;
			const float target = selected ? 1.f : hover;
			const float dt = ImGui::GetIO( ).DeltaTime;
			anim += ( target - anim ) * ( 1.f - std::exp( -16.f * dt ) );

			const float slide = anim * 2.f;

			if ( anim > 0.001f )
			{
				g_render->rect_filled(
					static_cast<int>( p.x ),
					static_cast<int>( p.y ),
					static_cast<int>( w ),
					30,
					c_color( 22, 25, 34 ).with_alpha( static_cast<int>( 200.f * anim ) ),
					6.f );
			}

			const c_color icon_col = c_color( 118, 124, 138 ).lerp( g_style->accent, anim );
			const c_color text_col = c_color( 138, 143, 156 ).lerp( c_color( 232, 234, 242 ), anim );

			paint_tex_icon( icon, p.x + 8.f + slide, p.y + 6.f, 16.f, icon_col );
			g_render->text( c_fonts::k_default_key, c_vector_2d( p.x + 32.f + slide, p.y + 7.f ), text_col, label, g_style->text_body );
		}

		void paint_sub_nav( const char* icon, const char* label, float y, int sub_index, float ox, float w, float reveal, int nav )
		{
			const float drop = ( 1.f - reveal ) * 8.f;
			const c_vector_2d p( m_pos.x + ox, m_pos.y + y + drop );
			const bool selected = m_nav_index == nav && current_sub( ) == sub_index;
			float& anim = m_sub_anim[sub_index];
			const float hover = ( g_input && g_input->mouse_in_region( p, c_vector_2d( w, 26.f ) ) ) ? 0.5f : 0.f;
			const float target = selected ? 1.f : hover;
			const float dt = ImGui::GetIO( ).DeltaTime;
			anim += ( target - anim ) * ( 1.f - std::exp( -16.f * dt ) );

			const float a = reveal;
			const float slide = anim * 2.f;

			if ( anim > 0.001f )
			{
				g_render->rect_filled(
					static_cast<int>( p.x ),
					static_cast<int>( p.y ),
					static_cast<int>( w ),
					26,
					c_color( 20, 23, 32 ).with_alpha( static_cast<int>( 200.f * anim * a ) ),
					5.f );
			}

			const c_color icon_col = c_color( 110, 116, 130 ).lerp( g_style->accent, anim ).with_alpha( static_cast<int>( 255.f * a ) );
			const c_color text_col = c_color( 118, 123, 136 ).lerp( c_color( 224, 227, 236 ), anim ).with_alpha( static_cast<int>( 255.f * a ) );

			paint_tex_icon( icon, p.x + 8.f + slide, p.y + 5.f, 14.f, icon_col );
			g_render->text( c_fonts::k_default_key, c_vector_2d( p.x + 28.f + slide, p.y + 5.f ), text_col, label, g_style->text_body );
		}

		void paint_toolbar( float sw, float th )
		{
			g_render->rect_filled(
				static_cast<int>( m_pos.x + sw ),
				static_cast<int>( m_pos.y ),
				static_cast<int>( m_size.x - sw ),
				static_cast<int>( th ),
				g_style->toolbar_bg,
				g_style->rounding,
				draw_flags_round_corners_top_right );

			g_render->line(
				c_vector_2d( m_pos.x + sw, m_pos.y + th ),
				c_vector_2d( m_pos.x + m_size.x, m_pos.y + th ),
				c_color( 27, 30, 39, 160 ) );

			const float pad = g_style->content_pad;
			const char* tab_name = tab_name_for( m_nav_index );

			const float title_e = c_render::ease_in_out_quint( m_content_anim );
			const float title_x = m_pos.x + sw + pad;
			const float title_y = m_pos.y + 18.f;
			g_render->text(
				c_fonts::k_title_key,
				c_vector_2d( title_x, title_y ),
				g_style->text_bright.with_alpha( static_cast<int>( 255.f * title_e ) ),
				tab_name,
				g_style->text_title );

			if ( sub_count_for( m_nav_index ) > 0 )
			{
				const char* sub = sub_label( m_nav_index, current_sub( ) );
				const c_vector_2d ts = g_render->measure_text( c_fonts::k_title_key, tab_name, g_style->text_title );
				g_render->text(
					c_fonts::k_default_key,
					c_vector_2d( title_x + ts.x + 10.f, title_y + 2.f ),
					c_color( 100, 105, 118, static_cast<int>( 255.f * title_e ) ),
					"/",
					g_style->text_body );
				g_render->text(
					c_fonts::k_default_key,
					c_vector_2d( title_x + ts.x + 22.f, title_y + 2.f ),
					c_color( 170, 174, 186, static_cast<int>( 255.f * title_e ) ),
					sub,
					g_style->text_body );
			}

			const c_vector_2d profile( m_pos.x + m_size.x - 180.f, m_pos.y + 14.f );

			paint_side_chips( sw, profile.x, th );
			m_config.paint( profile, c_vector_2d( 166.f, 30.f ) );
		}

		bool show_side_chips( ) const
		{
			return m_nav_index == 1;
		}

		int& chip_side( )
		{
			return m_player_side;
		}

		int chip_side( ) const
		{
			return m_player_side;
		}

		int chip_count( ) const
		{
			return show_side_chips( ) ? 3 : 0;
		}

		void paint_side_chips( float sw, float config_x, float th )
		{
			const float dt = ImGui::GetIO( ).DeltaTime;
			const float target_vis = show_side_chips( ) ? 1.f : 0.f;
			m_side_vis += ( target_vis - m_side_vis ) * ( 1.f - std::exp( -7.5f * dt ) );
			if ( std::fabs( m_side_vis - target_vis ) < 0.0008f )
				m_side_vis = target_vis;

			const int n = 3;
			for ( int i = 0; i < n; ++i )
			{
				const float exp_t = ( chip_side( ) == i ) ? 1.f : 0.f;
				m_side_expand[i] += ( exp_t - m_side_expand[i] ) * ( 1.f - std::exp( -8.5f * dt ) );
				if ( std::fabs( m_side_expand[i] - exp_t ) < 0.0008f )
					m_side_expand[i] = exp_t;
			}

			if ( m_side_vis < 0.01f )
			{
				for ( int i = 0; i < 3; ++i )
				{
					m_side_chip_pos[i] = {};
					m_side_chip_size[i] = {};
				}
				m_side_group_x = 0.f;
				return;
			}

			const float e = c_render::ease_in_out_quint( m_side_vis );
			const float chip_h = 30.f;
			const float icon_w = 32.f;
			const float gap = 8.f;
			const char* labels[3] = { "Enemy", "World", "Local" };
			const char* icons[3] = { "\xef\x80\x87", "\xef\x83\xac", "\xef\x80\xbe" };

			float full_w[3] {};
			float slot_w = icon_w;
			for ( int i = 0; i < n; ++i )
			{
				const c_vector_2d ts = g_render->measure_text( c_fonts::k_default_key, labels[i], g_style->text_control );
				full_w[i] = icon_w + 8.f + ts.x + 14.f;
				if ( full_w[i] > slot_w )
					slot_w = full_w[i];
			}

			float widths[3] {};
			float total = gap;
			for ( int i = 0; i < n; ++i )
			{
				const float expand = c_render::ease_in_out_quint( m_side_expand[i] );
				widths[i] = ImLerp( icon_w, slot_w, expand );
				total += widths[i] + ( i == 0 ? 0.f : gap );
			}

			const float area_l = m_pos.x + sw + 16.f;
			const float area_r = config_x - 16.f;
			const float mid = ( area_l + area_r ) * 0.5f;
			float target_x = mid - total * 0.5f;
			if ( target_x < area_l )
				target_x = area_l;
			if ( target_x + total > area_r )
				target_x = area_r - total;

			if ( m_side_group_x < 1.f )
				m_side_group_x = target_x;
			m_side_group_x = target_x;

			float x = m_side_group_x;
			const float y = m_pos.y + std::floor( ( th - chip_h ) * 0.5f );

			for ( int i = 0; i < n; ++i )
			{
				const float w = widths[i];
				const c_vector_2d p( std::floor( x ), y );
				m_side_chip_pos[i] = p;
				m_side_chip_size[i] = c_vector_2d( w, chip_h );

				const bool hovered = g_input && g_input->mouse_in_region( p, m_side_chip_size[i] );
				m_side_hover[i] += ( ( hovered || chip_side( ) == i ? 1.f : 0.f ) - m_side_hover[i] ) * ( 1.f - std::exp( -10.f * dt ) );

				const bool sel = chip_side( ) == i;
				const float ha = m_side_hover[i];
				const float expand = c_render::ease_in_out_quint( m_side_expand[i] );
				const c_color fill = c_color( 16, 18, 26 )
					.lerp( c_color( 24, 28, 40 ), ha * 0.9f )
					.lerp( g_style->accent.with_alpha( 48 ), sel ? 0.7f : 0.f )
					.with_alpha( static_cast<int>( 255.f * e ) );
				const c_color border = c_color( 36, 40, 52 )
					.lerp( g_style->accent, sel ? 0.85f : ha * 0.5f )
					.with_alpha( static_cast<int>( 230.f * e ) );

				g_render->rect_filled_f( p.x, p.y, w, chip_h, fill, 8.f );
				g_render->rect_f( p.x, p.y, w, chip_h, border, 8.f, 1.f );

				const c_color icon_col = c_color( 150, 154, 166 )
					.lerp( g_style->accent, sel ? 1.f : ha * 0.55f )
					.with_alpha( static_cast<int>( 255.f * e ) );
				g_render->text( c_fonts::k_icons_key, c_vector_2d( p.x + 10.f, p.y + 8.f ), icon_col, icons[i], 13.f );

				if ( expand > 0.02f )
				{
					const c_vector_2d ts = g_render->measure_text( c_fonts::k_default_key, labels[i], g_style->text_control );
					if ( ImDrawList* dl = g_render->draw_list( ) )
					{
						dl->PushClipRect( ImVec2( p.x + icon_w, p.y ), ImVec2( p.x + w - 4.f, p.y + chip_h ), true );
						g_render->text(
							c_fonts::k_default_key,
							c_vector_2d( p.x + icon_w + 2.f, p.y + std::floor( ( chip_h - ts.y ) * 0.5f ) ),
							c_color( 224, 227, 236, static_cast<int>( 255.f * e * expand ) ),
							labels[i],
							g_style->text_control );
						dl->PopClipRect( );
					}
				}

				x += w + gap;
			}
		}

		void tick_tab_anim( )
		{
			const bool menu_open = g_ctx->m_open;
			if ( menu_open && !m_menu_was_open )
			{
				m_content_anim = 0.f;
				play_tab_intros( );
			}
			else if ( !menu_open && m_menu_was_open )
			{
				close_overlays( );
			}
			m_menu_was_open = menu_open;

			const int key = m_nav_index * 100 + current_sub( ) * 10 + chip_side( );
			if ( key != m_tab_key )
			{
				m_tab_dir = ( key > m_tab_key ) ? 1.f : -1.f;
				m_tab_key = key;
				m_content_anim = 0.f;
				if ( menu_open )
					play_tab_intros( );
			}

			const float dt = ImGui::GetIO( ).DeltaTime;
			const float speed = 2.8f;
			m_content_anim += ( 1.f - m_content_anim ) * ( 1.f - std::exp( -speed * dt ) );
			if ( std::fabs( 1.f - m_content_anim ) < 0.001f )
				m_content_anim = 1.f;
		}

		void close_overlays( )
		{
			m_config.close( );

			for ( const std::shared_ptr<c_child>& child : m_childrens )
			{
				if ( !child )
					continue;
				child->close_overlays( );
			}

			if ( g_ctx )
				g_ctx->clear_all_modals( );
			if ( g_render )
				g_render->clear_deferred( );
		}

		void play_tab_intros( )
		{
			const char* want_tab = tab_name_for( m_nav_index );
			const char* want_sub = sub_count_for( m_nav_index ) ? sub_label( m_nav_index, current_sub( ) ) : "";
			const char* want_side = current_side( );

			for ( const std::shared_ptr<c_child>& child : m_childrens )
			{
				if ( !child )
					continue;
				const auto& attach = child->attach_data( );
				const bool tab_ok = attach.m_tab_name.empty( ) || attach.m_tab_name == want_tab;
				const bool sub_ok = attach.m_subtab_name.empty( ) || attach.m_subtab_name == want_sub;
				const bool side_ok = attach.m_side_name.empty( ) || ( want_side[0] && attach.m_side_name == want_side );
				if ( tab_ok && sub_ok && side_ok )
					child->play_intro( );
			}
		}

		void layout_children( float sw, float th )
		{
			const float pad = g_style->content_pad;
			const float label_gap = g_style->child_label_gap;
			const float content_x = m_pos.x + sw + pad;
			const float content_y = m_pos.y + th + pad + label_gap;
			const float content_w = m_size.x - sw - pad * 2.f;
			const float content_h = m_size.y - th - pad * 2.f - label_gap;
			const float col_gap = g_style->col_gap;
			const float row_gap = g_style->row_gap;
			const float col_w = std::floor( ( content_w - col_gap ) * 0.5f );

			float left_y = content_y;
			float right_y = content_y;
			std::size_t index = 0;

			const char* want_tab = tab_name_for( m_nav_index );
			const char* want_sub = sub_count_for( m_nav_index ) ? sub_label( m_nav_index, current_sub( ) ) : "";
			const char* want_side = current_side( );

			g_render->push_clip( static_cast<int>( m_pos.x + sw ), static_cast<int>( m_pos.y + th ), static_cast<int>( m_size.x - sw ), static_cast<int>( m_size.y - th ) );
			const int vtx = g_render->vtx_count( );
			const c_vector_2d pivot( content_x + content_w * 0.5f, content_y + content_h * 0.28f );

			for ( const std::shared_ptr<c_child>& child : m_childrens )
			{
				if ( !child )
					continue;

				const auto& attach = child->attach_data( );
				const bool tab_ok = attach.m_tab_name.empty( ) || attach.m_tab_name == want_tab;
				const bool sub_ok = attach.m_subtab_name.empty( ) || attach.m_subtab_name == want_sub;
				const bool side_ok = attach.m_side_name.empty( ) || ( want_side[0] && attach.m_side_name == want_side );
				child->set_visible( tab_ok && sub_ok && side_ok );
				if ( !child->visible( ) )
					continue;

				const int col = static_cast<int>( index % 2 );
				const float x = content_x + ( col == 0 ? 0.f : col_w + col_gap );
				float& y = ( col == 0 ) ? left_y : right_y;
				const float remaining = content_y + content_h - y;
				const float want_h = ( std::min )( child->measured_height( ), ( std::max )( remaining, g_style->row_height ) );

				child->m_size.x = col_w;
				child->set_max_height( want_h );
				child->m_pos = c_vector_2d( x, y );
				child->draw( );
				y += child->m_size.y + row_gap;
				++index;
			}

			g_render->apply_tab_switch( vtx, pivot, m_content_anim, m_tab_dir );
			g_render->restore_clip( );
		}

		void handle_nav_input( float sw )
		{
			constexpr float nav_x = 12.f;
			const float nav_w = sw - nav_x - 12.f;
			const float sub_x = nav_x + 14.f;
			const float sub_w = sw - sub_x - 12.f;
			const float nav_y0 = compute_logo_layout( sw ).block_h;
			for ( int i = 0; i < 4; ++i )
			{
				const c_vector_2d p( m_pos.x + nav_x, m_pos.y + m_nav_y[i] );
				if ( g_input->mouse_in_region( p, c_vector_2d( nav_w, 30.f ) ) && g_input->clicked( mouse_buttons::left ) )
					m_nav_index = i;
			}

			if ( sub_count_for( m_nav_index ) > 0 && m_sub_reveal_tab[m_nav_index] > 0.5f )
			{
				const int count = sub_count_for( m_nav_index );
				for ( int i = 0; i < count; ++i )
				{
					const c_vector_2d p( m_pos.x + sub_x, m_pos.y + m_sub_hit_y[m_nav_index][i] );
					if ( g_input->mouse_in_region( p, c_vector_2d( sub_w, 26.f ) ) && g_input->clicked( mouse_buttons::left ) )
					{
						if ( m_nav_index == 0 ) m_aim_sub = i;
						if ( m_nav_index == 1 ) m_visuals_sub = i;
						if ( m_nav_index == 2 ) m_misc_sub = i;
					}
				}
			}
		}

		void handle_side_input( )
		{
			if ( !show_side_chips( ) || m_side_vis < 0.5f )
				return;

			for ( int i = 0; i < 3; ++i )
			{
				if ( m_side_chip_size[i].x < 1.f )
					continue;
				if ( g_input->mouse_in_region( m_side_chip_pos[i], m_side_chip_size[i] ) && g_input->clicked( mouse_buttons::left ) )
				{
					chip_side( ) = i;
					return;
				}
			}
		}

		bool mouse_on_side_chips( ) const
		{
			if ( !show_side_chips( ) )
				return false;
			for ( int i = 0; i < 3; ++i )
			{
				if ( m_side_chip_size[i].x < 1.f )
					continue;
				if ( g_input && g_input->mouse_in_region( m_side_chip_pos[i], m_side_chip_size[i] ) )
					return true;
			}
			return false;
		}

		void handle_drag( float th )
		{
			if ( m_config.is_open( ) )
				return;

			if ( g_input->mouse_in_region( c_vector_2d( m_pos.x, m_pos.y ), c_vector_2d( m_size.x, th ) ) && g_input->click_down( mouse_buttons::left ) )
			{
				if ( !g_input->mouse_in_region( m_config.box_pos( ), m_config.box_size( ) ) && !mouse_on_side_chips( ) )
				{
					if ( !g_ctx->m_dragging )
					{
						g_ctx->m_dragging = true;
						m_drag_offset = g_input->get_mouse_position( ) - m_pos;
					}
				}
			}

			if ( g_ctx->m_dragging )
			{
				if ( g_input->click_down( mouse_buttons::left ) )
					m_pos = g_input->get_mouse_position( ) - m_drag_offset;
				else
					g_ctx->m_dragging = false;
			}
		}

		std::shared_ptr<c_tab> m_obj_tab {};
		std::string m_title {};
		c_vector_2d m_pos {};
		c_vector_2d m_size {};
		c_vector_2d m_drag_offset {};
		std::vector<std::shared_ptr<c_child>> m_childrens {};
		const char* tab_name_for( int nav ) const
		{
			static constexpr const char* k[] = { "Aimbot", "Visuals", "Misc", "Settings" };
			return k[nav];
		}

		const char* current_side( ) const
		{
			if ( m_nav_index != 1 )
				return "";
			if ( m_player_side == 1 ) return "World";
			if ( m_player_side == 2 ) return "Local";
			return "Enemies";
		}

		int m_nav_index { 0 };
		int m_aim_sub { 0 };
		int m_visuals_sub { 0 };
		int m_misc_sub { 0 };
		int m_player_side { 0 };
		int m_mod_side { 0 };
		int m_tab_key { -1 };
		float m_tab_dir { 1.f };
		float m_content_anim { 1.f };
		bool m_menu_was_open { false };
		float m_nav_anim[5] {};
		float m_sub_anim[4] {};
		float m_sub_reveal { 0.f };
		float m_sub_reveal_tab[5] {};
		float m_nav_y[5] {};
		float m_sub_hit_y[5][4] {};
		float m_nav_indicator_y { 0.f };
		float m_sub_indicator_y { 0.f };
		float m_side_vis { 0.f };
		float m_side_group_x { 0.f };
		float m_side_expand[3] { 1.f, 0.f, 0.f };
		float m_side_hover[3] {};
		c_vector_2d m_side_chip_pos[3] {};
		c_vector_2d m_side_chip_size[3] {};

		c_config m_config {};
	};
}
