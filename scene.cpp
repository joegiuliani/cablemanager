#include "scene.h"
#include "node.h"
#include <fstream>
#include <iostream>
#include "cmsutil.h"

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
		nodes.push_back(Node());
		Node& node = nodes.back();
		node.pane().options[qgl::Element::WORLD] = true;

		reader.open_key();

			reader.open_key();
				node.label().set_text(reader.next_value());
			reader.close_key();

			reader.open_key();
				node.pane().set_pos(qgl::world_to_screen_projection(glm::vec2(std::stof(reader.next_value()), std::stof(reader.next_value()))));
			reader.close_key();

			reader.open_key();
				node.pane().set_size(qgl::world_to_screen_scale(glm::vec2(std::stof(reader.next_value()), std::stof(reader.next_value()))));
			reader.close_key();

		reader.close_key();
	}
}

std::string Scene::get_file_name()
{
	return name + EXTENSION;
}
void Scene::save_as(std::string new_name)
{
	name = new_name;

	CMSWriter writer(new_name);

	for (auto& node : nodes)
	{
		writer.open_key(NODE);
		{
			writer.open_key(NODE_NAME);
			{
				writer.add_value(node.label().get_text());
			} writer.close_key();

			writer.open_key(NODE_POS);
			{
				writer.add_vec(qgl::screen_to_world_projection(node.pane().pos()));
			} writer.close_key();

			writer.open_key(NODE_SIZE);
			{
				writer.add_vec(qgl::screen_to_world_scale(node.pane().size()));
			} writer.close_key();

		} writer.close_key();
	}
}

