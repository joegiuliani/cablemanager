#pragma once
#include "qgl.h"
#include "port.h"

class Node
{
public:
	std::list<Port> inputs;
	std::list<Port> outputs;
	std::list<Port> uputs;

	Node();
	Node(const Node& node);
	Node& operator=(const Node& node);

	Port& add_input();

	qgl::Shape pane;
	qgl::TextBox label;
};