#pragma once

#include <vector>
#include <glm/common.hpp>
#include <glm/vec2.hpp>
#include <glm/vec4.hpp>


// Certain styling features only make sense for rects or rects and curves so that should be its own class
// And Ishould utilize multiple inheritance otherwise were wasting space.



namespace qgl
{
    typedef char Flag;
    typedef unsigned int FlagIndex;

    typedef glm::vec4 color;
    struct gradient
    {
        color top, bottom;
    };
    typedef glm::vec2 vec;

    class Element;
    class ChildElement;

    typedef std::unique_ptr<Element> ElementPtr;

    typedef std::function<void(Element*)> CallbackFn;
    constexpr void* NO_FUNCTION = nullptr;

    class Element
    {
    public:
        static const FlagIndex WORLD = 0;
        static const FlagIndex OCCLUDE_CHILDREN = 1;
        static const FlagIndex MOUSE_LISTENER = 2;
        Flag options[3] = { false };

        vec pos;

        gradient fill{ color(1), color(1) };
        gradient outline;
        gradient shadow;

        float outline_thickness;
        float shadow_sharpness;
        vec shadow_offset;

        std::vector<ElementPtr> child_storage;
        Element* parent = nullptr;

        //virtual void remove() = 0;

        Element();

        void clip_children(bool flag);
        void on_press(CallbackFn cf);
        void on_drag(CallbackFn cf);
        //void on_hover(CallbackFn cf);
        //void on_release(CallbackFn cf);
        //void on_enter(CallbackFn cf);
        //void on_exit(CallbackFn cf);

        const vec& size();
        void set_size(const vec& v);
        //void send_to_front();
        void remove();

        virtual void draw();
        CallbackFn pressed = nullptr, hovered = nullptr, dragged = nullptr, released = nullptr, entered = nullptr, exited = nullptr;

    protected:
        vec dim;
    };

    inline Element god_element;

    //Element& new_Element();
    //Element& new_Element(Element* parent_ptr);

    class Curve : public Element
    {
    public:
        Curve();
        std::vector<vec> points;
        virtual void draw();
    };

    class TextBox : public Element
    {
    public:
        virtual void draw();
        TextBox();
        void set_size(const vec& s);
        void set_text(const std::string& str);
        void set_text_scale(float s);
    protected:
        std::string text;
        std::vector<std::string> lines;
        float text_scale = 1;

        void calculate_wrap();
    };

    class Shape : public Element
    {
    public:
        Shape();
        float corner_radius = 5;
        virtual void draw();
    };
    
    template<typename T>
    T& new_Element(Element* parent_ptr)
    {
        parent_ptr->child_storage.push_back(std::make_unique<T>());

        T* new_ptr = (T*)(parent_ptr->child_storage.back().get());
        new_ptr->parent = parent_ptr;
        return *new_ptr;
    }
    template<typename T>
    T& new_Element()
    {
        return new_Element<T>(&god_element);
    }

    /*
    Shape& new_Shape(Element* parent_ptr);
    Shape& new_Shape();

    Curve& new_Curve();
    Curve& new_Curve(Element* parent);

    TextBox& new_TextBox();
    TextBox& new_TextBox(Element* parent);*/

    class TowMouse
    {
    public:
        void begin(Element* t_element, std::function<bool()> t_end_condition_fn);

        void reset();

        void update();

    private:
        bool active = false;
        Element* element_ptr = nullptr;
        vec delta = vec(0);
        std::function<bool()> end_condition_fn = nullptr;
    };

    void follow_mouse(Element* element, std::function<bool()> stop_condition_fn = nullptr);

    void init();
    bool is_running();
    void on_frame();
    void terminate();
}
// everything can draw over everything, except well scissor the scene view.

