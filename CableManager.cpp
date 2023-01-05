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
#include <cassert>

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

Scene* active_scene_ptr = nullptr;
Scene& active_scene()
{
	return *active_scene_ptr;
}

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


bool inside_rectangle(const glm::vec2& v, const glm::vec2& lower_bound, const glm::vec2& upper_bound) {
	return (v.x >= lower_bound.x && v.x <= upper_bound.x && v.y >= lower_bound.y && v.y <= upper_bound.y);
}

template <typename T>
std::function<bool(const std::function<T>&)> lambda_equal_pred(const std::function<T>& target)
{
	return [&](const std::function<T>& fn) -> bool
	{
		return target.target<T*>() == fn.target<T*>();
	};
}

Scene s;
CommandManager cm;



void node_move_callback()
{
	s.nodes.back().pane().set_pos(qgl::Mouse::pos);
}

void node_release_callback()
{
	cm.add_command(MoveNode(s.nodes.back(), qgl::Mouse::pos));

	qgl::Mouse::remove_this_callback();
	qgl::Mouse::remove_callback(qgl::Mouse::move, node_move_callback);		
	
	//qgl::Mouse::release.erase(std::remove(qgl::Mouse::move.begin(), qgl::Mouse::move.end(), node_release_callback), qgl::Mouse::release.end());
}

void node_press_callback()
{
	if (inside_rectangle(qgl::Mouse::pos, s.nodes.back().pane().pos(), s.nodes.back().pane().pos() + s.nodes.back().pane().size()))
	{
		qgl::Mouse::move.push_back(node_move_callback);
		qgl::Mouse::release.push_back(node_release_callback);
	}
}
/*
typedef void (*fn_ptr)();
std::vector<fn_ptr> fns;

void foo()
{

	fns.erase(std::remove(fns.begin(), fns.end(), foo), fns.end());

}

int main()
{
	fns.push_back(foo);

	foo();

	return 0;
}
*/
int main()
{
	active_scene_ptr = &s;
    qgl::init();

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


	s.load("first_scene.cms");

	qgl::Mouse::press.push_back(
		node_press_callback
	);

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