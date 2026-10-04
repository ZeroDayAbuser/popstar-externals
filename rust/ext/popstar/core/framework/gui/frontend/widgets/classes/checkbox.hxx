#pragma once

#include <cmath>
#include <functional>
#include <memory>
#include <string>

#include <core/framework/gui/frontend/widgets/classes/base_element.hxx>
#include <core/framework/gui/frontend/widgets/settings.hxx>
#include <core/framework/gui/backend/render/render.hxx>

namespace core::gui
{
	class c_popup;
	class c_bind_popup;

	class c_checkbox : public c_base_element
	{
	public:
		c_checkbox( std::string label, bool* value )
			: m_value( value )
		{
			m_label = std::move( label );
			m_type = element_type::checkbox;
			m_visible = true;
			m_focus_priority = focus_priority::interactive;
			m_size = c_vector_2d( 200.f, g_style ? g_style->row_height : 37.f );
			if ( m_value && *m_value )
				m_anim = 1.f;
		}

		c_checkbox& attach_popup( std::string title, std::function<void( c_popup* )> build );
		c_checkbox& attach_binds( );
		bool hosts( c_base_element* element ) const override;
		void play_intro( ) override;
		void close_overlay( ) override;
		void draw( ) override;
		void input( ) override;

		std::shared_ptr<c_popup>& popup( ) { return m_popup; }
		const std::shared_ptr<c_popup>& popup( ) const { return m_popup; }
		std::shared_ptr<c_bind_popup>& binds( ) { return m_binds; }
		bool* value_ptr( ) const { return m_value; }
		float& anim( ) { return m_anim; }

	private:
		bool* m_value { nullptr };
		float m_anim { 0.f };
		std::shared_ptr<c_popup> m_popup {};
		std::shared_ptr<c_bind_popup> m_binds {};
	};
}
