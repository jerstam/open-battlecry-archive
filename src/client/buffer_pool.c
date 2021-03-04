#include "buffer_pool.h"
#include "volk.h"

#include <assert.h>

static VkDevice _device;

void buffer_pool_init(VkDevice device)
{
	assert(device != VK_NULL_HANDLE);

	_device = device;
}

void buffer_pool_quit(void)
{
}
