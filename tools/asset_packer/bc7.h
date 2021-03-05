#pragma once

#include <stdint.h>

#define NUM_PIXELS_PER_BLOCK 16

void bc7_init(void);
void bc7_quit(void);
void bc7_encode(uint8_t* bc, const uint8_t* color);
