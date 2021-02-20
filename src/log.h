#pragma once

#include "macros.h"

#define _LINE_TOK(LINE) #LINE
#define _LINE_TOK2(LINE) _LINE_TOK(LINE)
#define FILE_LINE "[" __FILE__ ":" _LINE_TOK2(__LINE__) "] "

typedef enum {
    LOG_TYPE_INFO,
    LOG_TYPE_DEBUG,
    LOG_TYPE_ERROR
} log_type_t;

void log_init(const char* file_name);
void log_quit(void);
void log_print(log_type_t log_type, const char* message, ...);

#define log_info(...) log_print(LOG_TYPE_INFO, __VA_ARGS__)
#define log_error(...) log_print(LOG_TYPE_ERROR, FILE_LINE, __VA_ARGS__)

#ifdef _DEBUG
#define log_debug(...) log_print(LOG_TYPE_DEBUG, FILE_LINE __VA_ARGS__)
#else
#define log_debug(...) ((void)0)
#endif