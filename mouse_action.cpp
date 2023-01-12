#include "mouse_action.h"
#include "draw.h"
#include "qgl.h"
#include "comman.h"
#include "scene.h"

namespace cm
{
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

	bool inside_rectangle(const glm::vec2& v, const glm::vec2& lower_bound, const glm::vec2& upper_bound) {
		return (v.x >= lower_bound.x && v.x <= upper_bound.x && v.y >= lower_bound.y && v.y <= upper_bound.y);
	}
	bool inside_shape(const vec& pos, qgl::Element* elem)
	{
		return inside_rectangle(pos, elem->pos(), elem->pos() + elem->size());
	}
	Node* node_under_mouse()
	{
		Node* ret = nullptr;
		Scene::active_scene_ptr->foreach([&](Node& n)
			{
				if (inside_shape(qgl::Mouse::pos, &n.pane))
				{
					ret = &n;
				}
			});


		return ret;
	}

	void formalize_rect(vec& start, vec& end)
	{
		if (start.x > end.x) std::swap(start.x, end.x);
		if (start.y > end.y) std::swap(start.y, end.y);
	}

	namespace MouseAction
	{
		void PressNode::init()
		{
			qgl::Mouse::press.push_back(press_callback);
			DragSelect::init();
		}
		
		void PressNode::press_callback()
		{
			Node* node_ptr = node_under_mouse();
			if (node_ptr == nullptr)
			{
				if (draw::is_mouse_down(draw::MOUSE_LEFT))
				{
					DragSelect::begin();
				}
			}
			else
			{
				Node& node = *node_ptr;

				if (draw::is_mouse_down(draw::MOUSE_LEFT))
				{


					Scene::active_scene_ptr->set_active_node(node); // change this when implementing selection

					DragNode::begin(node_ptr);
				}

				/*if (draw::is_mouse_down(draw::MOUSE_RIGHT))
				{
					qgl::Mouse::remove_callback(qgl::Mouse::press, press_callback);

					for (Button& b : node_popup)
					{
						qgl::Mouse::release.push_back(b.on_click);
					}
				}*/
			}
		}

		void DragNode::begin(Node* t_node_ptr)
		{
			node_ptr = t_node_ptr;
			node_move_start = node_ptr->pane.pos();
			node_move_sep = node_ptr->pane.pos() - qgl::Mouse::pos;
			qgl::Mouse::move.push_back(move_callback);
			qgl::Mouse::release.push_back(release_callback);

			qgl::Mouse::remove_callback(qgl::Mouse::press, PressNode::press_callback);
		}

		void DragNode::reset()
		{
			qgl::Mouse::remove_callback(qgl::Mouse::release, release_callback);
			qgl::Mouse::remove_callback(qgl::Mouse::move, move_callback);
			qgl::Mouse::press.push_back(PressNode::press_callback);
		}

		void DragNode::move_callback()
		{
			node_ptr->pane.set_pos(qgl::Mouse::pos + node_move_sep);
		}

		void DragNode::release_callback()
		{
			Scene::active_scene_ptr->comman.add_command(MoveNode(*node_ptr, node_move_start));

			reset();
		}

		void DragSelect::init()
		{
			box.set_enabled(false);
			box.fill.top = box.fill.bottom = color(1, 1, 1, 1);
			box.outline.top = box.outline.bottom = color(1, 1, 1, 1);
			box.outline_thickness = 1.5;
		}

		void DragSelect::begin()
		{
			qgl::Mouse::remove_callback(qgl::Mouse::press, PressNode::press_callback);
			qgl::Mouse::move.push_back(move_callback);
			qgl::Mouse::release.push_back(release_callback);
			start = qgl::Mouse::pos;
			box.set_pos(start);
			box.set_enabled(true);

		}
		void DragSelect::reset()
		{
			qgl::Mouse::remove_callback(qgl::Mouse::release, release_callback);
			qgl::Mouse::remove_callback(qgl::Mouse::move, move_callback);
			qgl::Mouse::press.push_back(PressNode::press_callback);
			box.set_enabled(false);
		}

		void DragSelect::move_callback()
		{
			end = qgl::Mouse::pos;
			vec fstart = start;
			vec fend = end;
			formalize_rect(fstart, fend);
			box.set_size(fend - fstart);
		}

		void DragSelect::release_callback()
		{
			Scene::active_scene_ptr->selected_nodes.clear();

			Scene::active_scene_ptr->foreach([&](Node& n)
				{
					if (inside_rectangle(n.pane.pos(), start, end) &&
						inside_rectangle(n.pane.pos() + n.pane.size(), start, end))
					{
						Scene::active_scene_ptr->selected_nodes.push_back(&n);
					}
				});
			//reset();
		}
	}
}




