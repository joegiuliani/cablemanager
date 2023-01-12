#pragma once
#include <string>
#include <vector>
#include <list>
#include <memory>
#include <functional>
#include "comman.h"
#include <set>

namespace cm
{
	class Node;

	class Scene
	{
	public:
		const std::string EXTENSION = ".cms";

		static inline Scene* active_scene_ptr = nullptr;
		static inline const std::string
			NODE = "node",
			NODE_SIZE = "size",
			NODE_POS = "position",
			NODE_NAME = "name";

		CommandManager comman;

		typedef std::set<Node*, std::less<Node*>> NodePtrSet;
		NodePtrSet selected_nodes;

		void load(std::string file_path);
		Node* active_node_ptr = nullptr;
		void remove_node(Node& node_to_remove);

		Node& add_node(const Node& n);

		std::string get_file_name();

		void save_as(std::string new_name);
		std::vector<std::shared_ptr<Node>> nodes;
	private:

	};

}


