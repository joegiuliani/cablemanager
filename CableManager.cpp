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
#include "node.h"
#include "comman.h"
#include "ui.h"
#include <cassert>

using namespace cm;

vec quadratic_bezier(float t, const vec& a, const vec& b, const vec& c)
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

Scene* active_scene_ptr = nullptr;
Scene& active_scene()
{
	return *active_scene_ptr;
}

class MoveNode : public Command
{
	Node* node_ptr = nullptr;
	vec last_pos_world;
	vec new_pos;
public:
	MoveNode(Node& n, vec pos)
	{
		last_pos_world = qgl::screen_to_world_projection(pos);
		new_pos = qgl::screen_to_world_projection(n.pane.pos());
		n.pane.set_pos(pos); // or whatever
		node_ptr = &n;
	}

	virtual void execute()
	{
		node_ptr->pane.set_pos(qgl::world_to_screen_projection(new_pos));
	}

	virtual void reverse()
	{
		node_ptr->pane.set_pos(qgl::world_to_screen_projection(last_pos_world));
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
		active_scene().remove_node(*node_ptr);
		node_ptr = nullptr;
	}

	virtual void reverse()
	{
		node_ptr = &active_scene().add_node(node_state);
	}
};

bool inside_rectangle(const glm::vec2& v, const glm::vec2& lower_bound, const glm::vec2& upper_bound) {
	return (v.x >= lower_bound.x && v.x <= upper_bound.x && v.y >= lower_bound.y && v.y <= upper_bound.y);
}

Scene s;
CommandManager comman;

vec node_move_sep;
vec node_move_start;

enum class Context
{
	Scene,
	Popup
};

Context curr_context = Context::Scene;

Node* node_under_mouse()
{
	if (curr_context == Context::Scene)
	{
		s.foreach([&](Node& n)
			{
				if (inside_rectangle(qgl::Mouse::pos, n.pane.pos(), n.pane.pos() + n.pane.size()))
				{
					return &n;
				}
			});
	}

	return nullptr;
}

void node_move_callback()
{
	s.active_node().pane.set_pos(qgl::Mouse::pos + node_move_sep);
}

void node_release_callback()
{
	comman.add_command(MoveNode(s.active_node(), node_move_start));

	qgl::Mouse::remove_callback(qgl::Mouse::release, node_release_callback);
	qgl::Mouse::remove_callback(qgl::Mouse::move, node_move_callback);
}

void popup_move_callback()
{

}

void node_press_callback()
{
	Node* node_ptr = node_under_mouse();
	if (node_ptr == nullptr) return;
	Node& node = *node_ptr;

	if (curr_context == Context::Scene && draw::is_mouse_down(draw::MOUSE_LEFT))
	{
		s.set_active_node(node); // change this when implementing selection
		node_move_start = node.pane.pos();
		node_move_sep = node.pane.pos() - qgl::Mouse::pos;
		qgl::Mouse::move.push_back(node_move_callback);
		qgl::Mouse::release.push_back(node_release_callback);
	}

	// If we have a callback in move, then we might be dragging a node around or a port connection.
	if (qgl::Mouse::move.size() == 0 && draw::is_mouse_down(draw::MOUSE_RIGHT))
	{
		curr_context = Context::Popup;
	}
}


// Given the context we will push and pull this from the callback list
// For instance, there is no popup, at least one node selected, etc
void node_delete_callback()
{
	if (qgl::Keyboard::matches(std::set<int> {draw::KEY_DELETE}))
	{
		comman.add_command(DeleteNode(s.active_node()));
	}
}

int main()
{
	active_scene_ptr = &s;
    qgl::init();

	// We can store a constant amount of vertices for the curve buffer. 
	// Then we can specify for each draw how many of those vertices were actually going to use

	//Node node("Hello");
	/*
	glm::vec2 in(20, 20);
	glm::vec2 magnet(200, 400);
	glm::vec2 out(400, 200);

	float k = std::min(glm::distance(in, magnet), distance(magnet, out)) / 2.71f;

	auto clamp_pos = [&](const glm::vec2& a, const glm::vec2& b, float c)
	{
		return a + (b - a) * c / (std::max(c, glm::distance(a, b)));
	};

	glm::vec2 pre_magnet = clamp_pos(magnet, in, k);
	glm::vec2 post_magnet = clamp_pos(magnet, out, k);*/


	s.load("first_scene.cms");


	qgl::Keyboard::add_callback(CommandManager::process_key_events);

	qgl::Mouse::press.push_back(
		node_press_callback
	);

	qgl::Keyboard::add_callback(node_delete_callback);

	// At some point i want to split classes like Node and Port into two separate structs - the gui and the actual data. 

	/*qgl::Curve& curve = qgl::head_element.add_child<qgl::Curve>();
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
	text_box.set_text_scale(24);*/

	glm::vec2 v1 = qgl::screen_to_world_projection(qgl::world_to_screen_projection(glm::vec2(0)));
    while (qgl::is_running())
    {
        qgl::on_frame();
    }

	s.save_as("first_scene");

    qgl::terminate();
}


// Two things might be happening

// 1. When copied the label doesnt properly take pane as its parent
// 2. pos() is broken.