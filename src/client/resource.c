#include "resource.h"
#include "render.h"
#include "../hash.h"
#include "../os.h"
#include "../log.h"
#include "../atomic.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

enum
{
	CHANGE_BUFFER_SIZE = 4096,
	BUFFER_COUNT = 2
};

typedef struct load_request_t
{
	u64 wait_index;

	union
	{
		const char* texture_name;
	};
} load_request_t;

static volatile bool run;
static thread_t thread;
static mutex_t queue_mutex;
static mutex_t token_mutex;
static condition_t queue_cond;
static condition_t token_cond;
static atomic64_t token_completed;
static atomic64_t token_counter;
static int next_set;
static u64 current_tokens[3];
static load_request_t requests[CHANGE_BUFFER_SIZE];
static u32 request_count;

static int loader_function(void* data)
{
	u64 max_token = 0;

	while (run)
	{
		os_lock_mutex(queue_mutex);

		// Check for pending tokens
		bool all_tokens_sigaled = (token_completed == atomic64_load(&token_counter));

		while ((request_count == 0) && all_tokens_sigaled && run)
		{
			// Wait until there's a request
			os_wait_condition(queue_cond, queue_mutex);
		}

		os_unlock_mutex(queue_mutex);

		next_set = (next_set + 1) % BUFFER_COUNT;

		// Signal pending tokens from previous frames
		os_lock_mutex(token_mutex);
		atomic64_store(&token_completed, current_tokens[next_set]);
		os_unlock_mutex(token_mutex);
		os_wake_all_condition(token_cond);

		u64 completion_mask = 0;

		os_lock_mutex(queue_mutex);

		request_count = 0;

		// Use a copy of requests for thread safety
		size_t size = sizeof(load_request_t) * request_count;
		load_request_t* active_requests = malloc(size);
		assert(active_requests);
		memcpy(active_requests, requests, size);

		os_unlock_mutex(queue_mutex);

		for (u32 i = 0; i < request_count; ++i)
		{
			load_request_t* request = &active_requests[i];

			// Actually load texture
			FILE* file = fopen(request->texture_name, "rb");
			assert(file);

			fseek(file, 0, SEEK_END);
			i64 size = ftell(file);
			rewind(file);

			u8* data = malloc(size);
			fread(data, size, 1, file);

			free(data);
			fclose(file);

			bool completed = true;
			if (request->wait_index && completed)
			{
				assert(max_token < request->wait_index);
				max_token = request->wait_index;
			}
		}

		u64 last_token = atomic64_load(&token_completed);
		u64 next_token = max(max_token, last_token);
		current_tokens[next_set] = next_token;
	}

	return 0;
}

void resource_init(void)
{
	run = true;

	queue_mutex = os_create_mutex();
	token_mutex = os_create_mutex();
	os_init_condition(queue_cond);
	os_init_condition(token_cond);

	thread = os_create_thread(loader_function, NULL);
}

void resource_quit(void)
{
	run = false;

	os_wake_condition(queue_cond);
	os_wait_thread(thread);

	os_destroy_mutex(queue_mutex);
	os_destroy_mutex(token_mutex);
}

void resource_add_buffer(buffer_t* buffer)
{
	assert(buffer);
}

void resource_remove_buffer(buffer_t* buffer)
{
	assert(buffer);
}

void resource_add_image(const char* name, image_t* image)
{
	assert(image);
	os_lock_mutex(queue_mutex);

	u64 token = atomic64_add(&token_counter, 1) + 1;

	requests[request_count++] = (load_request_t){
		.wait_index = token,
		.texture_name = name
	};

	log_debug("Added image %s with token %d", name, token);

	os_unlock_mutex(queue_mutex);
	os_wake_condition(queue_cond);
}

void resource_remove_image(image_t* image)
{
	assert(image);
}
