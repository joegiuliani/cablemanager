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

Port::Port()
{
	id = borrow_id();
	pane().fill.top = pane().fill.bottom = qgl::color(1);
	pane().outline.top = pane().outline.bottom = qgl::color(0, 0, 0, 1);
	pane().outline_thickness = 1.5;
	pane().corner_radius = 4;
	pane().set_size(qgl::vec(8));
}

Port::Port(const Port& p)
{
	name = p.name;
	id = borrow_id();
	connection = p.connection;
	m_pane = p.m_pane;
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

qgl::Shape& Port::pane()
{
	return m_pane.get();
}