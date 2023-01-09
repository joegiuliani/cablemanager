#include <iostream>
#include <vector>
#include <glm/common.hpp>
#include <glm/vec2.hpp>
#include <glm/vec4.hpp>
#include "draw.h"
#include <glfw/glfw3.h>
#include "qgl.h"
#include <set>

namespace qgl
{
     
    RootElement root_elem;

    void init()
    {
        draw::init(640, 480);
    }

    bool is_running()
    {
        return draw::is_running();
    }

    void on_frame()
    {
        draw::begin_frame();

        Keyboard::process_events();
        Mouse::process_mouse_events();

        root_elem.draw();
       
        draw::end_frame();
    }

    void terminate()
    {
        draw::terminate();
    }

    void set_world_center(const vec& pos)
    {
        root_elem.set_pos(pos);
    }

    vec world_center()
    {
        return root_elem.pos();
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
        return screen_to_world_scale(v - draw::viewport_size() * 0.5f) + root_elem.pos();
    }

    vec world_to_screen_projection(const vec& v)
    {
        return world_to_screen_scale(v-root_elem.pos()) + draw::viewport_size() * 0.5f;
    }

    bool contains(const vec& value, const vec& min, const vec& max)
    {
        return value.x >= min.x && value.y >= min.y && value.x <= max.x && value.y <= max.y;
    }

    RootElement::RootElement()
    {
    }

    RootElement::~RootElement()
    {
    }

    void IElement::draw()
    {
        if (child_storage.size())
        {
            for (IElement* cep : child_storage)
            {
                cep->draw();
            }
        }
    }

    vec IElement::pos()
    {
        return m_pos;
    }

    void IElement::set_pos(const vec& v)
    {
        m_pos = v;
    }

    void Element::clip_children(bool flag)
    {
        options[OCCLUDE_CHILDREN] = flag;
    }

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

        IElement* curr_parent_ptr = parent_ptr;
        while (curr_parent_ptr != &root_elem)
        {
            Element& curr_parent = *static_cast<Element*>(curr_parent_ptr);
            ret += curr_parent.options[WORLD] ? world_to_screen_scale(curr_parent.m_pos) : curr_parent.m_pos;
            parent_ptr = curr_parent.parent_ptr;
        }

        ret += root_elem.pos();

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

            for (IElement* cep : child_storage)
            {
                cep->draw();
            }

            draw::stop_scissor();
        }
    }

    Element::Element()
    {
        root_elem.child_storage.push_back(this);
    }

    Element::Element(const Element& elem)
    {
        operator=(elem);
    }

    Element& Element::operator=(const Element& elem)
    {
        elem.parent_ptr->child_storage.push_back(this);
        parent_ptr = elem.parent_ptr;
        std::memcpy(options, elem.options, 2);
        fill = elem.fill;
        outline = elem.outline;
        shadow = elem.shadow;
        outline_thickness = elem.outline_thickness;
        shadow_sharpness = elem.shadow_sharpness;
        shadow_offset = elem.shadow_offset;
        m_size = elem.m_size;
        m_pos = elem.m_pos;
    }

    Element::Element(IElement* t_parent_ptr)
    {
        parent_ptr = t_parent_ptr;
        parent_ptr->child_storage.push_back(this);
    }

    void Element::move_to_parent(IElement* t_parent_ptr)
    {
        parent_ptr->child_storage.erase(std::remove(child_storage.begin(), child_storage.end(), this), child_storage.end());
        t_parent_ptr->child_storage.push_back(this);
        parent_ptr = t_parent_ptr;
    }

    Element::~Element()
    {
        parent_ptr->child_storage.erase(std::remove(parent_ptr->child_storage.begin(), parent_ptr->child_storage.end(), this), parent_ptr->child_storage.end());
        for (auto* child_ptr : child_storage)
        {
            static_cast<Element*>(child_ptr)->move_to_parent(&root_elem);
        }
    }

    Element* Element::parent()
    {
        if (parent_ptr == static_cast<IElement*>(&root_elem))
        {
            return nullptr; // For now I don't want the end user having access to the root element.
        }

        return static_cast<Element*>(parent_ptr);
    }

    Curve::Curve():Element()
    {
    }
    Curve::Curve(IElement* parent):Element(parent)
    {
    }
    Curve::Curve(const Curve& curve)
    {
        operator=(curve);
    }
    Curve& Curve::operator=(const Curve& curve)
    {
        Element::operator=(curve);
        points = curve.points;
        return *this;
    }
    void Curve::draw()
    {
        draw::draw_curve(points);
    }

    Shape::Shape():Element()
    {
    }
    Shape::Shape(IElement* parent) :Element(parent)
    {
    }
    Shape::Shape(const Shape& shape)
    {
        operator=(shape);
    }
    Shape& Shape::operator=(const Shape& shape)
    {
        Element::operator=(shape);
        corner_radius = shape.corner_radius;
        return *this;
    }

    TextBox::TextBox() :Element()
    {
    }
    TextBox::TextBox(IElement* parent) :Element(parent)
    {
    }
    TextBox::TextBox(const TextBox& textbox)
    {
        operator=(textbox);
    }

    TextBox& TextBox::operator=(const TextBox& textbox)
    {   
        Element::operator=(textbox);

        text = textbox.text;
        lines = textbox.lines;
        text_scale = textbox.text_scale;

        return *this;
    }

    void TextBox::set_text(std::string str)
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

        Element::draw();
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

        int max_lines = std::ceilf(m_size.y / (draw::get_text_size("").y * text_scale));
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

    std::string TextBox::get_text()
    {
        return text;
    }

    Shape::Shape(const Shape& shape)
    {
    }

    void Shape::draw()
    {
        draw::apply_mask(false);
        draw::draw_mask(true);
        draw::use_texture(false);

        float scale = options[WORLD] ? view_scale : 1.0f;

        vec spos = pos();

        draw::shape_corner(scale * corner_radius);

        draw::shape_color(fill.top, fill.bottom);

        draw::draw_rect(pos(), m_size * scale);

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

            draw::draw_rect(pos() - outline_thickness * scale, size() + scale * outline_thickness * 2.0f);
        }

        Element::draw();
    }

    bool Mouse::is_down(int button)
    {
        return draw::is_mouse_down(button);
    }

    void Mouse::remove_callback(CallbackList& cbl, CallbackPtr cb)
    {
        callbacks_to_remove.insert(std::pair(&cbl, cb));
    }

    void Mouse::process_mouse_events()
    {
        pos = draw::get_mouse_pos();
        scroll_dir = draw::get_mouse_scroll();
        delta = draw::get_mouse_delta();

        auto call_list = [&](CallbackList cbl)
        {
            for (CallbackPtr cb : cbl)
            {
                cb();
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
            cbp.first->remove(cbp.second);
        }

        callbacks_to_remove.clear();
    }

    bool Keyboard::matches(const std::set<int>& keys)
    {
        // Check if the sets have different sizes
        if (draw::get_down_keys().size() != keys.size()) {
            return false;
        }

        // Iterate through the elements in set1 and check if they are all in set2
        for (int x : keys) {
            if (draw::get_down_keys().count(x) == 0) {
                return false;
            }
        }

        // If the loop completes, all the elements in set1 were found in set2
        return true;
        
    }

    void Keyboard::process_events()
    {
        if (draw::key_change_flag)
        {
            for (auto& cbp : callbacks)
                cbp();
        }
        for (auto& cbp : callbacks_to_remove)
        {
            auto it = std::find(callbacks.begin(), callbacks.end(), cbp);
            if (it != callbacks.end()) {
                callbacks.erase(it);
            }
        }

        callbacks_to_remove.clear();
    }

    void Keyboard::add_callback(CallbackPtr ptr)
    {
        callbacks.insert(ptr);
    }

    void Keyboard::remove_callback(CallbackPtr ptr)
    {
        callbacks_to_remove.insert(ptr);
    }
}

