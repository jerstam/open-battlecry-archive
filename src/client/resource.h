#pragma once

#include "../core/common.h"

enum
{
	RESOURCE_TYPE_TEXTURE,
	RESOURCE_TYPE_FONT,
	RESOURCE_TYPE_ARMY,
	RESOURCE_TYPE_BUILDING,
	RESOURCE_TYPE_ANIMATION
};

typedef struct { u16 index; } texture_handle_t;

typedef enum
{
	TEXTURE_FORMAT_UNKNOWN,
	TEXTURE_FORMAT_BC7,
	TEXTURE_FORMAT_RGBA8,
} texture_format_t;

typedef enum
{
	TEXTURE_WRAP_CLAMP,
	TEXTURE_WRAP_REPEAT
} texture_wrap_t;

typedef enum
{
	TEXTURE_FILTER_NEAREST,
	TEXTURE_FILTER_LINEAR,
} texture_filter_t;

typedef struct resource_t
{
	u32 type;
	i32 version;
	const char* json;
	const u8* data;
} resource_t;

void resource_init(void);
void resource_quit(void);

void resource_load(const char* file_name, resource_t* resource);

void calculate_texture_size(u16 width, u16 height, texture_format_t format, u32* size);
texture_handle_t add_texture(u16 width, u16 height, texture_format_t format, texture_filter_t filter, texture_wrap_t wrap, u8* data, u32 data_size);