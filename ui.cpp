#include "ui.h"

namespace cm
{
	Button::Button(const std::string& name, cm::CallbackPtr action)
	{
		label.set_text(name);
		on_click = action;
	}

	void Button::highlight()
	{

	}
}
