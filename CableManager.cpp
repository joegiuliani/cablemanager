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

class Port
{
public:
	std::string name;
	unsigned long int id;
	Port* connection = nullptr;

	Port()
	{
		id = borrow_id();
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

private:

	// Port ID <0> is reserved.
	unsigned long int id_index = 1;
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
};

class Node
{
public:
	qgl::Shape& pane = qgl::new_Element<qgl::Shape>();
	qgl::TextBox& label = qgl::new_Element<qgl::TextBox>(&pane);
	std::list<Port> inputs;
	std::list<Port> outputs;
	std::list<Port> uputs;

	void on_drag(qgl::Element)
	{

	}

	Node()
	{
		init();
	}
	Node(std::string name) :Node()
	{
		init();

		label.set_text(name);
		label.set_text_scale(18);
		label.set_size(qgl::vec(65, 50));
		label.fill.top = label.fill.bottom = qgl::color(1);
		label.pos = qgl::vec(5, 5);
	}

	private:
		void init()
		{
			pane.fill.top = qgl::color(0.3, 0.09, .15, 1);
			pane.fill.bottom = pane.fill.top * 0.8f;
			pane.set_size(qgl::vec(75, 75));

			pane.options[qgl::Element::MOUSE_LISTENER] = true;
			auto drag_element = [&](qgl::Element* element_ptr)
			{
				qgl::follow_mouse
				(
					element_ptr, // this
					[&]() {return draw::is_mouse_released();} // condition to stop following the mouse
				);
			};

			// Label will follow since its a child
			pane.on_press(drag_element);
		}


};

class Scene
{
public:
	std::list<Node> nodes;

	int count(const std::string& str, char c)
	{
		int ct = 0;
		for (auto& str_char : str)
		{
			if (c == str_char)
			{
				ct++;
			}
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

		while (num_open > num_close)
		{
			char c = file.get();
			sum += c;
			// TODO check if line contains '\n'

			if (c == open_delim) num_open++;
			else if (c == close_delim) num_close++;
		}

		return remove_white_space(sum.substr(1, sum.find_last_of(close_delim)-1));
	}
	
	std::string read_block(const std::string& str, char open_delim, char close_delim)
	{
		std::string sum = "";
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
		sum += open_delim;
		cursor++;

		int num_close = 0;
		while (cursor < str.length())
		{
			char c = str[cursor];
			sum += c;

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

		return remove_white_space(sum.substr(1, cursor-1));
	}

	void load(std::string file_path)
	{
		// validate extension
		std::string file_extension = file_path.substr(file_path.length() - 4);
		if (file_extension.compare(".cms") != 0)
		{
			std::cout << "Invalid file extension for \"" + file_path + "\"\n";
			return;
		}

		std::ifstream file;
		file.open(file_path);

		std::string header = read_block(file, '[', ']');
		if (header.compare("Node") == 0)
		{
			nodes.push_back(Node());
			
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

		parse_node_variable(block.substr(line_start), node);

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

	void parse_node_variable(const std::string& line, Node& node)
	{
		const std::string NAME = "name";
		const std::string SIZE = "size";
		
		std::string key = read_block(line, '[', ']');
		if (key.compare(NAME) == 0)
		{
			node.label.set_text(read_block(line, '{', '}'));
		}
		else if (key.compare(SIZE) == 0)
		{
			float dimensions[2];
			std::string value_block = read_block(line, '{', '}');
			read_float_array(value_block, dimensions, 2);
			node.pane.set_size(qgl::vec(dimensions[0], dimensions[1]));
		}
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
	text_box.pos = glm::vec2(30, 30);
	text_box.set_text("whats up");
	text_box.set_size(glm::vec2(40,40));
	text_box.set_text_scale(24);

	auto drag_element = [&](qgl::Element* element_ptr)
	{
		qgl::follow_mouse(element_ptr, [&]() {return draw::is_mouse_released(); });
	};*/

    qgl::set_corner_size(5);

	// I'd rather have something like elem.on_click(lamda)
	std::cout << sizeof(qgl::Element);
    while (qgl::is_running())
    {
        qgl::on_frame();
    }
    qgl::terminate();
}