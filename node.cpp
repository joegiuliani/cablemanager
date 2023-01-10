#include "node.h"
#include "qgl.h"

Node::Node()
{
	pane.fill.top = qgl::color(0.3, 0.09, .15, 1);
	pane.fill.bottom = pane.fill.top * 0.8f;
	pane.set_size(qgl::vec(75, 75));
	pane.options[qgl::Element::WORLD] = true;
	pane.outline_thickness = 1.5;
	pane.outline.top = qgl::color(1);
	pane.corner_radius = 4;

	label.set_text_scale(18);
	label.set_size(qgl::vec(100, 100));
	label.fill.top = label.fill.bottom = qgl::color(1);
	label.set_pos(qgl::vec(10));
	label.options[qgl::Element::WORLD] = true;
}

Node::Node(const Node& node)
{
	operator=(node);
}

Node& Node::operator=(const Node& node)
{
	inputs = node.inputs;
	outputs = node.outputs;
	uputs = node.uputs;

	pane = node.pane;
	label = node.label;
	label.move_to_parent(&pane);

	return *this;
}
