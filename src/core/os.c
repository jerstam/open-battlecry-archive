#include "os.h"

#if defined(PLATFORM_WINDOWS)
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#endif
#include <assert.h>

enum
{
    MAX_MUTEXES = 256,
    MAX_CONDITIONS = 256,

    DEFAULT_SPIN_COUNT = 1500
};

static i64 timer_frequency;

static u8 thread_count;
static u8 mutex_count;
static u8 condition_count;
static HANDLE threads[MAX_THREADS];
static CRITICAL_SECTION mutexes[MAX_MUTEXES];
static CONDITION_VARIABLE conditions[MAX_CONDITIONS];

void os_init(void)
{
    LARGE_INTEGER frequency;
    if (QueryPerformanceFrequency(&frequency))
    {
        timer_frequency = frequency.QuadPart;
    }
    else
    {
        timer_frequency = 1000LL;
    }
}

i64 os_tick(void)
{
    LARGE_INTEGER counter;
    QueryPerformanceCounter(&counter);
    return counter.QuadPart * (i64)1e6 / timer_frequency;
}

i64 os_time_frequency(void)
{
    return timer_frequency;
}

thread_t os_create_thread(thread_function_t function, void* data)
{
    assert(thread_count < MAX_THREADS);
    HANDLE handle = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)function, data, 0, 0);
    assert(handle);

    u8 index = ++thread_count;
    threads[index] = handle;
    return index;
}

void os_destroy_thread(thread_t thread)
{
    assert(thread != INVALID_THREAD);
    HANDLE handle = threads[thread];
    assert(handle);

    WaitForSingleObject(handle, INFINITE);
    CloseHandle(handle);

    u8 last = --thread_count;
    threads[thread] = threads[last];
}

void os_wait_thread(thread_t thread)
{
    assert(thread != INVALID_THREAD);
    HANDLE handle = threads[thread];
    assert(handle);
    WaitForSingleObject(handle, INFINITE);
}

mutex_t os_create_mutex()
{
    assert(mutex_count < MAX_MUTEXES);
    CRITICAL_SECTION handle;
    if (!InitializeCriticalSectionAndSpinCount(&handle, DEFAULT_SPIN_COUNT))
        assert(0);

    u8 index = ++mutex_count;
    mutexes[index] = handle;
    return index;
}

void os_destroy_mutex(mutex_t mutex)
{
    assert(mutex != INVALID_MUTEX);
    CRITICAL_SECTION handle = mutexes[mutex];
    DeleteCriticalSection(&handle);

    u8 last = --mutex_count;
    mutexes[mutex] = mutexes[last];
}

void os_lock_mutex(mutex_t mutex)
{
    assert(mutex != INVALID_MUTEX);
    CRITICAL_SECTION handle = mutexes[mutex];
    EnterCriticalSection(&handle);
}

void os_unlock_mutex(mutex_t mutex)
{
    assert(mutex != INVALID_MUTEX);
    CRITICAL_SECTION handle = mutexes[mutex];
    LeaveCriticalSection(&handle);
}

condition_t os_create_condition(void)
{
    assert(condition_count < MAX_CONDITIONS);

    CONDITION_VARIABLE handle;
    InitializeConditionVariable(&handle);

    u8 index = ++condition_count;
    conditions[index] = handle;
    return index;
}

void os_wait_condition(condition_t condition, mutex_t mutex)
{
    assert(condition != INVALID_CONDITION);
    assert(mutex != INVALID_MUTEX);
    CONDITION_VARIABLE condition_handle = conditions[condition];
    CRITICAL_SECTION mutex_handle = mutexes[mutex];
    SleepConditionVariableCS(&condition_handle, &mutex_handle, UINT32_MAX);
}

void os_condition_wake_single(condition_t condition)
{
    assert(condition != INVALID_CONDITION);
    CONDITION_VARIABLE handle = conditions[condition];
    WakeConditionVariable(&handle);
}

void os_condition_wake_all(condition_t condition)
{
    assert(condition != INVALID_CONDITION);
    CONDITION_VARIABLE handle = conditions[condition];
    WakeAllConditionVariable(&handle);
}

u32 os_cpu_count(void)
{
    SYSTEM_INFO system_info;
    GetSystemInfo(&system_info);
    return system_info.dwNumberOfProcessors;
}

void os_sleep(u32 ms)
{
    Sleep(ms);
}
