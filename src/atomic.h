#pragma once

#include "macros.h"

typedef volatile ALIGNAS(4) u32 atomic32_t;
typedef volatile ALIGNAS(8) u64 atomic64_t;

u32 atomic32_load(atomic32_t* value);
u32 atomic32_store(atomic32_t* target, u32 value);
u32 atomic32_add(atomic32_t* addend, u32 value);
u32 atomic32_compare_exchange(atomic32_t* destination, u32 exchange, u32 comparand);

u64 atomic64_load(atomic64_t* value);
u64 atomic64_store(atomic64_t* target, u64 value);
u64 atomic64_add(atomic64_t* addend, u64 value);
u64 atomic64_compare_exchange(atomic64_t* destination, u64 exchange, u64 comparand);