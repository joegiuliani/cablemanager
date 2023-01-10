#include "node.h"

Node::Node()
{
	pane.fill.top = cm::color(0.3, 0.09, .15, 1);
	pane.fill.bottom = pane.fill.top * 0.8f;
	pane.set_size(cm::vec(75, 75));
	pane.options[qgl::Element::WORLD] = true;
	pane.outline_thickness = 1.5;
	pane.outline.top = cm::color(1);
	pane.corner_radius = 4;

	label.set_text_scale(18);
	label.set_size(cm::vec(100, 100));
	label.fill.top = label.fill.bottom = cm::color(1);
	label.set_pos(cm::vec(10));
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

Port& Node::add_input()
{
	inputs.push_back(Port());
	Port& new_port = inputs.back();

	new_port.pane.move_to_parent(&pane);
	new_port.pane.set_size(cm::vec(50, 50));

	return new_port;
}