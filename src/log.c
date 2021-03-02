#include "log.h"
#include "os.h"

#if defined(PLATFORM_WINDOWS)
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#endif
#include <stdio.h>
#include <assert.h>

#define COLOR_NORMAL     "\x1B[0m"
#define COLOR_RED        "\x1B[31m"
#define COLOR_GREEN      "\x1B[32m"
#define COLOR_YELLOW     "\x1B[33m"
#define COLOR_BLUE       "\x1B[34m"
#define COLOR_MAGENTA    "\x1B[35m"
#define COLOR_CYAN       "\x1B[36m"
#define COLOR_WHITE      "\x1B[37m"
#define COLOR_RESET      "\x1B[0m"

enum
{
    CHANGE_BUFFER_SIZE = 4096
};

typedef struct date_t
{
    u16 year;
    u8 month;
    u8 day;
    u8 hour;
    u8 minute;
    u8 second;
    u8 millisecond;
} date_t;

static date_t date;
static mutex_t mutex;

void log_init(const char* file_name)
{
    (void)file_name;
    mutex = os_create_mutex();
}

void log_quit(void)
{
    os_destroy_mutex(mutex);
}

static void get_date()
{
    SYSTEMTIME time;
    GetSystemTime(&time);
    date.year = (u8)time.wYear;
    date.month = (u8)time.wMonth;
    date.day = (u8)time.wDay;
    date.hour = (u8)time.wHour;
    date.minute = (u8)time.wMinute;
    date.second = (u8)time.wSecond;
    date.millisecond = (u8)time.wMilliseconds;
}

static const char* get_tag(log_type_t log_type)
{
    switch (log_type)
    {
        case LOG_TYPE_INFO: return "[INFO]";
        case LOG_TYPE_DEBUG: return "[DEBUG]";
        case LOG_TYPE_ERROR: return "[ERROR]";
    }
    return "[INFO]";
}

static const char* get_color(log_type_t log_type)
{
    switch (log_type)
    {
        case LOG_TYPE_INFO: return COLOR_NORMAL;
        case LOG_TYPE_DEBUG: return COLOR_BLUE;
        case LOG_TYPE_ERROR: return COLOR_RED;
    }
    return COLOR_NORMAL;
}

static u32 get_thread_id(void)
{
    return (u32)GetCurrentThreadId();
}

void log_print(log_type_t log_type, const char* message, ...)
{
    assert(mutex != INVALID_MUTEX);
    os_lock_mutex(mutex);

    char prefix[32];
    snprintf(prefix, sizeof(prefix), "%s (tid:%u)", get_tag(log_type), get_thread_id());

    va_list args;
    va_start(args, message);
    char input[CHANGE_BUFFER_SIZE];
    vsnprintf(input, sizeof(input), message, args);
    va_end(args);

    char console_output[CHANGE_BUFFER_SIZE];
    snprintf(console_output, sizeof(console_output), "%s - %s\n", prefix, input);

    fprintf(log_type == LOG_TYPE_ERROR ? stderr : stdout, "%s", console_output);

    if (IsDebuggerPresent())
    {
        fflush(stdout);
    }

    // TODO: Log to file
//    char date_string[32];
//    get_date();
//    snprintf(date_string, sizeof(date_string), "%04d.%02d.%02d-%02d:%02d:%02d.%02d", date.year,
//             date.month, date.day, date.hour, date.minute, date.second, date.millisecond);

    //char file_output[BUFFER_SIZE];
    //snprintf(file_output, sizeof(file_output), )

    //OutputDebugString(console_output);

    os_unlock_mutex(mutex);
}
