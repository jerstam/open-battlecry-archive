#pragma once

#include "common.h"

typedef struct pool_t {
	u32 count;
	u32 last;
	u16* generations;
	u16* free_indices;
} pool_t;

void pool_init(pool_t* pool, u32 count);
void pool_free(pool_t* pool);

handle_t pool_get_handle(pool_t* pool);
void pool_return_handle(pool_t* pool, handle_t handle);