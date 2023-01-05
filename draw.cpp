#include "draw.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <memory>
#include <shader.h>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <stack>
#include <map>
#include <ft2build.h>
#include <set>
#include FT_FREETYPE_H

template <typename T>
struct state_tracker
{
    T previous;
    T current;

    void new_state(const T& state)
    {
        previous = current;
        current = state;
}
};

struct Glyph {
    unsigned int texture_id; // ID handle of the glyph texture
    glm::vec2   m_size;      // Size of glyph
    glm::vec2   bearing;   // Offset from baseline to left/top of glyph
    float advance;   // Horizontal offset to advance to next glyph
};

Glyph glyphs[128];
GLuint glyph_vao, glyph_vbo;

void window_size_callback(GLFWwindow* window, int width, int height);
void glfw_error_callback(int error, const char* description);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
glm::vec2 quadratic_bezier(float t, const glm::vec2& a, const glm::vec2& b, const glm::vec2& c);
float get_text_width(const std::string& text);
float get_text_height();
constexpr char SHAPE = 0, TEXT = 1;

GLFWwindow* window = nullptr;
glm::vec2 viewport_dim(640, 480);
double last_time = glfwGetTime();
double delta_time = 0;
state_tracker<glm::vec2> mouse_pos;
glm::vec2 mouse_delta;
constexpr int NUM_MOUSE_BUTTONS = 3;
state_tracker<int> mouse_buttons[NUM_MOUSE_BUTTONS];
state_tracker<int> mouse_scroll;
unsigned int scroll_receive_frame = 0;

unsigned int frame_counter;

std::set<int> down_keys;

Shader shape_shader("shape", "shape.vert", "rect.frag");
GLuint shape_vao, shape_vbo, shape_ebo;

glm::vec2 normal(const glm::vec2& v);
std::vector<glm::vec2> shape_verts(4);

std::vector<GLubyte> shape_elems
{
    2, 1, 0, 2, 0, 3
};

void set_draw_texture(unsigned int texture_id);
constexpr float GLOBAL_FONT_SCALE = 1.0f;
constexpr float advance_fac = 1.0f / float(1 << 6);
float font_resolution = 128;
float font_import_scale = 1.0f / font_resolution;// / font_resolution * GLOBAL_FONT_SCALE;
float font_offset = 0;
float font_height = 0;
float font_line_distance = 1;

float text_scale = 24;

float line_radius = 2.5;
void draw_shape();
void update_shape_verts();

// Eventually we will create a large buffer to store draw data.
// Which means we may be able to remove the imperative commands once and for all!
// If we store drawing data in a buffer, how is that different from an array of structs which wrap that data?
// We can have a link!
// It is implicitly object oriented!
// The issue with this is updating the buffer when there is a change to the object. Would we rebuffer the object each time? No. We would store the changes to the drawing data then sub buffer at the end of the frame. But multiple sub buffers?
// Is it faster to do a single buffer or multiple sub buffers?
// 
// we will create a vector containing the free indices of the buffer

namespace draw
{
    int init(int width, int height)
    {
        viewport_dim = glm::vec2(width, height);

        // Setup window
        glfwSetErrorCallback(glfw_error_callback);
        if (!glfwInit())
            return 1;

        const char* glsl_version = "#version 450 core";
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);  // 3.2+ only
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // 3.0+ only
        glfwWindowHint(GLFW_SAMPLES, 16);

        // Create window with graphics context
        window = glfwCreateWindow(viewport_dim.x, viewport_dim.y, "CableManager", NULL, NULL);
        if (window == NULL)
            return 1;
        glfwMakeContextCurrent(window);
        glfwWindowHint(GLFW_DECORATED, false);
        if (glfwRawMouseMotionSupported())
            glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
        //glfwSwapInterval(1); // Enable vsync
        glfwSetWindowSizeCallback(window, window_size_callback);
        glfwSetScrollCallback(window, scroll_callback);
        glfwSetKeyCallback(window, key_callback);
        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
        {
            std::cout << "Warning: Failed to load GL from GLAD." << "\n";
        }

        // Load fonts
        {
            FT_Library ft;

            if (FT_Init_FreeType(&ft))
            {
                std::cout << "Failed to initalize FreeType" << std::endl;
                return -1;
            }

            // find path to font
            std::string font_name = "C:/Windows/Fonts/segoeui.ttf";

            FT_Face face;
            if (FT_New_Face(ft, font_name.c_str(), 0, &face)) {
                std::cout << "Failed to load font" << '\n';
                return -1;
            }
            else
            {
                // set size to load glyphs as
                FT_Set_Pixel_Sizes(face, 0, font_resolution);

                glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

                // load first 128 characters of ASCII set
                for (unsigned char c = 0; c < 128; c++)
                {
                    // Load character glyph 
                    if (FT_Load_Char(face, c, FT_LOAD_RENDER))
                    {
                        std::cout << "Failed to load glyph for \'" << c << '\'\n';
                        continue;
                    }

                    // generate texture
                    unsigned int font_texture;
                    glGenTextures(1, &font_texture);
                    glBindTexture(GL_TEXTURE_2D, font_texture);

                    for (int x = 0; x < face->glyph->bitmap.width * face->glyph->bitmap.rows; x++)
                    {
                        if ((float)face->glyph->bitmap.buffer[x] < 0.2f)
                        {
                            face->glyph->bitmap.buffer[x] = 0;
                        }
                    } // Pre culling alphas on load

                    glTexImage2D(
                        GL_TEXTURE_2D,
                        0,
                        GL_RED,
                        face->glyph->bitmap.width,
                        face->glyph->bitmap.rows,
                        0,
                        GL_RED,
                        GL_UNSIGNED_BYTE,
                        face->glyph->bitmap.buffer
                    );

                    // set texture options
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                    // now store character for later use
                    Glyph glyph = {
                        font_texture,
                        glm::vec2(face->glyph->bitmap.width, face->glyph->bitmap.rows) * font_import_scale,
                        glm::vec2(face->glyph->bitmap_left, face->glyph->bitmap_top) * font_import_scale,
                        face->glyph->advance.x * advance_fac * font_import_scale
                    };
                    glyphs[c] = glyph;
                }

                float max_height = 0;
                for (const auto& g : glyphs)
                {
                    if (g.m_size.y > max_height)
                    {
                        max_height = g.bearing.y;
                    }
                }

                font_height = max_height;
                font_offset = max_height * 0.5f;

                glBindTexture(GL_TEXTURE_2D, 0);
            }
            FT_Done_Face(face);
            FT_Done_FreeType(ft);

            glGenVertexArrays(1, &glyph_vao);
            glGenBuffers(1, &glyph_vbo);
            glBindVertexArray(glyph_vao);
            glBindBuffer(GL_ARRAY_BUFFER, glyph_vbo);
            glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
            glBindVertexArray(0);
        }

        // For ellipses, rects, etc. Not for curves or text
        {
            glGenVertexArrays(1, &shape_vao);
            glGenBuffers(1, &shape_vbo);
            glGenBuffers(1, &shape_ebo);

            glBindVertexArray(shape_vao);

            // Position
            glBindBuffer(GL_ARRAY_BUFFER, shape_vbo);
            glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 2 * shape_verts.size(), &shape_verts[0], GL_DYNAMIC_DRAW);
            glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 2, (void*)0);
            glEnableVertexAttribArray(0);

            // UV
            {
                float shape_uvs[8]
                {
                    0,0,
                    1,0,
                    1,1,
                    0,1
                };
                unsigned int shape_uv_vbo;
                glGenBuffers(1, &shape_uv_vbo);
                glBindBuffer(GL_ARRAY_BUFFER, shape_uv_vbo);
                glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 8, shape_uvs, GL_STATIC_DRAW);
                glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 2, (void*)0);
                glEnableVertexAttribArray(1);
            }

            // Element
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, shape_ebo);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLubyte)* shape_elems.size(), &shape_elems.front(), GL_STATIC_DRAW);
        }

        shape_shader.init();

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        //glEnable(GL_DEPTH_TEST);
        glDisable(GL_DEPTH_TEST);
        glDepthFunc(GL_LEQUAL);
        glEnable(GL_MULTISAMPLE);
        glEnable(GL_STENCIL_TEST);
        glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
        window_size_callback(window, viewport_dim.x, viewport_dim.y);

        last_time = glfwGetTime();
    }

    bool is_running()
    {
        return !glfwWindowShouldClose(window);
    }

    void begin_frame()
    {
        glfwPollEvents();

        delta_time = glfwGetTime() - last_time;
        last_time = glfwGetTime();

        for (int k = 0; k < NUM_MOUSE_BUTTONS; k++)
        {
            mouse_buttons[k].new_state(glfwGetMouseButton(window, k));
        }

        double mx, my;
        glfwGetCursorPos(window, &mx, &my);
        mouse_pos.new_state(glm::vec2(mx, my));
        mouse_delta = mouse_pos.current - mouse_pos.previous;

        if (scroll_receive_frame != frame_counter)
        {
            mouse_scroll.new_state(0);
        }
        apply_mask(false);
        draw_mask(false);
        shape_color(glm::vec4(1));
    }

    void end_frame()
    {
        glfwSwapBuffers(window);
        glClearColor(0.15f, 0.15f, 0.155f, 1.0f);

        glStencilMask(~0);
        glDisable(GL_SCISSOR_TEST);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
        frame_counter++;
    }

    void terminate()
    {
        glfwTerminate();
    }

    // shape shader is going to change. it will no longer accept m_pos and dim. These will be prepared in the vertex buffer.
    // lines will be drawn using the shape shader.

    // we may want to set up an indirect draw function
    // we may also want to create a view mode where we cant zoom or move

    // once we draw a lot of curves, this is going to become very inefficient
    // well need to reserve buffer space and update it each frame

    void draw_rect(const glm::vec2& m_pos, const glm::vec2& size)
    {
        // Until we can figure out a better system for drawing corners well just use uniforms.
        shape_shader.setVec2("pos", m_pos);
        shape_shader.setVec2("dim", size);

        shape_verts[0] = m_pos;
        shape_verts[1] = glm::vec2(m_pos.x + size.x, m_pos.y);
        shape_verts[2] = m_pos + size;
        shape_verts[3] = glm::vec2(m_pos.x, m_pos.y + size.y);
        update_shape_verts();

        draw_shape();
    }

    void set_text_scale(float s)
    {
        text_scale = s;
    }

    void draw_text(const glm::vec2& m_pos, const std::string& text)
    {
        use_texture(true);
        shape_shader.setBool("disable_corners", true);
        glActiveTexture(GL_TEXTURE0);

        float curs = 0;

        Glyph& H = glyphs['H'];

        for (const char& c : text)
        {
            Glyph ch = glyphs[c];
            set_draw_texture(ch.texture_id);

            float offset = (H.bearing.y - ch.bearing.y) * text_scale;
            draw_rect(glm::vec2(m_pos.x + ch.bearing.x * text_scale + curs, m_pos.y + offset), glm::vec2(ch.m_size.x, ch.m_size.y) * text_scale);

            curs += ch.advance * text_scale;
        }
        
        shape_shader.setBool("disable_corners", false);
        use_texture(false);
        set_draw_texture(0);
    }

    // Renders a bunch of trapezoids 
    void draw_curve(const std::vector<glm::vec2>& verts)
    {
        if (verts.size() < 2)
        {
            std::cout << "Error: vector passed to draw::draw_curve() has length < 2\n";
            return;
        }

        shape_shader.setBool("disable_corners", true);

        const float ray_coeff = line_radius * 0.5f;

        glm::vec2 line_normal = normal(verts[1] - verts[0]);
        glm::vec2 previous_normal = line_normal;
        glm::vec2 next_normal;

        if (verts.size() > 2)
            next_normal = normal(verts[2] - verts[1]);
        else
            next_normal = line_normal;

        glm::vec2 start_ray = -line_normal * line_radius;
        glm::vec2 end_ray = -(line_normal + next_normal) * ray_coeff;

        shape_verts[0] = start_ray + verts[0];
        shape_verts[1] = end_ray + verts[1];
        shape_verts[2] = -end_ray + verts[1];
        shape_verts[3] = -start_ray + verts[0];

        // buffer
        update_shape_verts();

        draw_shape();

        if (verts.size() > 2)
        {
            int k = 1;
            while (k < verts.size() - 2)
            {
                // The end edge becomes the start edge on the next line segment
                shape_verts[0] = shape_verts[1];
                shape_verts[3] = shape_verts[2];

                line_normal = next_normal;
                next_normal = normal(verts[k + 2] - verts[k + 1]);

                end_ray = -(line_normal + next_normal) * ray_coeff;
                shape_verts[1] = end_ray + verts[k + 1];
                shape_verts[2] = -end_ray + verts[k + 1];

                // buffer
                update_shape_verts();
                draw_shape();

                k++;
            }
            shape_verts[0] = shape_verts[1];
            shape_verts[3] = shape_verts[2];

            end_ray = -next_normal * line_radius;
            shape_verts[1] = end_ray + verts[k + 1];
            shape_verts[2] = -end_ray + verts[k + 1];

            update_shape_verts();
            draw_shape();
        }

        shape_shader.setBool("disable_corners", false);
    }

    //void draw_ellipse();
    //void draw_line();
    void use_texture(bool flag)
    {
        shape_shader.setBool("use_texture", flag);
    }

    void scissor(const glm::vec2& m_pos, const glm::vec2& m_size)
    {
        glEnable(GL_SCISSOR_TEST);
        glScissor(m_pos.x, viewport_dim.y - m_pos.y - m_size.y, m_size.x, m_size.y);
    }

    void stop_scissor() { glDisable(GL_SCISSOR_TEST); }

    void draw_mask(bool flag)
    {
        // enable : disable writing to stencil buffer
        glStencilMask(flag ? 0xFF : 0x00);
    }

    void apply_mask(bool flag)
    {
        glStencilFunc(flag ? GL_NOTEQUAL : GL_ALWAYS, 1, 0xFF);
    }

    void shape_color(const glm::vec4& top, const glm::vec4& bottom)
    {
        shape_shader.setVec4("top_color", top);
        shape_shader.setVec4("bottom_color", bottom);
    }

    void shape_color(const glm::vec4& color)
    {
        shape_color(color, color);
    }

    void shape_corner(float size)
    {
        shape_shader.setFloat("corner_size", size);
    }

    double get_time()
    {
        return glfwGetTime();
    }

    double get_delta_time()
    {
        return delta_time;
    }

    bool is_mouse_up(int button)
    {
        return mouse_buttons[button].current == MOUSE_UP;
    }

    bool is_mouse_down(int button)
    {
        return mouse_buttons[button].current == MOUSE_DOWN;
    }

    bool is_mouse_pressed(int button)
    {
        return mouse_buttons[button].current == MOUSE_DOWN && mouse_buttons[button].previous == MOUSE_UP;
    }

    bool is_mouse_released(int button)
    {
        return mouse_buttons[button].current == MOUSE_UP && mouse_buttons[button].previous == MOUSE_DOWN;
    }

    bool is_mouse_moving()
    {
        return mouse_delta != glm::vec2(0);
    }

    glm::vec2 get_mouse_delta()
    {
        return mouse_delta;
    }

    glm::vec2 get_mouse_pos()
    {
        return mouse_pos.current;
    }

    int get_mouse_scroll()
    {
        return mouse_scroll.current;
    }

    const glm::vec2& viewport_size()
    {
        return viewport_dim;
    }

    // For enter/exit detection
    glm::vec2 get_last_mouse_pos()
    {
        return mouse_pos.previous;
    }

    // setters for resolution, thickness, etc 

    glm::vec2 get_text_size(const std::string& str)
    {
        return glm::vec2(get_text_width(str), get_text_height());
    }
}

std::set<int> get_down_keys()
{
    return down_keys;
}

glm::vec2 normal(const glm::vec2& v)
{
    return glm::vec2(v.y, -v.x) / glm::length(v);
}

void draw_shape()
{
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, 0);
}

void update_shape_verts()
{
    glBindBuffer(GL_ARRAY_BUFFER, shape_vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(GLfloat) * 2 * shape_verts.size(), &shape_verts[0]);
}

void glfw_error_callback(int error, const char* description)
{
    fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}

void window_size_callback(GLFWwindow* window, int width, int height)
{
    viewport_dim = glm::vec2(width, height);
    glViewport(0, 0, width, height);

    shape_shader.use();
    shape_shader.setVec2("transform", glm::vec2(1.0f / (viewport_dim - 1.0f)));
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    mouse_scroll.new_state(yoffset);
    scroll_receive_frame = frame_counter;
}

void set_draw_texture(unsigned int texture_id)
{
    glBindTexture(GL_TEXTURE_2D, texture_id);
}
// Uses the active atlas to calculate the width of str in world units.
float get_text_width(const std::string& text)
{
    float ret = 0;
    for (const char& c : text)
    {
        ret += glyphs[c].advance;
    }
    return ret * text_scale;
}

float get_text_height()
{
    // TODO Implement wrapping.
    return text_scale * font_height;
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (action == GLFW_PRESS)
    {
        down_keys.insert(key);
    }
    else if (action == GLFW_RELEASE)
    {
        down_keys.erase(key);
    }
}