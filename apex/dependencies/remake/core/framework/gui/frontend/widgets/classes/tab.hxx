#pragma once

#include <string>
#include <vector>

#include <core/framework/gui/frontend/widgets/classes/base_element.hxx>

namespace core::gui
{
	class c_tab : public c_base_element
	{
	public:
		c_tab( )
		{
			m_type = element_type::tab;
			m_visible = true;
		}

		void draw( ) override { }
		void input( ) override { }

		void add_tab( const std::string& name )
		{
			m_names.push_back( name );
		}

		std::vector<std::string> m_names {};
	};
}
