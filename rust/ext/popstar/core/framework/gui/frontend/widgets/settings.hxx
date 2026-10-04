#pragma once

#include <memory>
#include <core/framework/gui/backend/math/math.hxx>

namespace core::gui
{
	class c_style
	{
	public:
		c_color window_bg = c_color( 10, 12, 18, 232 );
		c_color sidebar_bg = c_color( 12, 14, 20, 240 );
		c_color toolbar_bg = c_color( 10, 12, 18, 236 );
		c_color accent = c_color( 214, 218, 226 );
		c_color text = c_color( 210, 213, 222 );
		c_color text_dim = c_color( 108, 114, 128 );
		c_color text_bright = c_color( 236, 238, 245 );
		c_color text_muted = c_color( 148, 154, 168 );
		c_color text_control_col = c_color( 216, 219, 228 );
		c_color outline = c_color( 36, 38, 48, 90 );
		c_color card_bg = c_color( 14, 16, 22, 220 );
		c_color row_line = c_color( 30, 32, 42, 140 );
		c_color frame = c_color( 22, 24, 32, 145 );
		c_color frame_border = c_color( 44, 46, 56, 110 );
		c_color toggle_off = c_color( 30, 34, 44 );
		c_color knob_off = c_color( 120, 125, 138 );
		c_color knob_on = c_color( 245, 246, 250 );
		c_color slider_track = c_color( 30, 34, 44 );
		c_color element_base = c_color( 14, 16, 22 );
		c_color child_top = c_color( 14, 16, 22 );
		c_color shadow = c_color( 0, 0, 0, 130 );
		c_color window_bars = c_color( 8, 10, 16 );
		c_color popup_plate = c_color( 12, 14, 20, 236 );
		c_color popup_border = c_color( 48, 50, 58, 120 );

		float window_w = 620.f;
		float window_h = 460.f;
		float sidebar_w = 128.f;
		float toolbar_h = 42.f;
		float content_pad = 10.f;
		float child_label_gap = 14.f;
		float rounding = 8.f;
		float card_rounding = 6.f;
		float frame_rounding = 4.f;
		float row_height = 30.f;
		float toggle_w = 26.f;
		float toggle_h = 16.f;
		float control_w = 110.f;
		float control_pad = 10.f;
		float label_pad = 10.f;
		float slider_track_w = 110.f;
		float slider_track_h = 5.f;
		float combo_h = 22.f;
		float combo_item_h = 24.f;
		float combo_pad = 5.f;
		float padding = 10.f;
		float animation_speed = 26.f;
		float text_body = 13.f;
		float text_caption = 11.f;
		float text_control = 13.f;
		float text_title = 14.f;
		float spacing = 0.f;
		float element_height = 30.f;
		float checkbox_size = 16.f;
		float button_height = 20.f;
		float slider_height = 5.f;
		float child_header = 0.f;
		float window_title = 42.f;
		float scroll_speed = 18.f;
		float child_w = 226.f;
		float col_gap = 10.f;
		float row_gap = 16.f;
		float logo_size = 72.f;
		float logo_pad = 8.f;

		float control_w_for( float row_w ) const
		{
			const float reserved = label_pad + 72.f + control_pad;
			const float avail = row_w - reserved;
			if ( avail >= control_w )
				return control_w;
			return avail > 80.f ? avail : 80.f;
		}

		float control_x( float row_x, float row_w ) const { return row_x + row_w - control_pad - control_w_for( row_w ); }
		float toggle_x( float row_x, float row_w ) const { return row_x + row_w - control_pad - toggle_w; }
	};

	using style_settings_t = c_style;
	inline std::shared_ptr<c_style> g_style = std::make_shared<c_style>( );
}
