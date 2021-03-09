#pragma once

#include <stdint.h>

#define NUM_PIXELS_PER_BLOCK 16

void bc7_init(void);
void bc7_quit(void);
void bc7_compresss(uint32_t width, uint32_t height, uint8_t* image_data, uint64_t size);
