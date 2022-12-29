#include <iostream>
#include <vector>
#include <glm/common.hpp>
#include <glm/vec2.hpp>
#include <glm/vec4.hpp>
#include "draw.h"
#include <glfw/glfw3.h>
#include "qgl.h"
#include <stack>
#include "scene.h"

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

class Scene;
class Node;
Scene* active_scene_ptr = nullptr;
Scene& active_scene()
{
	return *active_scene_ptr;
}

// issue with the memento class is that we have to keep track of all actions, not just per node
// So like if i hit ctrl z in a scene, i may undo one node but not another... its action specific not node specific.
// So really the scene needs a memento for all nodes



class Command
{
public:
	virtual void execute() = 0;
	virtual void reverse() = 0;
};

class MoveNode : public Command
{
	std::reference_wrapper<Node> node;
	qgl::vec last_pos_world; // We store the previous position in world since the camera center may have changed before undoing.
	qgl::vec new_pos;
public:
	MoveNode(Node& n, qgl::vec pos) :node(n)
	{
		last_pos_world = qgl::screen_to_world_projection(n.pane().pos());
		new_pos = pos;
	}

	virtual void execute()
	{
		node.get().pane().set_pos(new_pos);
	}

	virtual void reverse()
	{
		node.get().pane().set_pos(qgl::world_to_screen_projection(last_pos_world));
	}
};

class DeleteNode : public Command
{
	Node node_state;
	Node* node_ptr;

public:
	DeleteNode(Node& n)
	{
		node_state = n;
		node_ptr = &n;
	}

	virtual void execute()
	{
		auto pred = [&](const Node& n) -> bool
		{
			return &n == node_ptr;
		};
		active_scene().nodes.remove_if(pred);
		node_ptr = nullptr;
	}

	virtual void reverse()
	{
		active_scene().nodes.push_back(node_state);
		node_ptr = &active_scene().nodes.back();
	}
};

class CommandManager
{
public:
	typedef std::stack<std::shared_ptr<Command>> command_stack;
	command_stack available_undos;
	command_stack available_redos;

	template<typename T>
	void add_command(T command)
	{
		clear_stack(available_redos);

		available_undos.push(std::make_shared<T>(command));
		available_undos.top()->execute();
	}

	void undo()
	{
		if (available_undos.size())
		{
			available_undos.top()->reverse();
			available_redos.push(available_undos.top());
			available_undos.pop();
		}
	}

	void redo()
	{
		if (available_redos.size())
		{
			available_redos.top()->execute();
			available_undos.push(available_redos.top());
			available_redos.pop();
		}
	}

	void clear_stack(command_stack& s)
	{
		while (!s.empty())
			s.pop();
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
	active_scene_ptr = &s;
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

	CommandManager cm;
	cm.add_command(DeleteNode(s.nodes.back()));
	cm.undo();
	cm.redo();

	s.save_as("first_scene");

    qgl::terminate();
}