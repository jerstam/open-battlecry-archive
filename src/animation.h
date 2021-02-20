#pragma once

#include "macros.h"

typedef struct animation_t
{
	u8 frame_count;
	u16 origin_x;
	u16 origin_y;
	u16 width;
	u16 height;
	u16 selection_origin_x;
	u16 selection_origin_y;
	u16 selection_size;
} animation_t;

