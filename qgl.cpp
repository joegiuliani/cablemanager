#include <iostream>
#include <vector>
#include <glm/common.hpp>
#include <glm/vec2.hpp>
#include <glm/vec4.hpp>
#include "draw.h"
#include <glfw/glfw3.h>
#include "qgl.h"

#define QGL_DEBUG


namespace qgl
{
    float corner_size = 0;
    //Element god_element;
    glm::vec2 view_pos;
    float view_scale = 1;
    TowMouse tow_mouse;

    bool has_parent(const Element& element)
    {
        return element.parent != nullptr;
    }

    vec screen_to_world(const vec& v)
    {
        return (v - draw::viewport_size() * 0.5f) / view_scale + view_pos;
    }

    vec world_to_screen(const vec& v)
    {
        return view_scale * (v - view_pos) + draw::viewport_size() * 0.5f;
    }

    vec get_screen_pos(Element* element_ptr)
    {
#ifdef QGL_DEBUG
        if (element_ptr == nullptr)
        {
            std::cout << "get_screen_pos() was passed nullptr\n";
            return vec(0);
        }
#endif

        vec ret = element_ptr->options[Element::WORLD] ? world_to_screen(element_ptr->pos) : element_ptr->pos;

        // Offsets element by parents' positions
        // Child positions are always relative.
        if (has_parent(*element_ptr))
        {
            Element* parent_ptr = element_ptr->parent;

            while (has_parent(*parent_ptr))
            {
                if (parent_ptr->options[Element::WORLD])
                    ret += world_to_screen(parent_ptr->pos);
                else
                    ret += parent_ptr->pos;

                parent_ptr = parent_ptr->parent;
            }
        }


        return ret;
    }

    void set_corner_size(float s)
    {
        corner_size = s;
        draw::shape_corner(s);
    }

    bool contains(const vec& value, const vec& min, const vec& max)
    {
        return value.x >= min.x && value.y >= min.y && value.x <= max.x && value.y <= max.y;
    }

    Element::Element()
    {
    }

    Shape::Shape()
    {

    }
    TextBox::TextBox()
    {

    }

    void Element::clip_children(bool flag)
    {
        options[OCCLUDE_CHILDREN] = flag;
    }

    void Element::on_press(CallbackFn cf)
    {
        pressed = cf;
    }

    void Element::on_drag(CallbackFn cf)
    {
        dragged = cf;
    }

    const vec& Element::size()
    {
        return dim;
    }

    void Element::set_size(const vec& v)
    {
        if (v.x < 0 || v.y < 0)
        {
            std::cout << "Shape size can't be negative\n";
        }
        dim = glm::max(v, vec(0));
    }

    void Element::remove()
    {
        if (!has_parent(*this))
        {
            std::cout << "You tried removing the god element\n";
            return;
        }

        auto& store = parent->child_storage;
        auto it = std::find_if(store.begin(), store.end(), [&](ElementPtr& ep) {return ep.get() == this; });
        if (it != store.end()) // should always be true
        {
            store.erase(it);
        }
        else
        {
            std::cout << "Unable to remove element. It does not exist in its parents child_storage.";
        }
    }

    void Element::draw()
    {
        if (child_storage.size())
        {
            vec screen_pos = get_screen_pos(this);

            if (options[OCCLUDE_CHILDREN])
            {
                draw::scissor(screen_pos, dim * view_scale);
            }

            for (ElementPtr& cep : child_storage)
            {
                cep->draw();
            }

            draw::stop_scissor();
        }
    }

    void init()
    {
        draw::init(640, 480);
    }

    void follow_mouse(Element* element, std::function<bool()> stop_condition_fn)
    {
        tow_mouse.begin(element, stop_condition_fn);
    }

    void TowMouse::begin(Element* element, std::function<bool()> t_end_condition_fn)
    {
        active = true;
        element_ptr = element;
        end_condition_fn = t_end_condition_fn;

        if (element_ptr->options[Element::WORLD])
            delta = element->pos - screen_to_world(draw::get_mouse_pos());
        else
            delta = element->pos - draw::get_mouse_pos();
    }

    void TowMouse::reset()
    {
        active = false;
        element_ptr = nullptr;
        end_condition_fn = nullptr; // even tho it already should be 
    }
    void TowMouse::update()
    {
        // if true, the block assumes dragged_pos != nullptr
        if (active)
        {
#ifdef QGL_DEBUG
            if (element_ptr == nullptr)
            {
                std::cout << "QGL_DEBUG: TowMouse::element_ptr is null. If calling TowMouse::update() for the first time or after callign TowMouse::reset(), you must first call TowMouse::begin()\n";
                return;
            }
#endif      
            if (end_condition_fn())
            {
                TowMouse::reset();
            }
            else
            {
                if (element_ptr->options[Element::WORLD])
                    element_ptr->pos = screen_to_world(draw::get_mouse_pos()) + delta;
                else
                    element_ptr->pos = draw::get_mouse_pos() + delta;
            }
        }
    }

    bool process_mouse_events(Element* element_ptr)
    {
        Element& element = *element_ptr;

        // We draw one branch at a time in forward order, parent first
        // So we'll have to go the opposite direction

        for (auto it = element.child_storage.rbegin(); it != element.child_storage.rend(); ++it)
        {
            if (process_mouse_events((*it).get()))
            {
                // The the first, deepest element we iterate through that has mouse listening enabled is the only element that receives mouse events.
                return true;
            }
        }

        // If the element we're looking at doesn't care about the mouse, we continue looking down the branch.
        // For instance if a moveable object has a non-interactive label, we still want to move the object even if we click on the label.
        if (!element.options[Element::MOUSE_LISTENER])
            return false;

        vec screen_pos = get_screen_pos(element_ptr);
        vec screen_dim = element.size();
        if (element.options[Element::WORLD]) screen_dim *= view_scale;
        vec mouse_pos = draw::get_mouse_pos();

        if (contains(mouse_pos, screen_pos, screen_pos + screen_dim))
        {
            if (element.hovered != nullptr)
                element.hovered(element_ptr);

            if (element.pressed != nullptr && draw::is_mouse_pressed())
            {
                element.pressed(element_ptr);
            }

            if (draw::is_mouse_down() && draw::is_mouse_moving())
            {
                if (element.dragged != nullptr)
                    element.dragged(element_ptr);
            }

            return true;
        }

        return false;
    }

    void on_frame()
    {
        draw::begin_frame();

        const float zoom_fac = 0.1f;
        view_scale *= 1 + zoom_fac * draw::get_mouse_scroll();

        if (draw::is_mouse_down(draw::MOUSE_MIDDLE) && draw::is_mouse_moving())
        {
            view_pos -= draw::get_mouse_delta() / view_scale;
        }

        // Processes mouse events for all mouse listeners.
        process_mouse_events(&god_element);

        tow_mouse.update();

        for (ElementPtr& ep : god_element.child_storage)
        {
            ep->draw();
        }

        draw::shape_color(color(1));


        draw::end_frame();
    }

    void terminate()
    {
        draw::terminate();
    }
    bool is_running()
    {
        return draw::is_running();
    }
    void TextBox::set_text(const std::string& str)
    {
        text = str;
        calculate_wrap();
    }

    void TextBox::draw()
    {
        draw::apply_mask(false);
        draw::draw_mask(false);
        draw::shape_color(fill.top, fill.bottom);

        vec screen_pos = get_screen_pos(this);
        float scale = options[WORLD] ? view_scale : 1.0f;
        draw::set_text_scale(scale * text_scale);
        draw::scissor(screen_pos, scale * dim);

        vec cursor = screen_pos;
        for (const std::string& str : lines)
        {
            draw::draw_text(cursor, str);
            cursor.y += draw::get_text_size("").y;
        }

        if (child_storage.size())
        {
            if (options[OCCLUDE_CHILDREN])
            {
                draw::scissor(screen_pos, dim * scale);
            }

            for (ElementPtr& cep : child_storage)
            {
                cep->draw();
            }
        }
        draw::stop_scissor();
    }

    void TextBox::set_size(const vec& s)
    {
        Element::set_size(s);
        calculate_wrap();
    }

    void TextBox::set_text_scale(float s)
    {
        text_scale = s;
        calculate_wrap();
    }

    void TextBox::calculate_wrap()
    {
        lines.clear();        

        draw::set_text_scale(text_scale);

        int max_lines = std::ceilf(dim.y / (draw::get_text_size("").y*text_scale));
        std::string line = "";
        float line_width = 0;
        int token_start = 0;

        for (int k = 0; k < text.length() && lines.size() < max_lines; k++)
        {
            if (text[k] == ' ')
            {
                std::string token = text.substr(token_start, k + 1 - token_start);
                float token_width = draw::get_text_size(token).x * text_scale;
                if (line_width + token_width < dim.x)
                {
                    line += token;
                    line_width = line_width + token_width;
                }
                else
                {
                    if (line.empty())
                    {
                        lines.push_back(token);
                    }
                    else
                    {
                        lines.push_back(line);
                        line = token;
                        line_width = token_width;
                    }
                }

                token_start = k + 1;
            }
        }

        if (token_start < text.length())
        {
            std::string final_token = text.substr(token_start);
            if (line_width + draw::get_text_size(final_token).x < dim.x)
            {
                lines.push_back(line + final_token);
            }

            else
            {
                lines.push_back(line);
                lines.push_back(final_token);
            }
        }
        else
        {
            lines.push_back(line);
        }
    }

    void Shape::draw()
    {
        draw::apply_mask(false);
        draw::draw_mask(true);
        draw::use_texture(false);

        vec screen_pos = get_screen_pos(this);
        float scale = options[WORLD] ? view_scale : 1.0f;

        // All the shader access should be setters.
        draw::shape_corner(scale * corner_size);

        draw::shape_color(fill.top, fill.bottom);

        draw::draw_rect(screen_pos, dim * scale);

        // Draw outline
        if (outline_thickness > 0)
        {
            if (outline_thickness * scale < 1 && options[WORLD] || outline_thickness < 1)
                draw::shape_color(color(glm::vec3(outline.top), outline_thickness * 0.8), color(glm::vec3(outline.bottom), outline_thickness * 0.8));

            else
                draw::shape_color(outline.top, outline.bottom);

            draw::draw_mask(false);
            draw::apply_mask(true);

            if (corner_size > 0)
            {
                draw::shape_corner(scale * (corner_size + outline_thickness));
            }

            // We should set the alpha to scale
            // so that outlines fade away as they become thinner than a pixel.

            draw::draw_rect(screen_pos - outline_thickness * scale, scale * (dim + outline_thickness * 2.0f * glm::sign(dim)));
        } // End draw outline

        if (child_storage.size())
        {
            if (options[OCCLUDE_CHILDREN])
            {
                draw::scissor(screen_pos, dim * view_scale);
            }

            for (ElementPtr& cep : child_storage)
            {
                cep->draw();
            }

            draw::stop_scissor();
        }
    }

    Curve::Curve()
    {
    }

    void Curve::draw()
    {
        draw::draw_curve(points);
    }
}

