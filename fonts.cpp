


#include "fonts.h"
#include <iostream>
#include <glad/glad.h>
#include <ft2build.h>
#include FT_FREETYPE_H
#include <freetype/freetype.h>
#include <freetype/ftglyph.h>
#include <freetype/ftoutln.h>
#include <freetype/fttrigon.h>
#include "shader.h"
#include "draw.h"

bool initialized = false;
constexpr float GLOBAL_FONT_SCALE = 1.0f / 1.5f;
constexpr float advance_fac = 1.0f / float(1 << 6);

GLuint glyph_vao, glyph_vbo;
unsigned int glyph_shader = -1;

FT_Library ft_lib;

struct TLAtlas
{
    float offset = 0; // Used to center text on  y-axis;
    float max_height = 0; // Used for calculating text dimensions.
    std::map<char, TLGlyph> char_map;
};

// Removing elements causes undefined behavior
std::map<TLAtlasID, TLAtlas> atlas_map;
TLAtlas* active_atlas = nullptr;
unsigned int reg_count = 0;

void TL::init_fonts()
{
    // Init FreeType
    if (FT_Init_FreeType(&ft_lib))
    {
        throw std::exception("Could not init FreeType Library");
    }

    // Generate Vertex Buffers.
    glGenVertexArrays(1, &glyph_vao);
    glGenBuffers(1, &glyph_vbo);

    glBindVertexArray(glyph_vao);
    glBindBuffer(GL_ARRAY_BUFFER, glyph_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
    glEnableVertexAttribArray(0);

    glyph_shader = TLDraw::register_shader(glyph_vao, Shader("glyph", "char.vert", "char.frag"));

    initialized = true;
}

void validate_id(TLAtlasID ID)
{
    if (ID >= reg_count)
    {
        throw std::exception("atlas ID not generated");
    }
    if (ID >= atlas_map.size())
    {
        throw std::exception("Atlas not loaded for ID");
    }
}

// Currently it is not necessary to call init() before using this function,
// but being able to do so is a little misleading. Perhaps it should be required.
unsigned int TL::gen_atlas_id()
{
    return reg_count++;
}
unsigned int TL::get_glyph_vao()
{
    if (!initialized)
    {
        std::cout << "No Glyph VAO generated. Initialize fonts API first.\n";
        throw std::exception();
    }
    return glyph_vao;
}

// Fonts API must be initialized and ID must be generated.
// Replaces any existing atlas for the ID.
// If there is no active atlas, the loaded atlas will be set to the active atlas.
int TL::load_atlas(TLAtlasID ID, const char* path, unsigned int resolution)
{
    if (!initialized)
    {
        std::cout << "Attempting to load atlas before initializing fonts API\n";
        throw std::exception();

    }

    if (ID >= reg_count)
    {
        std::cout << "Attempting to load a font with an unregistered atlas ID\n";
        throw std::exception();
    }

#ifdef TECHLAYOUT_WARNINGS
    if (atlas_map.count(ID) > 0)
    {
        std::cout << "TechLayout Warning: ID already has an atlas generated. The old atlas will be replaced.";
    }
#endif

    FT_Face face;
    if (FT_New_Face(ft_lib, path, 0, &face))
    {
        std::cout << "ERROR::FREETYPE: Failed to load " << path << std::endl;
        return -1;
    }
    FT_Set_Pixel_Sizes(face, 0, resolution);

    TLAtlas new_atlas;
    float import_scale = 1.0f / resolution * GLOBAL_FONT_SCALE;

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // disable byte-alignment restriction
    for (unsigned char c = 0; c < 128; c++)
    {
        // load character glyph 
        if (FT_Load_Char(face, c, FT_LOAD_RENDER))
        {
            std::cout << "ERROR::FREETYTPE: Failed to load Glyph" << std::endl;
            continue;
        }
        // generate texture
        unsigned int texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
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
        TLGlyph ch = {
            texture,
            TLVec(face->glyph->bitmap.width, face->glyph->bitmap.rows) * import_scale,
            TLVec(face->glyph->bitmap_left, face->glyph->bitmap_top) * import_scale,
            face->glyph->advance.x * advance_fac * import_scale
        };
        new_atlas.char_map.insert(std::pair<const char, const TLGlyph>(c, ch));
    }

    float max_height = 0;
    for (const auto& ch : new_atlas.char_map)
    {
        if (ch.second.dim.y > max_height)
        {
            max_height = ch.second.bearing.y;
        }
    }     

    new_atlas.max_height = max_height;
    new_atlas.offset = max_height * 0.5f;

    // TODO this block may be broken.
    atlas_map[ID] = new_atlas;
    if (!active_atlas) active_atlas = &atlas_map[ID];

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    FT_Done_Face(face);
}
void TL::destroy_atlases()
{
    FT_Done_FreeType(ft_lib);
    initialized = false;
}
void TL::set_active_atlas(TLAtlasID id)
{
    validate_id(id);
    active_atlas = &atlas_map[id];
}
// Uses the active atlas to calculate the width of str in world units.
float TL::get_width(const TLText& text)
{
    if (!active_atlas)
        throw std::exception("Cannot get the width of the text if no atlases are loaded!");

    float ret = 0;
    std::string::const_iterator c;
    for (c = text.str.begin(); c != text.str.end(); c++) // for some fucking reason c doesnt need ++c
    {
        ret += active_atlas->char_map[*c].advance;
    }
    return ret;
}
float TL::get_height(const TLText& text)
{
    if (!active_atlas)
    {
        std::cout << "Cannot get the height of the text if no active atlases are loaded!\n";
        throw std::exception();
    }

    // TODO Implement wrapping.
    return active_atlas->max_height;
}

// Decl. in draw.h
void TLDraw::draw_text(const TLText& text, const TLColor& color)
{
    if (!active_atlas)
        throw std::exception("Cannot draw text if no atlases are loaded!");

    glActiveTexture(GL_TEXTURE0);
    TLDraw::push_shader(glyph_shader);

    TLDraw::active_shader->setVec2("pos", text.pos);
    TLDraw::active_shader->setVec4("color", color);
    TLDraw::active_shader->setFloat("offset", active_atlas->offset);

    float curs_x = 0, curs_y = 0;

    // iterate through all characters
    const TLString str = text.str;
    TLString::const_iterator c;
    for (c = str.begin(); c != str.end(); c++)
    {
        TLGlyph ch = active_atlas->char_map[*c];
        curs_y = -(ch.dim.y - ch.bearing.y);

        // update VBO for each character
        float w = ch.dim.x;
        float h = ch.dim.y;
        float vertices[6][4] =
        {
            { curs_x + ch.bearing.x + 0, curs_y + h,   0.0f, 0.0f },
            { curs_x + ch.bearing.x + 0, curs_y + 0,   0.0f, 1.0f },
            { curs_x + ch.bearing.x + w, curs_y + 0,   1.0f, 1.0f },

            { curs_x + ch.bearing.x + 0, curs_y + h,   0.0f, 0.0f },
            { curs_x + ch.bearing.x + w, curs_y + 0,   1.0f, 1.0f },
            { curs_x + ch.bearing.x + w, curs_y + h,   1.0f, 0.0f }
        };
        // render glyph texture over quad
        glBindTexture(GL_TEXTURE_2D, ch.texture_id);
        // update content of VBO memory
        glBindBuffer(GL_ARRAY_BUFFER, glyph_vbo);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices); // be sure to use glBufferSubData and not glBufferData

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        curs_x += ch.advance;
    }
    glBindTexture(GL_TEXTURE_2D, 0);
    TLDraw::pop_shader();
}