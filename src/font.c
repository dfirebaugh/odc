#include <stdio.h>
#include <stdlib.h>

#include "odc_font.h"

#define ATLAS_WIDTH 512
#define ATLAS_HEIGHT 512

int odc_font_load(const char *font_path, struct font *font)
{
	if (!font) {
		fprintf(stderr, "ERROR::FONT: Font pointer is NULL\n");
		return -1;
	}

	if (FT_Init_FreeType(&font->ft)) {
		fprintf(stderr,
			"ERROR::FREETYPE: Could not init FreeType Library\n");
		return -1;
	}

	if (FT_New_Face(font->ft, font_path, 0, &font->face)) {
		fprintf(stderr, "ERROR::FREETYPE: Failed to load font %s\n",
			font_path);
		FT_Done_FreeType(font->ft);
		return -1;
	}

	FT_Set_Pixel_Sizes(font->face, 0, 48);

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	unsigned char *atlas_bitmap = (unsigned char *)calloc(
		ATLAS_WIDTH * ATLAS_HEIGHT, sizeof(unsigned char));
	if (!atlas_bitmap) {
		fprintf(stderr,
			"ERROR::FONT: Failed to allocate memory for atlas "
			"bitmap\n");
		FT_Done_Face(font->face);
		FT_Done_FreeType(font->ft);
		return -1;
	}

	int x_offset = 0;
	int y_offset = 0;
	int row_height = 0;

	for (unsigned char c = 32; c < 32 + MAX_GLYPHS; c++) {
		// Load character glyph
		if (FT_Load_Char(font->face, c, FT_LOAD_RENDER)) {
			fprintf(stderr,
				"ERROR::FREETYPE: Failed to load Glyph for "
				"character '%c'\n",
				c);
			continue;
		}

		FT_GlyphSlot g = font->face->glyph;

		if (x_offset + g->bitmap.width > ATLAS_WIDTH) {
			x_offset = 0;
			y_offset += row_height;
			row_height = 0;
		}

		if (y_offset + g->bitmap.rows > ATLAS_HEIGHT) {
			fprintf(stderr,
				"ERROR::FONT: Texture atlas is too small for "
				"all glyphs\n");
			free(atlas_bitmap);
			FT_Done_Face(font->face);
			FT_Done_FreeType(font->ft);
			return -1;
		}

		for (int row = 0; row < g->bitmap.rows; row++) {
			for (int col = 0; col < g->bitmap.width; col++) {
				int atlas_x = x_offset + col;
				int atlas_y = y_offset + row;
				if (atlas_x < ATLAS_WIDTH &&
				    atlas_y < ATLAS_HEIGHT) {
					atlas_bitmap[atlas_y * ATLAS_WIDTH +
						     atlas_x] =
						g->bitmap.buffer
							[row * g->bitmap.width +
							 col];
				}
			}
		}

		int glyph_index = c - 32;
		struct glyph *glyph = &font->glyphs[glyph_index];
		glyph->width = g->bitmap.width;
		glyph->height = g->bitmap.rows;
		glyph->bearing_x = g->bitmap_left;
		glyph->bearing_y = g->bitmap_top;
		glyph->advance = g->advance.x >> 6;

		glyph->tex_offset_x = (float)x_offset / (float)ATLAS_WIDTH;
		glyph->tex_offset_y = (float)y_offset / (float)ATLAS_HEIGHT;

		x_offset += g->bitmap.width;
		if (g->bitmap.rows > row_height) {
			row_height = g->bitmap.rows;
		}
	}

	glGenTextures(1, &font->atlas);
	font->texture_id = font->atlas;
	glBindTexture(GL_TEXTURE_2D, font->atlas);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, ATLAS_WIDTH, ATLAS_HEIGHT, 0,
		     GL_RED, GL_UNSIGNED_BYTE, atlas_bitmap);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	free(atlas_bitmap);

	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

	font->scale = 1.0f / (float)ATLAS_WIDTH;

	FT_Done_Face(font->face);
	FT_Done_FreeType(font->ft);

	return 0;
}

void odc_font_free(struct font *font)
{
	if (!font)
		return;

	if (font->atlas) {
		glDeleteTextures(1, &font->atlas);
		font->atlas = 0;
	}
}
