#include "bc7.h"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include "volk.h"
#include <stdio.h>
#include <stdint.h>
#include <assert.h>

typedef struct bc7_buffer
{
	uint32_t color[4];
} bc7_buffer;

typedef struct bc7_constants
{
	uint32_t tex_width;
	uint32_t num_block_x;
	uint32_t format;
	uint32_t mode_id;
	uint32_t start_block_id;
	uint32_t num_total_blocks;
	float alpha_weight;
	uint32_t reserved;
} bc7_constants;

void bc7_init(void)
{

}

void bc7_quit(void)
{

}

void bc7_encode(uint8_t* bc, const uint8_t* color)
{

}
