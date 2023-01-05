#include <iostream>
#include <vector>
#include <glm/common.hpp>
#include <glm/vec2.hpp>
#include <glm/vec4.hpp>
#include "draw.h"
#include <glfw/glfw3.h>
#include "qgl.h"




namespace qgl
{
    class GodElement : public Element
    {
    public:
        GodElement()
        {

        }
    };

    GodElement GOD_ELEMENT;

    Element& head_element = GOD_ELEMENT.add_child<Element>();

    vec world_center()
    {
        return head_element.pos();
    }

    //TowMouse tow_mouse;

    bool has_parent(const Element& element)
    {
        return element.parent != nullptr;
    }

    vec screen_to_world_scale(const vec& v)
    {
        return v / view_scale;
    }

    vec world_to_screen_scale(const vec& v)
    {
        return view_scale * v;
    }

    vec screen_to_world_projection(const vec& v)
    {
        return screen_to_world_scale(v - draw::viewport_size() * 0.5f) + head_element.pos();
    }

    vec world_to_screen_projection(const vec& v)
    {
        return world_to_screen_scale(v-head_element.pos()) + draw::viewport_size() * 0.5f;
    }

    bool contains(const vec& value, const vec& min, const vec& max)
    {
        return value.x >= min.x && value.y >= min.y && value.x <= max.x && value.y <= max.y;
    }

    Element::Element()
    {

    }

    Element::Element(const Element& elem)
    {

    }

    Shape::Shape()
    {
    }
    Shape::Shape(const Shape& sh)
    {

    }
    Curve::Curve(const Curve& curve)
    {

    }
    TextBox::TextBox()
    {

    }
    TextBox::TextBox(const TextBox& tb)
    {

    }

    void Element::clip_children(bool flag)
    {
        options[OCCLUDE_CHILDREN] = flag;
    }

    //void Element::on_press(CallbackFn cf)
    //{
    //    pressed = cf;
    //}

    //void Element::on_drag(CallbackFn cf)
    //{
    //    dragged = cf;
    //}

    vec Element::size()
    {
        return options[WORLD] ? world_to_screen_scale(m_size) : m_size;
    }

    void Element::set_size(const vec& v)
    {
        if (v.x < 0 || v.y < 0)
        {
            std::cout << "Shape size can't be negative\n";
            return;
        }

        m_size = options[WORLD] ? screen_to_world_scale(v) : v;
    }

    vec Element::pos()
    {
        vec ret = options[WORLD] ? world_to_screen_scale(m_pos) : m_pos;

        Element* parent_ptr = parent;
        while (parent_ptr != nullptr)
        {
            ret += parent_ptr ? world_to_screen_scale(parent_ptr->m_pos) : parent_ptr->m_pos;
            parent_ptr = parent_ptr->parent;
        }

        return ret;
    }

    void Element::set_pos(const vec& v)
    {
        m_pos = options[WORLD] ? screen_to_world_scale(v) : v;
    }

    void Element::draw()
    {
        if (child_storage.size())
        {

            if (options[OCCLUDE_CHILDREN])
            {
                draw::scissor(pos(), size());
            }

            for (ElementPtr& cep : child_storage)
            {
                cep->draw();
            }

            draw::stop_scissor();
        }
    }
 
    bool Mouse::is_down(int button)
    {
        return draw::is_mouse_down(button);
    }

    void init()
    {
        draw::init(640, 480);
    }

    /*
    void follow_mouse(Element* element, std::function<bool()> stop_condition_fn)
    {
        tow_Mouse::begin(element, stop_condition_fn);
    }

   
    void TowMouse::begin(Element* element, std::function<bool()> t_end_condition_fn)
    {
        active = true;
        element_ptr = element;
        end_condition_fn = t_end_condition_fn;

        if (element_ptr->options[Element::WORLD])
            delta = element->m_pos - screen_to_world_projection(draw::get_mouse_pos());
        else
            delta = element->m_pos - draw::get_mouse_pos();
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
                    element_ptr->m_pos = screen_to_world_projection(draw::get_mouse_pos()) + delta;
                else
                    element_ptr->m_pos = draw::get_mouse_pos() + delta;
            }
        }
    } */


    void Mouse::remove_this_callback()
    {
        remove_flag = true;
    }

    void Mouse::remove_callback(std::list<MouseCallbackPtr>& cbl, MouseCallbackPtr cb)
    {
        callbacks_to_remove.insert(std::pair(&cbl, cb));
    }

    void Mouse::process_mouse_events()
    {
        pos = draw::get_mouse_pos();
        scroll_dir = draw::get_mouse_scroll();
        delta = draw::get_mouse_delta();

        auto call_list = [&](std::list<MouseCallbackPtr> cbl)
        {
            for (auto& cb : cbl)
            {
                cb();
                if (remove_flag)
                {
                    callbacks_to_remove.insert(std::pair(&cbl, cb));
                    remove_flag = false;
                }
            }
        };

        // Note the qualifiers on the for each vars
        if (draw::is_mouse_moving())
            call_list(move);
        
        if (draw::is_mouse_pressed(draw::MOUSE_LEFT) ||
            draw::is_mouse_pressed(draw::MOUSE_MIDDLE) ||
            draw::is_mouse_pressed(draw::MOUSE_RIGHT))

            call_list(press);

        if (draw::is_mouse_released(draw::MOUSE_LEFT) ||
            draw::is_mouse_released(draw::MOUSE_MIDDLE) ||
            draw::is_mouse_released(draw::MOUSE_RIGHT))
            call_list(release);

        if (draw::get_mouse_scroll())
            call_list(scroll);

        for (auto& cbp : callbacks_to_remove)
        {
            cbp.first->erase(std::find(cbp.first->begin(), cbp.first->end(), cbp.second));
        }

        callbacks_to_remove.clear();
    }
    /*
    bool Element::process_mouse_events()
    {
        // We draw one branch at a time in forward order, parent first
        // So we'll have to go the opposite direction

        for (auto it = child_storage.rbegin(); it != child_storage.rend(); ++it)
        {
            if ((*it)->process_mouse_events())
            {
                // The the first, deepest element we iterate through that has mouse listening enabled is the only element that receives mouse events.
                return true;
            }
        }

        // If the element we're looking at doesn't care about the mouse, we continue looking down the branch.
        // For instance if a moveable object has a non-interactive label, we still want to move the object even if we click on the label.
        if (!options[Element::MOUSE_LISTENER])
            return false;

        vec screen_pos = get_screen_pos(this);
        vec screen_dim = m_size;
        if (options[Element::WORLD]) screen_dim *= view_scale;
        vec mouse_pos = draw::get_mouse_pos();

        if (contains(mouse_pos, screen_pos, screen_pos + screen_dim))
        {
            if (hovered != nullptr)
                hovered(this);

            if (pressed != nullptr && draw::is_mouse_pressed())
            {
                pressed(this);
            }

            if (draw::is_mouse_down() && draw::is_mouse_moving())
            {
                if (dragged != nullptr)
                    dragged(this);
            }

            return true;
        }

        return false;
    }*/
    void on_frame()
    {
        draw::begin_frame();

        /*
        const float zoom_fac = 0.1f;
        view_scale *= 1 + zoom_fac * draw::get_mouse_scroll();

        if (draw::is_mouse_down(draw::MOUSE_MIDDLE) && draw::is_mouse_moving())
        {
            head_element.m_pos += draw::get_mouse_delta();
        }*/

        // Processes mouse events for all mouse listeners.
        Mouse::process_mouse_events();

        

        //tow_Mouse::update();

        head_element.draw();

        draw::end_frame();
    }

    void set_world_center(const vec& pos)
    {
        head_element.set_pos(pos);
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

        float scale = options[WORLD] ? view_scale : 1.0f;
        draw::set_text_scale(scale * text_scale);
        draw::scissor(pos(), size());

        vec cursor = pos();
        for (const std::string& str : lines)
        {
            draw::draw_text(cursor, str);
            cursor.y += draw::get_text_size("").y;
        }

        if (child_storage.size())
        {
            if (options[OCCLUDE_CHILDREN])
            {
                draw::scissor(pos(), size());
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

        int max_lines = std::ceilf(m_size.y / (draw::get_text_size("").y*text_scale));
        std::string line = "";
        float line_width = 0;
        int token_start = 0;

        for (int k = 0; k < text.length() && lines.size() < max_lines; k++)
        {
            if (text[k] == ' ')
            {
                std::string token = text.substr(token_start, k + 1 - token_start);
                float token_width = draw::get_text_size(token).x * text_scale;
                if (line_width + token_width < m_size.x)
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
            if (line_width + draw::get_text_size(final_token).x < m_size.x)
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

        float scale = options[WORLD] ? view_scale : 1.0f;

        vec spos = pos();

        // All the shader access should be setters.
        draw::shape_corner(scale * corner_radius);

        draw::shape_color(fill.top, fill.bottom);

        draw::draw_rect(pos(), m_size * scale);

        // Draw outline
        if (outline_thickness > 0)
        {
            if (outline_thickness * scale < 1 && options[WORLD] || outline_thickness < 1)
                draw::shape_color(color(glm::vec3(outline.top), outline_thickness * 0.8), color(glm::vec3(outline.bottom), outline_thickness * 0.8));

            else
                draw::shape_color(outline.top, outline.bottom);

            draw::draw_mask(false);
            draw::apply_mask(true);

            if (corner_radius > 0)
            {
                draw::shape_corner(scale * (corner_radius + outline_thickness));
            }

            // We should set the alpha to scale
            // so that outlines fade away as they become thinner than a pixel.

            draw::draw_rect(pos() - outline_thickness * scale, size() + scale * outline_thickness * 2.0f * glm::sign(size()));
        } // End draw outline

        if (child_storage.size())
        {
            if (options[OCCLUDE_CHILDREN])
            {
                draw::scissor(pos(), size());
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

    std::string TextBox::get_text()
    {
        return text;
    }
}

