#pragma once
#include "qgl.h"

namespace cm
{
	class Button
	{
	public:
		qgl::Shape pane;
		qgl::TextBox label = qgl::TextBox(&pane);
		cm::CallbackPtr on_click = nullptr;

		Button(const std::string& name, cm::CallbackPtr action);
		void highlight();
	};
}
