#include <iostream>
#include <vector>
#include <glm/common.hpp>
#include <glm/vec2.hpp>
#include <glm/vec4.hpp>
#include "draw.h"
#include <glfw/glfw3.h>
#include "qgl.h"

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
	Port* connection = nullptr;

	~Port()
	{
		sever_connection();
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
		label.set_size(qgl::vec(75, 75));
		label.fill.top = label.fill.bottom = qgl::color(1);
	}

	private:
		void init()
		{
			pane.fill.top = qgl::color(0.3, 0.09, .15, 1);
			pane.fill.bottom = pane.fill.top * 0.8f;
			pane.set_size(qgl::vec(75, 75));


			auto drag_element = [&](qgl::Element* element_ptr)
			{
				qgl::follow_mouse
				(
					element_ptr, // this
					[&]() {return draw::is_mouse_released();} // condition to stop following the mouse
				);
			};

			// Label will follow since its a child
			pane.on_drag(drag_element);
		}
};

int main()
{
    qgl::init();

	// We may need to abstract further to include lines and text

	// We can store a constant amount of vertices for the curve buffer. 
	// Then we can specify for each draw how many of those vertices were actually going to use


	Node node("Hello");

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