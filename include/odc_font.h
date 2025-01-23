#ifndef FONT_H
#define FONT_H

#include "glad.h"
#include <GLFW/glfw3.h>

#include <ft2build.h>
#include <stdint.h>
#include FT_FREETYPE_H

#include "odc.h"

#ifndef MAX_GLYPHS
#define MAX_GLYPHS 96 // Printable ASCII
#endif

struct glyph {
	GLuint texture_id;
	int width;
	int height;
	int bearing_x;
	int bearing_y;
	long advance;
	float tex_offset_x;
	float tex_offset_y;
};

struct font {
	GLuint texture_id;
	FT_Library ft;
	FT_Face face;
	GLuint atlas;
	struct glyph glyphs[MAX_GLYPHS];
	float scale;
	int units_per_em;
	int ascender;
	int descender;
	int line_gap;
};

ODC_API int odc_font_load(const char *font_path, struct font *font);
ODC_API void odc_font_free(struct font *font);

#endif // FONT_H
