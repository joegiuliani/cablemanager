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
        gradient& operator=(const gradient& other)
        {
            top = other.top;
            bottom = other.bottom;
        }
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

        Element* parent = nullptr;

        //virtual void remove() = 0;

        Element();
        Element(const Element& elem);

        template<typename T>
        T& add_child()
        {
            child_storage.push_back(std::make_unique<T>(T()));
            child_storage.back()->parent = this;
            return *((T*)(child_storage.back().get()));
        }

        template<typename T>
        bool remove_child(const T& ch)
        {
            Element* ch_ptr = (Element*)&ch;
            auto& store = child_storage;
            auto it = std::find_if(store.begin(), store.end(), [&](ElementPtr& ep) {return ep.get() == ch_ptr; });
            if (it != store.end())
            {
                store.erase(it);

                return false;
            }

            return false;
        }

        virtual void copy(const Element& e);

        void clip_children(bool flag);
        void on_press(CallbackFn cf);
        void on_drag(CallbackFn cf);
        //void on_hover(CallbackFn cf);
        //void on_release(CallbackFn cf);
        //void on_enter(CallbackFn cf);
        //void on_exit(CallbackFn cf);

        bool process_mouse_events();

        const vec& size();
        void set_size(const vec& v);
        //void send_to_front();

        virtual void draw();
        CallbackFn pressed = nullptr, hovered = nullptr, dragged = nullptr, released = nullptr, entered = nullptr, exited = nullptr;

    protected:
        vec dim;
        std::vector<ElementPtr> child_storage;
        Element(const Element& elem);
        Element& operator=(const Element& elem);
    };

    inline qgl::Element head_element;

    class Curve : public Element
    {
    public:
        Curve();
        Curve(const Curve& curve);
        std::vector<vec> points;
        virtual void draw();
        virtual void copy(const Curve& e);

    };

    class TextBox : public Element
    {
    public:
        virtual void draw();
        TextBox();
        TextBox(const TextBox& tb);
        void set_size(const vec& s);
        void set_text(const std::string& str);
        void set_text_scale(float s);
        virtual void copy(const TextBox& e);
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
        Shape(const Shape& sh);
        float corner_radius = 5;
        virtual void draw();
        virtual void copy(const Shape& e);

    };

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

