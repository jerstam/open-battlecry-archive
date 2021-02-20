#pragma once

#include "../macros.h"

typedef struct Char
{
	u16 x;
	u16 y;
	u16 width;
	u16 height;
	i16 xOffset;
	i16 yOffset;
	i16 advance;
} Char;

typedef struct Kerning
{
	u32 first;
	u32 second;
	i16 amount;
} Kerning;

typedef struct Font
{
	const char* name;
	u16 size;
	u16 lineHeight;
	u16 base;
	u16 width;
	u16 height;
	const char* file;
	u16 charCount;
	u16 first;
	const Char* chars;
	u16 kerningCount;
	const Kerning* kernings;
} Font;



extern Font arial_32;