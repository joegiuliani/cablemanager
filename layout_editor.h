#pragma once
#include "techlayout.h"
#include <vector>
#include "text.h"
#include "node.h"
#include "memento.h"

struct ImVec2;
struct ImVec4;

namespace LayoutEditor
{
	void init(int width, int height);
	void draw();
	bool handle_input();

	inline TLStateManager node_memento;

	TLNode& active_node();
	bool has_active_node();
};