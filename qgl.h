#pragma once

#include <vector>
#include <glm/common.hpp>
#include <glm/vec2.hpp>
#include <glm/vec4.hpp>
#include <memory>
#include <functional>
#include <stack>
#include <set>

// Certain styling features only make sense for rects or rects and curves so that should be its own class
// And Ishould utilize multiple inheritance otherwise were wasting space.


#define QGL_DEBUG

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

    inline float view_scale = 1;

    class Mouse
    {
    public:
        typedef void (*MouseCallbackPtr)();

        static inline vec pos;
        static inline vec delta;
        static inline int scroll_dir;
        static inline std::list<MouseCallbackPtr> move, press, release, scroll;

        static bool is_down(int button = 0);
        static void remove_this_callback();
        static void remove_callback(std::list<MouseCallbackPtr>& cbl, MouseCallbackPtr cb);
        static void process_mouse_events();

    private:
        static inline bool remove_flag;

        struct cb_pair_comp
        {
            bool operator()(const std::pair<std::list<MouseCallbackPtr>*, MouseCallbackPtr>& a,
                const std::pair<std::list<MouseCallbackPtr>*, MouseCallbackPtr>& b) const
            {
                return std::less<MouseCallbackPtr>()(a.second, b.second);
            }
        };
        static inline std::set<std::pair<std::list<MouseCallbackPtr>*, MouseCallbackPtr>, cb_pair_comp> callbacks_to_remove;
    };

    class Element
    {
    public:
        static const FlagIndex WORLD = 0;
        static const FlagIndex OCCLUDE_CHILDREN = 1;
        static const FlagIndex MOUSE_LISTENER = 2;
        Flag options[3] = { false };

        gradient fill{ color(1), color(1) };
        gradient outline;
        gradient shadow;

        float outline_thickness;
        float shadow_sharpness;
        vec shadow_offset;

        Element* parent = nullptr;

        CallbackFn pressed = nullptr, hovered = nullptr, dragged = nullptr, released = nullptr, entered = nullptr, exited = nullptr;

        //virtual void remove() = 0;

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

        void clip_children(bool flag);

        // All deal with pixel space
        vec pos();
        vec size();
        void set_size(const vec& v);
        void set_pos(const vec& v);
        //void send_to_front();

        virtual void draw();

    protected:
        vec m_size;
        std::vector<ElementPtr> child_storage;
        Element& operator=(const Element& elem);
        vec m_pos;

    protected:
        Element();
    };

    extern qgl::Element& head_element;

    class Curve : public Element
    {
    public:
        Curve();
        Curve(const Curve& curve);
        std::vector<vec> points;
        virtual void draw();

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
        std::string get_text();
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

    };

    vec screen_to_world_scale(const vec& v);
    vec world_to_screen_scale(const vec& v);
    vec screen_to_world_projection(const vec& v);
    vec world_to_screen_projection(const vec& v);

    void init();
    bool is_running();
    void on_frame();
    void terminate();
    void set_world_center(const vec& m_pos);
    vec world_center();

}

