#include "hash.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define xxh_rotl64(x,r) _rotl64(x,r)

static const u64 XXH_PRIME64_1 = 0x9E3779B185EBCA87ULL;
static const u64 XXH_PRIME64_2 = 0xC2B2AE3D27D4EB4FULL;
static const u64 XXH_PRIME64_3 = 0x165667B19E3779F9ULL;
static const u64 XXH_PRIME64_4 = 0x85EBCA77C2B2AE63ULL;
static const u64 XXH_PRIME64_5 = 0x27D4EB2F165667C5ULL;

static u32 xxh_read32(const void* memPtr)
{
    u32 val;
    memcpy(&val, memPtr, sizeof(val));
    return val;
}

static u64 xxh_read64(const void* memPtr)
{
    u64 val;
    memcpy(&val, memPtr, sizeof(val));
    return val;
}

static u64 xxh64_round(u64 acc, u64 input)
{
    acc += input * XXH_PRIME64_2;
    acc = xxh_rotl64(acc, 31);
    acc *= XXH_PRIME64_1;
    return acc;
}

static u64 xxh64_mergeRound(u64 acc, u64 val)
{
    val = xxh64_round(0, val);
    acc ^= val;
    acc = acc * XXH_PRIME64_1 + XXH_PRIME64_4;
    return acc;
}

static u64 xxh64_avalanche(u64 h64)
{
    h64 ^= h64 >> 33;
    h64 *= XXH_PRIME64_2;
    h64 ^= h64 >> 29;
    h64 *= XXH_PRIME64_3;
    h64 ^= h64 >> 32;
    return h64;
}

static u64 xxh64_finalize(u64 h64, const u8* ptr, u64 len)
{
#define XXH_PROCESS1_64 do {                                   \
    h64 ^= (*ptr++) * XXH_PRIME64_5;                           \
    h64 = xxh_rotl64(h64, 11) * XXH_PRIME64_1;                 \
} while (0)

#define XXH_PROCESS4_64 do {                                   \
    h64 ^= (u64)(xxh_read32(ptr)) * XXH_PRIME64_1;      \
    ptr += 4;                                              \
    h64 = xxh_rotl64(h64, 23) * XXH_PRIME64_2 + XXH_PRIME64_3;     \
} while (0)

#define XXH_PROCESS8_64 do {                                   \
    u64 const k1 = xxh64_round(0, xxh_read64(ptr)); \
    ptr += 8;                                              \
    h64 ^= k1;                                             \
    h64  = xxh_rotl64(h64,27) * XXH_PRIME64_1 + XXH_PRIME64_4;     \
} while (0)

    switch (len & 31) {
    case 24: XXH_PROCESS8_64;
        /* fallthrough */
    case 16: XXH_PROCESS8_64;
        /* fallthrough */
    case  8: XXH_PROCESS8_64;
        return xxh64_avalanche(h64);

    case 28: XXH_PROCESS8_64;
        /* fallthrough */
    case 20: XXH_PROCESS8_64;
        /* fallthrough */
    case 12: XXH_PROCESS8_64;
        /* fallthrough */
    case  4: XXH_PROCESS4_64;
        return xxh64_avalanche(h64);

    case 25: XXH_PROCESS8_64;
        /* fallthrough */
    case 17: XXH_PROCESS8_64;
        /* fallthrough */
    case  9: XXH_PROCESS8_64;
        XXH_PROCESS1_64;
        return xxh64_avalanche(h64);

    case 29: XXH_PROCESS8_64;
        /* fallthrough */
    case 21: XXH_PROCESS8_64;
        /* fallthrough */
    case 13: XXH_PROCESS8_64;
        /* fallthrough */
    case  5: XXH_PROCESS4_64;
        XXH_PROCESS1_64;
        return xxh64_avalanche(h64);

    case 26: XXH_PROCESS8_64;
        /* fallthrough */
    case 18: XXH_PROCESS8_64;
        /* fallthrough */
    case 10: XXH_PROCESS8_64;
        XXH_PROCESS1_64;
        XXH_PROCESS1_64;
        return xxh64_avalanche(h64);

    case 30: XXH_PROCESS8_64;
        /* fallthrough */
    case 22: XXH_PROCESS8_64;
        /* fallthrough */
    case 14: XXH_PROCESS8_64;
        /* fallthrough */
    case  6: XXH_PROCESS4_64;
        XXH_PROCESS1_64;
        XXH_PROCESS1_64;
        return xxh64_avalanche(h64);

    case 27: XXH_PROCESS8_64;
        /* fallthrough */
    case 19: XXH_PROCESS8_64;
        /* fallthrough */
    case 11: XXH_PROCESS8_64;
        XXH_PROCESS1_64;
        XXH_PROCESS1_64;
        XXH_PROCESS1_64;
        return xxh64_avalanche(h64);

    case 31: XXH_PROCESS8_64;
        /* fallthrough */
    case 23: XXH_PROCESS8_64;
        /* fallthrough */
    case 15: XXH_PROCESS8_64;
        /* fallthrough */
    case  7: XXH_PROCESS4_64;
        /* fallthrough */
    case  3: XXH_PROCESS1_64;
        /* fallthrough */
    case  2: XXH_PROCESS1_64;
        /* fallthrough */
    case  1: XXH_PROCESS1_64;
        /* fallthrough */
    case  0: return xxh64_avalanche(h64);
    }

    /* impossible to reach */
    assert(0);
    return 0;  /* unreachable, but some compilers complain without it */
}

static inline u64 xxh64_endian_align(const u8* input, size_t length, u64 seed)
{
	const u8* end = input + length;
    u64 h64;

    if (length >= 32) {
        const u8* const limit = end - 32;
        u64 v1 = seed + XXH_PRIME64_1 + XXH_PRIME64_2;
        u64 v2 = seed + XXH_PRIME64_2;
        u64 v3 = seed + 0;
        u64 v4 = seed - XXH_PRIME64_1;

        do {
            v1 = xxh64_round(v1, xxh_read64(input)); input += 8;
            v2 = xxh64_round(v2, xxh_read64(input)); input += 8;
            v3 = xxh64_round(v3, xxh_read64(input)); input += 8;
            v4 = xxh64_round(v4, xxh_read64(input)); input += 8;
        } while (input <= limit);

        h64 = xxh_rotl64(v1, 1) + xxh_rotl64(v2, 7) + xxh_rotl64(v3, 12) + xxh_rotl64(v4, 18);
        h64 = xxh64_mergeRound(h64, v1);
        h64 = xxh64_mergeRound(h64, v2);
        h64 = xxh64_mergeRound(h64, v3);
        h64 = xxh64_mergeRound(h64, v4);

    }
    else {
        h64 = seed + XXH_PRIME64_5;
    }

    h64 += (u64)length;

    return xxh64_finalize(h64, input, length);
}

u32 hash32(const char* input, u64 length, u32 seed)
{
	return (u32)xxh64_endian_align(input, length, seed);
}
