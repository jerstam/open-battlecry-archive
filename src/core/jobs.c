#include "jobs.h"
#include "os.h"
#include "log.h"

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

static volatile bool run;
static thread_t threads[MAX_THREADS];
static u32 thread_count;
static u32 idle_thread_count;
static u32 begin, end;
static mutex_t queue_mutex;
static condition_t queue_cond;
static condition_t idle_cond;

static job_t jobs[MAX_JOBS];
static u32 job_count;

static u32 worker_function(void* data)
{
	char name[10];
	u32 index = (u32)(uintptr_t)data;
	snprintf(name, 10, "Worker %d", index);
	log_info("Started worker thread %s", name);

//	TracyCSetThreadName(name);

	while (run)
	{
//		TracyCZoneN(ctx, "Worker Update", true);
		os_lock_mutex(queue_mutex);
		idle_thread_count++;
		while (run && begin == end)
		{
			os_wake_all_condition(idle_cond);
			os_wait_condition(queue_cond, queue_mutex);
		}
		idle_thread_count--;
		if (begin != end)
		{
			job_t* job = &jobs[end];
			if (job->start + 1 == job->end)
			{
				end++;
				end = end % MAX_JOBS;
			}
			else
			{
				job->start++;
			}
			os_unlock_mutex(queue_mutex);
			job->function(job->data, job->start);
		}
		else
		{
			os_unlock_mutex(queue_mutex);
		}
//		TracyCZoneEnd(ctx);
	}

	os_lock_mutex(queue_mutex);
	idle_thread_count++;
	os_wake_all_condition(idle_cond);
	os_unlock_mutex(queue_mutex);

    log_info("Stopped worker thread %s", name);

	return 0;
}

void jobs_init(void)
{
//	TracyCSetThreadName("Main");

	thread_count = max(1, os_cpu_count() - 2); // CPUs minus main and resource thread

	queue_mutex = os_create_mutex();
	os_init_condition(queue_cond);
    os_init_condition(idle_cond);

	run = true;
	idle_thread_count = 0;
	begin = 0;
	end = 0;

	for (u32 i = 0; i < thread_count; ++i)
	{
		threads[i] = os_create_thread(worker_function, (void*)((u64)i + 1));
	}
}

void jobs_quit(void)
{
	os_lock_mutex(queue_mutex);
	run = false;
	os_unlock_mutex(queue_mutex);
	os_wake_all_condition(queue_cond);
	begin = 0;
	end = 0;

	os_destroy_mutex(queue_mutex);
}

u32 jobs_thread_count(void)
{
	return thread_count;
}

void jobs_add(job_function_t function, void* data, uintptr_t index)
{
	os_lock_mutex(queue_mutex);
	jobs[begin++] = (job_t) {
		.function = function,
		.data = data,
		.start = index,
		.end = index + 1
	};
	begin = begin % MAX_JOBS;
	if (begin == end) {
        log_error("Maximum number of jobs reached: begin(%d), end(%d), max(%d)", begin, end, MAX_JOBS);
		assert(false);
	}
	os_unlock_mutex(queue_mutex);
	os_wake_all_condition(queue_cond);
}

void jobs_add_count(job_function_t function, void* data, uintptr_t count)
{
	os_lock_mutex(queue_mutex);
	jobs[begin++] = (job_t){
		.function = function,
		.data = data,
		.start = 0,
		.end = count
	};
	begin = begin % MAX_JOBS;
	if (begin == end) {
        log_error("Maximum number of jobs reached: begin(%d), end(%d), max(%d)", begin, end, MAX_JOBS);
		assert(false);
	}
	os_unlock_mutex(queue_mutex);
	os_wake_all_condition(queue_cond);
}

void jobs_wait_idle(void)
{
	os_lock_mutex(queue_mutex);
	while ((begin != end || idle_thread_count < thread_count) && run)
	{
		os_wait_condition(idle_cond, queue_mutex);
	}
	os_unlock_mutex(queue_mutex);
}
