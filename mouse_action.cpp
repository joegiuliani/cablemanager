#include "mouse_action.h"
#include "draw.h"
#include "qgl.h"
#include "comman.h"
#include "scene.h"

namespace cm
{

	class MoveNodes : public Command
	{
		Scene::NodePtrSet nodes;
		vec delta_world;
	public:
		MoveNodes(Scene::NodePtrSet nodes, vec delta)
		{
			delta_world = qgl::screen_to_world_scale(delta);
			reverse();
		}

		virtual void execute()
		{
			for (Node* node_ptr : nodes)
				node_ptr->pane.set_pos(node_ptr->pane.pos() + qgl::world_to_screen_scale(delta_world));
		}

		virtual void reverse()
		{
			for (Node* node_ptr : nodes)
				node_ptr->pane.set_pos(node_ptr->pane.pos() - qgl::world_to_screen_scale(delta_world));
		}
	};

	bool inside_rectangle(const glm::vec2& v, const glm::vec2& lower_bound, const glm::vec2& upper_bound) {
		return (v.x >= lower_bound.x && v.x <= upper_bound.x && v.y >= lower_bound.y && v.y <= upper_bound.y);
	}
	bool inside_shape(const vec& pos, qgl::Element* elem)
	{
		return inside_rectangle(pos, elem->pos(), elem->pos() + elem->size());
	}

	void formalize_rect(vec& start, vec& end)
	{
		if (start.x > end.x) std::swap(start.x, end.x);
		if (start.y > end.y) std::swap(start.y, end.y);
	}

	void reverse_foreach_node(std::function<bool(Node& n)> body)
	{
		auto& nodes = Scene::active_scene_ptr->nodes;
		for (auto it = nodes.rbegin(); it != nodes.rend(); ++it)
		{
			if (body(**it)) return;
		}
	}

	void foreach_node(std::function<bool(Node& n)> body)
	{
		for (auto& node_ptr : Scene::active_scene_ptr->nodes)
		{
			if (body(*node_ptr)) return;
		}
	}

	void send_to_front(Node& n) {

		auto& nodes = Scene::active_scene_ptr->nodes;

		auto it = std::find_if(nodes.begin(), nodes.end(), [&n](const std::shared_ptr<Node>& p) { return p.get() == &n; });
		if (it != nodes.end()) {
			std::rotate(it, it + 1, nodes.end());
		}
	}


	Node* node_under_mouse()
	{
		Node* ret = nullptr;
		auto fn = [&](Node& node) -> bool
		{
			if (inside_shape(qgl::Mouse::pos, &node.pane))
			{
				ret = &node;
				return true;
			}

			return false;
		};
		reverse_foreach_node(fn);

		return ret;
	}

	void highlight_node(Node& n)
	{
		n.pane.outline.top = Node::OUTLINE * 2.0f;
	}

	void select_node(Node& n)
	{
		Scene::active_scene_ptr->selected_nodes.insert(&n);
		highlight_node(n);
	}

	void deselect_all_nodes()
	{
		for (Node* node_ptr : Scene::active_scene_ptr->selected_nodes)
		{
			node_ptr->pane.outline.top = Node::OUTLINE;
		}
		Scene::active_scene_ptr->selected_nodes.clear();
		Scene::active_scene_ptr->active_node_ptr = nullptr;
	}

	void set_active_node(Node& n)
	{
		Node* active_node_ptr = Scene::active_scene_ptr->active_node_ptr;
		if (active_node_ptr != nullptr)
		{
			if (Scene::active_scene_ptr->selected_nodes.count(active_node_ptr) == 0)
				highlight_node(*active_node_ptr);

			active_node_ptr->pane.outline.top = Node::OUTLINE;
		}

		Scene::active_scene_ptr->active_node_ptr = &n;
		n.pane.outline.top = color(1.0f, 0.6f, 0.6f, 1.0f);

		send_to_front(n);
		n.pane.send_to_front();
	}

	namespace MouseAction
	{
		void give_back_control()
		{
			qgl::Mouse::press.push_back(PressNode::press_callback);
			qgl::Mouse::scroll.push_back(Zoom::scroll_callback);
		}
		void take_control()
		{
			qgl::Mouse::remove_callback(qgl::Mouse::press, PressNode::press_callback);
			qgl::Mouse::remove_callback(qgl::Mouse::scroll, Zoom::scroll_callback);
		}

		void PressNode::init()
		{
			qgl::Mouse::press.push_back(press_callback);
			qgl::Mouse::scroll.push_back(Zoom::scroll_callback);
			DragSelect::init();
		}
		
		void PressNode::press_callback()
		{
			if (draw::is_mouse_down(draw::MOUSE_MIDDLE))
			{
				Pan::begin();
				return;
			}

			Node* node_ptr = node_under_mouse();
			if (node_ptr == nullptr)
			{
				if (draw::is_mouse_down(draw::MOUSE_LEFT))
				{
					DragSelect::begin();
					return;
				}
			}
			else
			{
				Node& node = *node_ptr;

				if (draw::is_mouse_down(draw::MOUSE_LEFT))
				{
					if (Scene::active_scene_ptr->selected_nodes.count(&node) == 0)
					{
						deselect_all_nodes();
						select_node(node);
					}

					set_active_node(node);

					DragNode::begin();
					return;
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

		void DragNode::begin()
		{
			take_control();
			start = qgl::Mouse::pos;
			qgl::Mouse::move.push_back(move_callback);
			qgl::Mouse::release.push_back(release_callback);
		}

		void DragNode::move_callback()
		{
			for (Node* node_ptr : Scene::active_scene_ptr->selected_nodes)
			{
				node_ptr->pane.set_pos(node_ptr->pane.pos() + qgl::Mouse::delta);
			}
		}

		void DragNode::release_callback()
		{
			Scene::active_scene_ptr->comman.add_command(MoveNodes(Scene::active_scene_ptr->selected_nodes, qgl::Mouse::pos - start));

			qgl::Mouse::remove_callback(qgl::Mouse::release, release_callback);
			qgl::Mouse::remove_callback(qgl::Mouse::move, move_callback);
			give_back_control();
		}

		void DragSelect::init()
		{
			box.set_enabled(false);
			box.fill.top = box.fill.bottom = color(0);
			box.outline.top = box.outline.bottom = color(1, 1, 1, 1);
			box.outline_thickness = 1.5;
			box.corner_radius = 0;
		}

		void DragSelect::begin()
		{
			take_control();
			qgl::Mouse::move.push_back(move_callback);
			qgl::Mouse::release.push_back(release_callback);
			start = end = qgl::Mouse::pos;
			box.set_pos(start);
			box.set_size(vec(0));
			box.set_enabled(true);

		}

		void DragSelect::move_callback()
		{
			end = qgl::Mouse::pos;
			vec fstart = start;
			vec fend = end;
			formalize_rect(fstart, fend);
			box.set_pos(fstart);
			box.set_size(fend - fstart);
		}

		void DragSelect::release_callback()
		{
			deselect_all_nodes();

			formalize_rect(start, end);

			auto fn = [&](Node& node) -> bool
			{
				if (inside_rectangle(node.pane.pos(), start, end) &&
					inside_rectangle(node.pane.pos() + node.pane.size(), start, end))
				{
					select_node(node);
				}

				return false;
			};
			reverse_foreach_node(fn);

			qgl::Mouse::remove_callback(qgl::Mouse::release, release_callback);
			qgl::Mouse::remove_callback(qgl::Mouse::move, move_callback);
			give_back_control();
			box.set_enabled(false);
		}

		void Zoom::scroll_callback()
		{
			qgl::view_scale += 0.01f * qgl::Mouse::scroll_dir * qgl::view_scale;
		}

		void Pan::begin()
		{
			take_control();
			qgl::Mouse::move.push_back(move_callback);
			qgl::Mouse::release.push_back(release_callback);
		}

		void Pan::move_callback()
		{
			qgl::set_world_center(qgl::world_center() + qgl::Mouse::delta);
		}

		void Pan::release_callback()
		{
			qgl::Mouse::remove_callback(qgl::Mouse::move, move_callback);
			qgl::Mouse::remove_callback(qgl::Mouse::release, release_callback);
			give_back_control();
		}
	}
}