#pragma once

#include "../macros.h"

typedef struct sprite_t
{
	float x, y;
	float scale;
	i32 image_index;
	float r, g, b, a;
} sprite_t;

void renderer_init(void);
void renderer_draw(void);
void renderer_quit(void);

void add_sprite(sprite_t);