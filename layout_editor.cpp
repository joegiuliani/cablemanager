#include "layout_editor.h"
#include <memory>
#include <vector>
#include <stack>
#include <imgui/imgui.h>
#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui/imgui_internal.h>
#include "animation.h"
#include "techlayout.h"
#include "memento.h"
#include "fonts.h"
#include "draw.h"

typedef std::shared_ptr<TLNode>TLNodePtr;
TLNode* active_node_ptr = nullptr;
TLNode& LayoutEditor::active_node()
{
    if (!active_node_ptr)
    {
        std::cout << "No active node set";
        throw std::exception(); // We could change exception type and also handle from callers.
    }
      
    return *active_node_ptr;
}
bool LayoutEditor::has_active_node()
{
    return active_node_ptr != nullptr;
}

namespace glm
{
    bool operator<=(const TLVec& lhs, const TLVec& rhs)
    {
        return lhs.x <= rhs.x && lhs.y <= rhs.y;
    }

    bool operator>=(const TLVec& lhs, const TLVec& rhs)
    {
        return lhs.x >= rhs.x && lhs.y >= rhs.y;
    }
}
#define in_bounds(x, min, max) x >= min && x <= max

typedef std::reference_wrapper<TLNode> TLNodeRef;
std::vector<TLNodeRef> nodes;

TLVec mouse_world;

constexpr float SCALE_MAX = 0.175326f;

void LayoutEditor::init(int width, int height)
{
    // For ellipses, rects, etc. Not for curves or text

    TLDraw::init(width, height);
    TLDraw::set_max_scale(SCALE_MAX);


    TL::init_fonts(); // Generates glyph vao

    // TODO read from toml
    TLNode::header_font = TL::gen_atlas_id();
    TL::load_atlas(TLNode::header_font, "C:/Windows/Fonts/segoeuib.ttf", 64);

    TLNode::body_font = TL::gen_atlas_id();
    TL::load_atlas(TLNode::body_font, "C:/Windows/Fonts/segoeui.ttf", 64);

    // No shaders may be added after this point.
}
void LayoutEditor::draw()
{
    TLDraw::begin();

    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    for (auto& node : nodes)
        TLNode::draw(node);

    // TODO comment out or make TLDraw::end() func
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
bool LayoutEditor::handle_input()
{
    ImGuiIO& io = ImGui::GetIO();

    bool inputs_used = false;

    if (io.MouseWheel)
    {
        float new_scale = TLDraw::get_scale() * (1 + io.MouseWheel * 0.1f);
        TLDraw::set_scale(new_scale);
        inputs_used = true;
    }   

    // convert mouse to ndc space
    ImVec2 mouse_canvas = io.MousePos - ImGui::GetItemRectMin();
    TLVec mouse_ndc = (TLVec(mouse_canvas.x, mouse_canvas.y) / TLDraw::fbo_dim) * 2.0f - 1.0f;
    mouse_world = TLDraw::ndc_to_world(mouse_ndc);

    TLVec mouse_world_delta = TLDraw::fbo_to_world(TLVec(io.MouseDelta.x, io.MouseDelta.y), true);

    if (ImGui::IsMouseDragging(ImGuiMouseButton_Middle))
    {
        TLDraw::move_camera(mouse_world_delta);
        inputs_used = true;
    }

    // Search for node under mouse click
    // nodes should be ordered closest to furthest.
    // This determines the active node
    for (TLNode& n : nodes)
    {
        if (in_bounds(mouse_world, n.bounds().p0, n.bounds().p1))
        {
            if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
            {
                active_node_ptr = &n; // nodes' elements are reference wrapper. They should be unwrapped implicitly by the for loop header statement. We should be accessing the heap pointer to the actual node here.
                node_memento.push_state(active_node_ptr, n);
                TL::animate(&n.hlt_perc, n.hlt_perc, 1.0f, 1.5f);
                active_node().set_name("Cardiovascular Bronchidis");
                inputs_used = true;
            }

            else if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
            {
                if (!TL::is_animating(&n.collapse_perc))
                    TL::animate(&n.collapse_perc, n.collapse_perc, 1 - n.collapse_perc, 0.175);

                inputs_used = true;
            }
        }
    }


    if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_Z, false)) // change to true in future once you figure out cooldowns.
    {
        node_memento.undo();
        inputs_used = true;
    }

    if (io.KeyCtrl && io.KeyShift && ImGui::IsKeyPressed(ImGuiKey_Z, false)) // change to true in future once you figure out cooldowns.
    {
        node_memento.redo();
        inputs_used = true;
    }

    return inputs_used;
}