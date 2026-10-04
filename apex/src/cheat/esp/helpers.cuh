#pragma once

#include <dependencies/includes.h>
#include <src/cheat/helper/helper.cuh>
#include <algorithm>
#include <cmath>
#include <cfloat>

namespace esp
{
	constexpr int k_count = 16;

	struct store_t
	{
		ImDrawList* draw = nullptr;
		ImVec2 box_min { };
		ImVec2 box_max { };
		ImU32 col = 0;
		ImU32 hp_col = 0;
		ImU32 sh_col = 0;
		float alpha = 1.f;

		data::_vector2 screen[ k_count ] { };
		data::_vector3 world[ k_count ] { };
		bool ok[ k_count ] { };
	};

	inline store_t store { };

	inline auto bounds( classes::c_entity& player ) -> bool
	{
		using b = data::e_bone_type;
		static constexpr b k_ids[ k_count ] = {
			b::head , b::neck , b::upper_chest , b::pelvis ,
			b::left_shoulder , b::left_elbow , b::left_hand ,
			b::right_shoulder , b::right_elbow , b::right_hand ,
			b::left_hip , b::left_knee , b::left_foot ,
			b::right_hip , b::right_knee , b::right_foot ,
		};

		int valid = 0;
		float min_x = FLT_MAX , min_y = FLT_MAX;
		float max_x = -FLT_MAX , max_y = -FLT_MAX;

		for ( int i = 0; i < k_count; ++i )
		{
			store.ok[ i ] = false;

			auto w = player.bone_pos( k_ids[ i ] );
			if ( w.is_zero( ) ) continue;

			auto s = data::world_to_screen( w );
			if ( !data::on_screen( s ) ) continue;

			store.world[ i ] = w;
			store.screen[ i ] = s;
			store.ok[ i ] = true;
			++valid;

			min_x = ( std::min )( min_x , s.x );
			min_y = ( std::min )( min_y , s.y );
			max_x = ( std::max )( max_x , s.x );
			max_y = ( std::max )( max_y , s.y );
		}

		if ( valid >= 3 )
		{
			const float pad_x = ( max_x - min_x ) * 0.22f;
			const float pad_y = ( max_y - min_y ) * 0.12f;
			store.box_min = ImVec2( min_x - pad_x , min_y - pad_y );
			store.box_max = ImVec2( max_x + pad_x , max_y + pad_y );
			return true;
		}

		auto head_w = player.bone_pos( b::head );
		if ( head_w.is_zero( ) )
			head_w = data::_vector3( player.m_origin.x , player.m_origin.y , player.m_origin.z + 70.f );

		auto head = store.ok[ 0 ] ? store.screen[ 0 ] : data::world_to_screen( head_w );
		auto feet = data::world_to_screen( player.m_origin );
		if ( !data::on_screen( head ) || !data::on_screen( feet ) )
			return false;

		const float height = feet.y - head.y;
		if ( height < 8.f )
			return false;

		const float width = height * 0.55f;
		store.box_min = ImVec2( head.x - width * 0.5f , head.y - height * 0.05f );
		store.box_max = ImVec2( store.box_min.x + width , store.box_min.y + height * 1.12f );
		return true;
	}

	inline ImFont* name_font = nullptr;
	inline ImFont* distance_font = nullptr;
	inline ImFont* flags_font = nullptr;

	inline auto box( ImDrawList* canvas , ImVec2 box_min , ImVec2 box_max , ImU32 color , float thickness = 2.f , bool outline = true , bool filled = false , ImU32 filled_col = IM_COL32( 0 , 0 , 0 , 100 ) ) -> void
	{
		const int old = canvas->Flags;
		canvas->Flags &= ~( ImDrawListFlags_AntiAliasedLines | ImDrawListFlags_AntiAliasedLinesUseTex | ImDrawListFlags_AntiAliasedFill );

		box_min.x = std::round( box_min.x );
		box_min.y = std::round( box_min.y );
		box_max.x = std::round( box_max.x );
		box_max.y = std::round( box_max.y );

		const ImU32 black = IM_COL32( 0 , 0 , 0 , ( ( color >> IM_COL32_A_SHIFT ) & 0xff ) );

		if ( outline )
		{
			canvas->AddRect( box_min , box_max , black , 0.f );
			canvas->AddRect(
				ImVec2( box_min.x - thickness , box_min.y - thickness ) ,
				ImVec2( box_max.x + thickness , box_max.y + thickness ) ,
				black , 0.f );
		}

		if ( filled )
		{
			canvas->AddRectFilled(
				ImVec2( box_min.x + 1.f , box_min.y + 1.f ) ,
				ImVec2( box_max.x - 1.f , box_max.y - 1.f ) ,
				filled_col );
		}

		canvas->AddRect(
			ImVec2( box_min.x - ( thickness - 1.f ) , box_min.y - ( thickness - 1.f ) ) ,
			ImVec2( box_max.x + ( thickness - 1.f ) , box_max.y + ( thickness - 1.f ) ) ,
			color , 0.f , 0 , thickness - 1.f );

		canvas->Flags = old;
	}

	inline auto corner_box( ImDrawList* canvas , ImVec2 box_min , ImVec2 box_max , ImU32 color , float thickness = 2.f , bool outline = true ) -> void
	{
		const int old = canvas->Flags;
		canvas->Flags &= ~( ImDrawListFlags_AntiAliasedLines | ImDrawListFlags_AntiAliasedLinesUseTex | ImDrawListFlags_AntiAliasedFill );

		box_min.x = std::round( box_min.x );
		box_min.y = std::round( box_min.y );
		box_max.x = std::round( box_max.x );
		box_max.y = std::round( box_max.y );

		const float expand = ( std::max )( 1.f , thickness - 1.f );
		const float x1 = box_min.x - expand;
		const float y1 = box_min.y - expand;
		const float x2 = box_max.x + expand;
		const float y2 = box_max.y + expand;
		const float w = x2 - x1;
		const float h = y2 - y1;
		if ( w < 2.f || h < 2.f )
		{
			canvas->Flags = old;
			return;
		}

		float len = ( w < h ? w : h ) * 0.25f;
		if ( len < 3.f ) len = 3.f;
		const float max_arm = ( w < h ? w : h ) * 0.45f;
		if ( len > max_arm ) len = max_arm;
		if ( len > 16.f ) len = 16.f;

		const ImU32 black = IM_COL32( 0 , 0 , 0 , ( color >> IM_COL32_A_SHIFT ) & 0xff );

		auto corner = [ & ]( float ox , float oy , float dx , float dy , ImU32 col )
		{
			const float hx0 = ( dx > 0.f ) ? ox : ox - len + 1.f;
			const float hy = oy;
			const float vx = ox;
			const float vy0 = ( dy > 0.f ) ? oy : oy - len + 1.f;

			if ( outline )
			{
				canvas->AddRectFilled( ImVec2( hx0 - 1.f , hy - 1.f ) , ImVec2( hx0 + len + 1.f , hy + 2.f ) , black );
				canvas->AddRectFilled( ImVec2( vx - 1.f , vy0 - 1.f ) , ImVec2( vx + 2.f , vy0 + len + 1.f ) , black );
			}

			canvas->AddRectFilled( ImVec2( hx0 , hy ) , ImVec2( hx0 + len , hy + 1.f ) , col );
			canvas->AddRectFilled( ImVec2( vx , vy0 ) , ImVec2( vx + 1.f , vy0 + len ) , col );

			if ( outline )
			{
				const float ix = ox + dx;
				const float iy = oy + dy;
				canvas->AddRectFilled( ImVec2( ix , iy ) , ImVec2( ix + 1.f , iy + 1.f ) , black );
			}
		};

		corner( x1 , y1 , +1.f , +1.f , color );
		corner( x2 - 1.f , y1 , -1.f , +1.f , color );
		corner( x1 , y2 - 1.f , +1.f , -1.f , color );
		corner( x2 - 1.f , y2 - 1.f , -1.f , -1.f , color );

		canvas->Flags = old;
	}

	inline auto health_bar( ImDrawList* canvas , ImVec2 box_min , ImVec2 box_max , float health , float max_health , float box_thickness , bool box_outline , float bar_width , int side , float gap , int mode , ImU32 static_col , ImU32 end_col ) -> void
	{
		if ( max_health <= 0.f )
			return;

		const int old_flags = canvas->Flags;
		canvas->Flags &= ~ImDrawListFlags_AntiAliasedLines;

		box_min.x = std::round( box_min.x );
		box_min.y = std::round( box_min.y );
		box_max.x = std::round( box_max.x );
		box_max.y = std::round( box_max.y );

		const float ratio = std::clamp( health / max_health , 0.f , 1.f );
		const float outer = box_thickness - 1.f;
		const float adjust = box_outline ? 0.f : 1.f;

		ImVec2 bar_min , bar_max;

		switch ( side )
		{
		case 0:
			bar_min = ImVec2( std::round( box_min.x - outer - gap - bar_width ) - 1.f , std::round( box_min.y - outer + adjust ) );
			bar_max = ImVec2( std::round( box_min.x - outer - gap ) - 1.f , std::round( box_max.y + outer - adjust ) );
			break;
		case 1:
			bar_min = ImVec2( std::round( box_max.x + outer + gap ) , std::round( box_min.y - outer + adjust ) );
			bar_max = ImVec2( std::round( box_max.x + outer + gap + bar_width ) , std::round( box_max.y + outer - adjust ) );
			break;
		case 2:
			bar_min = ImVec2( std::round( box_min.x - outer + adjust ) , std::round( box_min.y - outer - gap - bar_width ) );
			bar_max = ImVec2( std::round( box_max.x + outer - adjust ) , std::round( box_min.y - outer - gap ) );
			break;
		case 3:
			bar_min = ImVec2( std::round( box_min.x - outer + adjust ) , std::round( box_max.y + outer + gap ) + 1.f );
			bar_max = ImVec2( std::round( box_max.x + outer - adjust ) , std::round( box_max.y + outer + gap + bar_width ) + 1.f );
			break;
		default:
			canvas->Flags = old_flags;
			return;
		}

		const bool vertical = ( side == 0 || side == 1 );
		const float bar_length = vertical ? bar_max.y - bar_min.y : bar_max.x - bar_min.x;
		const float health_length = std::round( bar_length * ratio );

		canvas->AddRect( ImVec2( bar_min.x - 1.f , bar_min.y - 1.f ) , ImVec2( bar_max.x + 1.f , bar_max.y + 1.f ) , IM_COL32( 0 , 0 , 0 , 255 ) );
		canvas->AddRectFilled( bar_min , bar_max , IM_COL32( 30 , 30 , 30 , 200 ) );

		ImVec2 fill_min , fill_max;
		if ( vertical )
		{
			fill_min = ImVec2( bar_min.x , bar_max.y - health_length );
			fill_max = bar_max;
		}
		else
		{
			fill_min = bar_min;
			fill_max = ImVec2( bar_min.x + health_length , bar_max.y );
		}

		if ( mode == 1 )
		{
			const ImU32 top = vertical ? end_col : static_col;
			const ImU32 bot = vertical ? static_col : end_col;
			if ( vertical )
				canvas->AddRectFilledMultiColor( fill_min , fill_max , top , top , bot , bot );
			else
				canvas->AddRectFilledMultiColor( fill_min , fill_max , static_col , end_col , end_col , static_col );
			canvas->Flags = old_flags;
			return;
		}

		ImU32 fill_col;
		if ( mode == 2 )
		{
			if ( ratio > 0.5f )
			{
				const float t = ( ratio - 0.5f ) * 2.f;
				fill_col = IM_COL32( ( int )( 255 * ( 1.f - t ) ) , 255 , 0 , 255 );
			}
			else
			{
				const float t = ratio * 2.f;
				fill_col = IM_COL32( 255 , ( int )( 255 * t ) , 0 , 255 );
			}
		}
		else
		{
			fill_col = static_col;
		}

		canvas->AddRectFilled( fill_min , fill_max , fill_col );
		canvas->Flags = old_flags;
	}

	inline void label( ImDrawList* draw , const std::string& text , ImU32 text_color , const ImVec2& box_pos , const ImVec2& box_size , float offset , helper::sides_t side = helper::sides_t::RIGHT , float side_padding = 4.0f , ImFont* font = nullptr )
	{
		float font_size = font ? font->LegacySize : ImGui::GetFontSize( );
		if ( font_size <= 0.f )
			font_size = 13.f;

		const ImVec2 text_size = font
			? font->CalcTextSizeA( font_size , FLT_MAX , 0.f , text.c_str( ) )
			: ImGui::CalcTextSize( text.c_str( ) );

		helper::sides_t side_helper( side );
		ImVec2 text_pos = side_helper.get_position( box_pos , box_size , ImVec2( text_size.x , text_size.y ) , side_padding );
		text_pos.y += offset;

		for ( auto dx = -1.0f; dx <= 1.0f; dx++ )
		{
			for ( auto dy = -1.0f; dy <= 1.0f; dy++ )
			{
				if ( dx == 0.0f && dy == 0.0f ) continue;
				if ( font )
					draw->AddText( font , font_size , ImVec2( text_pos.x + dx , text_pos.y + dy ) , IM_COL32( 0 , 0 , 0 , 255 ) , text.c_str( ) );
				else
					draw->AddText( ImVec2( text_pos.x + dx , text_pos.y + dy ) , IM_COL32( 0 , 0 , 0 , 255 ) , text.c_str( ) );
			}
		}

		if ( font )
			draw->AddText( font , font_size , text_pos , text_color , text.c_str( ) );
		else
			draw->AddText( text_pos , text_color , text.c_str( ) );
	}
}
