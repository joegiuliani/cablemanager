#pragma once

#include <string>
#include <vector>
#include <list>
#include <memory>
#include <functional>

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

	void load(std::string file_path);
	Node& active_node();
	void set_active_node(Node& n);
	void remove_node(Node& node_to_remove);
	void foreach(std::function<void(Node& n)> fn);

	Node& add_node(const Node& n);

	std::string get_file_name();

	void save_as(std::string new_name);
private:
	std::vector<std::unique_ptr<Node>> nodes;
	Node* active_node_ptr = nullptr;

};
