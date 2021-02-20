#include "gpu.h"
#include "window.h"
#include "../log.h"
#include "../cvar.h"
#define VK_NO_PROTOTYPES
#include "volk.h"

#include <GLFW/glfw3.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

enum
{
	// Descriptors
	MAX_DESCRIPTOR_SETS = 10,
	MAX_SAMPLERS = 2,
	MAX_SAMPLED_IMAGES = 8192,
	MAX_STORAGE_IMAGES = 1,
	MAX_UNIFORM_TEXEL_BUFFERS = 1,
	MAX_STORAGE_TEXEL_BUFFERS = 1,
	MAX_UNIFORM_BUFFERS = 10,
	MAX_STORAGE_BUFFERS = 10,
	MAX_UNIFORM_BUFFER_DYNAMIC = 1
};

static const VkFormat swapchain_format = VK_FORMAT_B8G8R8A8_UNORM;
static const u32 swapchain_image_count = 2;

static VkExtensionProperties available_instance_extensions[32];
static VkExtensionProperties available_device_extensions[256];

gpu_t gpu = {
	.width = 1920,
	.height = 1080,
	.frame_count = 0
};

static cvar_t cv_enable_validation = {
	.name = "gpu_validation",
	.description = "Enables GPU validation (very slow).",
	.type = CVAR_TYPE_BOOL,
	.bool_value = false
};

static cvar_t cv_gpu_index = {
	.name = "gpu_index",
	.description = "The index of the GPU to render with.",
	.type = CVAR_TYPE_INT,
	.int_value = 0,
	.flags = CVAR_SAVE
};

static cvar_t cv_vsync = {
	.name = "vsync",
	.type = CVAR_TYPE_BOOL,
	.bool_value = false,
	.flags = CVAR_SAVE
};

static VkBool32 VKAPI_PTR debug_callback(
	VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType,
	const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)
{
	int32_t message_id_number = pCallbackData->messageIdNumber;
	const char* message_id_name = pCallbackData->pMessageIdName;
	const char* message = pCallbackData->pMessage;

	if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT)
	{
		log_info("%i - %s: %s", message_id_number, message_id_name, message);
	}
	else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
	{
		log_info("%i - %s: %s", message_id_number, message_id_name, message);
	}
	else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
	{
		log_error("%i - %s: %s", message_id_number, message_id_name, message);
	}

	return VK_FALSE;
}

static void create_instance()
{
	VkApplicationInfo application_info = { VK_STRUCTURE_TYPE_APPLICATION_INFO };
	application_info.pApplicationName = "Battlecry";
	application_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	application_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	application_info.apiVersion = VK_API_VERSION_1_2;

	const char* instance_extensions[] = {
		VK_KHR_SURFACE_EXTENSION_NAME,
#if defined(VK_USE_PLATFORM_WIN32_KHR)
		VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
#elif defined(VK_USE_PLATFORM_XLIB_KHR)
		VK_KHR_XLIB_SURFACE_EXTENSION_NAME,
#elif defined(VK_USE_PLATFORM_XCB_KHR)
		VK_KHR_XCB_SURFACE_EXTENSION_NAME,
#elif defined(VK_USE_PLATFORM_ANDROID_KHR)
		VK_KHR_ANDROID_SURFACE_EXTENSION_NAME,
#elif defined(VK_USE_PLATFORM_GGP)
		VK_GGP_STREAM_DESCRIPTOR_SURFACE_EXTENSION_NAME,
#elif defined(VK_USE_PLATFORM_VI_NN)
		VK_NN_VI_SURFACE_EXTENSION_NAME,
#endif
#ifndef NDEBUG
		VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
#endif
	};
	u32 instance_extension_count = sizeof(instance_extensions) / sizeof(instance_extensions[0]);

	u32 available_extension_count = 0;
	VK_CHECK(vkEnumerateInstanceExtensionProperties(NULL, &available_extension_count, NULL));
	VK_CHECK(vkEnumerateInstanceExtensionProperties(NULL, &available_extension_count, available_instance_extensions));
	assert(available_extension_count > 0);

	for (u32 i = 0; i < instance_extension_count; i++)
	{
		bool found = false;
		for (u32 j = 0; j < available_extension_count; j++)
		{
			if (strcmp(instance_extensions[i], available_instance_extensions[j].extensionName) == 0)
			{
				found = true;
				break;
			}
		}
		if (!found)
		{
			log_error("Instance extension not available: %s", instance_extensions[i]);
		}
	}

#ifndef NDEBUG
	const char* validation_layers[] =
	{
		"VK_LAYER_KHRONOS_validation"
	};
#endif

	VkInstanceCreateInfo instance_info = { VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO };
	instance_info.pApplicationInfo = &application_info;
	instance_info.enabledExtensionCount = instance_extension_count;
	instance_info.ppEnabledExtensionNames = instance_extensions;

#ifndef NDEBUG
	VkDebugUtilsMessengerCreateInfoEXT debug_info = {
		.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
		.pfnUserCallback = debug_callback,
		.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
		.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT
	};

	instance_info.pNext = &debug_info;
	instance_info.enabledLayerCount = sizeof(validation_layers) / sizeof(validation_layers[0]);
	instance_info.ppEnabledLayerNames = validation_layers;
#endif

	VK_CHECK(vkCreateInstance(&instance_info, NULL, &gpu.instance));

	volkLoadInstanceOnly(gpu.instance);

#ifndef NDEBUG
	VK_CHECK(vkCreateDebugUtilsMessengerEXT(gpu.instance, &debug_info, NULL, &gpu.debug_messenger));
#endif

	log_info("Vulkan instance created");
}

static void find_queue_family(VkQueueFlags queue, u32 queue_family_count,
	VkQueueFamilyProperties* queue_family_properties, uint8_t* queue_family)
{
	VkQueueFlags min_flags = ~0u;
	u32 best_family = UINT32_MAX;
	for (u32 i = 0; i < queue_family_count; i++)
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
	*queue_family = (uint8_t)best_family;
}

static void create_surface()
{
	VK_CHECK(glfwCreateWindowSurface(gpu.instance, window_get_handle(), NULL, &gpu.surface));

	log_info("Vulkan surface created");
}

static void choose_physical_device()
{
	VK_CHECK(vkEnumeratePhysicalDevices(gpu.instance, &gpu.physical_device_count, NULL));
	VK_CHECK(vkEnumeratePhysicalDevices(gpu.instance, &gpu.physical_device_count, gpu.physical_devices));

	u32 best_index = UINT32_MAX;
	u32 discrete_indices[4] = { 0 };
	u32 discrete_count = 0;
	VkPhysicalDeviceProperties properties = { 0 };
	VkPhysicalDeviceMemoryProperties memory_properties = { 0 };
	for (u32 i = 0; i < gpu.physical_device_count; i++)
	{
		vkGetPhysicalDeviceProperties(gpu.physical_devices[i], &properties);

		log_info("Found GPU: %s", properties.deviceName);
		gpu.gpu_properties[i].buffer_image_granularity = properties.limits.bufferImageGranularity;
		gpu.gpu_properties[i].non_coherent_atom_size = properties.limits.nonCoherentAtomSize;
		gpu.gpu_properties[i].is_integrated = properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU;
		strcpy_s(gpu.gpu_properties[i].name, sizeof(properties.deviceName), properties.deviceName);

		vkGetPhysicalDeviceMemoryProperties(gpu.physical_devices[i], &memory_properties);

		gpu.gpu_properties[i].memory_type_count = memory_properties.memoryTypeCount;
		gpu.gpu_properties[i].memory_heap_count = memory_properties.memoryHeapCount;

		for (u32 t = 0; t < memory_properties.memoryTypeCount; t++)
		{
			gpu.gpu_properties[i].memory_types[t].heap_index = memory_properties.memoryTypes[t].heapIndex;
			gpu.gpu_properties[i].memory_types[t].property_flags = memory_properties.memoryTypes[t].propertyFlags;
		}

		for (u32 h = 0; h < memory_properties.memoryHeapCount; h++)
		{
			gpu.gpu_properties[i].memory_heaps[h].size = memory_properties.memoryHeaps[h].size;
			gpu.gpu_properties[i].memory_heaps[h].flags = memory_properties.memoryHeaps[h].flags;
		}

		// Count discrete gpus
		if (!gpu.gpu_properties[i].is_integrated)
		{
			discrete_indices[discrete_count] = i;
			discrete_count++;
		}
	}

	if (discrete_count > 0)
	{
		// Select the discrete gpu with most memory
		VkDeviceSize biggest_size = 0;
		for (u32 i = 0; i < discrete_count; i++)
		{
			u32 index = discrete_indices[i];
			if (gpu.gpu_properties[index].memory_heaps[0].size > biggest_size)
			{
				best_index = index;
			}
		}
	}

	// If we didn't find anything specific, pick the first
	if (best_index == UINT32_MAX)
	{
		best_index = 0;
	}

	gpu.physical_device_index = best_index;

	log_info("Selected GPU: %s", gpu.gpu_properties[gpu.physical_device_index].name);
}

static void create_device()
{
	VkPhysicalDevice physical_device = gpu.physical_devices[gpu.physical_device_index];

	vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &gpu.queue_family_count, NULL);
	VkQueueFamilyProperties queue_family_properties[4];
	vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &gpu.queue_family_count, queue_family_properties);

	find_queue_family(VK_QUEUE_GRAPHICS_BIT, gpu.queue_family_count, queue_family_properties, &gpu.graphics_queue_index);
	find_queue_family(VK_QUEUE_TRANSFER_BIT, gpu.queue_family_count, queue_family_properties, &gpu.transfer_queue_index);
	find_queue_family(VK_QUEUE_COMPUTE_BIT, gpu.queue_family_count, queue_family_properties, &gpu.compute_queue_index);

	VkPhysicalDeviceFeatures2 features2 = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2 };
	vkGetPhysicalDeviceFeatures2(physical_device, &features2);

	u32 queue_info_count = 0;
	VkDeviceQueueCreateInfo queue_infos[4] = { 0 };
	float queue_priorities[1] = { 0.0f };

	VkDeviceQueueCreateInfo queue_info = { VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO };
	queue_info.queueCount = 1;
	queue_info.pQueuePriorities = queue_priorities;

	for (queue_info_count = 0; queue_info_count < gpu.queue_family_count; queue_info_count++)
	{
		queue_info.queueFamilyIndex = queue_info_count;
		queue_infos[queue_info_count] = queue_info;
	}

	const char* device_extensions[] = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};
	u32 device_extension_count = sizeof(device_extensions) / sizeof(device_extensions[0]);

	u32 available_extension_count = 0;
	VK_CHECK(vkEnumerateDeviceExtensionProperties(physical_device, NULL, &available_extension_count, NULL));
	VK_CHECK(vkEnumerateDeviceExtensionProperties(physical_device, NULL, &available_extension_count, available_device_extensions));
	assert(available_extension_count > 0);

	for (u32 i = 0; i < device_extension_count; i++)
	{
		bool found = false;
		for (u32 j = 0; j < available_extension_count; j++)
		{
			if (strcmp(device_extensions[i], available_device_extensions[j].extensionName) == 0)
			{
				found = true;
				break;
			}
		}
		if (!found)
		{
			log_error("Device extension not available: %s", device_extensions[i]);
		}
	}

	VkDeviceCreateInfo device_info = {
		.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
		.pNext = &features2,
		.queueCreateInfoCount = queue_info_count,
		.pQueueCreateInfos = queue_infos,
		.enabledExtensionCount = device_extension_count,
		.ppEnabledExtensionNames = device_extensions
	};

	VK_CHECK(vkCreateDevice(physical_device, &device_info, NULL, &gpu.device));

	volkLoadDevice(gpu.device);

	vkGetDeviceQueue(gpu.device, gpu.graphics_queue_index, 0, &gpu.graphics_queue);
	vkGetDeviceQueue(gpu.device, gpu.transfer_queue_index, 0, &gpu.transfer_queue);
	vkGetDeviceQueue(gpu.device, gpu.compute_queue_index, 0, &gpu.compute_queue);

	log_info("Vulkan device created");
}

static void create_swapchain()
{
	//    VkSurfaceFullScreenExclusiveInfoEXT fullscreen_exclusive_info = {
	//        .sType = VK_STRUCTURE_TYPE_SURFACE_FULL_SCREEN_EXCLUSIVE_INFO_EXT,
	//        .fullScreenExclusive = VK_FULL_SCREEN_EXCLUSIVE_APPLICATION_CONTROLLED_EXT
	//    };

	// Get surface properties
	VkPhysicalDevice physical_device = gpu.physical_devices[gpu.physical_device_index];
	
	VkBool32 supports_present;
	VK_CHECK(vkGetPhysicalDeviceSurfaceSupportKHR(physical_device, gpu.graphics_queue_index, gpu.surface, &supports_present));
	assert(supports_present == VK_TRUE);

	VkSurfaceCapabilitiesKHR surface_capabilities;
	VK_CHECK(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device, gpu.surface, &surface_capabilities));

	u32 present_mode_count;
	VK_CHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, gpu.surface, &present_mode_count, NULL));
	VkPresentModeKHR present_modes[8];
	VK_CHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, gpu.surface, &present_mode_count, present_modes));

	VkPresentModeKHR present_mode = -1;
	VkPresentModeKHR preferred_present_mode = cv_vsync.bool_value
		? VK_PRESENT_MODE_FIFO_KHR
		: VK_PRESENT_MODE_IMMEDIATE_KHR;
	for (u32 i = 0; i < present_mode_count; i++)
	{
		if (present_modes[i] == preferred_present_mode)
		{
			present_mode = preferred_present_mode;
			break;
		}
	}

	if (present_mode == -1)
	{
		present_mode = VK_PRESENT_MODE_FIFO_KHR;
	}

	VkExtent2D extent;
	if (surface_capabilities.currentExtent.width == UINT32_MAX || surface_capabilities.currentExtent.height == UINT32_MAX)
	{
		extent = surface_capabilities.currentExtent;
	}
	else
	{
		extent = (VkExtent2D){ gpu.width, gpu.height };
	}
	extent.width = max(surface_capabilities.minImageExtent.width, min(surface_capabilities.maxImageExtent.width, gpu.width));
	extent.height = max(surface_capabilities.minImageExtent.height, min(surface_capabilities.maxImageExtent.height, gpu.height));

	VkSwapchainKHR old_swapchain = gpu.swapchain;
	VkSwapchainCreateInfoKHR swapchain_info = {
		.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
		.pNext = NULL, // TODO: Handle exclusive fullscreen
		.surface = gpu.surface,
		.minImageCount = swapchain_image_count,
		.imageFormat = swapchain_format,
		.imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR,
		.imageExtent = extent,
		.imageArrayLayers = 1,
		.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
		.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
		.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR,
		.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
		.presentMode = present_mode,
		.clipped = VK_TRUE,
		.oldSwapchain = old_swapchain
	};

	VK_CHECK(vkCreateSwapchainKHR(gpu.device, &swapchain_info, NULL, &gpu.swapchain));

	if (old_swapchain != VK_NULL_HANDLE)
		vkDestroySwapchainKHR(gpu.device, old_swapchain, NULL);

	// Get swapchain images
	VK_CHECK(vkGetSwapchainImagesKHR(gpu.device, gpu.swapchain, &gpu.swapchain_image_count, NULL));
	VK_CHECK(vkGetSwapchainImagesKHR(gpu.device, gpu.swapchain, &gpu.swapchain_image_count, gpu.swapchain_images));

	log_info("Created swapchain with size %dx%d and %d images.", extent.width, extent.height, gpu.swapchain_image_count);

	// Create color attachments
	VkImageViewCreateInfo image_view_info = {
		.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
		.format = VK_FORMAT_B8G8R8A8_UNORM,
		.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
		.subresourceRange.levelCount = 1,
		.subresourceRange.layerCount = 1,
		.viewType = VK_IMAGE_VIEW_TYPE_2D
	};

	for (u32 i = 0; i < gpu.swapchain_image_count; i++)
	{
		image_view_info.image = gpu.swapchain_images[i];

		VK_CHECK(vkCreateImageView(gpu.device, &image_view_info, NULL, &gpu.swapchain_image_views[i]));
	}

	log_info("Vulkan swapchain created");
}

static void recreate_swapchain(void)
{
	vkDeviceWaitIdle(gpu.device);

	for (u32 i = 0; i < gpu.swapchain_image_count; i++)
	{
		vkDestroyImageView(gpu.device, gpu.swapchain_image_views[i], NULL);
	}
	create_swapchain();
}

static void create_synchronization(void)
{
	VkFenceCreateInfo fence_info = {
		.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
		.flags = VK_FENCE_CREATE_SIGNALED_BIT
	};
	VkSemaphoreCreateInfo semaphore_info = { VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
	for (uint32_t i = 0; i < FRAME_COUNT; ++i)
	{
		VK_CHECK(vkCreateFence(gpu.device, &fence_info, NULL, &gpu.render_complete_fences[i]));
		VK_CHECK(vkCreateSemaphore(gpu.device, &semaphore_info, NULL, &gpu.render_complete_semaphores[i]));
	}
	VK_CHECK(vkCreateSemaphore(gpu.device, &semaphore_info, NULL, &gpu.image_acquired_semaphore));
}

static void create_pipeline_cache()
{
	FILE* file = fopen("pipeline.cache", "rb");
	u64 size = 0;
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
			fread(data, sizeof(data), 1, file);
		}
		fclose(file);
	}

	VkPipelineCacheCreateInfo pipeline_cache_info = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO,
		.initialDataSize = size,
		.pInitialData = data
	};
	VK_CHECK(vkCreatePipelineCache(gpu.device, &pipeline_cache_info, NULL, &gpu.pipeline_cache));

	if (data)
	{
		free(data);
	}

	log_info("Loaded Vulkan pipeline cache of %u bytes", size);
}

static void create_command_pools()
{
	VkCommandPoolCreateInfo command_pool_info = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
		.queueFamilyIndex = gpu.graphics_queue_index,
		.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT
	};
	VK_CHECK(vkCreateCommandPool(gpu.device, &command_pool_info, NULL, &gpu.graphics_command_pool));
	command_pool_info.queueFamilyIndex = gpu.transfer_queue_index;
	VK_CHECK(vkCreateCommandPool(gpu.device, &command_pool_info, NULL, &gpu.transfer_command_pool));
}

static void allocate_command_buffers()
{

}

static void create_samplers()
{
	VkSamplerCreateInfo sampler_info = {
		.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
		.magFilter = VK_FILTER_LINEAR,
		.minFilter = VK_FILTER_LINEAR,
		.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST,
		.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
		.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
		.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
		.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE
	};
	VK_CHECK(vkCreateSampler(gpu.device, &sampler_info, NULL, &gpu.linear_clamp_sampler));

	sampler_info.magFilter = VK_FILTER_NEAREST;
	sampler_info.minFilter = VK_FILTER_NEAREST;
	VK_CHECK(vkCreateSampler(gpu.device, &sampler_info, NULL, &gpu.nearest_clamp_sampler));
}

void gpu_init()
{
	cvar_register(&cv_enable_validation);
	cvar_register(&cv_gpu_index);
	cvar_register(&cv_vsync);

	volkInitialize();
	create_instance();
	create_surface();
	choose_physical_device();
	create_device();
	create_swapchain();
	create_synchronization();
	create_pipeline_cache();
	create_command_pools();
	create_samplers();

	log_info("GPU initialized");
}

static void destroy_pipeline_cache()
{
	u64 pipeline_cache_size = 0;
	VK_CHECK(vkGetPipelineCacheData(gpu.device, gpu.pipeline_cache, &pipeline_cache_size, NULL));
	u8* pipeline_cache_data = malloc(pipeline_cache_size);
	assert(pipeline_cache_data);
	VK_CHECK(vkGetPipelineCacheData(gpu.device, gpu.pipeline_cache, &pipeline_cache_size, pipeline_cache_data));
	if (pipeline_cache_size > 0)
	{
		FILE* file = fopen("pipeline.cache", "wb");
		assert(file);
		fwrite(pipeline_cache_data, pipeline_cache_size, 1, file);
		fclose(file);
	}
	free(pipeline_cache_data);
	vkDestroyPipelineCache(gpu.device, gpu.pipeline_cache, NULL);
}

void gpu_quit()
{
	vkDeviceWaitIdle(gpu.device);

	vkDestroySampler(gpu.device, gpu.nearest_clamp_sampler, NULL);
	vkDestroySampler(gpu.device, gpu.linear_clamp_sampler, NULL);

	vkDestroyCommandPool(gpu.device, gpu.transfer_command_pool, NULL);
	vkDestroyCommandPool(gpu.device, gpu.graphics_command_pool, NULL);

	destroy_pipeline_cache();

	vkDestroySemaphore(gpu.device, gpu.image_acquired_semaphore, NULL);
	for (uint32_t i = 0; i < FRAME_COUNT; ++i)
	{
		vkDestroySemaphore(gpu.device, gpu.render_complete_semaphores[i], NULL);
		vkDestroyFence(gpu.device, gpu.render_complete_fences[i], NULL);
	}

	for (u32 i = 0; i < gpu.swapchain_image_count; i++)
	{
		vkDestroyImageView(gpu.device, gpu.swapchain_image_views[i], NULL);
	}
	vkDestroySwapchainKHR(gpu.device, gpu.swapchain, NULL);

	vkDestroyDevice(gpu.device, NULL);

	vkDestroySurfaceKHR(gpu.instance, gpu.surface, NULL);
	if (gpu.debug_messenger != VK_NULL_HANDLE)
		vkDestroyDebugUtilsMessengerEXT(gpu.instance, gpu.debug_messenger, NULL);
	vkDestroyInstance(gpu.instance, NULL);
}

void gpu_begin_frame(void)
{
	VkResult result;

	// Acquire swapchain image
	result = vkAcquireNextImageKHR(gpu.device, gpu.swapchain, UINT64_MAX, gpu.image_acquired_semaphore, VK_NULL_HANDLE, &gpu.swapchain_image_index);
	if (result == VK_ERROR_OUT_OF_DATE_KHR)
	{
		log_info("Swapchain incompatible with the surface, recreating...");
		recreate_swapchain();
	}
	else if (result == VK_SUBOPTIMAL_KHR)
	{
		log_info("Swapchain suboptimal for surface, recreating...");
		recreate_swapchain();
	}
	else if (result != VK_SUCCESS)
	{
		log_error("Failed to acquire swapchain image");
		return;
	}

	// Wait until GPU is done rendering
	VkFence render_complete_fence = gpu.render_complete_fences[gpu.frame_index];
	VK_CHECK(vkWaitForFences(gpu.device, 1, &render_complete_fence, VK_TRUE, UINT64_MAX));
	VK_CHECK(vkResetFences(gpu.device, 1, &render_complete_fence));

	// Reset commands
	VK_CHECK(vkResetCommandPool(gpu.device, gpu.graphics_command_pool, 0));

	// Record commands
	VkCommandBuffer command_buffer = gpu.graphics_command_buffers[gpu.frame_index];
	VkCommandBufferBeginInfo command_buffer_begin_info = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
	};
	VK_CHECK(vkBeginCommandBuffer(command_buffer, &command_buffer_begin_info));

	VkClearValue clear_value = {0};
	memcpy(clear_value.color.float32, gpu.clear_value.color, sizeof(gpu.clear_value.color));
	clear_value.depthStencil.depth = gpu.clear_value.depth;
	clear_value.depthStencil.stencil = gpu.clear_value.stencil;

	VkRenderPassBeginInfo render_pass_begin_info = {
		.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
		.renderPass = gpu.default_render_pass,
		.framebuffer = gpu.framebuffers[gpu.swapchain_image_index],
		.renderArea.extent = (VkExtent2D) { gpu.width, gpu.height },
		.clearValueCount = 1,
		.pClearValues = &clear_value
	};
	vkCmdBeginRenderPass(command_buffer, &render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);
}

void gpu_end_frame(void)
{
	VkResult result;

	VkCommandBuffer command_buffer = gpu.graphics_command_buffers[gpu.frame_index];
	vkCmdEndRenderPass(command_buffer);

	VK_CHECK(vkEndCommandBuffer(command_buffer));

	// Submit commands
	VkFence render_complete_fence = gpu.render_complete_fences[gpu.frame_index];
	VkPipelineStageFlags wait_stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

	VkSubmitInfo submit_info = {
		.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
		.pWaitDstStageMask = &wait_stage,
		.waitSemaphoreCount = 1,
		.pWaitSemaphores = &gpu.image_acquired_semaphore,
		.signalSemaphoreCount = 1,
		.pSignalSemaphores = &gpu.render_complete_semaphores[gpu.frame_index],
		.commandBufferCount = 1,
		.pCommandBuffers = &command_buffer
	};
	VK_CHECK(vkQueueSubmit(gpu.graphics_queue, 1, &submit_info, render_complete_fence));

	// Present 
	VkPresentInfoKHR present_info = {
		.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
		.waitSemaphoreCount = 1,
		.pWaitSemaphores = &gpu.render_complete_semaphores[gpu.frame_index],
		.swapchainCount = 1,
		.pSwapchains = &gpu.swapchain,
		.pImageIndices = &gpu.swapchain_image_index
	};

	result = vkQueuePresentKHR(gpu.graphics_queue, &present_info);
	if (result == VK_ERROR_OUT_OF_DATE_KHR)
	{
		log_info("Swapchain incompatible with the surface, recreating...");
		recreate_swapchain();
	}
	else if (result == VK_SUBOPTIMAL_KHR)
	{
		log_info("Swapchain suboptimal for surface, recreating...");
		recreate_swapchain();
	}
	else if (result != VK_SUCCESS)
	{
		log_error("Failed to present swapchain image");
		return;
	}

	// Advance frame index
	gpu.frame_index = (gpu.frame_index + 1) % FRAME_COUNT;
}
