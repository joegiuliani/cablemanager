#pragma once
#include "qgl.h"
#include "port.h"

namespace cm
{
	class Node
	{
	public:

		static inline color OUTLINE = color(0.3, 0.3, 0.3,1);
		static inline color FILL = color(0.05, 0.05, 0.05, 1);

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
}
