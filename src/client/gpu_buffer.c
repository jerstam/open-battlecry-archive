//#include "gpu.h"
//#define VK_NO_PROTOTYPES
//#include "volk.h"
//
//#include <string.h>
//#include <assert.h>
//
//void gpu_create_buffer(u32 size, buffer_flags_t buffer_flags, buffer_t* buffer)
//{
//	assert(buffer);
//	assert(size > 0);
//
//	buffer->size = size;
//
//	VkBufferUsageFlagBits buffer_usage = 0;
//	if ((buffer_flags & BUFFER_VERTEX) != 0)
//		buffer_usage |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
//	if ((buffer_flags & BUFFER_INDEX) != 0)
//		buffer_usage |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
//	if ((buffer_flags & BUFFER_UNIFORM) != 0)
//		buffer_usage |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
//	if ((buffer_flags & BUFFER_STORAGE) != 0)
//		buffer_usage |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
//	if ((buffer_flags & BUFFER_INDIRECT) != 0)
//		buffer_usage |= VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;
//
//	VkBufferCreateInfo buffer_info = {
//		.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
//		.size = size,
//		.usage = buffer_usage,
//		.sharingMode = VK_SHARING_MODE_EXCLUSIVE
//	};
//
//	buffer->persistent = (buffer_flags & BUFFER_PERSISTENT);
//
//	VmaAllocationCreateFlags flags = buffer->persistent ? VMA_ALLOCATION_CREATE_MAPPED_BIT : 0;
//
//#ifdef VK_USE_PLATFORM_MACOS_MVK
//	flags &= ~VMA_ALLOCATION_CREATE_MAPPED_BIT;
//	buffer->persistent = false;
//#endif
//
//	VmaMemoryUsage memory_usage = 0;
//	if ((buffer_flags & BUFFER_CPU) != 0)
//		memory_usage |= VMA_MEMORY_USAGE_CPU_TO_GPU;
//	else
//		memory_usage |= VMA_MEMORY_USAGE_GPU_ONLY;
//
//	VmaAllocationCreateInfo allocation_create_info = {
//		.usage = memory_usage,
//		.flags = flags
//	};
//
//	VmaAllocationInfo allocation_info = { 0 };
//	VK_CHECK(vmaCreateBuffer(gpu.allocator, &buffer_info, &allocation_create_info, &buffer->buffer, &buffer->allocation, &allocation_info));
//
//	if (buffer->persistent)
//	{
//		buffer->data = allocation_info.pMappedData;
//	}
//}
//
//void gpu_destroy_buffer(buffer_t* buffer)
//{
//	assert(buffer);
//
//	buffer->size = 0;
//
//	vmaDestroyBuffer(gpu.allocator, buffer->buffer, buffer->allocation);
//}
//
//static void update_buffer(buffer_t* buffer, const u8* data, const u32 size, const u32 offset)
//{
//	if (buffer->persistent)
//	{
//		memcpy(buffer->data, data + offset, size);
//		VK_CHECK(vmaFlushAllocation(gpu.allocator, buffer->allocation, offset, size));
//	}
//	else
//	{
//		VK_CHECK(vmaMapMemory(gpu.allocator, buffer->allocation, &buffer->data));
//		memcpy(buffer->data, data + offset, size);
//		VK_CHECK(vmaFlushAllocation(gpu.allocator, buffer->allocation, offset, size));
//		vmaUnmapMemory(gpu.allocator, buffer->allocation);
//	}
//}