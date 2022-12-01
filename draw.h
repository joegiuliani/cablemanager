#pragma once
#include "shader.h"
#include <functional>
#include <GLFW/glfw3.h>

namespace draw
{
	void begin_frame();
	void end_frame();
	bool is_running();

	int init(int width, int height);
	void terminate();

	// convert these to take true/false params
	void draw_mask(bool flag);
	void apply_mask(bool flag);
	void draw_rect(const glm::vec2& pos, const glm::vec2& size);
	void draw_text(const glm::vec2& pos, const std::string& text);
	void draw_curve(const std::vector<glm::vec2>& points);
	void set_text_scale(float s);

	glm::vec2 get_text_size(const std::string& str);

	void stop_scissor();
	void scissor(const glm::vec2& pos, const glm::vec2& dim);
	void use_texture(bool flag);

	void shape_color(const glm::vec4& top, const glm::vec4& bottom);
	void shape_color(const glm::vec4& color);
	void shape_corner(float size);

	double get_time();
	double get_delta_time();

	constexpr int MOUSE_LEFT = GLFW_MOUSE_BUTTON_LEFT;
	constexpr int MOUSE_RIGHT = GLFW_MOUSE_BUTTON_RIGHT;
	constexpr int MOUSE_MIDDLE = GLFW_MOUSE_BUTTON_MIDDLE;
	constexpr int MOUSE_UP = GLFW_RELEASE;
	constexpr int MOUSE_DOWN = GLFW_PRESS;

	bool is_mouse_up(int button = MOUSE_LEFT);
	bool is_mouse_down(int button = MOUSE_LEFT);
	bool is_mouse_pressed(int button = MOUSE_LEFT);
	bool is_mouse_released(int button = MOUSE_LEFT);
	bool is_mouse_moving();
	glm::vec2 get_mouse_delta();
	glm::vec2 get_mouse_pos();
	int get_mouse_scroll();
	const glm::vec2& viewport_size();

}


