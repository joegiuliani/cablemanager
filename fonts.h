#pragma once

// might convert to singleton since order of init matters

#include <map>
#include "techlayout.h"
#include "text.h"

struct TLGlyph {
	unsigned int texture_id;  // ID handle of the glyph texture
	TLVec dim;       // Size of glyph
	TLVec bearing;    // Offset from baseline to left/top of glyph
	float advance;    // Offset to advance cursor to next glyph
};

namespace TL
{
	void init_fonts();
	unsigned int get_glyph_vao();
	unsigned int gen_atlas_id();
	int load_atlas(TLAtlasID ID, const char* path, unsigned int resolution);
	void set_active_atlas(TLAtlasID ID);
	float get_width(const TLText& text);
	float get_height(const TLText& text);
	void destroy_atlases();
}


