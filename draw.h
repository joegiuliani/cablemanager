#pragma once
#include "shader.h"
#include <functional>
#include <GLFW/glfw3.h>
#include <set>

namespace draw
{
	constexpr int KEY_UNKNOWN = GLFW_KEY_UNKNOWN;
	constexpr int KEY_SPACE = GLFW_KEY_SPACE;
	constexpr int KEY_APOSTROPHE = GLFW_KEY_APOSTROPHE;
	constexpr int KEY_COMMA = GLFW_KEY_COMMA;
	constexpr int KEY_MINUS = GLFW_KEY_MINUS;
	constexpr int KEY_PERIOD = GLFW_KEY_PERIOD;
	constexpr int KEY_SLASH = GLFW_KEY_SLASH;
	constexpr int KEY_0 = GLFW_KEY_0;
	constexpr int KEY_1 = GLFW_KEY_1;
	constexpr int KEY_2 = GLFW_KEY_2;
	constexpr int KEY_3 = GLFW_KEY_3;
	constexpr int KEY_4 = GLFW_KEY_4;
	constexpr int KEY_5 = GLFW_KEY_5;
	constexpr int KEY_6 = GLFW_KEY_6;
	constexpr int KEY_7 = GLFW_KEY_7;
	constexpr int KEY_8 = GLFW_KEY_8;
	constexpr int KEY_9 = GLFW_KEY_9;
	constexpr int KEY_SEMICOLON = GLFW_KEY_SEMICOLON;
	constexpr int KEY_EQUAL = GLFW_KEY_EQUAL;
	constexpr int KEY_A = GLFW_KEY_A;
	constexpr int KEY_B = GLFW_KEY_B;
	constexpr int KEY_C = GLFW_KEY_C;
	constexpr int KEY_D = GLFW_KEY_D;
	constexpr int KEY_E = GLFW_KEY_E;
	constexpr int KEY_F = GLFW_KEY_F;
	constexpr int KEY_G = GLFW_KEY_G;
	constexpr int KEY_H = GLFW_KEY_H;
	constexpr int KEY_I = GLFW_KEY_I;
	constexpr int KEY_J = GLFW_KEY_J;
	constexpr int KEY_K = GLFW_KEY_K;
	constexpr int KEY_L = GLFW_KEY_L;
	constexpr int KEY_M = GLFW_KEY_M;
	constexpr int KEY_N = GLFW_KEY_N;
	constexpr int KEY_O = GLFW_KEY_O;
	constexpr int KEY_P = GLFW_KEY_P;
	constexpr int KEY_Q = GLFW_KEY_Q;
	constexpr int KEY_R = GLFW_KEY_R;
	constexpr int KEY_S = GLFW_KEY_S;
	constexpr int KEY_T = GLFW_KEY_T;
	constexpr int KEY_U = GLFW_KEY_U;
	constexpr int KEY_V = GLFW_KEY_V;
	constexpr int KEY_W = GLFW_KEY_W;
	constexpr int KEY_X = GLFW_KEY_X;
	constexpr int KEY_Y = GLFW_KEY_Y;
	constexpr int KEY_Z = GLFW_KEY_Z;
	constexpr int KEY_LEFT_BRACKET = GLFW_KEY_LEFT_BRACKET;
	constexpr int KEY_BACKSLASH = GLFW_KEY_BACKSLASH;
	constexpr int KEY_RIGHT_BRACKET = GLFW_KEY_RIGHT_BRACKET;
	constexpr int KEY_GRAVE_ACCENT = GLFW_KEY_GRAVE_ACCENT;
	constexpr int KEY_WORLD_1 = GLFW_KEY_WORLD_1;
	constexpr int KEY_WORLD_2 = GLFW_KEY_WORLD_2;
	constexpr int KEY_ESCAPE = GLFW_KEY_ESCAPE;
	constexpr int KEY_ENTER = GLFW_KEY_ENTER;
	constexpr int KEY_TAB = GLFW_KEY_TAB;
	constexpr int KEY_BACKSPACE = GLFW_KEY_BACKSPACE;
	constexpr int KEY_INSERT = GLFW_KEY_INSERT;
	constexpr int KEY_DELETE = GLFW_KEY_DELETE;
	constexpr int KEY_RIGHT = GLFW_KEY_RIGHT;
	constexpr int KEY_LEFT = GLFW_KEY_LEFT;
	constexpr int KEY_DOWN = GLFW_KEY_DOWN;
	constexpr int KEY_UP = GLFW_KEY_UP;
	constexpr int KEY_PAGE_UP = GLFW_KEY_PAGE_UP;
	constexpr int KEY_PAGE_DOWN = GLFW_KEY_PAGE_DOWN;
	constexpr int KEY_HOME = GLFW_KEY_HOME;
	constexpr int KEY_END = GLFW_KEY_END;
	constexpr int KEY_CAPS_LOCK = GLFW_KEY_CAPS_LOCK;
	constexpr int KEY_SCROLL_LOCK = GLFW_KEY_SCROLL_LOCK;
	constexpr int KEY_NUM_LOCK = GLFW_KEY_NUM_LOCK;
	constexpr int KEY_PRINT_SCREEN = GLFW_KEY_PRINT_SCREEN;
	constexpr int KEY_PAUSE = GLFW_KEY_PAUSE;
	constexpr int KEY_F1 = GLFW_KEY_F1;
	constexpr int KEY_F2 = GLFW_KEY_F2;
	constexpr int KEY_F3 = GLFW_KEY_F3;
	constexpr int KEY_F4 = GLFW_KEY_F4;
	constexpr int KEY_F5 = GLFW_KEY_F5;
	constexpr int KEY_F6 = GLFW_KEY_F6;
	constexpr int KEY_F7 = GLFW_KEY_F7;
	constexpr int KEY_F8 = GLFW_KEY_F8;
	constexpr int KEY_F9 = GLFW_KEY_F9;
	constexpr int KEY_F10 = GLFW_KEY_F10;
	constexpr int KEY_F11 = GLFW_KEY_F11;
	constexpr int KEY_F12 = GLFW_KEY_F12;
	constexpr int KEY_F13 = GLFW_KEY_F13;
	constexpr int KEY_F14 = GLFW_KEY_F14;
	constexpr int KEY_F15 = GLFW_KEY_F15;
	constexpr int KEY_F16 = GLFW_KEY_F16;
	constexpr int KEY_F17 = GLFW_KEY_F17;
	constexpr int KEY_F18 = GLFW_KEY_F18;
	constexpr int KEY_F19 = GLFW_KEY_F19;
	constexpr int KEY_F20 = GLFW_KEY_F20;
	constexpr int KEY_F21 = GLFW_KEY_F21;
	constexpr int KEY_F22 = GLFW_KEY_F22;
	constexpr int KEY_F23 = GLFW_KEY_F23;
	constexpr int KEY_F24 = GLFW_KEY_F24;
	constexpr int KEY_F25 = GLFW_KEY_F25;
	constexpr int KEY_KP_0 = GLFW_KEY_KP_0;
	constexpr int KEY_KP_1 = GLFW_KEY_KP_1;
	constexpr int KEY_KP_2 = GLFW_KEY_KP_2;
	constexpr int KEY_KP_3 = GLFW_KEY_KP_3;
	constexpr int KEY_KP_4 = GLFW_KEY_KP_4;
	constexpr int KEY_KP_5 = GLFW_KEY_KP_5;
	constexpr int KEY_KP_6 = GLFW_KEY_KP_6;
	constexpr int KEY_KP_7 = GLFW_KEY_KP_7;
	constexpr int KEY_KP_8 = GLFW_KEY_KP_8;
	constexpr int KEY_KP_9 = GLFW_KEY_KP_9;
	constexpr int KEY_KP_DECIMAL = GLFW_KEY_KP_DECIMAL;
	constexpr int KEY_KP_DIVIDE = GLFW_KEY_KP_DIVIDE;
	constexpr int KEY_KP_MULTIPLY = GLFW_KEY_KP_MULTIPLY;
	constexpr int KEY_KP_SUBTRACT = GLFW_KEY_KP_SUBTRACT;
	constexpr int KEY_KP_ADD = GLFW_KEY_KP_ADD;
	constexpr int KEY_KP_ENTER = GLFW_KEY_KP_ENTER;
	constexpr int KEY_KP_EQUAL = GLFW_KEY_KP_EQUAL;
	constexpr int KEY_LEFT_SHIFT = GLFW_KEY_LEFT_SHIFT;
	constexpr int KEY_LEFT_CONTROL = GLFW_KEY_LEFT_CONTROL;
	constexpr int KEY_LEFT_ALT = GLFW_KEY_LEFT_ALT;
	constexpr int KEY_LEFT_SUPER = GLFW_KEY_LEFT_SUPER;
	constexpr int KEY_RIGHT_SHIFT = GLFW_KEY_RIGHT_SHIFT;
	constexpr int KEY_RIGHT_CONTROL = GLFW_KEY_RIGHT_CONTROL;
	constexpr int KEY_RIGHT_ALT = GLFW_KEY_RIGHT_ALT;
	constexpr int KEY_RIGHT_SUPER = GLFW_KEY_RIGHT_SUPER;
	constexpr int KEY_MENU = GLFW_KEY_MENU;
	constexpr int KEY_LAST = GLFW_KEY_LAST;

	void begin_frame();
	void end_frame();
	bool is_running();

	int init(int width, int height);
	void terminate();

	// convert these to take true/false params
	void draw_mask(bool flag);
	void apply_mask(bool flag);
	void draw_rect(const glm::vec2& m_pos, const glm::vec2& size);
	void draw_text(const glm::vec2& m_pos, const std::string& text);
	void draw_curve(const std::vector<glm::vec2>& points);
	void set_text_scale(float s);

	glm::vec2 get_text_size(const std::string& str);

	void stop_scissor();
	void scissor(const glm::vec2& m_pos, const glm::vec2& m_size);
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

	inline bool key_change_flag = false;

	bool is_mouse_up(int button = MOUSE_LEFT);
	bool is_mouse_down(int button = MOUSE_LEFT);
	bool is_mouse_pressed(int button = MOUSE_LEFT);
	bool is_mouse_released(int button = MOUSE_LEFT);
	bool is_mouse_moving();
	glm::vec2 get_mouse_delta();
	glm::vec2 get_mouse_pos();
	int get_mouse_scroll();
	const glm::vec2& viewport_size();
	std::set<int>& get_down_keys();

}


