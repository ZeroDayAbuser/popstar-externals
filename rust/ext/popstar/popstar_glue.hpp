#pragma once

#include <unordered_map>
#include <core/framework/gui/backend/math/math.hxx>
#include <core/framework/gui/backend/manager/keybinds/keybinds.hxx>
#include <menu/framework/color.hpp>
#include <menu/framework/widgets/keybind/keybind.hpp>

namespace popstar_glue
{
	struct ColorBridge
	{
		Color* src {};
		core::gui::c_color dst {};
	};
	struct KeyBridge
	{
		Keybind* src {};
		core::gui::key_var_t dst {};
	};

	inline std::unordered_map<Color*, ColorBridge> g_colors;
	inline std::unordered_map<Keybind*, KeyBridge> g_keys;

	inline core::gui::c_color* color( Color* src )
	{
		if ( !src )
			return nullptr;
		auto& b = g_colors[src];
		b.src = src;
		b.dst = core::gui::c_color( src->r, src->g, src->b, src->a );
		return &b.dst;
	}

	inline core::gui::key_var_t* key( Keybind* src )
	{
		if ( !src )
			return nullptr;
		auto& b = g_keys[src];
		b.src = src;
		b.dst.key = src->key;
		if ( src->mode == 3 )
			b.dst.mode = core::gui::key_mode_t::always;
		else if ( src->mode == 2 )
			b.dst.mode = core::gui::key_mode_t::toggle;
		else
			b.dst.mode = core::gui::key_mode_t::hold;
		return &b.dst;
	}

	inline void pull( )
	{
		for ( auto& [ptr, b] : g_colors )
		{
			if ( !b.src )
				continue;
			b.dst = core::gui::c_color( b.src->r, b.src->g, b.src->b, b.src->a );
		}
		for ( auto& [ptr, b] : g_keys )
		{
			if ( !b.src )
				continue;
			b.dst.key = b.src->key;
		}
	}

	inline void push( )
	{
		for ( auto& [ptr, b] : g_colors )
		{
			if ( !b.src )
				continue;
			b.src->r = static_cast<std::uint8_t>( b.dst.r );
			b.src->g = static_cast<std::uint8_t>( b.dst.g );
			b.src->b = static_cast<std::uint8_t>( b.dst.b );
			b.src->a = static_cast<std::uint8_t>( b.dst.a );
		}
		for ( auto& [ptr, b] : g_keys )
		{
			if ( !b.src )
				continue;
			b.src->key = b.dst.key;
			if ( b.dst.mode == core::gui::key_mode_t::always )
				b.src->mode = 3;
			else if ( b.dst.mode == core::gui::key_mode_t::toggle )
				b.src->mode = 2;
			else
				b.src->mode = 1;
		}
	}
}
