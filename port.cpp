#include "port.h"
#include <stack>

// Port ID <0> is reserved.
unsigned long int id_index;
std::stack<unsigned long int> unused_ids;

unsigned long int borrow_id()
{
	if (unused_ids.empty())
	{
		return id_index++;
	}

	else
	{
		auto ret = unused_ids.top();
		unused_ids.pop();
		return ret;
	}
}

void yield_id(unsigned long int id)
{
	unused_ids.push(id);
}

namespace cm
{
	Port::Port()
	{
		id = borrow_id();
		pane.fill.top = pane.fill.bottom = cm::color(1);
		pane.outline.top = pane.outline.bottom = cm::color(0, 0, 0, 1);
		pane.outline_thickness = 1.5;
		pane.corner_radius = 4;
		pane.set_size(cm::vec(8));
	}

	Port::Port(const Port& p)
	{
		name = p.name;
		id = borrow_id();
		connection = p.connection;
	}

	Port::~Port()
	{
		sever_connection();
		yield_id(id);
	}

	void Port::sever_connection()
	{
		if (connection != nullptr)
		{
			if (connection->connection != this)
			{
				std::cout << "Invalid connection sever";
			}

			connection->connection_severed();
		}
	}

	void Port::connection_severed()
	{
		connection = nullptr;
	}
}

