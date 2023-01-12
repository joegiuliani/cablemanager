#include "scene.h"
#include <fstream>
#include <iostream>
#include "cmsutil.h"
#include "qgl.h"
#include "node.h"

namespace cm
{
	std::string name = "Untitled";
	void Scene::load(std::string file_path)
	{
		// validate extension
		size_t dot_index = file_path.length() - 4;
		std::string file_extension = file_path.substr(dot_index);
		if (file_extension.compare(EXTENSION) != 0)
		{
			std::cout << "Invalid file extension for \"" + file_path + "\"\n";
			return;
		}

		name = file_path.substr(0, dot_index);

		CMSReader reader(name);
		while (reader.has_next_key())
		{
			Node node;

			reader.open_key();

			reader.open_key();
			node.label.set_text(reader.next_value());
			reader.close_key();

			reader.open_key();
			node.pane.set_pos(qgl::world_to_screen_scale(glm::vec2(std::stof(reader.next_value()), std::stof(reader.next_value()))));
			reader.close_key();

			reader.open_key();
			node.pane.set_size(qgl::world_to_screen_scale(glm::vec2(std::stof(reader.next_value()), std::stof(reader.next_value()))));
			reader.close_key();

			reader.close_key();

			nodes.push_back(std::make_shared<Node>(node));
		}
	}

	std::string Scene::get_file_name()
	{
		return name + EXTENSION;
	}
	void Scene::foreach(std::function<void(Node& n)> fn)
	{
		for (auto& node : nodes)
		{
			fn(*node);
		}
	}
	void Scene::save_as(std::string new_name)
	{
		name = new_name;

		CMSWriter writer(new_name);

		for (auto& node_ptr : nodes)
		{
			Node& node = *node_ptr;
			writer.open_key(NODE);
			{
				writer.open_key(NODE_NAME);
				{
					writer.add_value(node.label.get_text());
				} writer.close_key();

				writer.open_key(NODE_POS);
				{
					writer.add_vec(qgl::screen_to_world_scale(node.pane.pos()));
				} writer.close_key();

				writer.open_key(NODE_SIZE);
				{
					writer.add_vec(qgl::screen_to_world_scale(node.pane.size()));
				} writer.close_key();

			} writer.close_key();
		}
	}

	Node& Scene::active_node()
	{
		return *active_node_ptr;
	}

	void Scene::set_active_node(Node& n)
	{
		active_node_ptr = &n;
	}

	void Scene::remove_node(Node& node_to_remove)
	{
		if (active_node_ptr == &node_to_remove) active_node_ptr = nullptr;

		auto pred = [&](const std::shared_ptr<Node>& node_ptr_to_check) -> bool
		{
			return node_ptr_to_check.get() == &node_to_remove;
		};
		nodes.erase(std::remove_if(nodes.begin(), nodes.end(), pred), nodes.end());
	}

	Node& Scene::add_node(const Node& node)
	{
		nodes.push_back(std::make_shared<Node>(Node(node)));
		return *nodes.back();
	}
}