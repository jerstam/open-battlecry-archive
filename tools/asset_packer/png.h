#pragma once

#include <stdint.h>

void png_init(uint32_t max_width, uint32_t max_height);
void png_quit(void);
void png_load(const char* file_name, uint32_t* size, uint8_t* pixels);