#pragma once

#include "../core/common.h"

void render_init(void);
void render_quit(void);
void render_frame(void);

// 
// Buffer
//
typedef enum
{
    BUFFER_VERTEX = 1,
    BUFFER_INDEX = 2,
    BUFFER_UNIFORM = 4,
    BUFFER_STORAGE = 8,
    BUFFER_INDIRECT = 16,

    BUFFER_CPU = 32,
    BUFFER_CPU_TO_GPU = 64,
    BUFFER_PERSISTENT = 128
} buffer_flags_t;

// 
// Image
//
typedef enum
{
    IMAGE_TYPE_SAMPLED,
    IMAGE_TYPE_STORAGE,
    IMAGE_TYPE_RENDER_TARGET,
} image_type_t;

typedef struct image_desc_t
{
    u64 memory_size;
    u64 memory_offset;
    image_type_t image_type;
    u32 width;
    u32 height;
    u32 mip_levels;
    u32 sample_count;
    const char* tag;
} image_desc_t;
