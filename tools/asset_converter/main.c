#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
#include <ctype.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

enum
{
	PIXEL_SHADOW_START = 250,
	PIXEL_SHADOW_END = 254,
	PIXEL_SHADOW_COUNT = PIXEL_SHADOW_END - PIXEL_SHADOW_START + 1,
	PIXEL_TRANSPARENT = 255,

	PIXEL_SIDE_START = 224,
	PIXEL_SIZE_END = 238
};

typedef enum
{
	FILE_TYPE_DATA,
	FILE_TYPE_TEXT,
	FILE_TYPE_STRING
} file_type_t;

typedef enum
{
	ANIMATION_TYPE_STAND,
	ANIMATION_TYPE_WALK,
	ANIMATION_TYPE_FIGHT,
	ANIMATION_TYPE_DIE,
	ANIMATION_TYPE_AMBIENT,
	ANIMATION_TYPE_SPECIAL,
	ANIMATION_TYPE_CONVERT,
	ANIMATION_TYPE_SPELL,
	ANIMATION_TYPE_INTERFACE,
	ANIMATION_TYPE_COUNT
} animation_type_t;

typedef enum
{
	PALETTE_TYPE_NORMAL = 0,
	PALETTE_TYPE_DAYTIME = 0,
	PALETTE_TYPE_DARKER = 1,
	PALETTE_TYPE_LIGHTER = 2,
	PALETTE_TYPE_NIGHTTIME = 3,
	PALETTE_TYPE_SUNSET1 = 4,
	PALETTE_TYPE_SUNSET2 = 5,
	PALETTE_TYPE_SUNSET3 = 6,
	PALETTE_TYPE_SUNSET4 = 7,
	PALETTE_TYPE_COUNT = 8
} palette_type_t;

typedef struct xcr_header_t
{
	char identifier[20];
	int32_t resource_count;
	uint32_t file_size;
} xcr_header_t;

typedef struct resource_header_t
{
	char file_name[256];
	char full_path[256];
	uint32_t offset;
	uint32_t file_size;
	int32_t file_type;
	uint32_t crc_block;
	bool crc_check;
	bool encrypted;
} resource_header_t;

typedef struct animation_t
{
	bool used;
	uint8_t frame_count;
	uint8_t effects[2];
	int16_t origin_x;
	int16_t origin_y;
	int16_t width;
	int16_t height;
	int16_t effect_origin_x;
	int16_t effect_origin_y;
	int16_t selection_origin_x;
	int16_t selection_origin_y;
	int16_t selection_sie;
	int16_t effect_type;
	int16_t effect_activation;
} animation_t;

typedef struct animation_data_t
{
	animation_t animations[ANIMATION_TYPE_COUNT];
} animation_data_t;

#define MAX_RLE_PALETTE 256
typedef struct rle_header_t
{
	int32_t size;
	int16_t width;
	int16_t height;
	int16_t palettes[PALETTE_TYPE_COUNT][MAX_RLE_PALETTE];
	int32_t pointer_block_size;
	int32_t total_pointer_block_size;
	int16_t pointer_block_count;
} rle_header_t;

static uint32_t image_count;
static uint32_t image_sizes[256];
static uint32_t image_offsets[256];
static char image_names[256][32];

static const uint8_t shadow_color = 0;
static uint8_t shadow_alpha[PIXEL_SHADOW_COUNT] = {
	255-152, 255-216, 255-228, 255-240, 255-248
};

static const char* xcr_id = "xcr File 1.00";

static const uint8_t decode_table[256] = {
	109,114,56,34,142,31,242,214,119,35,107,215,127,204,195,171,
	81,216,150,201,117,252,32,18,202,186,92,46,223,209,180,116,
	230,135,124,115,144,64,53,30,14,23,173,148,80,101,60,251,
	52,177,156,217,94,219,157,207,208,85,76,67,89,131,190,98,
	147,100,57,77,40,237,72,6,110,41,203,229,128,154,226,198,
	63,96,79,22,42,183,105,48,104,126,3,141,243,49,255,122,
	106,50,108,155,12,69,221,189,24,159,123,68,16,185,90,227,
	234,245,125,200,78,61,249,170,175,247,51,160,233,120,193,220,
	199,54,241,83,95,13,238,75,167,158,194,236,179,21,161,55,
	146,172,244,70,212,118,181,218,102,174,29,151,143,169,17,196,
	47,43,5,36,65,222,4,235,176,139,26,45,206,113,140,165,
	153,8,87,211,20,82,93,152,74,111,182,38,188,7,248,86,
	59,19,133,210,88,99,253,62,27,225,37,254,103,205,129,191,
	164,132,58,134,15,145,231,0,112,213,138,73,33,39,187,224,
	1,240,250,137,166,9,97,11,84,197,130,163,71,25,178,91,
	239,228,136,168,28,121,192,149,184,44,232,2,162,10,66,246 
};

static inline uint8_t convert_565_to_R(uint16_t pixel) { return (((uint8_t)((pixel >> 11) & 0x001F)) << 3); }
static inline uint8_t convert_565_to_G(uint16_t pixel) { return (((uint8_t)((pixel >> 5) & 0x003F)) << 2); }
static inline uint8_t convert_565_to_B(uint16_t pixel) { return (((uint8_t)((pixel >> 0) & 0x001F)) << 3); }

/* SSG RLS encoding:
 *	- Two 255 values mean a blank line
 *  - 250 is a shadow
 *  - The X offsets wrap around 127
 *  - The block is the x position >> 7 
 *  - 224-238 are the side colors (15 max)
 */
static void rle_to_png(const char* file_name, const rle_header_t* rle_header, const uint8_t* data)
{
	assert(file_name);
	assert(rle_header);
	assert(data);

	// RLE Encoding
	int32_t size = rle_header->size - rle_header->total_pointer_block_size;
	int16_t width = rle_header->width;
	int16_t height = rle_header->height;

	/*uint8_t* palette_data = calloc(MAX_RLE_PALETTE * 3, 1);
	assert(palette_data);

	for (int i = 0; i < MAX_RLE_PALETTE; i++)
	{
		int16_t color = rle_header->palettes[0][i];
		uint8_t r = convert_565_to_R(color);
		uint8_t g = convert_565_to_G(color);
		uint8_t b = convert_565_to_B(color);

		palette_data[i * 3 + 2] = r;
		palette_data[i * 3 + 1] = g;
		palette_data[i * 3 + 0] = b;
	}
	stbi_write_bmp("palette.bmp", 16, 16, 3, palette_data);
	free(palette_data);*/

	uint8_t* png_data = calloc((size_t)width * (size_t)height * 4, 1);
	assert(png_data);

	int png_data_index = 0;
	for (int x = 0; x < size; x++)
	{
		int16_t value = data[x];

		if (value == 255)
		{
			int16_t count = data[x + 1];

			// Two consecutive 255 means a blank line
			if (x < size - 1 && data[x + 1] == 255)
			{
				count = rle_header->width;
			}

			for (int16_t i = 0; i < count; i++)
			{
				png_data[png_data_index++] = 0;
				png_data[png_data_index++] = 0;
				png_data[png_data_index++] = 0;
				png_data[png_data_index++] = 0;
			}
			x++;
		}
		else if (value == PIXEL_SHADOW_START)
		{
			uint8_t alpha = shadow_alpha[value - PIXEL_SHADOW_START];
			int16_t count = data[x + 1];

			for (int16_t i = 0; i < count; i++)
			{
				png_data[png_data_index++] = shadow_color;
				png_data[png_data_index++] = shadow_color;
				png_data[png_data_index++] = shadow_color;
				png_data[png_data_index++] = alpha;
			}
			x++;
		}
		else if (value > PIXEL_SHADOW_START)
		{
			uint8_t alpha = shadow_alpha[value - PIXEL_SHADOW_START];

			png_data[png_data_index++] = shadow_color;
			png_data[png_data_index++] = shadow_color;
			png_data[png_data_index++] = shadow_color;
			png_data[png_data_index++] = alpha;
		}
		else
		{
			int16_t color = rle_header->palettes[0][value];

			png_data[png_data_index++] = convert_565_to_R(color);
			png_data[png_data_index++] = convert_565_to_G(color);
			png_data[png_data_index++] = convert_565_to_B(color);
			png_data[png_data_index++] = 255;
		}
	}

	stbi_write_png(file_name, width, height, 4, png_data, width * 4);

	free(png_data);
}

static const char* get_extension(const char* file_name) 
{
	const char* dot = strrchr(file_name, '.');
	if (!dot || dot == file_name) return "";
	return dot + 1;
}

static void to_snake_case(char* out, size_t out_size, const char* in)
{
	const char* inp = in;
	size_t n = 0;
	while (n < out_size - 1 && *inp)
	{
		if (*inp >= 'A' && *inp <= 'Z')
		{
			if (n > out_size - 3)
			{
				out[n++] = 0;
				return;
			}
			out[n++] = '_';
			out[n++] = *inp + ('a' - 'A');
		}
		else
		{
			out[n++] = *inp;
		}
		++inp;
	}
	out[n++] = 0;
}

int main(int argc, const char* argv[])
{
#if _DEBUG
	const char* xcr_file_name = "Daemons.xcr";
#else
	if (argc < 2)
		return 0;
	const char* xcr_file_name = argv[1];
#endif 

	FILE* xcr_file = fopen(xcr_file_name, "rb");
	assert(xcr_file);
	printf("Opened %s\n", xcr_file_name);

	// Read XCR header
	xcr_header_t xcr_header = { 0 };
	fread(&xcr_header, sizeof(xcr_header), 1, xcr_file);
	printf("  Resource count: %d\n", xcr_header.resource_count);

	// Read resource headers
	for (int32_t i = 0; i < xcr_header.resource_count; i++)
	{
		resource_header_t resource_header = { 0 };
		fread(&resource_header, sizeof(resource_header), 1, xcr_file);
		printf("  Reading %s (%s)\n", resource_header.file_name, resource_header.full_path);
		printf("    Encrypted: %d\n", resource_header.encrypted);

		const char* extension = get_extension(resource_header.file_name);
		if (strncmp(extension, "RLE", 3) == 0)
		{
			image_sizes[image_count] = resource_header.file_size;
			image_offsets[image_count] = resource_header.offset;
			strncpy(image_names[image_count], resource_header.file_name, strlen(resource_header.file_name));
			++image_count;
		}
	}
	printf("\n");

	/*char xcr_without_ext[32];
	strncpy(xcr_without_ext, xcr_file_name, strlen(xcr_file_name));
	char* dot = strrchr(xcr_without_ext, '.');
	*dot = '\0';
	char directory[32];
	to_snake_case(directory, 32, xcr_without_ext);*/

	// Convert RLE images to PNG
	for (uint32_t i = 0; i < image_count; i++)
	{
		printf("Converting %s to PNG...\n", image_names[i]);
		fseek(xcr_file, image_offsets[i], SEEK_SET);

		// Read ID
		char id[3];
		id[2] = '\0';
		fread(id, 2, 1, xcr_file);
		if (id[1] == 'L')
		{
			// TODO: Handle true RLE images
			continue;
		}
		printf("  ID: %s\n", id);

		rle_header_t rle_header = { 0 };
		fread(&rle_header, sizeof(rle_header), 1, xcr_file);

		// Skip the pointer blocks
		fseek(xcr_file, rle_header.total_pointer_block_size, SEEK_CUR);

		uint32_t size = rle_header.size - rle_header.total_pointer_block_size;
		uint8_t* data = malloc(size);
		assert(data);
		fread(data, size, 1, xcr_file);

		printf("  Read: %d pixels\n", size);

		char* dot = strrchr(image_names[i], '.');
		dot[1] = 'p';
		dot[2] = 'n';
		dot[3] = 'g';

		/*char full_file_name[64];
		snprintf(full_file_name, 64, "%s/%s", directory, image_names[i]);
		printf(full_file_name);*/

		rle_to_png(image_names[i], &rle_header, data);

		free(data);
	}

	fclose(xcr_file);

	return 0;
}