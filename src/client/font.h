#pragma once

#include "../macros.h"

typedef struct glyph_t
{
	u16 x;
	u16 y;
	u16 width;
	u16 height;
	i16 x_offset;
	i16 y_offset;
	i16 advance;
} glyph_t;

typedef struct kerning_t
{
	u32 first;
	u32 second;
	i16 amount;
} kerning_t;

typedef struct font_t
{
	const char* name;
	u16 size;
	u16 line_height;
	u16 base;
	u16 width;
	u16 height;
	const char* file;
	u16 char_count;
	u16 first;
	const glyph_t* chars;
	u16 kerning_count;
	const kerning_t* kernings;
} font_t;
