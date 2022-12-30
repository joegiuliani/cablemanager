#pragma once
#include "qgl.h"
#include "port.h"

class Node
{
public:
	std::list<Port> inputs;
	std::list<Port> outputs;
	std::list<Port> uputs;

	typedef std::reference_wrapper<qgl::Shape> Pane;
	typedef std::reference_wrapper<qgl::TextBox> Label;

	Node();

	qgl::Shape& pane();
	qgl::TextBox& label();

private:
	Pane m_pane = qgl::head_element.add_child<qgl::Shape>();
	Label m_label = m_pane.get().add_child<qgl::TextBox>();
};
