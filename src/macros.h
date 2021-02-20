#pragma once

#include <stdbool.h>

#if defined(_WIN32) || defined(_WIN64) || defined(__CYGWIN__)
#define PLATFORM_WINDOWS 1
#elif defined(__APPLE__)
#define PLATFORM_MAC 1
#elif defined(__linux__)
#define PLATFORM_LINUX 1
#else
#error Unable to detect current platform
#endif

#if PLATFORM_WINDOWS
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#endif

#ifdef _DEBUG
    #define TRACY_ENABLE 1
#endif

typedef signed char i8;
typedef signed short i16;
typedef signed int i32;
typedef signed long word;
typedef signed long long i64;
typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long dword;
typedef unsigned long long u64;

static_assert(sizeof(i8) == 1, "sizeof(i8) must be 1");
static_assert(sizeof(u8) == 1, "sizeof(u8) must be 1");
static_assert(sizeof(i16) == 2, "sizeof(i16) must be 2");
static_assert(sizeof(u16) == 2, "sizeof(u16) must be 2");
static_assert(sizeof(i32) == 4, "sizeof(i32) must be 4");
static_assert(sizeof(u32) == 4, "sizeof(u32) must be 4");
static_assert(sizeof(i64) == 8, "sizeof(i64) must be 8");
static_assert(sizeof(u64) == 8, "sizeof(u64) must be 8");

#define INT8_MIN         (-127i8 - 1)
#define INT16_MIN        (-32767i16 - 1)
#define INT32_MIN        (-2147483647i32 - 1)
#define INT64_MIN        (-9223372036854775807i64 - 1)
#define INT8_MAX         127i8
#define INT16_MAX        32767i16
#define INT32_MAX        2147483647i32
#define INT64_MAX        9223372036854775807i64
#define UINT8_MAX        0xffui8
#define UINT16_MAX       0xffffui16
#define UINT32_MAX       0xffffffffui32
#define UINT64_MAX       0xffffffffffffffffui64

enum
{
	MAX_THREADS = 256
};

typedef struct { u16 index; u16 generation; } handle_t;

#define array_length(x) ( sizeof(x) / sizeof((x)[0]) )

typedef float vec2_t[2];

#define dot(x, y) (x[0] * y[0] + x[1] * y[1])
#define add(x, y, result) (result[0] = x[0] + y[0], result[1] = x[1] + y[1])