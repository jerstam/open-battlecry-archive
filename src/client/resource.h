#pragma once

#include "../macros.h"

typedef struct buffer_t buffer_t;
typedef struct image_t image_t;

void resource_init(void);
void resource_quit(void);

void resource_add_buffer(buffer_t* buffer);
void resource_remove_buffer(buffer_t* buffer);

void resource_add_image(const char* name, image_t* image);
void resource_remove_image(image_t* image);