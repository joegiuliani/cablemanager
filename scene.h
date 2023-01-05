#pragma once

#include <string>
#include <vector>
#include <list>

class Node;

class Scene
{
public:

	const std::string EXTENSION = ".cms";

	static inline const std::string
		NODE = "node",
		NODE_SIZE = "size",
		NODE_POS = "position",
		NODE_NAME = "name";

	std::list<Node> nodes;

	void load(std::string file_path);

	std::string get_file_name();

	void save_as(std::string new_name);
};
