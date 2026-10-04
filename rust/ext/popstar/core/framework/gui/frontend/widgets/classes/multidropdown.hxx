#pragma once

#include <string>

#include <core/framework/gui/frontend/widgets/classes/base_element.hxx>

namespace core::gui
{
	class c_multidropdown : public c_base_element
	{
	public:
		c_multidropdown( std::string label )
		{
			m_label = std::move( label );
			m_type = element_type::dropdown;
			m_visible = true;
		}

		void draw( ) override { }
		void input( ) override { }
	};
}
