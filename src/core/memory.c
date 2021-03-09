#include "memory.h"

#include <stdlib.h>
#include <assert.h>

void linear_allocator_init(u64 capacity, linear_allocator_t* allocator)
{
	assert(allocator);

	allocator->capacity = capacity;
	allocator->memory = malloc(capacity);
	allocator->size = 0;
}

void linear_allocator_free(linear_allocator_t* allocator)
{
	assert(allocator);
	free(allocator->memory);
	allocator->capacity = 0;
	allocator->size = 0;
}

u8* linear_allocator_allocate(linear_allocator_t* allocator, u64 size)
{
	assert(allocator);
	u8* memory = &allocator->memory[size];
	allocator->size += size;
	assert(allocator->size < allocator->capacity);
	return memory;
}

void linear_allocator_clear(linear_allocator_t* allocator)
{
	assert(allocator);
	allocator->size = 0;
}
