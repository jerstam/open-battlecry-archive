#pragma once

typedef enum
{
    KTX_FORMAT_BC7,
    KTX_FORMAT_R8,
    KTX_FORMAT_RGBA8,
    KTX_FORMAT_COUNT
} ktx_format_t;

typedef struct ktx_info_t
{
	int data_offset;
    int size_bytes;
    ktx_format_t format;
    unsigned int flags;
    int width;
    int height;
    int depth;
    int num_layers;
    int num_mips;
    int bytes_per_pixel;
    int metadata_offset;
    int metadata_size;
} ktx_info_t;

void ktx_parse(const void* data, int size, ktx_info_t* info);