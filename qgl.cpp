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
    float view_scale = 1;
    TowMouse tow_mouse;

    bool has_parent(const Element& element)
    {
        return element.parent != nullptr;
    }

    vec screen_to_world(const vec& v)
    {
        return (v - draw::viewport_size() * 0.5f) / view_scale + head_element.pos;
    }

    vec world_to_screen(const vec& v)
    {
        return view_scale * (v - head_element.pos) + draw::viewport_size() * 0.5f;
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

        vec ret = element_ptr->options[Element::WORLD] ? element_ptr->pos * view_scale : element_ptr->pos;

        // Offsets element by parents' positions
        // Child positions are always relative.
        // Has parent should always be true unless element_ptr points to the god element
        if (has_parent(*element_ptr))
        {
            Element* parent_ptr = element_ptr->parent;

            while (parent_ptr != nullptr)
            {
                ret += parent_ptr->options[Element::WORLD] ? view_scale * parent_ptr->pos : parent_ptr->pos;
                parent_ptr = parent_ptr->parent;
            }
        }

        return ret;
    }

    bool contains(const vec& value, const vec& min, const vec& max)
    {
        return value.x >= min.x && value.y >= min.y && value.x <= max.x && value.y <= max.y;
    }

    void Element::copy(const Element& elem)
    {
        for (int k = 0; k < 3; k++)
            options[k] = elem.options[k];

        pos = elem.pos;
        fill = elem.fill;
        outline = elem.outline;
        shadow = elem.shadow;
        outline_thick
    }



    Element::Element()
    {
        // Because I want people to be able to put 

        // Shape shape
        // instead of
        // Shape& shape = head.add_child<Shape>();
        

        // So i propose that element adds a ptr of itself to god element's child storage.
        // and when destructed, we remove the pointer from the storage.
        // that way.

        // This would only work for non-contained Elements whos parent is the head element.
        // 


        // THIS WHOLE THING IS IMPOSSIBLE


        // I want elements. 

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

    void process_mouse_events(Element* element_ptr)
    {
        //Element& element = *element_ptr;
        element_ptr->process_mouse_events();
        
    }
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
        vec screen_dim = dim;
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
    }
    void on_frame()
    {
        draw::begin_frame();

        const float zoom_fac = 0.1f;
        view_scale *= 1 + zoom_fac * draw::get_mouse_scroll();

        if (draw::is_mouse_down(draw::MOUSE_MIDDLE) && draw::is_mouse_moving())
        {
            head_element.pos += draw::get_mouse_delta();
        }

        // Processes mouse events for all mouse listeners.
        process_mouse_events(&head_element);

        tow_mouse.update();

        head_element.draw();

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
        draw::shape_corner(scale * corner_radius);

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

            if (corner_radius > 0)
            {
                draw::shape_corner(scale * (corner_radius + outline_thickness));
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

