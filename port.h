#pragma once

#include "qgl.h"
#include <string>
#include <iostream>

class Port
{
public:
	std::string name;
	unsigned long int id;
	Port* connection = nullptr;

	Port();
	Port(const Port& p);
	~Port();

	void sever_connection();
	void connection_severed();
	qgl::Shape pane;
};