#pragma once

#include <string>
#include <vector>

namespace CMSUtil
{
	int count(const std::string& str, char c)
	{
		int ct = 0;
		size_t index = str.find(c, 0);
		while (index != std::string::npos)
		{
			ct++;
			index = str.find(c, index);
		}
		return ct;
	}

	std::string remove_white_space(const std::string& str)
	{
		std::string ret = "";

		bool in_quotes = false;
		for (const char& c : str)
		{
			if (c == '\"')
			{
				in_quotes = !in_quotes;
				continue;
			}

			if (in_quotes) ret += c;

			else if (c == '\n' || c == ' ' || c == '\t')
			{
				continue;
			}

			else ret += c;
		}

		return ret;
	}

	std::string read_block(std::ifstream& file, char open_delim, char close_delim)
	{
		std::string sum = "";

		int num_open = 0;

		char c = file.get();
		while (!file.eof() && c != open_delim)
		{
			if (c == close_delim)
			{
				std::cout << "Invalid arguments. Found first close delim before first open delim\n";
				return "";
			}

			c = file.get();
		}

		if (file.eof())
		{
			std::cout << "No open delim found";
			return "";
		}

		num_open = 1;
		sum += open_delim;
		int num_close = 0;

		while (!file.eof() && num_open > num_close)
		{
			char c = file.get();
			sum += c;
			// TODO check if line contains '\n'

			if (c == open_delim) num_open++;
			else if (c == close_delim) num_close++;
		}

		if (num_open > num_close)
		{
			std::cout << "NOt enough close delims found in file\n";
			return "";
		}

		return remove_white_space(sum.substr(1, sum.find_last_of(close_delim) - 1));
	}

	std::string read_block(const std::string& str, char open_delim, char close_delim)
	{
		int cursor = 0;
		int num_open = 0;

		while (cursor < str.length() && str[cursor] != open_delim)
		{
			if (str[cursor] == close_delim)
			{
				std::cout << "Invalid arguments. Found first close delim before first open delim\n";
				return "";
			}
			cursor++;
		}

		if (cursor == str.length())
		{
			std::cout << "No open delim found";
			return "";
		}

		num_open = 1;
		size_t first_delim_index = cursor;
		cursor++;

		int num_close = 0;
		while (cursor < str.length())
		{
			char c = str[cursor];

			if (c == open_delim) num_open++;
			else if (c == close_delim)
			{
				num_close++;
				if (num_close == num_open)
				{
					break;
				}
			}

			cursor++;
		}

		if (cursor == str.length())
		{
			std::cout << "Invalid arguments. Not enough close delims found\n";
			return "";
		}

		std::string contents = str.substr(first_delim_index + 1, cursor - first_delim_index - 1);
		std::string contents_no_white_space = remove_white_space(contents);

		return contents_no_white_space;
	}

	size_t find_at_scope(const std::string& str, char c, size_t offset = 0)
	{
		int num_open = 0;
		int num_close = 0;
		for (size_t k = offset; k < str.length(); k++)
		{
			if (str[k] == '{') num_open++;
			if (str[k] == '}') num_close++;

			if (num_open == num_close)
			{
				if (str[k] == c)
					return k;
			}
		}

		return std::string::npos;
	}

	/*
		reads a list of comma separated floats
		block may only contain white space, digits, dots, and commas
		invalid input: "1,a", "3 4", "3. 4", "3,"
	*/
	std::vector<float> read_floats(const std::string& block)
	{
		int cursor = 0;
		size_t comma_pos = block.find(',');
		std::vector<float> floats_read;

		while (comma_pos != std::string::npos && cursor < block.length())
		{
			floats_read.push_back(std::stof(block.substr(cursor, comma_pos - cursor)));
			cursor = comma_pos + 1;
			comma_pos = block.find(',', cursor);
		}
		
		floats_read.push_back(std::stof(block.substr(cursor)));
	}
}

class Scene
{
public:
	std::list<Node> nodes;
	std::string name = "Untitled";

	const std::string EXTENSION = ".cms";

	static inline const std::string
		NODE = "node",
		NODE_SIZE = "size",
		NODE_POS = "position",
		NODE_NAME = "name";


	void load(std::string file_path)
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


	void parse_node(const std::string& block, Node& node)
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



#define key_matches(value) key.compare(value) == 0

	void parse_node_variable(const std::string& line, Node& node)
	{

		std::string key = read_block(line, '[', ']');
		if (key_matches(NODE_NAME))
		{
			std::string value_block = read_block(line, '{', '}');
			node.label().set_text(value_block);
		}
		else if (key_matches(NODE_SIZE))
		{
			float dimensions[2];
			std::string value_block = read_block(line, '{', '}');
			read_float_array(value_block, dimensions, 2);
			node.pane().set_size(qgl::vec(dimensions[0], dimensions[1]));
		}
		else if (key_matches(NODE_POS))
		{
			float dimensions[2];
			std::string value_block = read_block(line, '{', '}');
			read_float_array(value_block, dimensions, 2);
			node.pane().set_pos(qgl::vec(dimensions[0], dimensions[1]));
		}
	}

#undef key_matches

	std::string get_file_name()
	{
		return name + EXTENSION;
	}


	void save_as(std::string new_name)
	{
		name = new_name;

		std::ofstream file;
		file.open(get_file_name(), std::ios::trunc);

		std::stack<int> depth_tracker;
		depth_tracker.push(0);

		auto add_comma_if_needed = [&]()
		{
			if (depth_tracker.size() && depth_tracker.top() > 0)
				file << ',';
		};

		auto open_key = [&](std::string name)
		{
			add_comma_if_needed();

			name = '[' + name + "] {";
			file << name;

			depth_tracker.top()++;

			depth_tracker.push(0);
		};

		auto close_key = [&]()
		{
			file << "}";
			depth_tracker.pop();
		};

		auto add_value = [&](std::string val)
		{
			add_comma_if_needed();

			file << val;
			depth_tracker.top()++;
		};

		auto add_vec = [&](const glm::vec2& vec)
		{
			add_value(std::to_string(vec.x));
			add_value(std::to_string(vec.y));
		};

		for (auto& node : nodes)
		{
			open_key(NODE);
			{
				open_key(NODE_NAME);
				{
					add_value(node.label().get_text());
				} close_key();

				open_key(NODE_SIZE);
				{
					add_vec(node.pane().size());
				} close_key();

				open_key(NODE_POS);
				{
					add_vec(node.pane().pos());
				} close_key();

			} close_key();
		}

		file.close();
	}
};
