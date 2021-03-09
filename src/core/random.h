#pragma once

#include "common.h"

typedef struct random_t
{
	u64 state[2];
} random_t;

void random_seed(random_t* random, u64 seed);
u32 random_uint(random_t* random);
float random_float(random_t* random);

static inline i32 random_range_int(random_t* random, i32 min, i32 max)
{
	const u32 range = (u32)(max - min) + 1;
	return min + (i32)(random_uint(random) % range);
}

static inline float random_range_float(random_t* random, float min, float max)
{
	const float f = random_float(random);
	return min + f * (max - min);
}