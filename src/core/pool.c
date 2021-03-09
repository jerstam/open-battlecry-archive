#include "pool.h"

#include <malloc.h>
#include <assert.h>

void pool_init(pool_t* pool, u32 count)
{
	assert(pool);
	assert(count >= 2);
	assert(count < UINT16_MAX);
	pool->count = count;						
	pool->last = 0;		
	pool->generations = calloc(pool->count, sizeof(u32));
	assert(pool->generations);
	pool->free_indices = malloc(pool->count * sizeof(u32));
	assert(pool->free_indices);
	for (i32 i = pool->count - 1; i >= 0; i--) {
		pool->free_indices[pool->last++] = (u16)i;		
	}
}

void pool_free(pool_t* pool)
{
	assert(pool);
	assert(pool->free_indices);
	assert(pool->generations);
	free(pool->free_indices);
	free(pool->generations);
	pool->last = 0;
}

handle_t pool_get_handle(pool_t* pool)
{
	assert(pool);
	assert(pool->free_indices);
	assert(pool->last > 0);
	u16 index = pool->free_indices[--pool->last];
	u16 generation = pool->generations[index];
	return (handle_t) { index, generation };
}

void pool_return_handle(pool_t* pool, handle_t handle)
{
	assert(pool);
	assert(pool->free_indices);
	assert(pool->last < pool->count);
	assert(handle.index < pool->count);
	pool->free_indices[pool->last++] = handle.index;
	pool->generations[handle.index]++;
}
