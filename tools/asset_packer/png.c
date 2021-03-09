#include "png.h"
#include <png.h>
#include <stdlib.h>
#include <assert.h>

static uint32_t s_max_width;
static uint32_t s_max_height;
static png_structp s_png;
static png_infop s_png_info;
static uint8_t** s_rows;

void png_init(uint32_t max_width, uint32_t max_height)
{
	assert(max_width > 0);
	assert(max_height > 0);

	s_max_width = max_width;
	s_max_height = max_height;

	s_png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	assert(s_png);

	s_png_info = png_create_info_struct(s_png);
	assert(s_png_info);

	if (setjmp(png_jmpbuf(s_png))) abort();

	s_rows = malloc(max_height);
	assert(s_rows);
	for (uint32_t y = 0; y < max_height; y++)
	{
		s_rows[y] = malloc((size_t)max_width * 4);
		assert(s_rows[y]);
	}
}

void png_quit()
{
	png_destroy_read_struct(&s_png, &s_png_info, NULL);

	for (uint32_t y = 0; y < s_max_height; y++)
	{
		free(s_rows[y]);
	}
	free(s_rows);
}

void png_load(const char* file_name, uint32_t* size, uint8_t* pixels)
{
	FILE* file = fopen(file_name, "rb");
	assert(file);

	png_init_io(s_png, file);
	png_read_info(s_png, s_png_info);

	uint32_t width = png_get_image_width(s_png, s_png_info);
	uint32_t height = png_get_image_height(s_png, s_png_info);
	uint8_t color_type = png_get_color_type(s_png, s_png_info);
	uint8_t bit_depth = png_get_bit_depth(s_png, s_png_info);

	if (bit_depth == 16) 
		png_set_strip_16(s_png);

	if (color_type == PNG_COLOR_TYPE_PALETTE)
		png_set_palette_to_rgb(s_png);

	if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8)
		png_set_expand_gray_1_2_4_to_8(s_png);

	if (png_get_valid(s_png, s_png_info, PNG_INFO_tRNS))
		png_set_tRNS_to_alpha(s_png);

	if (color_type == PNG_COLOR_TYPE_RGB ||
		color_type == PNG_COLOR_TYPE_GRAY ||
		color_type == PNG_COLOR_TYPE_PALETTE)
		png_set_filler(s_png, 0xFF, PNG_FILLER_AFTER);

	if (color_type == PNG_COLOR_TYPE_GRAY ||
		color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
		png_set_gray_to_rgb(s_png);

	png_read_update_info(s_png, s_png_info);

	png_read_image(s_png, s_rows);

	for (uint32_t y = 0; y < height; y++)
	{
		for (uint32_t x = 0; x < width; x++)
		{
			pixels[y * width + x] = s_rows[y][x];
			(*size)++;
		}
	}

	fclose(file);
}
