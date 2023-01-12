#pragma once

#include <vector>
#include <glm/common.hpp>
#include <memory>
#include <functional>
#include <stack>
#include <set>
#include "types.h"

// Certain styling features only make sense for rects or rects and curves so that should be its own class
// And Ishould utilize multiple inheritance otherwise were wasting space.

#define QGL_DEBUG
namespace cm
{
    namespace qgl
    {
        typedef char Flag;
        typedef unsigned int FlagIndex;

        struct gradient
        {
            color top, bottom;
            gradient& operator=(const gradient& other)
            {
                top = other.top;
                bottom = other.bottom;

                return *this;
            }
        };

        inline float view_scale = 1;

        class Mouse
        {

        private:
            static inline bool remove_flag;

            class CallbackList : public std::vector<CallbackPtr>
            {
            public:
                void add(CallbackPtr p)
                {
                    push_back(p);
                }
                void remove(CallbackPtr p)
                {

                    auto it = std::find(begin(), end(), p);
                    if (it != end()) {
                        erase(it);
                    }
                }
            };

            struct cb_pair_comp
            {
                bool operator()(const std::pair<CallbackList*, CallbackPtr>& a,
                    const std::pair<CallbackList*, CallbackPtr>& b) const
                {
                    return std::less<CallbackPtr>()(a.second, b.second);
                }
            };

            static inline std::set<std::pair<CallbackList*, CallbackPtr>, cb_pair_comp> callbacks_to_remove;

        public:

            static inline vec pos;
            static inline vec delta;
            static inline int scroll_dir;
            static inline CallbackList move, press, release, scroll;

            static bool is_down(int button = 0);
            static void remove_callback(CallbackList& l, CallbackPtr cb);
            static void process_mouse_events();

        };

        class Keyboard
        {
        public:
            static void process_events();
            static bool matches(const std::set<int>& keys);
            static void add_callback(CallbackPtr ptr);
            static void remove_callback(CallbackPtr ptr);
        private:
            static inline std::set<CallbackPtr> callbacks_to_remove;
            static inline std::set<CallbackPtr> callbacks;

        };

        class IElement
        {
        public:

            std::vector<IElement*> child_storage;
            virtual void draw() = 0;
            virtual vec pos() = 0;
            virtual void set_pos(const vec& v) = 0;
            void set_enabled(bool flag);
            IElement();

        protected:
            IElement& operator=(const IElement& elem);
            IElement(const IElement& elem);
            vec m_pos;
            bool enabled = true;
        };

        class RootElement : public IElement
        {
        public:
            virtual vec pos();
            virtual void set_pos(const vec& v);
            virtual void draw();
        };

        class Element : public IElement
        {
        public:
            static const FlagIndex WORLD = 0;
            static const FlagIndex OCCLUDE_CHILDREN = 1;
            Flag options[2] = { false, false };

            gradient fill{ color(1), color(1) };
            gradient outline;
            gradient shadow;

            float outline_thickness;
            float shadow_sharpness;
            vec shadow_offset;

            Element();
            Element(IElement* t_parent_ptr);
            Element(const Element& elem);
            ~Element();

            void move_to_parent(IElement* t_parent_ptr);
            virtual void draw();
            void clip_children(bool flag);

            virtual Element& operator=(const Element& elem);

            // These methods deal with pixel space
            virtual vec pos();
            vec size();
            void set_size(const vec& v);
            virtual void set_pos(const vec& v);

            Element* parent();

            //void send_to_front();        
        protected:
            vec m_size;
            IElement* parent_ptr = nullptr;

        };

        class Curve : public Element
        {
        public:
            std::vector<vec> points;
            virtual void draw();
            Curve();
            Curve(IElement* parent);
            Curve& operator=(const Curve& elem);
            Curve(const Curve& curve);
        };

        class TextBox : public Element
        {
        public:
            virtual void draw();
            void set_size(const vec& s);
            void set_text(std::string str);
            void set_text_scale(float s);
            std::string get_text();
            TextBox();
            TextBox(IElement* parent);
            TextBox& operator=(const TextBox& elem);
            TextBox(const TextBox& textbox);

        protected:
            std::string text = "";
            std::vector<std::string> lines;
            float text_scale = 1;
            void calculate_wrap();
        };

        class Shape : public Element
        {
        public:
            float corner_radius = 5;
            virtual void draw();
            Shape();
            Shape(IElement* parent);
            Shape& operator=(const Shape& elem);
            Shape(const Shape& shape);
        };

        vec screen_to_world_scale(const vec& v);
        vec world_to_screen_scale(const vec& v);
        vec screen_to_world_projection(const vec& v);
        vec world_to_screen_projection(const vec& v);

        void set_world_center(const vec& m_pos);
        vec world_center();

        void init();
        bool is_running();
        void on_frame();
        void terminate();
    }

}

