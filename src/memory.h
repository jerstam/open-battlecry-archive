#pragma once

#include "macros.h"

typedef struct linear_allocator_t
{
	u8* memory;
	u64 capacity;
	u64 size;
} linear_allocator_t;

void linear_allocator_init(u64 capacity, linear_allocator_t* allocator);
void linear_allocator_free(linear_allocator_t* allocator);

u8* linear_allocator_allocate(linear_allocator_t* allocator, u64 size);
void linear_allocator_clear(linear_allocator_t* allocator);