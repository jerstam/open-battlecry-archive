#pragma once

#include "common.h"

enum
{
	MAX_JOBS = 128
};

typedef void (*job_function_t)(void* data, u64 arg);

typedef struct job_t
{
	job_function_t function;
	void* data;
	u64 start;
	u64 end;
} job_t;

void jobs_init(void);
void jobs_quit(void);
u32 jobs_thread_count(void);
void jobs_add(job_function_t function, void* data, u64 index);
void jobs_add_count(job_function_t function, void* data, u64 count);
void jobs_wait_idle(void);