#include "random.h"

// TODO: Compare perf with: https://github.com/lemire/SIMDxorshift

static inline float random_float_normalized(u32 value)
{
    u32 exponent = 127;
    u32 mantissa = value >> 9;
    u32 result = (exponent << 23) | mantissa;
    float fresult = *(float*)(&result);
    return fresult - 1.0f;
}

static inline u64 random_avalanche64(u64 h)
{
    h ^= h >> 33;
    h *= 0xff51afd7ed558ccd;
    h ^= h >> 33;
    h *= 0xc4ceb9fe1a85ec53;
    h ^= h >> 33;
    return h;
}

void random_seed(random_t* random, u64 seed)
{
    u64 value = ((seed) << 1ull) | 1ull; // Make it odd
    value = random_avalanche64(value);
    random->state[0] = 0ull;
    random->state[1] = (value << 1ull) | 1ull;
    random_uint(random);
    random->state[0] += random_avalanche64(value);
    random_uint(random);
}

u32 random_uint(random_t* random)
{
    u64 old_state = random->state[0];
    random->state[0] = old_state * 0x5851f42d4c957f2dull + random->state[1];
    u32 xorshifted = (u32)(((old_state >> 18ull) ^ old_state) >> 27ull);
    u32 rot = (u32)(old_state >> 59ull);
    return (xorshifted >> rot) | (xorshifted << ((-(int)rot) & 31));
}

float random_float(random_t* random)
{
    return random_float_normalized(random_uint(random));
}
