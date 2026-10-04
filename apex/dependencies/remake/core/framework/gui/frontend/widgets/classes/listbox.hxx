#pragma once

#include <string>
#include <vector>

#include <core/framework/gui/frontend/widgets/classes/base_element.hxx>

namespace core::gui
{
	class c_listbox : public c_base_element
	{
	public:
		c_listbox( std::string label, int* value, std::vector<std::string> items, float height, bool hide_label = false )
			: m_value( value ), m_items( std::move( items ) ), m_height( height )
		{
			m_label = std::move( label );
			m_type = element_type::listbox;
			m_visible = true;
			m_size.y = height;
			if ( hide_label )
				this->hide_label( );
		}

		void draw( ) override { }
		void input( ) override { }

	private:
		int* m_value { nullptr };
		std::vector<std::string> m_items {};
		float m_height { 100.f };
	};
}
