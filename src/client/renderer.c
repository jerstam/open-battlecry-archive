#include "renderer.h"
#include "window.h"
#include "gpu.h"
#include "../os.h"

enum
{
	MAX_SPRITES = 10000,
	MAX_IMAGES = 3000,
};

static shader_t sprite_shader;

static u32 sprite_count;
static sprite_t sprites[MAX_SPRITES];

static image_t images[MAX_IMAGES];
static descriptor_set_t descriptor_set_images;
static descriptor_set_t descriptor_set_buffer;

void renderer_init(void)
{
	gpu_init();
}

void renderer_draw(void)
{
	gpu_begin_frame();



	gpu_end_frame();
}

void renderer_quit(void)
{
	gpu_quit();
}
