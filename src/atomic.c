#include "atomic.h"

#define WIN32_LEAN_AND_MEAN
#include <intrin.h>

u32 atomic32_load(atomic32_t* value)
{
	return *value;
}

u32 atomic32_store(atomic32_t* target, u32 value)
{
	_InterlockedExchange(target, value);
}

u32 atomic32_add(atomic32_t* addend, u32 value)
{
	_InterlockedExchangeAdd(addend, value);
}

u32 atomic32_compare_exchange(atomic32_t* destination, u32 exchange, u32 comparand)
{
	_InterlockedCompareExchange(destination, exchange, comparand);
}

u64 atomic64_load(atomic64_t* value)
{
	return *value;
}

u64 atomic64_store(atomic64_t* target, u64 value)
{
	_InterlockedExchange64(target, value);
}

u64 atomic64_add(atomic64_t* addend, u64 value)
{
	_InterlockedExchangeAdd64(addend, value);
}

u64 atomic64_compare_exchange(atomic64_t* destination, u64 exchange, u64 comparand)
{
	_InterlockedCompareExchange64(destination, exchange, comparand);
}
