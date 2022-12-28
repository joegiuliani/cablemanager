#include <iostream>
#include <vector>
#include <glm/common.hpp>
#include <glm/vec2.hpp>
#include <glm/vec4.hpp>
#include "draw.h"
#include <glfw/glfw3.h>
#include "qgl.h"
#include <stack>

glm::vec2 quadratic_bezier(float t, const glm::vec2& a, const glm::vec2& b, const glm::vec2& c)
{
	float t1 = 1 - t;
	return t1 * (t1 * a + t * b) + t * (t1 * b + t * c);
}

glm::vec3 hsv(float h, float s, float v)
{
	float chroma = v * s;

	//              = hue / 360d in [0,1] space
	float hue_prime = h * 6;
	float x = chroma * (1.0f - abs(fmodf(hue_prime, 2.0f) - 1.0f));
	glm::vec3 ret;
	if (hue_prime < 3.0f)
	{
		if (hue_prime < 2.0f)
		{
			if (hue_prime < 1.0f)
				ret = glm::vec3(chroma, x, 0.0f);
			else
				ret = glm::vec3(x, chroma, 0.0f);
		}
		else
			ret = glm::vec3(0.0f, chroma, x);
	}
	else
	{
		if (hue_prime < 5.0f)
		{
			if (hue_prime < 4.0f)
				ret = glm::vec3(0.0f, x, chroma);
			else
				ret = glm::vec3(x, 0.0f, chroma);
		}

		else
			ret = glm::vec3(chroma, 0.0f, x);
	}

	return ret + (v - chroma);
}

typedef std::reference_wrapper<qgl::Shape> Pane;
typedef std::reference_wrapper<qgl::TextBox> Label;

class Port
{
public:
	std::string name;
	unsigned long int id;
	Port* connection = nullptr;

	Port()
	{
		id = borrow_id();
		pane().fill.top = pane().fill.bottom = qgl::color(1);
		pane().outline.top = pane().outline.bottom = qgl::color(0, 0, 0, 1);
		pane().outline_thickness = 1.5;
		pane().corner_radius = 4;
		pane().set_size(qgl::vec(8));
	}

	Port(const Port& p)
	{
		name = p.name;
		id = borrow_id();
		connection = p.connection;
		m_pane = p.m_pane;
	}

	~Port()
	{
		sever_connection();
		yield_id(id);
	}

	void sever_connection()
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

	void connection_severed()
	{
		connection = nullptr;
	}

	qgl::Shape& pane()
	{
		return m_pane.get();
	}

protected:

	// Port ID <0> is reserved.
	static unsigned long int id_index;
	static std::stack<unsigned long int> unused_ids;
	
	static unsigned long int borrow_id()
	{
		if (Port::unused_ids.empty())
		{
			return id_index++;
		}

		else
		{
			auto ret = Port::unused_ids.top();
			Port::unused_ids.pop();
			return ret;
		}
	}

	static void yield_id(unsigned long int id)
	{
		Port::unused_ids.push(id);
	}

private:
	Pane m_pane = qgl::head_element.add_child<qgl::Shape>();
};

unsigned long int Port::id_index = 1;
std::stack<unsigned long int> Port::unused_ids = std::stack<unsigned long int>();

class Node
{
public:
	std::list<Port> inputs;
	std::list<Port> outputs;
	std::list<Port> uputs;

	Node()
	{
		pane().fill.top = qgl::color(0.3, 0.09, .15, 1);
		pane().fill.bottom = pane().fill.top * 0.8f;
		pane().set_size(qgl::vec(75, 75));
		pane().options[qgl::Element::WORLD] = true;
		pane().outline_thickness = 1.5;
		pane().outline.top = qgl::color(1);
		pane().corner_radius = 4;

		label().set_text_scale(18);
		label().set_size(qgl::vec(100, 100));
		label().fill.top = label().fill.bottom = qgl::color(1);
		label().set_pos(qgl::vec(10));
		label().options[qgl::Element::WORLD] = true;

		pane().options[qgl::Element::MOUSE_LISTENER] = true;
	}
	
	qgl::Shape& pane()
	{
		return m_pane.get();
	}

	qgl::TextBox& label()
	{
		return m_label.get();
	}

private:
	Pane m_pane = qgl::head_element.add_child<qgl::Shape>();
	Label m_label = m_pane.get().add_child<qgl::TextBox>();
};

// issue with the memento class is that we have to keep track of all actions, not just per node
// So like if i hit ctrl z in a scene, i may undo one node but not another... its action specific not node specific.
// So really the scene needs a memento for all nodes

class Scene
{
public:
	std::list<Node> nodes;
	std::string name = "Untitled";

	const std::string EXTENSION = ".cms";

	typedef std::string token;
	const token NODE = "node";
	const token NODE_SIZE = "size";
	const token NODE_POS = "position";
	const token NODE_NAME = "name";


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

		return remove_white_space(sum.substr(1, sum.find_last_of(close_delim)-1));
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
	
	size_t find_at_scope(const std::string& str, char c, int offset = 0)
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

	/*
		Expects a string in the form of "3.932,390,0.11". No white space, no unnecessary commas. Only digits, dots, and commas.
	*/
	void read_float_array(const std::string& block, float* arr, int array_length)
	{
		int cursor = 0;
		int array_index = 0;
		size_t comma_pos = block.find(',');

		while (comma_pos != std::string::npos && array_index < array_length && cursor < block.length())
		{
			arr[array_index] = std::stof(block.substr(cursor, comma_pos - cursor));
			cursor = comma_pos + 1;
			comma_pos = block.find(',', cursor);
			array_index++;
		}

		if (array_index < array_length)
		{
			arr[array_index] = std::stof(block.substr(cursor));
		}

		if (array_index < array_length) std::cout << array_index + 1 << " out of " << array_length << " float found.\n";
	} 	

#define if_key_matches(value) if (key.compare(value) == 0)
#define else_if_key_matches(value) else if_key_matches(value)

	void parse_node_variable(const std::string& line, Node& node)
	{

		std::string key = read_block(line, '[', ']');
		if_key_matches(NODE_NAME)
		{
			std::string value_block = read_block(line, '{', '}');
			node.label().set_text(value_block);
		}
		else_if_key_matches(NODE_SIZE)
		{
			float dimensions[2];
			std::string value_block = read_block(line, '{', '}');
			read_float_array(value_block, dimensions, 2);
			node.pane().set_size(qgl::vec(dimensions[0], dimensions[1]));
		}
		else_if_key_matches(NODE_POS)
		{
			float dimensions[2];
			std::string value_block = read_block(line, '{', '}');
			read_float_array(value_block, dimensions, 2);
			node.pane().set_pos(qgl::vec(dimensions[0], dimensions[1]));
		}	
	}

#undef if_key_matches
#undef else_if_key_matches

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

int main()
{
    qgl::init();

	// We may need to abstract further to include lines and text

	// We can store a constant amount of vertices for the curve buffer. 
	// Then we can specify for each draw how many of those vertices were actually going to use


	//Node node("Hello");

	glm::vec2 in(20, 20);
	glm::vec2 magnet(200, 400);
	glm::vec2 out(400, 200);

	float k = std::min(glm::distance(in, magnet), distance(magnet, out)) / 2.71f;

	auto clamp_pos = [&](const glm::vec2& a, const glm::vec2& b, float c)
	{
		return a + (b - a) * c / (std::max(c, glm::distance(a, b)));
	};

	glm::vec2 pre_magnet = clamp_pos(magnet, in, k);
	glm::vec2 post_magnet = clamp_pos(magnet, out, k);

	Scene s;
	s.load("first_scene.cms");

		/*qgl::Curve& curve = qgl::new_Element<qgl::Curve>();
	curve.fill.top = glm::vec4(1);
	curve.fill.bottom = curve.fill.top;

	float resolution = 32;
	for (float k = 0; k <= resolution; k++)
	{
		curve.points.push_back(quadratic_bezier(k / resolution, in, magnet, out));
	}*/

	//draw_bezier(pre_magnet, magnet, post_magnet);
	//draw::draw_curve({ in, pre_magnet });
	//draw::draw_curve({ post_magnet, out });

	/*qgl::TextBox& text_box = qgl::new_Element<qgl::TextBox>();
	text_box.fill.top = glm::vec4(1, 0, 1, 1);
	text_box.fill.bottom = text_box.fill.top;
	text_box.m_pos = glm::vec2(30, 30);
	text_box.set_text("whats up");
	text_box.set_size(glm::vec2(40,40));
	text_box.set_text_scale(24);

	auto drag_element = [&](qgl::Element* element_ptr)
	{
		qgl::follow_mouse(element_ptr, [&]() {return draw::is_mouse_released(); });
	};*/


	// I'd rather have something like elem.on_click(lamda)
    while (qgl::is_running())
    {
        qgl::on_frame();
    }

	s.save_as("first_scene");

    qgl::terminate();
}