#include "bc7.h"

#if _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#endif
#include "volk.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>

#include "bc7_encode.h"
#include "bc7_try_mode_456.h"

#ifdef _DEBUG
#define VK_CHECK(expr) do { VkResult _res = (expr); assert(_res == VK_SUCCESS); } while(0)
#else
#define VK_CHECK(expr) do { (expr); } while(0)
#endif

#define ARRAY_LENGTH(x) ( sizeof(x) / sizeof((x)[0]) )

enum
{
	DEVICE_LOCAL_MEMORY_SIZE = 1024 * 1024 * 20,
	HOST_VISIBLE_MEMORY_SIZE = 1024 * 1024 * 20
};

typedef struct bc7_buffer
{
	uint32_t color[4];
} bc7_buffer;

typedef struct bc7_constants
{
	uint32_t tex_width;
	uint32_t num_block_x;
	uint32_t format;
	uint32_t mode_id;
	uint32_t start_block_id;
	uint32_t num_total_blocks;
	float alpha_weight;
	uint32_t reserved;
} bc7_constants;

static VkInstance instance;
static VkDebugUtilsMessengerEXT debug_messenger;
static VkPhysicalDevice physical_device;
static VkDevice device;
static uint32_t queue_family_count;
static uint32_t compute_queue_index;
static VkQueue compute_queue;

static VkDeviceMemory device_local_memory;
static VkDeviceMemory host_visible_memory;

static VkShaderModule encode_shader;
static VkShaderModule try_mode_456_shader;

static VkPipelineCache pipeline_cache;
static VkPipelineLayout pipeline_layout;
static VkPipeline encode_pipeline;
static VkPipeline try_mode_456_pipeline;

static VkCommandPool command_pool;
static VkCommandBuffer command_buffer;

static VkDescriptorSetLayout descriptor_set_layout;
static VkDescriptorPool descriptor_pool;
static VkDescriptorSet descriptor_set;

static VkImage input_image;
static VkBuffer input_buffer;
static VkBuffer output_buffer;

static VkBool32 VKAPI_PTR debug_callback(
	VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType,
	const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)
{
	(void)messageType;
	(void)pUserData;
	int32_t message_id_number = pCallbackData->messageIdNumber;
	const char* message_id_name = pCallbackData->pMessageIdName;
	const char* message = pCallbackData->pMessage;

	if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT)
	{
		printf("%i - %s: %s", message_id_number, message_id_name, message);
	}
	else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
	{
		printf("%i - %s: %s", message_id_number, message_id_name, message);
	}
	else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
	{
		printf("%i - %s: %s", message_id_number, message_id_name, message);
	}

	return VK_FALSE;
}

static void create_instance()
{
	VkApplicationInfo application_info = {
		.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
		.pApplicationName = "Asset Packer",
		.applicationVersion = VK_MAKE_VERSION(1, 0, 0),
		.engineVersion = VK_MAKE_VERSION(1, 0, 0),
		.apiVersion = VK_API_VERSION_1_2
	};

#ifdef _DEBUG
	const char* instance_extensions[] = {
		VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
	};
	uint32_t extension_count = 0;
	extension_count = ARRAY_LENGTH(instance_extensions);

	const char* validation_layers[] =
	{
		"VK_LAYER_KHRONOS_validation"
	};

	VkDebugUtilsMessengerCreateInfoEXT debug_info = {
		.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
		.pfnUserCallback = debug_callback,
		.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
		.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT
	};
#endif

	VkInstanceCreateInfo instance_info = {
		.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
		.pApplicationInfo = &application_info,
#ifdef _DEBUG
		.enabledExtensionCount = extension_count,
		.ppEnabledExtensionNames = instance_extensions,
		.pNext = &debug_info,
		.enabledLayerCount = ARRAY_LENGTH(validation_layers),
		.ppEnabledLayerNames = validation_layers
#endif
	};

	VK_CHECK(vkCreateInstance(&instance_info, NULL, &instance));

	volkLoadInstanceOnly(instance);

#ifdef _DEBUG
	VK_CHECK(vkCreateDebugUtilsMessengerEXT(instance, &debug_info, NULL, &debug_messenger));
#endif
}

static void find_queue_family(VkQueueFlags queue, VkQueueFamilyProperties* queue_family_properties, uint32_t* queue_family)
{
	VkQueueFlags min_flags = ~0u;
	uint32_t best_family = UINT32_MAX;
	for (uint32_t i = 0; i < queue_family_count; i++)
	{
		if ((queue_family_properties[i].queueFlags & queue) == queue)
		{
			if (queue_family_properties[i].queueFlags < min_flags)
			{
				min_flags = queue_family_properties[i].queueFlags;
				best_family = i;
			}
		}
	}
	*queue_family = best_family;
}

static void create_device()
{
	vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, NULL);
	VkQueueFamilyProperties queue_family_properties[4];
	vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, queue_family_properties);

	find_queue_family(VK_QUEUE_COMPUTE_BIT, queue_family_properties, &compute_queue_index);

	uint32_t queue_info_count = 0;
	VkDeviceQueueCreateInfo queue_infos[4] = { 0 };
	float queue_priorities[1] = { 0.0f };

	VkDeviceQueueCreateInfo queue_info = { VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO };
	queue_info.queueCount = 1;
	queue_info.pQueuePriorities = queue_priorities;

	for (queue_info_count = 0; queue_info_count < queue_family_count; queue_info_count++)
	{
		queue_info.queueFamilyIndex = queue_info_count;
		queue_infos[queue_info_count] = queue_info;
	}

	VkDeviceCreateInfo device_info = {
		.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
		.queueCreateInfoCount = queue_info_count,
		.pQueueCreateInfos = queue_infos
	};
	VK_CHECK(vkCreateDevice(physical_device, &device_info, NULL, &device));

	volkLoadDevice(device);

	vkGetDeviceQueue(device, compute_queue_index, 0, &compute_queue);
}

static void allocate_memory()
{
	VkPhysicalDeviceMemoryProperties memory_properties;
	vkGetPhysicalDeviceMemoryProperties(physical_device, &memory_properties);

	bool device_local_found = false;
	bool host_visible_found = false;
	for (uint32_t i = 0; i < memory_properties.memoryTypeCount; ++i)
	{
		if (!device_local_found && (memory_properties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) != 0)
		{
			VkMemoryAllocateInfo allocate_info = {
				.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
				.allocationSize = DEVICE_LOCAL_MEMORY_SIZE,
				.memoryTypeIndex = i
			};

			VK_CHECK(vkAllocateMemory(device, &allocate_info, NULL, &device_local_memory));
			device_local_found = true;
		}

		if (!host_visible_found && (memory_properties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) != 0)
		{
			VkMemoryAllocateInfo allocate_info = {
				.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
				.allocationSize = HOST_VISIBLE_MEMORY_SIZE,
				.memoryTypeIndex = i
			};

			VK_CHECK(vkAllocateMemory(device, &allocate_info, NULL, &host_visible_memory));

			host_visible_found = true;
		}
	}
}

static void create_descriptor_set()
{
	const VkDescriptorSetLayoutBinding bindings[] = {
		{
			.binding = 0,
			.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
			.descriptorCount = 1,
			.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT
		},
		{
			.binding = 1,
			.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
			.descriptorCount = 1,
			.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT
		},
		{
			.binding = 2,
			.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
			.descriptorCount = 1,
			.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT
		},
	};

	VkDescriptorSetLayoutCreateInfo layout_info = {
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
		.bindingCount = ARRAY_LENGTH(bindings),
		.pBindings = bindings
	};
	VK_CHECK(vkCreateDescriptorSetLayout(device, &layout_info, NULL, &descriptor_set_layout));

	VkDescriptorPoolSize pool_sizes[] =
	{
		{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 2 }
	};

	VkDescriptorPoolCreateInfo descriptor_pool_info = {
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
		.poolSizeCount = ARRAY_LENGTH(pool_sizes),
		.pPoolSizes = pool_sizes,
		.maxSets = 1
	};
	VK_CHECK(vkCreateDescriptorPool(device, &descriptor_pool_info, NULL, &descriptor_pool));

	VkDescriptorSetAllocateInfo allocate_info = {
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
		.descriptorPool = descriptor_pool,
		.descriptorSetCount = 1,
		.pSetLayouts = &descriptor_set_layout
	};
	VK_CHECK(vkAllocateDescriptorSets(device, &allocate_info, &descriptor_set));
}

static void create_pipeline_cache()
{
	FILE* file = fopen("pipeline.cache", "rb");
	uint64_t size = 0;
	void* data = NULL;
	if (file)
	{
		fseek(file, 0L, SEEK_END);
		size = ftell(file);
		if (size > 0)
		{
			rewind(file);
			data = malloc(size);
			assert(data);

			fread(data, size, 1, file);
		}
		fclose(file);
	}

	VkPipelineCacheCreateInfo pipeline_cache_info = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO,
		.initialDataSize = size,
		.pInitialData = data
	};
	VK_CHECK(vkCreatePipelineCache(device, &pipeline_cache_info, NULL, &pipeline_cache));

	if (data)
	{
		free(data);
	}
}

static void destroy_pipeline_cache()
{
	uint64_t pipeline_cache_size = 0;
	VK_CHECK(vkGetPipelineCacheData(device, pipeline_cache, &pipeline_cache_size, NULL));
	uint8_t* pipeline_cache_data = malloc(pipeline_cache_size);
	assert(pipeline_cache_data);
	VK_CHECK(vkGetPipelineCacheData(device, pipeline_cache, &pipeline_cache_size, pipeline_cache_data));
	if (pipeline_cache_size > 0)
	{
		FILE* file = fopen("pipeline.cache", "wb");
		assert(file);
		fwrite(pipeline_cache_data, pipeline_cache_size, 1, file);
		fclose(file);
	}
	free(pipeline_cache_data);
	vkDestroyPipelineCache(device, pipeline_cache, NULL);
}

static void create_pipelines()
{
	VkShaderModuleCreateInfo shader_info = {
		.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
		.codeSize = sizeof(bc7_encode_spirv),
		.pCode = bc7_encode_spirv
	};
	VK_CHECK(vkCreateShaderModule(device, &shader_info, NULL, &encode_shader));

	shader_info.codeSize = sizeof(bc7_try_mode_456_spirv);
	shader_info.pCode = bc7_try_mode_456_spirv;
	VK_CHECK(vkCreateShaderModule(device, &shader_info, NULL, &try_mode_456_shader));

	VkPipelineShaderStageCreateInfo shader_stage = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
		.stage = VK_SHADER_STAGE_COMPUTE_BIT,
		.module = encode_shader,
		.pName = "main"
	};

	VkPushConstantRange push_constant_range = {
		.size = sizeof(bc7_constants),
		.offset = 0,
		.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT
	};

	VkPipelineLayoutCreateInfo layout_info = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
		.setLayoutCount = 1,
		.pSetLayouts = &descriptor_set_layout,
		.pushConstantRangeCount = 1,
		.pPushConstantRanges = &push_constant_range
	};
	VK_CHECK(vkCreatePipelineLayout(device, &layout_info, NULL, &pipeline_layout));

	VkComputePipelineCreateInfo compute_info = {
		.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
		.stage = shader_stage,
		.layout = pipeline_layout,
	};
	VK_CHECK(vkCreateComputePipelines(device, pipeline_cache, 1, &compute_info, NULL, &encode_pipeline));

	shader_stage.module = try_mode_456_shader;
	compute_info.stage = shader_stage;
	VK_CHECK(vkCreateComputePipelines(device, pipeline_cache, 1, &compute_info, NULL, &try_mode_456_pipeline));
}

static void create_command_buffer()
{
	VkCommandPoolCreateInfo command_pool_info = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
		.queueFamilyIndex = compute_queue_index,
		.flags = 0
	};
	VK_CHECK(vkCreateCommandPool(device, &command_pool_info, NULL, &command_pool));

	VkCommandBufferAllocateInfo allocate_info = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
		.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
		.commandPool = command_pool,
		.commandBufferCount = 1
	};
	VK_CHECK(vkAllocateCommandBuffers(device, &allocate_info, &command_buffer));

	VkCommandBufferBeginInfo begin_info = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
	};
	VK_CHECK(vkBeginCommandBuffer(command_buffer, &begin_info));

	vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, encode_pipeline);
	vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout, 0, 1, &descriptor_set, 0, NULL);

	vkCmdDispatch(command_buffer, 1, 1, 1);

	VK_CHECK(vkEndCommandBuffer(command_buffer));
}

void bc7_init(void)
{
	VK_CHECK(volkInitialize());
	create_instance();

	uint32_t physical_device_count = 0;
	VK_CHECK(vkEnumeratePhysicalDevices(instance, &physical_device_count, NULL));
	VkPhysicalDevice physical_devices[4];
	VK_CHECK(vkEnumeratePhysicalDevices(instance, &physical_device_count, physical_devices));
	physical_device = physical_devices[0];

	create_device();
	allocate_memory();
	create_descriptor_set();
	create_pipeline_cache();
	create_pipelines();
	create_command_buffer();
}

void bc7_quit(void)
{
	vkDestroyCommandPool(device, command_pool, NULL);

	destroy_pipeline_cache();

	vkDestroyPipeline(device, try_mode_456_pipeline, NULL);
	vkDestroyPipeline(device, encode_pipeline, NULL);
	vkDestroyPipelineLayout(device, pipeline_layout, NULL);
	vkDestroyShaderModule(device, encode_shader, NULL);
	vkDestroyShaderModule(device, try_mode_456_shader, NULL);
	vkDestroyImage(device, input_image, NULL);
	vkDestroyBuffer(device, input_buffer, NULL);
	vkDestroyBuffer(device, output_buffer, NULL);
	vkDestroyDescriptorSetLayout(device, descriptor_set_layout, NULL);
	vkDestroyDescriptorPool(device, descriptor_pool, NULL);
	vkFreeMemory(device, host_visible_memory, NULL);
	vkFreeMemory(device, device_local_memory, NULL);
	vkDestroyDevice(device, NULL);
#if _DEBUG
	vkDestroyDebugUtilsMessengerEXT(instance, debug_messenger, NULL);
#endif
	vkDestroyInstance(instance, NULL);
}

void bc7_compresss(uint32_t width, uint32_t height, uint8_t* image_data, uint64_t size)
{
	assert(width > 0);
	assert(height > 0);

	size_t xblocks = max(1, (width + 3) >> 2);
	size_t yblocks = max(1, (height + 3) >> 2);
	size_t num_blocks = xblocks * yblocks;

	// Create buffers
	VkDeviceSize buffer_size = num_blocks * sizeof(uint32_t) * 4;

	VkBufferCreateInfo buffer_info = {
		.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
		.size = buffer_size,
		.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		.sharingMode = VK_SHARING_MODE_EXCLUSIVE
	};
	VK_CHECK(vkCreateBuffer(device, &buffer_info, NULL, &output_buffer));
}
