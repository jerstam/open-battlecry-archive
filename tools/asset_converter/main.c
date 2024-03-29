#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
#include <ctype.h>

#define WIN32_LEAN_AND_MEAN
#include <png.h>
#include <zlib.h>
#include <tchar.h>
#include <Windows.h>
#include <Shlobj.h>

#pragma comment(lib,"Shell32.lib")

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

static FILE* xcr_file;
static uint32_t image_count;
static uint32_t image_offsets[256];
static char image_names[256][32];

static char mbs_output_directory[MAX_PATH];
static TCHAR output_directory[MAX_PATH];

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
 *  - 250-254 are shadows
 *  - 224-238 are the side colors (15 max)
 */
static void rle_to_png(const char* file_path, const rle_header_t* rle_header, const uint8_t* data, bool shadowed)
{
	assert(file_path);
	assert(rle_header);
	assert(data);

	// RLE Encoding
	int32_t size = rle_header->size - rle_header->total_pointer_block_size;
	int16_t width = rle_header->width;
	int16_t height = rle_header->height;

	png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	assert(png_ptr);

	png_infop info_ptr = png_create_info_struct(png_ptr);
	assert(info_ptr);

	if (setjmp(png_jmpbuf(png_ptr))) assert(0);

	png_set_IHDR(
		png_ptr,
		info_ptr,
		width, height,
		8,
		PNG_COLOR_TYPE_RGBA,
		PNG_INTERLACE_NONE,
		PNG_COMPRESSION_TYPE_DEFAULT,
		PNG_FILTER_TYPE_DEFAULT
	);

	png_byte** png_rows = png_malloc(png_ptr, height * sizeof(png_byte*));
	assert(png_rows);
	size_t row_size = width * sizeof(uint8_t) * 4;
	for (int y = 0; y < height; y++)
	{
		png_rows[y] = png_malloc(png_ptr, row_size);
		assert(png_rows[y]);
	}

	int byte_count = 0;
	int row = 0;
	png_byte* png_row = png_rows[row];
	for (int pixel = 0; pixel < size; pixel++)
	{
		int16_t value = data[pixel];

		if (value == 255)
		{
			int16_t count = data[pixel + 1];

			// Two consecutive 255 means a blank line
			if (pixel < size - 1 && data[pixel + 1] == 255)
			{
				count = rle_header->width;
			}

			for (int16_t i = 0; i < count; i++)
			{
				if (byte_count >= row_size)
				{
					row++;
					png_row = png_rows[row];
					byte_count = 0;
				}

				png_row[byte_count++] = 0;
				png_row[byte_count++] = 0;
				png_row[byte_count++] = 0;
				png_row[byte_count++] = 0;
			}
			pixel++;
		}
		else if (shadowed && value == PIXEL_SHADOW_START)
		{
			uint8_t alpha = shadow_alpha[value - PIXEL_SHADOW_START];
			int16_t count = data[pixel + 1];

			for (int16_t i = 0; i < count; i++)
			{
				if (byte_count >= row_size)
				{
					row++;
					png_row = png_rows[row];
					byte_count = 0;
				}

				png_row[byte_count++] = shadow_color;
				png_row[byte_count++] = shadow_color;
				png_row[byte_count++] = shadow_color;
				png_row[byte_count++] = alpha;
			}
			pixel++;
		}
		else if (shadowed && value > PIXEL_SHADOW_START)
		{
			uint8_t alpha = shadow_alpha[value - PIXEL_SHADOW_START];

			if (byte_count >= row_size)
			{
				row++;
				png_row = png_rows[row];
				byte_count = 0;
			}

			png_row[byte_count++] = shadow_color;
			png_row[byte_count++] = shadow_color;
			png_row[byte_count++] = shadow_color;
			png_row[byte_count++] = alpha;
		}
		else 
		{
			int16_t color = rle_header->palettes[0][value];

			if (byte_count >= row_size)
			{
				row++;
				png_row = png_rows[row];
				byte_count = 0;
			}

			png_row[byte_count++] = convert_565_to_R(color);
			png_row[byte_count++] = convert_565_to_G(color);
			png_row[byte_count++] = convert_565_to_B(color);
			png_row[byte_count++] = 255;
		}
	}

	FILE* png_file = fopen(file_path, "wb");
	assert(png_file);
	png_init_io(png_ptr, png_file);

	png_set_rows(png_ptr, info_ptr, png_rows);
	png_write_png(png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, NULL);

	for (int y = 0; y < height; y++)
	{
		png_free(png_ptr, png_rows[y]);
	}
	png_free(png_ptr, png_rows);
	png_destroy_write_struct(&png_ptr, &info_ptr);
	fclose(png_file);
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
			if (n > 0)
			{
				out[n++] = '_';
			}
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

static void process_rle(uint32_t index, const char* xcr_name)
{
	assert(xcr_name);

	printf("  Processing %s\n", image_names[index]);

	fseek(xcr_file, image_offsets[index], SEEK_SET);

	// Read ID
	char id[3];
	id[2] = '\0';
	fread(id, 2, 1, xcr_file);

	rle_header_t rle_header = { 0 };
	fread(&rle_header, sizeof(rle_header), 1, xcr_file);

	// Skip the pointer blocks
	fseek(xcr_file, rle_header.total_pointer_block_size, SEEK_CUR);

	uint32_t size = rle_header.size - rle_header.total_pointer_block_size;
	uint8_t* data = malloc(size);
	assert(data);
	fread(data, size, 1, xcr_file);

	char* dot = strrchr(image_names[index], '.');
	dot[1] = 'p';
	dot[2] = 'n';
	dot[3] = 'g';

	char png_path[MAX_PATH];
	strcpy(png_path, mbs_output_directory);
	strcat(png_path, xcr_name);
	strcat(png_path, "\\");
	strcat(png_path, image_names[index]);

	rle_to_png(png_path, &rle_header, data, id[1] == 'S');

	free(data);
}

static void process_xcr(const char* file_path)
{
	image_count = 0;
	memset(image_names, 0, sizeof(image_names));
	memset(image_offsets, 0, sizeof(image_offsets));

	xcr_file = fopen(file_path, "rb");
	assert(xcr_file);
	printf("Opened %s\n", file_path);

	// Read XCR header
	xcr_header_t xcr_header = { 0 };
	fread(&xcr_header, sizeof(xcr_header), 1, xcr_file);
	printf("  Resource count: %d\n", xcr_header.resource_count);

	// Read resource headers
	for (int32_t i = 0; i < xcr_header.resource_count; i++)
	{
		resource_header_t resource_header = { 0 };
		fread(&resource_header, sizeof(resource_header), 1, xcr_file);

		const char* extension = get_extension(resource_header.file_name);
		if (strncmp(extension, "RLE", 3) == 0)
		{
			image_offsets[image_count] = resource_header.offset;
			strncpy(image_names[image_count], resource_header.file_name, strlen(resource_header.file_name));
			++image_count;
		}
	}

	char* xcr_name = strrchr(file_path, '\\');
	xcr_name++;
	char* dot = strrchr(file_path, '.');
	*dot = '\0';

	char snake_xcr_name[32];
	to_snake_case(snake_xcr_name, 32, xcr_name);

	TCHAR wcs_xcr_name[32];
	mbstowcs(wcs_xcr_name, snake_xcr_name, 32);

	TCHAR out_dir[MAX_PATH];
	wcscpy(out_dir, output_directory);
	wcscat(out_dir, TEXT("\\"));
	wcscat(out_dir, wcs_xcr_name);
	SHCreateDirectoryEx(NULL, out_dir, NULL);

	// Process resources
	for (uint32_t i = 0; i < image_count; i++)
	{
		process_rle(i, snake_xcr_name);
	}

	fclose(xcr_file);
}

static void process_dir(const TCHAR* path)
{
	HANDLE find = INVALID_HANDLE_VALUE;
	WIN32_FIND_DATA find_data;

	TCHAR search_path[MAX_PATH];
	wcscpy(search_path, path);
	wcscat(search_path, TEXT("\\*"));

	find = FindFirstFile(search_path, &find_data);
	if (find == INVALID_HANDLE_VALUE)
	{
		return;
	}

	int subdir_count = 0;
	TCHAR subdirs[28][MAX_PATH];

	do
	{
		if (find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			TCHAR* dir_name = find_data.cFileName;

			if (dir_name[0] != '.')
			{
				wcscpy(subdirs[subdir_count], path);
				wcscat(subdirs[subdir_count], TEXT("\\"));
				wcscat(subdirs[subdir_count], dir_name);
				subdir_count++;
				wprintf(TEXT("%s\n"), dir_name);
			}
		}
		else
		{
			TCHAR* dot = wcsrchr(find_data.cFileName, '.');
			if (dot[1] == 'x' && dot[2] == 'c' && dot[3] == 'r')
			{
				wprintf(TEXT("  %s\n"), find_data.cFileName);

				char mbs_file_name[32];
				wcstombs(mbs_file_name, find_data.cFileName, 32);

				char mbs_path[MAX_PATH];
				wcstombs(mbs_path, path, MAX_PATH);
				strcat(mbs_path, "\\");
				strcat(mbs_path, mbs_file_name);

				process_xcr(mbs_path);
			}
		}
	} while (FindNextFile(find, &find_data) != 0);

	for (int i = 0; i < subdir_count; i++)
	{
		process_dir(subdirs[i]);
	}

	FindClose(find);
}

int _tmain(int argc, TCHAR* argv[])
{
#if 1
	TCHAR search_directory[MAX_PATH];
	wcscpy(search_directory, TEXT("C:\\Games\\Warlords Battlecry 3\\Assets"));

	wcscpy(output_directory, argv[0]);
	TCHAR* end = wcsrchr(output_directory, '\\');
	end[1] = '\0';
	wcscat(output_directory, TEXT("converted\\"));

	wcstombs(mbs_output_directory, output_directory, MAX_PATH);
#else
	if (argc != 3)
	{
		_tprintf(TEXT("\nUsage: %s <search directory> <output directory>\n"), argv[0]);
		return -1;
	}

	TCHAR search_directory[MAX_PATH];
	wcscpy(search_directory, argv[1]);
	wcscat(search_directory, "\\*");

	TCHAR output_directory[MAX_PATH];
	wcscpy(output_directory, argv[2]);
#endif

	process_dir(search_directory);

	return 0;
}