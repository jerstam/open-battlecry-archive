#pragma once

#include "../core/common.h"

typedef struct VkDevice_T* VkDevice;



void buffer_pool_init(VkDevice device);
void buffer_pool_quit(void);