#include "resource.h"
#include "render.h"
#include "../core/hash.h"
#include "../core/os.h"
#include "../core/log.h"
#include "../core/atomic.h"
#include "volk.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#ifdef _DEBUG
#define VK_CHECK(expr) do { VkResult _res = (expr); assert(_res == VK_SUCCESS); } while(0)
#else
#define VK_CHECK(expr) do { (expr); } while(0)
#endif

enum
{
	MAX_REQUESTS = 4096,
	BUFFER_COUNT = 2,
	BUFFER_SIZE = 8 * 1024 * 1024 // 8 MiB
};

typedef struct resource_request_t
{
	u64 wait_index;
	resource_request_type_t type;

	union
	{
		const char* texture_name;
	};
} resource_request_t;

static volatile bool run;

static thread_t thread;
static mutex_t queue_mutex;
static mutex_t token_mutex;
static condition_t queue_cond;
static condition_t token_cond;

static u64 current_tokens[3];
static atomic64_t token_completed;
static atomic64_t token_counter;

static u32 request_count;
static resource_request_t request_queue[MAX_REQUESTS];
static resource_request_t active_requests[MAX_REQUESTS];

static u8 transfer_queue_index;
static VkQueue transfer_queue;

VkDeviceSize device_local_memory_offset;
VkDeviceSize host_visible_memory_offset;
static VkDeviceMemory device_local_memory;
static VkDeviceMemory host_visible_memory;

static i32 next_set;
static VkFence transfer_complete_fences[BUFFER_COUNT];
static VkCommandPool command_pools[BUFFER_COUNT];
static VkCommandBuffer command_buffers[BUFFER_COUNT];
static VkBuffer buffers[BUFFER_COUNT];
static VkDeviceSize allocated_size[BUFFER_COUNT];

static u32 loader_function(void* data);
static void create_buffer(VkDeviceSize size, VkBuffer* buffer);
static void create_staging_buffer(VkDeviceSize size, VkBuffer* buffer);
static void load_texture(u32 set, const char* file_name);

void resource_init(void)
{
	run = true;

	queue_mutex = os_create_mutex();
	token_mutex = os_create_mutex();
	queue_cond = os_create_condition();
	token_cond = os_create_condition();

	VkDevice device = get_vulkan_device();

	transfer_queue_index = get_queue_family_index(QUEUE_TRANSFER);
	vkGetDeviceQueue(device, transfer_queue_index, 0, &transfer_queue);

	VkFenceCreateInfo fence_info = {
		.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
		.flags = VK_FENCE_CREATE_SIGNALED_BIT
	};
	VkCommandPoolCreateInfo command_pool_info = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
		.queueFamilyIndex = transfer_queue_index,
		.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT
	};

	for (u32 i = 0; i < BUFFER_COUNT; i++)
	{
		VK_CHECK(vkCreateFence(device, &fence_info, NULL, &transfer_complete_fences[i]));
		VK_CHECK(vkCreateCommandPool(device, &command_pool_info, NULL, &command_pools[i]));

		VkCommandBufferAllocateInfo allocate_info = {
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
			.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
			.commandBufferCount = 1,
			.commandPool = command_pools[i]
		};
		VK_CHECK(vkAllocateCommandBuffers(device, &allocate_info, &command_buffers[i]));

		create_buffer(BUFFER_SIZE, &buffers[i]);
	}

	thread = os_create_thread(loader_function, NULL);
}

void resource_quit(void)
{
	run = false;

	os_condition_wake_single(queue_cond);
	os_wait_thread(thread);

	os_destroy_mutex(queue_mutex);
	os_destroy_mutex(token_mutex);
}

void load_texture_file(const char* file_name, texture_handle_t* texture_handle, sync_token_t* sync_token)
{
	assert(file_name);
	assert(texture_handle);

	os_lock_mutex(queue_mutex);

	sync_token_t token = atomic64_add(&token_counter, 1) + 1;

	request_queue[request_count++] = (resource_request_t){
		.wait_index = token,
		.texture_name = file_name
	};

	os_unlock_mutex(queue_mutex);
	os_condition_wake_single(queue_cond);

	if (sync_token) 
		*sync_token = max(token, *sync_token);
}

static u32 loader_function(void* data)
{
	(void)data;
	u64 max_token = 0;
	VkDevice device = get_vulkan_device();

	while (run)
	{
		os_lock_mutex(queue_mutex);

		// Check for pending tokens
		bool all_tokens_signaled = (token_completed == atomic64_load(&token_counter));

		while ((request_count == 0) && all_tokens_signaled && run)
		{
			// Wait until there's a request
			os_wait_condition(queue_cond, queue_mutex);
		}

		os_unlock_mutex(queue_mutex);

		// Use next set, but make sure it's done transfering
		next_set = (next_set + 1) % BUFFER_COUNT;
		VK_CHECK(vkWaitForFences(device, 1, &transfer_complete_fences[next_set], VK_TRUE, UINT64_MAX));
		VK_CHECK(vkResetFences(device, 1, &transfer_complete_fences[next_set]));
		allocated_size[next_set] = 0;

		// Signal pending tokens from previous frames
		os_lock_mutex(token_mutex);
		atomic64_store(&token_completed, current_tokens[next_set]);
		os_unlock_mutex(token_mutex);
		os_condition_wake_all(token_cond);

		if (request_count > 0)
		{
			// Use a copy of requests for thread safety
			os_lock_mutex(queue_mutex);
			u32 active_request_count = request_count;
			size_t size = sizeof(resource_request_t) * active_request_count;
			memcpy(active_requests, request_queue, size);
			os_unlock_mutex(queue_mutex);

			for (u32 i = 0; i < active_request_count; ++i)
			{
				resource_request_t* request = &request_queue[i];

				switch (request->type)
				{
					case RESOURCE_REQUEST_LOAD_TEXTURE:
						load_texture(next_set, request->texture_name);
						break;
				}

				if (request->wait_index > 0)
				{
					assert(max_token < request->wait_index);
					max_token = request->wait_index;
				}
			}
		}

		u64 last_token = atomic64_load(&token_completed);
		u64 next_token = max(max_token, last_token);
		current_tokens[next_set] = next_token;
	}

	VK_CHECK(vkQueueWaitIdle(transfer_queue));
	for (u32 i = 0; i < BUFFER_COUNT; i++)
	{
		vkDestroyFence(device, transfer_complete_fences[i], NULL);
		vkDestroyCommandPool(device, command_pools[i], NULL);
		vkDestroyBuffer(device, buffers[i], NULL);
	}
	vkFreeMemory(device, host_visible_memory, NULL);
	vkFreeMemory(device, device_local_memory, NULL);

	return 0;
}

static void load_texture(u32 set, const char* file_name)
{
	FILE* file = fopen(file_name, "rb");
	assert(file);

	fseek(file, 0, SEEK_END);
	i64 file_size = ftell(file);
	assert(file_size > 0);
	rewind(file);

	// Read KTX data

	// Create texture

	// Update texture

	fclose(file);
}