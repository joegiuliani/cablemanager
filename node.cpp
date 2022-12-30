#include "node.h"
#include "qgl.h"

Node::Node()
{
	pane().fill.top = qgl::color(0.3, 0.09, .15, 1);
	pane().fill.bottom = pane().fill.top * 0.8f;
	pane().set_size(qgl::vec(75, 75));
	pane().options[qgl::Element::WORLD] = true;
	pane().outline_thickness = 1.5;
	pane().outline.top = qgl::color(1);
	pane().corner_radius = 4;

	label().set_text_scale(18);
	label().set_size(qgl::vec(100, 100));
	label().fill.top = label().fill.bottom = qgl::color(1);
	label().set_pos(qgl::vec(10));
	label().options[qgl::Element::WORLD] = true;

	pane().options[qgl::Element::MOUSE_LISTENER] = true;
}

qgl::Shape& Node::pane()
{
	return m_pane.get();
}

qgl::TextBox& Node::label()
{
	return m_label.get();
}


