#include "scene.h"
#include "node.h"
#include <fstream>
#include <iostream>
#include "cmsutil.h"

using namespace CMSUtil;
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

	std::ifstream file;
	file.open(file_path);

	std::string header = read_block(file, '[', ']');
	if (header.compare(NODE) == 0)
	{
		nodes.push_back(Node());
		nodes.back().pane().options[qgl::Element::WORLD] = true;

		// Get everything contained by the Node block
		std::string blck = read_block(file, '{', '}');
		parse_node(blck, nodes.back());
	}
}
void Scene::parse_node(const std::string& block, Node& node)
{
	size_t line_start = 0;
	size_t next_comma = find_at_scope(block, ',', line_start);

	while (next_comma != std::string::npos)
	{
		std::string var_block = block.substr(line_start, next_comma - line_start);
		parse_node_variable(var_block, node);

		line_start = next_comma + 1;
		next_comma = find_at_scope(block, ',', line_start);
	}

	parse_node_variable(block.substr(line_start, next_comma), node);

	// add code to get last node variable.
}
void Scene::parse_node_variable(const std::string& line, Node& node)
{
#define key_matches(value) key.compare(value) == 0

	std::string key = read_block(line, '[', ']');
	if (key_matches(NODE_NAME))
	{
		std::string value_block = read_block(line, '{', '}');
		node.label().set_text(value_block);
	}
	else if (key_matches(NODE_SIZE))
	{
		std::string value_block = read_block(line, '{', '}');
		std::vector<float> vals = read_floats(value_block);
		node.pane().set_size(qgl::world_to_screen_scale(qgl::vec(vals[0], vals[1])));
	}
	else if (key_matches(NODE_POS))
	{
		std::string value_block = read_block(line, '{', '}');
		std::vector<float> vals = read_floats(value_block);
		node.pane().set_pos(qgl::world_to_screen_projection(qgl::vec(vals[0], vals[1])));
	}
#undef key_matches
}
std::string Scene::get_file_name()
{
	return name + EXTENSION;
}
void Scene::save_as(std::string new_name)
{
	name = new_name;

	Writer writer(new_name);

	for (auto& node : nodes)
	{
		writer.open_key(NODE);
		{
			writer.open_key(NODE_NAME);
			{
				writer.add_value(node.label().get_text());
			} writer.close_key();

			writer.open_key(NODE_SIZE);
			{
				writer.add_vec(node.pane().size());
			} writer.close_key();

			writer.open_key(NODE_POS);
			{
				writer.add_vec(node.pane().pos());
			} writer.close_key();

		} writer.close_key();
	}
}

