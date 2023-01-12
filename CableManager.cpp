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
#include "mouse_action.h"
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

glm::vec3 gamma(glm::vec3 color, float g)
{
	return glm::pow(color, glm::vec3(1.0 / g));
}

Scene& active_scene()
{
	return *Scene::active_scene_ptr;
}

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

Scene s;
std::vector<Button> node_popup;
/*
void popup_move_callback()
{
	for (Button& b : node_popup)
	{
		if (inside_shape(qgl::Mouse::pos, &b.pane))
		{
			b.highlight();
		}
	}
}

void popup_end()
{
	// Remove move callback
	// Add node press callback
	// Close popup
}

void popup_press_callback()   
{
	bool inside_a_button = false;
	for (Button& b : node_popup)
	{
		if (inside_shape(qgl::Mouse::pos, &b.pane))
		{
			inside_a_button = true; // change this to a fixed boundary once the class is created
			b.on_click();
			
			popup_end();

			return;
		}
	}

	if (!inside_a_button)
	{
		popup_end();
	}
}

// Given the context we will push and pull this from the callback list
// For instance, there is no popup, at least one node selected, etc
void node_delete_callback()
{
	if (qgl::Keyboard::matches(std::set<int> {draw::KEY_DELETE}))
	{
		s.comman.add_command(DeleteNode(s.active_node()));
	}
}*/

int main()
{
    qgl::init();

	s.load("first_scene.cms");
	Scene::active_scene_ptr = &s;

	qgl::Keyboard::add_callback(CommandManager::process_key_events);
	//qgl::Keyboard::add_callback(node_delete_callback);

	MouseAction::PressNode::init();

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

	// We can store a constant amount of vertices for the curve buffer. 
	// Then we can specify for each draw how many of those vertices were actually going to use

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


	glm::vec2 v1 = qgl::screen_to_world_projection(qgl::world_to_screen_projection(glm::vec2(0)));
    while (qgl::is_running())
    {
        qgl::on_frame();
    }

	s.save_as("first_scene");

    qgl::terminate();
}