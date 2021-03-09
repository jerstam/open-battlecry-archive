#pragma once

#include "common.h"

#define INVALID_THREAD 0
#define INVALID_MUTEX 0

typedef u8 thread_t;
typedef u8 fiber_t;
typedef u8 mutex_t;
typedef u8 condition_t;

typedef u32 (*thread_function_t)(void*);

void os_init(void);

i64 os_tick(void);
i64 os_time_frequency(void);

thread_t os_create_thread(thread_function_t function, void* data);
void os_destroy_thread(thread_t thread);
void os_wait_thread(thread_t thread);

mutex_t os_create_mutex(void);
void os_destroy_mutex(mutex_t mutex);
void os_lock_mutex(mutex_t mutex);
void os_unlock_mutex(mutex_t mutex);

void os_init_condition(condition_t condition);
void os_wait_condition(condition_t condition, mutex_t mutex);
void os_wake_condition(condition_t condition);
void os_wake_all_condition(condition_t condition);

u32 os_cpu_count(void);
void os_sleep(u32 ms);