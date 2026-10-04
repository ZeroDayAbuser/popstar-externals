#pragma once

#include <algorithm>
#include <cmath>
#include <cfloat>
#include <cstring>
#include <functional>
#include <memory>
#include <string>
#include <vector>
#include <windows.h>
#include <d3d11.h>

#include <imgui.h>
#include <imgui_internal.h>

#include <core/framework/gui/backend/math/math.hxx>
#include <core/framework/gui/backend/render/device.hxx>
#include <core/framework/gui/backend/manager/fonts/fonts.hxx>
#include <core/framework/gui/backend/inputs/inputs.hxx>
#include <core/framework/gui/backend/blur/blur.hxx>
#include <core/framework/gui/frontend/widgets/settings.hxx>

namespace core::gui
{
	enum draw_flags_
	{
		draw_flags_none = 0,
		draw_flags_closed = 1 << 0,
		draw_flags_round_corners_top_left = 1 << 4,
		draw_flags_round_corners_top_right = 1 << 5,
		draw_flags_round_corners_bottom_left = 1 << 6,
		draw_flags_round_corners_bottom_right = 1 << 7,
		draw_flags_round_corners_none = 1 << 8,
		draw_flags_round_corners_top = draw_flags_round_corners_top_left | draw_flags_round_corners_top_right,
		draw_flags_round_corners_bottom = draw_flags_round_corners_bottom_left | draw_flags_round_corners_bottom_right,
		draw_flags_round_corners_left = draw_flags_round_corners_bottom_left | draw_flags_round_corners_top_left,
		draw_flags_round_corners_right = draw_flags_round_corners_bottom_right | draw_flags_round_corners_top_right,
		draw_flags_round_corners_all = draw_flags_round_corners_top_left | draw_flags_round_corners_top_right | draw_flags_round_corners_bottom_left | draw_flags_round_corners_bottom_right,
		draw_flags_round_corners_default = draw_flags_round_corners_all,
		draw_flags_round_corners_mask = draw_flags_round_corners_all | draw_flags_round_corners_none,
		draw_flags_shadow_cut_out_shape_background = 1 << 9
	};
	typedef int draw_flags;

	enum fade_direction : int
	{
		vertically,
		horizontally,
		diagonally,
		diagonally_reversed,
	};

	enum class render_layer : int
	{
		normal = 0,
		middle,
		prioritized,
		modal,
		count
	};

	class c_gradient_data
	{
	public:
		c_color m_top_r {}, m_top_l {};
		c_color m_bot_r {}, m_bot_l {};

		c_gradient_data( ) = default;
		c_gradient_data( c_color top_r, c_color top_l, c_color bottom_r, c_color bottom_l )
			: m_top_r( top_r ), m_top_l( top_l ), m_bot_r( bottom_r ), m_bot_l( bottom_l )
		{ }
	};

	using draw_list_t = ImDrawList;

	struct rotation_data_t
	{
		void set_draw_list( draw_list_t* draw_list )
		{
			m_draw_list = draw_list;
		}

		void rotation_start( )
		{
			m_start_index = m_draw_list->VtxBuffer.Size;
		}

		ImVec2 rotation_center( )
		{
			ImVec2 l( FLT_MAX, FLT_MAX ), u( -FLT_MAX, -FLT_MAX );
			const ImVector<ImDrawVert>& buf = m_draw_list->VtxBuffer;
			for ( int i = m_start_index; i < buf.Size; i++ )
				l = ImMin( l, buf[i].pos ), u = ImMax( u, buf[i].pos );
			return ImVec2( ( l.x + u.x ) / 2, ( l.y + u.y ) / 2 );
		}

		void rotation_end( float rad, ImVec2 center )
		{
			float s = sinf( rad ), c = cosf( rad );
			const ImVec2 r = ImRotate( center, s, c );
			center = ImVec2( r.x - center.x, r.y - center.y );
			ImVector<ImDrawVert>& buf = m_draw_list->VtxBuffer;
			for ( int i = m_start_index; i < buf.Size; i++ )
			{
				const ImVec2 j = ImRotate( buf[i].pos, s, c );
				buf[i].pos = ImVec2( j.x - center.x, j.y - center.y );
			}
		}

		int get_start_index( ) const
		{
			return m_start_index;
		}

	private:
		int m_start_index = 0;
		draw_list_t* m_draw_list = nullptr;
	};

	namespace modifiers
	{
		enum font_flags
		{
			none = 0,
			drop_shadow,
			outline
		};

		inline c_color default_shadow = c_color( 0, 0, 0, 200 );
	}

	class c_render
	{
	private:
		ImDrawList* m_draw_list {};
		ImDrawListSplitter m_splitter {};
		render_layer m_current_layer { render_layer::normal };
		bool m_layers_active { false };
		ID3D11DeviceContext* m_context { nullptr };
		ID3D11ShaderResourceView* override_resource { nullptr };
		std::vector<std::function<void( )>> m_deferred {};

		static ImDrawFlags to_imgui_flags( draw_flags flags )
		{
			ImDrawFlags out = ImDrawFlags_None;
			if ( flags & draw_flags_round_corners_top_left ) out |= ImDrawFlags_RoundCornersTopLeft;
			if ( flags & draw_flags_round_corners_top_right ) out |= ImDrawFlags_RoundCornersTopRight;
			if ( flags & draw_flags_round_corners_bottom_left ) out |= ImDrawFlags_RoundCornersBottomLeft;
			if ( flags & draw_flags_round_corners_bottom_right ) out |= ImDrawFlags_RoundCornersBottomRight;
			if ( flags & draw_flags_round_corners_none ) out |= ImDrawFlags_RoundCornersNone;
			if ( ( flags & draw_flags_round_corners_mask ) == 0 )
				out |= ImDrawFlags_RoundCornersAll;
			return out;
		}

		float m_alpha { 1.f };

		ImU32 col_u32( c_color c ) const
		{
			const int a = static_cast<int>( static_cast<float>( c.a ) * ImClamp( m_alpha, 0.f, 1.f ) );
			return IM_COL32( c.r, c.g, c.b, a );
		}

	public:
		void set_alpha( float alpha )
		{
			m_alpha = ImClamp( alpha, 0.f, 1.f );
		}

		float alpha( ) const
		{
			return m_alpha;
		}

		int vtx_count( ) const
		{
			return m_draw_list ? m_draw_list->VtxBuffer.Size : 0;
		}

		static float smoothstep( float t )
		{
			t = ImClamp( t, 0.f, 1.f );
			return t * t * ( 3.f - 2.f * t );
		}

		static float ease_out_cubic( float t )
		{
			t = ImClamp( t, 0.f, 1.f );
			const float u = 1.f - t;
			return 1.f - u * u * u;
		}

		static float ease_out_quart( float t )
		{
			t = ImClamp( t, 0.f, 1.f );
			const float u = 1.f - t;
			return 1.f - u * u * u * u;
		}

		static float fade_alpha( float t )
		{
			const float e = ease_out_cubic( t );
			return e * e;
		}

		void apply_reveal( int vtx_start, c_vector_2d pivot, float t, float from_scale = 0.58f )
		{
			if ( !m_draw_list || vtx_start < 0 )
				return;

			const float e = ease_out_cubic( t );
			const float scale = ImLerp( from_scale, 1.f, e );
			const float alpha = fade_alpha( t );

			for ( int i = vtx_start; i < m_draw_list->VtxBuffer.Size; ++i )
			{
				ImDrawVert& v = m_draw_list->VtxBuffer[i];
				v.pos.x = pivot.x + ( v.pos.x - pivot.x ) * scale;
				v.pos.y = pivot.y + ( v.pos.y - pivot.y ) * scale;
				const ImU32 a = ( v.col >> IM_COL32_A_SHIFT ) & 255u;
				v.col = ( v.col & ~IM_COL32_A_MASK ) | ( static_cast<ImU32>( a * alpha ) << IM_COL32_A_SHIFT );
			}
		}

		static float ease_in_out_cubic( float t )
		{
			t = ImClamp( t, 0.f, 1.f );
			return t < 0.5f
				? 4.f * t * t * t
				: 1.f - std::pow( -2.f * t + 2.f, 3.f ) * 0.5f;
		}

		static float ease_in_out_quint( float t )
		{
			t = ImClamp( t, 0.f, 1.f );
			return t < 0.5f
				? 16.f * t * t * t * t * t
				: 1.f - std::pow( -2.f * t + 2.f, 5.f ) * 0.5f;
		}

		void apply_scale( int vtx_start, c_vector_2d pivot, float t, float from_scale = 0.92f, float from_y = 18.f )
		{
			if ( !m_draw_list || vtx_start < 0 )
				return;

			const float e = ease_in_out_quint( t );
			const float scale = ImLerp( from_scale, 1.f, e );
			const float y_off = ImLerp( from_y, 0.f, e );

			for ( int i = vtx_start; i < m_draw_list->VtxBuffer.Size; ++i )
			{
				ImDrawVert& v = m_draw_list->VtxBuffer[i];
				v.pos.x = pivot.x + ( v.pos.x - pivot.x ) * scale;
				v.pos.y = pivot.y + ( v.pos.y - pivot.y ) * scale + y_off;
			}
		}

		void apply_fade( int vtx_start, float t )
		{
			if ( !m_draw_list || vtx_start < 0 )
				return;

			const float alpha = ease_in_out_quint( t );
			for ( int i = vtx_start; i < m_draw_list->VtxBuffer.Size; ++i )
			{
				ImDrawVert& v = m_draw_list->VtxBuffer[i];
				const ImU32 a = ( v.col >> IM_COL32_A_SHIFT ) & 255u;
				v.col = ( v.col & ~IM_COL32_A_MASK ) | ( static_cast<ImU32>( a * alpha ) << IM_COL32_A_SHIFT );
			}
		}

		void apply_open( int vtx_start, c_vector_2d pivot, float t, float from_scale = 0.88f, float from_y = 16.f )
		{
			if ( !m_draw_list || vtx_start < 0 )
				return;

			const float e = ease_in_out_quint( t );
			const float scale = ImLerp( from_scale, 1.f, e );
			const float y_off = ImLerp( from_y, 0.f, e );

			for ( int i = vtx_start; i < m_draw_list->VtxBuffer.Size; ++i )
			{
				ImDrawVert& v = m_draw_list->VtxBuffer[i];
				v.pos.x = pivot.x + ( v.pos.x - pivot.x ) * scale;
				v.pos.y = pivot.y + ( v.pos.y - pivot.y ) * scale + y_off;
				const ImU32 a = ( v.col >> IM_COL32_A_SHIFT ) & 255u;
				v.col = ( v.col & ~IM_COL32_A_MASK ) | ( static_cast<ImU32>( a * e ) << IM_COL32_A_SHIFT );
			}
		}

		void apply_tab_switch( int vtx_start, c_vector_2d pivot, float t, float dir )
		{
			(void)pivot;
			(void)dir;
			if ( !m_draw_list || vtx_start < 0 )
				return;

			const float e = ease_in_out_quint( t );
			const float y_off = ( 1.f - e ) * 18.f;
			for ( int i = vtx_start; i < m_draw_list->VtxBuffer.Size; ++i )
			{
				ImDrawVert& v = m_draw_list->VtxBuffer[i];
				v.pos.y += y_off;
				const ImU32 a = ( v.col >> IM_COL32_A_SHIFT ) & 255u;
				v.col = ( v.col & ~IM_COL32_A_MASK ) | ( static_cast<ImU32>( a * e ) << IM_COL32_A_SHIFT );
			}
		}

		void apply_side_switch( int vtx_start, float t )
		{
			apply_fade( vtx_start, t );
		}

		void set_draw_list( ImDrawList* draw_list )
		{
			m_draw_list = draw_list;
		}

		ImDrawList* draw_list( ) const
		{
			return m_draw_list;
		}

		void set_ctx( ID3D11DeviceContext* ctx )
		{
			m_context = ctx;
		}

		ID3D11DeviceContext* get_ctx( ) const
		{
			return m_context ? m_context : ( g_device ? g_device->m_context : nullptr );
		}

		ID3D11Device* get_device( ) const
		{
			return g_device ? g_device->m_device : nullptr;
		}

		void override_texture( ID3D11ShaderResourceView* resource )
		{
			override_resource = resource;
		}

		void setup( )
		{
			m_draw_list = ImGui::GetForegroundDrawList( );
			m_context = g_device ? g_device->m_context : nullptr;
			m_layers_active = false;
			m_current_layer = render_layer::normal;
			m_deferred.clear( );
		}

		void defer( std::function<void( )> fn )
		{
			m_deferred.push_back( std::move( fn ) );
		}

		void clear_deferred( )
		{
			m_deferred.clear( );
		}

		bool has_deferred( ) const
		{
			return !m_deferred.empty( );
		}

		void flush_deferred( )
		{
			auto pending = std::move( m_deferred );
			m_deferred.clear( );
			for ( auto& fn : pending )
				fn( );
			if ( !m_deferred.empty( ) )
				flush_deferred( );
		}

		// Process only the current deferred queue; nested defer() calls stay queued for a later pass.
		void flush_deferred_once( )
		{
			auto pending = std::move( m_deferred );
			m_deferred.clear( );
			for ( auto& fn : pending )
				fn( );
		}

		void begin_layers( )
		{
			if ( !m_draw_list )
				m_draw_list = ImGui::GetForegroundDrawList( );
			m_splitter.Clear( );
			m_splitter.Split( m_draw_list, static_cast<int>( render_layer::count ) );
			m_layers_active = true;
			set_layer( render_layer::normal );
		}

		void set_layer( render_layer layer )
		{
			m_current_layer = layer;
			if ( m_layers_active && m_draw_list )
				m_splitter.SetCurrentChannel( m_draw_list, static_cast<int>( layer ) );
		}

		void end_layers( )
		{
			if ( m_layers_active && m_draw_list )
				m_splitter.Merge( m_draw_list );
			m_layers_active = false;
			m_current_layer = render_layer::normal;
		}

		template <typename function>
		void use_layer( render_layer layer, function&& func, bool ignore_clipping = false )
		{
			const render_layer previous = m_current_layer;
			set_layer( layer );
			if ( ignore_clipping && m_draw_list )
				m_draw_list->PushClipRectFullScreen( );
			func( );
			if ( ignore_clipping && m_draw_list )
				m_draw_list->PopClipRect( );
			set_layer( previous );
		}

		void rect_filled( int x, int y, int w, int h, c_color col, float rounding = 0.f, draw_flags flags = draw_flags_none )
		{
			if ( !m_draw_list )
				return;
			m_draw_list->AddRectFilled(
				ImVec2( static_cast<float>( x ), static_cast<float>( y ) ),
				ImVec2( static_cast<float>( x + w ), static_cast<float>( y + h ) ),
				col_u32( col ),
				rounding,
				to_imgui_flags( flags ) );
		}

		void rect( int x, int y, int w, int h, c_color col, float rounding = 0.f, float thickness = 1.f, draw_flags flags = draw_flags_none )
		{
			if ( !m_draw_list )
				return;
			m_draw_list->AddRect(
				ImVec2( static_cast<float>( x ), static_cast<float>( y ) ),
				ImVec2( static_cast<float>( x + w ), static_cast<float>( y + h ) ),
				col_u32( col ),
				rounding,
				to_imgui_flags( flags ),
				thickness );
		}

		void rect_shadow( int x, int y, int w, int h, c_color col, float thickness = 25.f, float rounding = 0.f )
		{
			if ( !m_draw_list )
				return;
			m_draw_list->AddShadowRect(
				ImVec2( static_cast<float>( x ), static_cast<float>( y ) ),
				ImVec2( static_cast<float>( x + w ), static_cast<float>( y + h ) ),
				col_u32( col ),
				thickness,
				ImVec2( 0.f, 0.f ),
				ImDrawFlags_None,
				rounding );
		}

		void image( int x, int y, int w, int h, ImTextureID texture_id, c_color col, float rounding = 0.f )
		{
			image( x, y, w, h, texture_id, col, rounding, c_vector_2d( 0.f, 0.f ), c_vector_2d( 1.f, 1.f ) );
		}

		void image( int x, int y, int w, int h, ImTextureID texture_id, c_color col, float rounding, c_vector_2d uv0, c_vector_2d uv1 )
		{
			if ( !m_draw_list )
				return;
			if ( override_resource )
				texture_id = reinterpret_cast<ImTextureID>( override_resource );
			const ImVec2 p0( static_cast<float>( x ), static_cast<float>( y ) );
			const ImVec2 p1( static_cast<float>( x + w ), static_cast<float>( y + h ) );
			const ImVec2 u0( uv0.x, uv0.y );
			const ImVec2 u1( uv1.x, uv1.y );
			if ( rounding > 0.f )
				m_draw_list->AddImageRounded( texture_id, p0, p1, u0, u1, col_u32( col ), rounding );
			else
				m_draw_list->AddImage( texture_id, p0, p1, u0, u1, col_u32( col ) );
		}

		void set_linear_alpha( int vert_start_idx, int vert_end_idx, ImVec2 gradient_p0, ImVec2 gradient_p1, ImU32 col0, ImU32 col1 )
		{
			if ( !m_draw_list )
				return;
			const ImVec2 gradient_extent( gradient_p1.x - gradient_p0.x, gradient_p1.y - gradient_p0.y );
			float gradient_inv_length2 = 1.0f / ImLengthSqr( gradient_extent );
			const int r0 = ( col0 >> IM_COL32_R_SHIFT ) & 0xFF;
			const int g0 = ( col0 >> IM_COL32_G_SHIFT ) & 0xFF;
			const int b0 = ( col0 >> IM_COL32_B_SHIFT ) & 0xFF;
			const int a0 = ( col0 >> IM_COL32_A_SHIFT ) & 0xFF;
			const int r1 = ( col1 >> IM_COL32_R_SHIFT ) & 0xFF;
			const int g1 = ( col1 >> IM_COL32_G_SHIFT ) & 0xFF;
			const int b1 = ( col1 >> IM_COL32_B_SHIFT ) & 0xFF;
			const int a1 = ( col1 >> IM_COL32_A_SHIFT ) & 0xFF;
			for ( int i = vert_start_idx; i < vert_end_idx; i++ )
			{
				ImDrawVert* v = &m_draw_list->VtxBuffer[i];
				const ImVec2 delta( v->pos.x - gradient_p0.x, v->pos.y - gradient_p0.y );
				float d = ImDot( delta, gradient_extent );
				float t = ImClamp( d * gradient_inv_length2, 0.0f, 1.0f );
				int r = static_cast<int>( r0 + ( r1 - r0 ) * t );
				int g = static_cast<int>( g0 + ( g1 - g0 ) * t );
				int b = static_cast<int>( b0 + ( b1 - b0 ) * t );
				int a = static_cast<int>( a0 + ( a1 - a0 ) * t );
				v->col = IM_COL32( r, g, b, a );
			}
		}

		void fade_rect_filled( int x, int y, int w, int h, c_color col1, c_color col2, fade_direction direction, float rounding = 0.f, draw_flags flags = draw_flags_none )
		{
			if ( !m_draw_list )
				return;
			const ImVec2 p0( static_cast<float>( x ), static_cast<float>( y ) );
			const ImVec2 p1( static_cast<float>( x + w ), static_cast<float>( y + h ) );
			const int vtx_start = m_draw_list->VtxBuffer.Size;
			m_draw_list->AddRectFilled( p0, p1, col_u32( col1 ), rounding, to_imgui_flags( flags ) );
			const int vtx_end = m_draw_list->VtxBuffer.Size;
			ImVec2 g0 = p0;
			ImVec2 g1 = p1;
			if ( direction == vertically )
			{
				g0 = ImVec2( p0.x, p0.y );
				g1 = ImVec2( p0.x, p1.y );
			}
			else if ( direction == horizontally )
			{
				g0 = ImVec2( p0.x, p0.y );
				g1 = ImVec2( p1.x, p0.y );
			}
			else if ( direction == diagonally )
			{
				g0 = p0;
				g1 = p1;
			}
			else
			{
				g0 = ImVec2( p1.x, p0.y );
				g1 = ImVec2( p0.x, p1.y );
			}
			set_linear_alpha( vtx_start, vtx_end, g0, g1, col_u32( col1 ), col_u32( col2 ) );
		}

		void fade_rect( int x, int y, int w, int h, c_color col, c_color col2, fade_direction direction, float rounding, float thickness )
		{
			if ( !m_draw_list )
				return;
			const ImVec2 p0( static_cast<float>( x ), static_cast<float>( y ) );
			const ImVec2 p1( static_cast<float>( x + w ), static_cast<float>( y + h ) );
			const int vtx_start = m_draw_list->VtxBuffer.Size;
			m_draw_list->AddRect( p0, p1, col_u32( col ), rounding, 0, thickness );
			const int vtx_end = m_draw_list->VtxBuffer.Size;
			ImVec2 g0 = p0;
			ImVec2 g1 = p1;
			if ( direction == vertically )
			{
				g0 = ImVec2( p0.x, p0.y );
				g1 = ImVec2( p0.x, p1.y );
			}
			else if ( direction == horizontally )
			{
				g0 = ImVec2( p0.x, p0.y );
				g1 = ImVec2( p1.x, p0.y );
			}
			else if ( direction == diagonally )
			{
				g0 = p0;
				g1 = p1;
			}
			else
			{
				g0 = ImVec2( p1.x, p0.y );
				g1 = ImVec2( p0.x, p1.y );
			}
			set_linear_alpha( vtx_start, vtx_end, g0, g1, col_u32( col ), col_u32( col2 ) );
		}

		void gradient( int x, int y, int w, int h, c_color color, c_color color2, fade_direction flags, int rounding = 0, c_color background_helper = c_color( ), ImDrawFlags draw_flags = 0 )
		{
			(void)background_helper;
			(void)draw_flags;
			fade_rect_filled( x, y, w, h, color, color2, flags, static_cast<float>( rounding ) );
		}

		void gradient( c_vector_2d pos, c_vector_2d size, c_color color, c_color color2, fade_direction flags, int rounding = 0, c_color background_helper = c_color( ), ImDrawFlags draw_flags = ImDrawFlags_RoundCornersAll )
		{
			gradient( static_cast<int>( pos.x ), static_cast<int>( pos.y ), static_cast<int>( size.x ), static_cast<int>( size.y ), color, color2, flags, rounding, background_helper, draw_flags );
		}

		void rect_filled_multi_color( int x, int y, int w, int h, c_gradient_data gradient_data, float rounding = 0.f, draw_flags flags = draw_flags_none )
		{
			if ( !m_draw_list )
				return;
			(void)rounding;
			(void)flags;
			m_draw_list->AddRectFilledMultiColor(
				ImVec2( static_cast<float>( x ), static_cast<float>( y ) ),
				ImVec2( static_cast<float>( x + w ), static_cast<float>( y + h ) ),
				col_u32( gradient_data.m_top_l ),
				col_u32( gradient_data.m_top_r ),
				col_u32( gradient_data.m_bot_r ),
				col_u32( gradient_data.m_bot_l ) );
		}

		void radial_gradient( int x, int y, int w, int h, float radius, c_color col1, c_color col2 )
		{
			if ( !m_draw_list )
				return;
			const ImVec2 center( static_cast<float>( x ) + w * 0.5f, static_cast<float>( y ) + h * 0.5f );
			const float r = radius > 0.f ? radius : ( std::max )( w, h ) * 0.5f;
			m_draw_list->AddCircleFilled( center, r, col_u32( col1 ), 48 );
			m_draw_list->AddCircleFilled( center, r * 0.35f, col_u32( col2 ), 32 );
		}

		void radial_gradient_rect_filled( int x, int y, int w, int h, c_vector_2d size, float rounding, c_color col1, c_color col2 )
		{
			(void)size;
			rect_filled( x, y, w, h, col2, rounding );
			radial_gradient( x, y, w, h, 0.f, col1, col2 );
		}

		void enlarged_arrow( c_vector_2d pos, c_color col, int dir, float scale )
		{
			if ( !m_draw_list )
				return;
			const float s = scale;
			ImVec2 a, b, c;
			if ( dir == 0 )
			{
				a = ImVec2( pos.x, pos.y + s );
				b = ImVec2( pos.x + s * 2.f, pos.y + s );
				c = ImVec2( pos.x + s, pos.y );
			}
			else if ( dir == 1 )
			{
				a = ImVec2( pos.x, pos.y );
				b = ImVec2( pos.x + s * 2.f, pos.y );
				c = ImVec2( pos.x + s, pos.y + s );
			}
			else if ( dir == 2 )
			{
				a = ImVec2( pos.x + s, pos.y );
				b = ImVec2( pos.x + s, pos.y + s * 2.f );
				c = ImVec2( pos.x, pos.y + s );
			}
			else
			{
				a = ImVec2( pos.x, pos.y );
				b = ImVec2( pos.x, pos.y + s * 2.f );
				c = ImVec2( pos.x + s, pos.y + s );
			}
			m_draw_list->AddTriangleFilled( a, b, c, col_u32( col ) );
		}

		void check_mark( int x, int y, float size, c_color col )
		{
			if ( !m_draw_list )
				return;
			const float thickness = ( std::max )( size / 5.f, 1.f );
			const ImVec2 a( static_cast<float>( x ) + size * 0.15f, static_cast<float>( y ) + size * 0.5f );
			const ImVec2 b( static_cast<float>( x ) + size * 0.4f, static_cast<float>( y ) + size * 0.75f );
			const ImVec2 c( static_cast<float>( x ) + size * 0.85f, static_cast<float>( y ) + size * 0.25f );
			m_draw_list->PathLineTo( a );
			m_draw_list->PathLineTo( b );
			m_draw_list->PathLineTo( c );
			m_draw_list->PathStroke( col_u32( col ), 0, thickness );
		}

		void circle_filled( c_vector_2d center, float radius, c_color col, int segments = 0 )
		{
			if ( !m_draw_list )
				return;
			const int segs = segments > 0 ? segments : 48;
			m_draw_list->AddCircleFilled( ImVec2( center.x, center.y ), radius, col_u32( col ), segs );
		}

		void circle( c_vector_2d center, float radius, c_color col, float thickness = 1.5f, int segments = 0 )
		{
			if ( !m_draw_list )
				return;
			const int segs = segments > 0 ? segments : 48;
			m_draw_list->AddCircle( ImVec2( center.x, center.y ), radius, col_u32( col ), segs, thickness );
		}

		void rect_filled_f( float x, float y, float w, float h, c_color col, float rounding = 0.f, draw_flags flags = draw_flags_none )
		{
			if ( !m_draw_list || w <= 0.f || h <= 0.f )
				return;
			m_draw_list->AddRectFilled(
				ImVec2( x, y ),
				ImVec2( x + w, y + h ),
				col_u32( col ),
				rounding,
				to_imgui_flags( flags ) );
		}

		void rect_f( float x, float y, float w, float h, c_color col, float rounding = 0.f, float thickness = 1.f, draw_flags flags = draw_flags_none )
		{
			if ( !m_draw_list || w <= 0.f || h <= 0.f )
				return;
			m_draw_list->AddRect(
				ImVec2( x, y ),
				ImVec2( x + w, y + h ),
				col_u32( col ),
				rounding,
				to_imgui_flags( flags ),
				thickness );
		}

		void frosted_panel( float x, float y, float w, float h, float rounding, bool sample_ui = false )
		{
			if ( !m_draw_list || w <= 0.f || h <= 0.f || !g_style )
				return;
			c_blur* blur = sample_ui ? g_blur_ui.get( ) : g_blur.get( );
			if ( !blur || !blur->ready( ) )
				blur = g_blur.get( );
			if ( blur && blur->ready( ) )
				blur->draw_region( m_draw_list, c_vector_2d( x, y ), c_vector_2d( x + w, y + h ), rounding, c_color( 255, 255, 255, 255 ) );
			rect_filled_f( x, y, w, h, g_style->popup_plate, rounding );
			stroke_rounded( x, y, w, h, rounding, g_style->popup_border, 1.f );
		}

		void circle_shadow( c_vector_2d center, float radius, c_color col, float thickness = 12.f )
		{
			if ( !m_draw_list )
				return;
			m_draw_list->AddShadowCircle(
				ImVec2( center.x, center.y ),
				radius,
				col_u32( col ),
				thickness,
				ImVec2( 0.f, 1.f ),
				ImDrawFlags_None,
				24 );
		}

		void checkerboard( int x, int y, int w, int h, int cell = 4, c_color a = c_color( 210, 210, 214 ), c_color b = c_color( 150, 152, 158 ) )
		{
			checkerboard_rounded( x, y, w, h, 0.f, cell, a, b );
		}

		void checkerboard_rounded( int x, int y, int w, int h, float rounding, int cell = 4, c_color a = c_color( 210, 210, 214 ), c_color b = c_color( 150, 152, 158 ) )
		{
			if ( !m_draw_list || w <= 0 || h <= 0 )
				return;

			const float fx = static_cast<float>( x );
			const float fy = static_cast<float>( y );
			const float fw = static_cast<float>( w );
			const float fh = static_cast<float>( h );
			const float r = ImClamp( rounding, 0.f, ( std::min )( fw, fh ) * 0.5f );

			m_draw_list->AddRectFilled( ImVec2( fx, fy ), ImVec2( fx + fw, fy + fh ), col_u32( b ), r );

			const int cols = ( w + cell - 1 ) / cell;
			const int rows = ( h + cell - 1 ) / cell;
			for ( int row = 0; row < rows; ++row )
			{
				for ( int col = 0; col < cols; ++col )
				{
					if ( ( ( row + col ) & 1 ) == 0 )
						continue;

					const float cx0 = fx + static_cast<float>( col * cell );
					const float cy0 = fy + static_cast<float>( row * cell );
					const float cx1 = ( std::min )( cx0 + static_cast<float>( cell ), fx + fw );
					const float cy1 = ( std::min )( cy0 + static_cast<float>( cell ), fy + fh );
					if ( !point_in_rounded_rect( cx0 + 0.5f, cy0 + 0.5f, fx, fy, fw, fh, r )
						|| !point_in_rounded_rect( cx1 - 0.5f, cy0 + 0.5f, fx, fy, fw, fh, r )
						|| !point_in_rounded_rect( cx0 + 0.5f, cy1 - 0.5f, fx, fy, fw, fh, r )
						|| !point_in_rounded_rect( cx1 - 0.5f, cy1 - 0.5f, fx, fy, fw, fh, r ) )
						continue;

					m_draw_list->AddRectFilled( ImVec2( cx0, cy0 ), ImVec2( cx1, cy1 ), col_u32( a ) );
				}
			}
		}

		void seal_rounded_corners( float x, float y, float w, float h, float r, c_color col )
		{
			if ( !m_draw_list || r <= 0.5f )
				return;

			const ImU32 c = col_u32( col );
			auto corner = [&]( float cx, float cy, float a0, float a1, float ox, float oy )
			{
				m_draw_list->PathClear( );
				m_draw_list->PathLineTo( ImVec2( ox, oy ) );
				m_draw_list->PathArcTo( ImVec2( cx, cy ), r, a0, a1, 24 );
				m_draw_list->PathFillConvex( c );
			};

			corner( x + r, y + r, IM_PI, IM_PI * 1.5f, x, y );
			corner( x + w - r, y + r, IM_PI * 1.5f, IM_PI * 2.f, x + w, y );
			corner( x + w - r, y + h - r, 0.f, IM_PI * 0.5f, x + w, y + h );
			corner( x + r, y + h - r, IM_PI * 0.5f, IM_PI, x, y + h );
		}

		void stroke_rounded( float x, float y, float w, float h, float rounding, c_color col, float thickness = 1.f )
		{
			if ( !m_draw_list || w <= 0.f || h <= 0.f )
				return;
			const float r = ImClamp( rounding, 0.f, ( std::min )( w, h ) * 0.5f );
			m_draw_list->AddRect( ImVec2( x + 0.5f, y + 0.5f ), ImVec2( x + w - 0.5f, y + h - 0.5f ), col_u32( col ), r, 0, thickness );
		}

		void alpha_rect( int x, int y, int w, int h, c_color col, float rounding = 0.f, c_color seal = c_color( 17, 19, 27 ), bool stroke = true )
		{
			if ( !m_draw_list || w <= 0 || h <= 0 )
				return;

			const float fx = static_cast<float>( x );
			const float fy = static_cast<float>( y );
			const float fw = static_cast<float>( w );
			const float fh = static_cast<float>( h );
			const float r = ImClamp( rounding, 0.f, ( std::min )( fw, fh ) * 0.5f );

			// Solid plate first so translucent colors never expose a square checker silhouette.
			m_draw_list->AddRectFilled( ImVec2( fx, fy ), ImVec2( fx + fw, fy + fh ), col_u32( seal ), r );

			if ( col.a < 255 )
			{
				checkerboard_rounded( x, y, w, h, r, 3, c_color( 58, 60, 68 ), c_color( 36, 38, 46 ) );
				if ( r > 0.5f )
					seal_rounded_corners( fx, fy, fw, fh, r, seal );
			}

			m_draw_list->AddRectFilled( ImVec2( fx, fy ), ImVec2( fx + fw, fy + fh ), col_u32( col ), r );
			if ( stroke && col.a > 8 )
				stroke_rounded( fx, fy, fw, fh, r, c_color( 255, 255, 255, static_cast<int>( 28.f * ( static_cast<float>( col.a ) / 255.f ) ) ), 1.f );
		}

		void alpha_gradient( int x, int y, int w, int h, c_color solid, float rounding = 0.f, c_color seal = c_color( 15, 17, 25 ) )
		{
			if ( !m_draw_list || w <= 0 || h <= 0 )
				return;

			const float fx = static_cast<float>( x );
			const float fy = static_cast<float>( y );
			const float fw = static_cast<float>( w );
			const float fh = static_cast<float>( h );
			const float r = ImClamp( rounding, 0.f, ( std::min )( fw, fh ) * 0.5f );
			(void)seal;

			checkerboard_rounded( x, y, w, h, r, 4, c_color( 58, 60, 68 ), c_color( 36, 38, 46 ) );

			constexpr int strips = 64;
			const float sw = fw / static_cast<float>( strips );
			for ( int i = 0; i < strips; ++i )
			{
				const float t = ( static_cast<float>( i ) + 0.5f ) / static_cast<float>( strips );
				const int alpha = static_cast<int>( t * 255.f + 0.5f );
				ImDrawFlags flags = ImDrawFlags_RoundCornersNone;
				if ( i == 0 )
					flags = ImDrawFlags_RoundCornersLeft;
				else if ( i == strips - 1 )
					flags = ImDrawFlags_RoundCornersRight;

				const float x0 = fx + sw * static_cast<float>( i );
				const float x1 = ( i == strips - 1 ) ? ( fx + fw ) : ( fx + sw * static_cast<float>( i + 1 ) );
				m_draw_list->AddRectFilled(
					ImVec2( x0, fy ),
					ImVec2( x1 + 0.5f, fy + fh ),
					IM_COL32( solid.r, solid.g, solid.b, alpha ),
					r,
					flags );
			}

			stroke_rounded( fx, fy, fw, fh, r, c_color( 48, 52, 64, 220 ), 1.f );
		}

		void hue_gradient( int x, int y, int w, int h, float rounding = 0.f, c_color seal = c_color( 15, 17, 25 ) )
		{
			if ( !m_draw_list || w <= 0 || h <= 0 )
				return;

			const float fx = static_cast<float>( x );
			const float fy = static_cast<float>( y );
			const float fw = static_cast<float>( w );
			const float fh = static_cast<float>( h );
			const float r = ImClamp( rounding, 0.f, ( std::min )( fw, fh ) * 0.5f );
			(void)seal;

			constexpr int strips = 64;
			const float sw = fw / static_cast<float>( strips );
			for ( int i = 0; i < strips; ++i )
			{
				const float hue = ( static_cast<float>( i ) + 0.5f ) / static_cast<float>( strips ) * 360.f;
				const c_color c = c_color::from_hsv( hue, 1.f, 1.f );
				ImDrawFlags flags = ImDrawFlags_RoundCornersNone;
				if ( i == 0 )
					flags = ImDrawFlags_RoundCornersLeft;
				else if ( i == strips - 1 )
					flags = ImDrawFlags_RoundCornersRight;

				const float x0 = fx + sw * static_cast<float>( i );
				const float x1 = ( i == strips - 1 ) ? ( fx + fw ) : ( fx + sw * static_cast<float>( i + 1 ) );
				m_draw_list->AddRectFilled(
					ImVec2( x0, fy ),
					ImVec2( x1 + 0.5f, fy + fh ),
					col_u32( c ),
					r,
					flags );
			}

			stroke_rounded( fx, fy, fw, fh, r, c_color( 48, 52, 64, 220 ), 1.f );
		}

		static bool point_in_rounded_rect( float px, float py, float x, float y, float w, float h, float r )
		{
			if ( px < x || py < y || px > x + w || py > y + h )
				return false;
			if ( r <= 0.f )
				return true;
			if ( px >= x + r && px <= x + w - r )
				return true;
			if ( py >= y + r && py <= y + h - r )
				return true;

			const float cx = ( px < x + r ) ? ( x + r ) : ( x + w - r );
			const float cy = ( py < y + r ) ? ( y + r ) : ( y + h - r );
			const float dx = px - cx;
			const float dy = py - cy;
			return dx * dx + dy * dy <= r * r;
		}

		void line( c_vector_2d a, c_vector_2d b, c_color col, float thickness = 1.f )
		{
			if ( !m_draw_list )
				return;
			m_draw_list->AddLine( ImVec2( a.x, a.y ), ImVec2( b.x, b.y ), col_u32( col ), thickness );
		}

		void push_clip( int x, int y, int w, int h )
		{
			if ( !m_draw_list )
				return;
			m_draw_list->PushClipRect(
				ImVec2( static_cast<float>( x ), static_cast<float>( y ) ),
				ImVec2( static_cast<float>( x + w ), static_cast<float>( y + h ) ),
				true );
		}

		void restore_clip( )
		{
			if ( !m_draw_list )
				return;
			m_draw_list->PopClipRect( );
		}

		static c_vector_2d snap( c_vector_2d pos )
		{
			return c_vector_2d( std::floor( pos.x + 0.5f ), std::floor( pos.y + 0.5f ) );
		}

		c_vector_2d measure_text( ImFont* font, const char* text, float font_size = 0.f ) const
		{
			if ( !font || !text )
				return {};
			const float size = font_size > 0.f ? font_size : font->LegacySize;
			const ImVec2 s = font->CalcTextSizeA( size, FLT_MAX, 0.f, text );
			return c_vector_2d( s.x, s.y );
		}

		c_vector_2d measure_text( const std::string& key, const char* text, float font_size = 0.f ) const
		{
			return measure_text( g_fonts ? g_fonts->get_font( key ) : nullptr, text, font_size );
		}

		void text( ImFont* font, c_vector_2d pos, c_color color, const char* text, float font_size = 0.f, modifiers::font_flags flags = modifiers::none, c_color shadow = modifiers::default_shadow )
		{
			if ( !m_draw_list || !font || !text || !text[0] )
				return;

			pos = snap( pos );
			const ImVec2 ipos = pos.imgui( );
			float size = font_size > 0.f ? font_size : font->LegacySize;
			if ( std::fabs( size - font->LegacySize ) < 0.51f )
				size = font->LegacySize;
			const bool push_tex = font->OwnerAtlas && m_draw_list->_CmdHeader.TexRef != font->OwnerAtlas->TexRef;
			if ( push_tex )
				m_draw_list->PushTexture( font->OwnerAtlas->TexRef );

			if ( flags == modifiers::drop_shadow )
			{
				m_draw_list->AddText( font, size, ImVec2( ipos.x + 1.f, ipos.y + 1.f ), col_u32( shadow ), text );
			}
			else if ( flags == modifiers::outline )
			{
				const ImU32 black = col_u32( shadow );
				m_draw_list->AddText( font, size, ImVec2( ipos.x - 1.f, ipos.y - 1.f ), black, text );
				m_draw_list->AddText( font, size, ImVec2( ipos.x, ipos.y - 1.f ), black, text );
				m_draw_list->AddText( font, size, ImVec2( ipos.x + 1.f, ipos.y - 1.f ), black, text );
				m_draw_list->AddText( font, size, ImVec2( ipos.x - 1.f, ipos.y ), black, text );
				m_draw_list->AddText( font, size, ImVec2( ipos.x + 1.f, ipos.y ), black, text );
				m_draw_list->AddText( font, size, ImVec2( ipos.x - 1.f, ipos.y + 1.f ), black, text );
				m_draw_list->AddText( font, size, ImVec2( ipos.x, ipos.y + 1.f ), black, text );
				m_draw_list->AddText( font, size, ImVec2( ipos.x + 1.f, ipos.y + 1.f ), black, text );
			}

			m_draw_list->AddText( font, size, ipos, col_u32( color ), text );
			if ( push_tex )
				m_draw_list->PopTexture( );
		}

		void text( const std::string& key, c_vector_2d pos, c_color color, const char* str, float font_size = 0.f, modifiers::font_flags flags = modifiers::none, c_color shadow = modifiers::default_shadow )
		{
			text( g_fonts ? g_fonts->get_font( key ) : nullptr, pos, color, str, font_size, flags, shadow );
		}

		void text_outlined( ImFont* font, c_vector_2d pos, c_color color, const char* str, float font_size = 0.f )
		{
			text( font, pos, color, str, font_size, modifiers::outline );
		}

		void text_outlined( const std::string& key, c_vector_2d pos, c_color color, const char* str, float font_size = 0.f )
		{
			text( key, pos, color, str, font_size, modifiers::outline );
		}
	};

	inline std::shared_ptr<c_render> g_render = std::make_shared<c_render>( );
}
