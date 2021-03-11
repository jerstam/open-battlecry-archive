#pragma once

#include "../core/common.h"

typedef enum
{
	RESOURCE_REQUEST_LOAD_TEXTURE,
	RESOURCE_REQUEST_UPDATE_TEXTURE,
	RESOURCE_REQUEST_TRANSITION_TEXTURE,
	RESOURCE_REQUEST_UPDATE_BUFFER,
	RESOURCE_REQUEST_TRANSITION_BUFFER
} resource_request_type_t;

typedef enum
{
	TEXTURE_FORMAT_UNKNOWN,
	TEXTURE_FORMAT_BC7,
	TEXTURE_FORMAT_RGBA8,
	TEXTURE_FORMAT_R32
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

typedef u64 sync_token_t;

typedef struct { u16 index; } texture_handle_t;

void resource_init(void);
void resource_quit(void);

void load_texture_file(const char* file_name, texture_handle_t* texture_handle, sync_token_t* sync_token);