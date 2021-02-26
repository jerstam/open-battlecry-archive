#include "render.h"
#include "window.h"
#include "shaders.h"
#include "../os.h"
#include "../log.h"
#include "../cvar.h"
#include "volk.h"

#include <GLFW/glfw3.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#ifdef _DEBUG
#define VK_CHECK(expr) do { VkResult _res = (expr); assert(_res == VK_SUCCESS); } while(0)
#else
#define VK_CHECK(expr) do { (expr); } while(0)
#endif

enum
{
	FRAME_COUNT = 2,

	// Limits
	MAX_PHYSICAL_DEVICES = 4,
	MAX_SWAPCHAIN_IMAGES = 3,
	MAX_SPRITES = UINT16_MAX,

	// Memory
	INDEX_BUFFER_SIZE = MAX_SPRITES * 6 * sizeof(u16),
	DEVICE_LOCAL_MEMORY_SIZE = ALIGN(INDEX_BUFFER_SIZE, 256),
	HOST_VISIBLE_MEMORY_SIZE = ALIGN(INDEX_BUFFER_SIZE, 256),

	// Descriptors
	MAX_SAMPLERS = 1024,
	MAX_SAMPLED_IMAGES = 10 * 1024,
	MAX_STORAGE_IMAGES = 1024,
	MAX_STORAGE_BUFFERS = 1024,
};

static const swapchain_format = VK_FORMAT_B8G8R8A8_UNORM;

static VkExtensionProperties available_instance_extensions[32];
static VkExtensionProperties available_device_extensions[256];

static VkInstance instance;
static VkDebugUtilsMessengerEXT debug_messenger;

static u32 physical_device_index;
static u32 physical_device_count;
static VkPhysicalDevice physical_devices[MAX_PHYSICAL_DEVICES];
static VkPhysicalDeviceProperties physical_device_properties[MAX_PHYSICAL_DEVICES];
static VkPhysicalDeviceMemoryProperties physical_device_memory_properties[MAX_PHYSICAL_DEVICES];

static u32 width;
static u32 height;
static VkSurfaceKHR surface;

static VkDevice device;

static u32 queue_family_count;
static u8 graphics_queue_index;
static u8 transfer_queue_index;
static u8 compute_queue_index;
static VkQueue graphics_queue;
static VkQueue transfer_queue;
static VkQueue compute_queue;

static VkPipelineCache pipeline_cache;

static VkSwapchainKHR swapchain;
static VkImage swapchain_images[MAX_SWAPCHAIN_IMAGES];
static VkImageView swapchain_image_views[MAX_SWAPCHAIN_IMAGES];

static VkCommandPool graphics_command_pools[FRAME_COUNT];
static VkCommandPool transfer_command_pool;
static VkCommandBuffer graphics_command_buffers[MAX_SWAPCHAIN_IMAGES];
static VkCommandBuffer transfer_command_buffer;

static VkFence render_complete_fences[FRAME_COUNT];
static VkSemaphore image_acquired_semaphore;
static VkSemaphore render_complete_semaphores[FRAME_COUNT];

static u32 frame_count;
static u32 frame_index;
static u32 swapchain_image_count;
static u32 swapchain_image_index;

static VkDescriptorSetLayout descriptor_set_layout;
static VkDescriptorPool descriptor_pool;
static VkDescriptorSet descriptor_set;

static VkRenderPass draw_render_pass;
static VkRenderPass post_process_render_pass;
static VkFramebuffer framebuffers[FRAME_COUNT];
static VkClearValue clear_value;

static VkShaderModule sprite_vertex_shader;
static VkShaderModule sprite_fragment_shader;
static VkPipeline sprite_pipeline;
static VkPipelineLayout sprite_pipeline_layout;

VkDeviceSize device_local_memory_offset;
VkDeviceSize host_visible_memory_offset;
static VkDeviceMemory device_local_memory;
static VkDeviceMemory host_visible_memory;

static VkBuffer staging_buffer;
static VkBuffer sprite_index_buffer;
static u16 sprite_indices[MAX_SPRITES * 6];

static VkSampler linear_clamp_sampler;
static VkSampler nearest_clamp_sampler;

static u32 next_sampler_index;
static u32 next_sampled_image_index;
static u32 next_storage_image_index;
static u32 next_storage_buffer_index;
static u32 free_sampler_indices[MAX_SAMPLERS];
static u32 free_sampled_image_indices[MAX_SAMPLED_IMAGES];
static u32 free_storage_image_indices[MAX_STORAGE_IMAGES];
static u32 free_storage_buffer_indices[MAX_STORAGE_BUFFERS];

static cvar_t cv_enable_validation = {
	.name = "gpu_validation",
	.description = "Enables GPU validation.",
	.type = CVAR_TYPE_BOOL,
	.bool_value = true
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
	VkApplicationInfo application_info = { 
		.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
		.pApplicationName = "Battlecry",
		.applicationVersion = VK_MAKE_VERSION(1, 0, 0),
		.engineVersion = VK_MAKE_VERSION(1, 0, 0),
		.apiVersion = VK_API_VERSION_1_2
	};
	
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
#ifdef _DEBUG
		VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
#endif
	};
	u32 instance_extension_count = array_length(instance_extensions);
	if (!cv_enable_validation.bool_value) --instance_extension_count;

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

#ifdef _DEBUG
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
		.enabledExtensionCount = instance_extension_count,
		.ppEnabledExtensionNames = instance_extensions,
#ifdef _DEBUG
		.pNext = cv_enable_validation.bool_value ? &debug_info : NULL,
		.enabledLayerCount = cv_enable_validation.bool_value ? array_length(validation_layers) : 0,
		.ppEnabledLayerNames = validation_layers
#endif
	};

	VK_CHECK(vkCreateInstance(&instance_info, NULL, &instance));

	volkLoadInstanceOnly(instance);

#ifdef _DEBUG
	if (cv_enable_validation.bool_value)
	{
		VK_CHECK(vkCreateDebugUtilsMessengerEXT(instance, &debug_info, NULL, &debug_messenger));
	}
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
	VK_CHECK(glfwCreateWindowSurface(instance, window_get_handle(), NULL, &surface));

	log_info("Vulkan surface created");
}

static void choose_physical_device()
{
	VK_CHECK(vkEnumeratePhysicalDevices(instance, &physical_device_count, NULL));
	VK_CHECK(vkEnumeratePhysicalDevices(instance, &physical_device_count, physical_devices));

	u32 best_index = UINT32_MAX;
	u32 discrete_indices[4] = { 0 };
	u32 discrete_count = 0;
	for (u32 i = 0; i < physical_device_count; i++)
	{
		vkGetPhysicalDeviceProperties(physical_devices[i], &physical_device_properties[i]);
		vkGetPhysicalDeviceMemoryProperties(physical_devices[i], &physical_device_memory_properties[i]);

		log_info("Found GPU: %s", physical_device_properties[i].deviceName);

		// Count discrete gpus
		if (physical_device_properties[i].deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
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
			if (physical_device_memory_properties[index].memoryHeaps[0].size > biggest_size)
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

	physical_device_index = best_index;

	log_info("Selected GPU: %s", physical_device_properties[physical_device_index].deviceName);
}

static void create_device()
{
	VkPhysicalDevice physical_device = physical_devices[physical_device_index];

	vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, NULL);
	VkQueueFamilyProperties queue_family_properties[4];
	vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, queue_family_properties);

	find_queue_family(VK_QUEUE_GRAPHICS_BIT, queue_family_count, queue_family_properties, &graphics_queue_index);
	find_queue_family(VK_QUEUE_TRANSFER_BIT, queue_family_count, queue_family_properties, &transfer_queue_index);
	find_queue_family(VK_QUEUE_COMPUTE_BIT, queue_family_count, queue_family_properties, &compute_queue_index);

	VkPhysicalDeviceDescriptorIndexingFeatures descriptor_indexing_features = {
		.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES,
		.shaderSampledImageArrayNonUniformIndexing = VK_TRUE,
		.descriptorBindingSampledImageUpdateAfterBind = VK_TRUE,
		.descriptorBindingPartiallyBound = VK_TRUE,
		.descriptorBindingUpdateUnusedWhilePending = VK_TRUE,
		.descriptorBindingVariableDescriptorCount = VK_TRUE,
		.runtimeDescriptorArray = VK_TRUE
	};

	VkPhysicalDeviceFeatures2 features2 = {
		.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
		.pNext = &descriptor_indexing_features
	};
	vkGetPhysicalDeviceFeatures2(physical_device, &features2);

	VkPhysicalDeviceDescriptorIndexingProperties descriptor_indexing_properties = {
		.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_PROPERTIES,
	};

	VkPhysicalDeviceProperties2 physical_device_properties2 = {
		.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2,
		.pNext = &descriptor_indexing_properties
	};
	vkGetPhysicalDeviceProperties2(physical_device, &physical_device_properties2);

	u32 queue_info_count = 0;
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

	const char* device_extensions[] = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME,
		VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME
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

	VK_CHECK(vkCreateDevice(physical_device, &device_info, NULL, &device));

	volkLoadDevice(device);

	vkGetDeviceQueue(device, graphics_queue_index, 0, &graphics_queue);
	vkGetDeviceQueue(device, transfer_queue_index, 0, &transfer_queue);
	vkGetDeviceQueue(device, compute_queue_index, 0, &compute_queue);

	log_info("Vulkan device created");
}

static void create_swapchain()
{
	//    VkSurfaceFullScreenExclusiveInfoEXT fullscreen_exclusive_info = {
	//        .sType = VK_STRUCTURE_TYPE_SURFACE_FULL_SCREEN_EXCLUSIVE_INFO_EXT,
	//        .fullScreenExclusive = VK_FULL_SCREEN_EXCLUSIVE_APPLICATION_CONTROLLED_EXT
	//    };

	// Get surface properties
	VkPhysicalDevice physical_device = physical_devices[physical_device_index];

	VkBool32 supports_present;
	VK_CHECK(vkGetPhysicalDeviceSurfaceSupportKHR(physical_device, graphics_queue_index, surface, &supports_present));
	assert(supports_present == VK_TRUE);

	VkSurfaceCapabilitiesKHR surface_capabilities;
	VK_CHECK(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device, surface, &surface_capabilities));

	u32 present_mode_count;
	VK_CHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface, &present_mode_count, NULL));
	VkPresentModeKHR present_modes[8];
	VK_CHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface, &present_mode_count, present_modes));

	VkPresentModeKHR present_mode = -1;
	VkPresentModeKHR preferred_present_mode = cv_vsync.bool_value
		? VK_PRESENT_MODE_FIFO_RELAXED_KHR
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
		window_size(&width, &height);
		extent = (VkExtent2D){ width, height };
	}
	else
	{
		extent = surface_capabilities.currentExtent;
	}
	extent.width = max(surface_capabilities.minImageExtent.width, min(surface_capabilities.maxImageExtent.width, width));
	extent.height = max(surface_capabilities.minImageExtent.height, min(surface_capabilities.maxImageExtent.height, height));

	VkSwapchainKHR old_swapchain = swapchain;
	VkSwapchainCreateInfoKHR swapchain_info = {
		.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
		.pNext = NULL, // TODO: Handle exclusive fullscreen
		.surface = surface,
		.minImageCount = cv_vsync.bool_value ? 3 : 2,
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

	VK_CHECK(vkCreateSwapchainKHR(device, &swapchain_info, NULL, &swapchain));

	if (old_swapchain != VK_NULL_HANDLE)
		vkDestroySwapchainKHR(device, old_swapchain, NULL);

	// Get swapchain images
	VK_CHECK(vkGetSwapchainImagesKHR(device, swapchain, &swapchain_image_count, NULL));
	VK_CHECK(vkGetSwapchainImagesKHR(device, swapchain, &swapchain_image_count, swapchain_images));

	log_info("Created swapchain with size %dx%d and %d images.", extent.width, extent.height, swapchain_image_count);

	// Create color attachments
	VkImageViewCreateInfo image_view_info = {
		.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
		.format = VK_FORMAT_B8G8R8A8_UNORM,
		.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
		.subresourceRange.levelCount = 1,
		.subresourceRange.layerCount = 1,
		.viewType = VK_IMAGE_VIEW_TYPE_2D
	};

	for (u32 i = 0; i < swapchain_image_count; i++)
	{
		image_view_info.image = swapchain_images[i];

		VK_CHECK(vkCreateImageView(device, &image_view_info, NULL, &swapchain_image_views[i]));
	}

	log_info("Vulkan swapchain created");
}

static void recreate_swapchain(void)
{
	vkDeviceWaitIdle(device);

	for (u32 i = 0; i < swapchain_image_count; i++)
	{
		vkDestroyImageView(device, swapchain_image_views[i], NULL);
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
		VK_CHECK(vkCreateFence(device, &fence_info, NULL, &render_complete_fences[i]));
		VK_CHECK(vkCreateSemaphore(device, &semaphore_info, NULL, &render_complete_semaphores[i]));
	}
	VK_CHECK(vkCreateSemaphore(device, &semaphore_info, NULL, &image_acquired_semaphore));
}

static void create_command_pools()
{
	VkCommandPoolCreateInfo command_pool_info = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
		.queueFamilyIndex = graphics_queue_index,
		.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT
	};
	for (u32 i = 0; i < FRAME_COUNT; i++)
	{
		VK_CHECK(vkCreateCommandPool(device, &command_pool_info, NULL, &graphics_command_pools[i]));
	}
	command_pool_info.queueFamilyIndex = transfer_queue_index;
	VK_CHECK(vkCreateCommandPool(device, &command_pool_info, NULL, &transfer_command_pool));
}

static void allocate_command_buffers()
{
	VkCommandBufferAllocateInfo allocate_info = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
		.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
		.commandBufferCount = 1
	};

	for (int i = 0; i < FRAME_COUNT; i++)
	{
		allocate_info.commandPool = graphics_command_pools[i];
		VK_CHECK(vkAllocateCommandBuffers(device, &allocate_info, &graphics_command_buffers[i]));
	}

	allocate_info.commandPool = transfer_command_pool;
	VK_CHECK(vkAllocateCommandBuffers(device, &allocate_info, &transfer_command_buffer));
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
	VK_CHECK(vkCreateSampler(device, &sampler_info, NULL, &linear_clamp_sampler));

	sampler_info.magFilter = VK_FILTER_NEAREST;
	sampler_info.minFilter = VK_FILTER_NEAREST;
	VK_CHECK(vkCreateSampler(device, &sampler_info, NULL, &nearest_clamp_sampler));
}

static void create_descriptor_set()
{
	const VkDescriptorSetLayoutBinding bindings[] = {
		{
			.binding = 0,
			.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
			.descriptorCount = MAX_STORAGE_BUFFERS,
			.stageFlags = VK_SHADER_STAGE_VERTEX_BIT
		},
		{
			.binding = 1,
			.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
			.descriptorCount = MAX_SAMPLED_IMAGES,
			.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT
		},
		{
			.binding = 2,
			.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
			.descriptorCount = MAX_STORAGE_IMAGES,
			.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT
		},
		{
			.binding = 3,
			.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER,
			.descriptorCount = MAX_SAMPLERS,
			.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT
		}
	};

	VkDescriptorSetLayoutCreateInfo layout_info = {
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
		.bindingCount = array_length(bindings),
		.pBindings = bindings,
		.flags = 0
	};
	VK_CHECK(vkCreateDescriptorSetLayout(device, &layout_info, NULL, &descriptor_set_layout));

	VkDescriptorPoolSize pool_sizes[] =
	{
		{ VK_DESCRIPTOR_TYPE_SAMPLER, MAX_SAMPLERS },
		{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1 },
		{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, MAX_SAMPLED_IMAGES },
		{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, MAX_STORAGE_IMAGES },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, MAX_STORAGE_BUFFERS },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1 },
		{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1 }
	};

	VkDescriptorPoolCreateInfo descriptor_pool_info = {
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
		.poolSizeCount = array_length(pool_sizes),
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

static void create_render_passes()
{
	VkAttachmentDescription color_attachment = {
		.format = swapchain_format,
		.samples = VK_SAMPLE_COUNT_1_BIT, // TODO: MSAA
		.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
		.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
		.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
		.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
		.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
		.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
	};

	VkAttachmentReference color_attachment_ref = {
		color_attachment_ref.attachment = 0,
		color_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
	};

	VkSubpassDescription subpass = {
		.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
		.colorAttachmentCount = 1,
		.pColorAttachments = &color_attachment_ref
	};

	VkRenderPassCreateInfo render_pass_info = {
		.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
		.attachmentCount = 1,
		.pAttachments = &color_attachment,
		.subpassCount = 1,
		.pSubpasses = &subpass
	};

	VK_CHECK(vkCreateRenderPass(device, &render_pass_info, NULL, &draw_render_pass));

	VkFramebufferCreateInfo framebuffer_info = {
		.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
		.renderPass = draw_render_pass,
		.attachmentCount = 1,
		.width = width,
		.height = height,
		.layers = 1
	};

	for (uint32_t i = 0; i < swapchain_image_count; i++) 
	{
		framebuffer_info.pAttachments = &swapchain_image_views[i];
		VK_CHECK(vkCreateFramebuffer(device, &framebuffer_info, NULL, &framebuffers[i]));
	}
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
	VK_CHECK(vkCreatePipelineCache(device, &pipeline_cache_info, NULL, &pipeline_cache));

	if (data)
	{
		free(data);
	}

	log_info("Loaded Vulkan pipeline cache of %u bytes", size);
}

void create_sprite_pipeline()
{
	VkPushConstantRange fragment_push_constant_range = {
		.size = sizeof(float) * 4,
		.offset = 0,
		.stageFlags = VK_SHADER_STAGE_VERTEX_BIT
	};

	VkPipelineLayoutCreateInfo layout_info = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
		.setLayoutCount = 1,
		.pSetLayouts = &descriptor_set_layout,
		.pushConstantRangeCount = 1,
		.pPushConstantRanges = (const VkPushConstantRange[]) { fragment_push_constant_range }
	};

	VK_CHECK(vkCreatePipelineLayout(device, &layout_info, NULL, &sprite_pipeline_layout));

	VkShaderModuleCreateInfo shader_info = { VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO };

	shader_info.codeSize = sizeof(sprite_vert_spirv);
	shader_info.pCode = sprite_vert_spirv;
	VK_CHECK(vkCreateShaderModule(device, &shader_info, NULL, &sprite_vertex_shader));
	shader_info.codeSize = sizeof(sprite_frag_spirv);
	shader_info.pCode = sprite_frag_spirv;
	VK_CHECK(vkCreateShaderModule(device, &shader_info, NULL, &sprite_fragment_shader));

	VkPipelineShaderStageCreateInfo shader_stages[2] = { 0 };
	shader_stages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shader_stages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
	shader_stages[0].module = sprite_vertex_shader;
	shader_stages[0].pName = "main";
	shader_stages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shader_stages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	shader_stages[1].module = sprite_fragment_shader;
	shader_stages[1].pName = "main";

	VkPipelineVertexInputStateCreateInfo input_state = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO
	};

	VkPipelineInputAssemblyStateCreateInfo input_assembly = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
		.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST
	};

	VkPipelineRasterizationStateCreateInfo rasterization_state = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
		.polygonMode = VK_POLYGON_MODE_FILL,
		.lineWidth = 1.0f,
		.cullMode = VK_CULL_MODE_BACK_BIT,
		.frontFace = VK_FRONT_FACE_CLOCKWISE
	};

	VkPipelineMultisampleStateCreateInfo multisample_state = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
		.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
		.minSampleShading = 1.0f
	};

	VkViewport viewport = {
		.width = (float)width,
		.height = (float)height,
		.minDepth = 0.0f,
		.maxDepth = 1.0f
	};

	VkRect2D scissor = { 0 };
	scissor.extent = (VkExtent2D){ width, height };

	VkPipelineViewportStateCreateInfo viewport_state = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
		.viewportCount = 1,
		.pViewports = &viewport,
		.scissorCount = 1,
		.pScissors = &scissor
	};

	VkPipelineColorBlendAttachmentState color_blend_attachment_state = {
		.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
			VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT
	};

	VkPipelineColorBlendStateCreateInfo color_blend_state = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
		.logicOp = VK_LOGIC_OP_COPY,
		.attachmentCount = 1,
		.pAttachments = &color_blend_attachment_state
	};

	VkGraphicsPipelineCreateInfo pipeline_info = {
		.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
		.stageCount = 2,
		.pStages = shader_stages,
		.pVertexInputState = &input_state,
		.pInputAssemblyState = &input_assembly,
		.pRasterizationState = &rasterization_state,
		.pMultisampleState = &multisample_state,
		.pColorBlendState = &color_blend_state,
		.pViewportState = &viewport_state,
		.layout = sprite_pipeline_layout,
		.renderPass = draw_render_pass,
	};
	VK_CHECK(vkCreateGraphicsPipelines(device, pipeline_cache, 1, &pipeline_info, NULL, &sprite_pipeline));
}

static void allocate_memory()
{
	VkPhysicalDeviceMemoryProperties* memory_properties = &physical_device_memory_properties[physical_device_index];

	bool device_local_found = false;
	bool host_visible_found = false;
	for (u32 i = 0; i < memory_properties->memoryTypeCount; ++i)
	{
		if (!device_local_found && (memory_properties->memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) != 0)
		{
			VkMemoryAllocateInfo allocate_info = {
				.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
				.allocationSize = DEVICE_LOCAL_MEMORY_SIZE,
				.memoryTypeIndex = i
			};

			VK_CHECK(vkAllocateMemory(device, &allocate_info, NULL, &device_local_memory));
			device_local_found = true;
		}

		if (!host_visible_found && (memory_properties->memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) != 0)
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

void render_init()
{
	cvar_register(&cv_enable_validation);
	cvar_register(&cv_gpu_index);
	cvar_register(&cv_vsync);

	window_size(&width, &height);

	clear_value.color = (VkClearColorValue){
		.float32 = { 0.1f, 0.2f, 0.3f, 1.0f }
	};
	clear_value.depthStencil = (VkClearDepthStencilValue){
		.depth = 0.0f,
		.stencil = 0
	};

	volkInitialize();
	create_instance();
	create_surface();
	choose_physical_device();
	create_device();
	create_swapchain();
	create_synchronization();
	create_command_pools();
	allocate_command_buffers();
	create_samplers();
	create_descriptor_set();
	create_render_passes();
	create_pipeline_cache();
	create_sprite_pipeline();
	allocate_memory();

	// Sprite index buffer
	const u16 quad_indices[6] = { 0, 1, 2, 0, 2, 3 };

	for (u32 i = 0; i < MAX_SPRITES * 6; i++)
	{
		int instance_index = i / 6;
		int quad_index = i % 6;
		sprite_indices[i] = quad_indices[quad_index] + instance_index * 4;
	}
	VkDeviceSize index_buffer_size = MAX_SPRITES * 6 * sizeof(sprite_indices[0]);

	VkBufferCreateInfo buffer_info = {
		.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
		.size = index_buffer_size,
		.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
		.sharingMode = VK_SHARING_MODE_EXCLUSIVE
	};
	VK_CHECK(vkCreateBuffer(device, &buffer_info, NULL, &sprite_index_buffer));

	VkMemoryRequirements memory_requirements = { 0 };
	vkGetBufferMemoryRequirements(device, sprite_index_buffer, &memory_requirements);
	assert((memory_requirements.memoryTypeBits & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) != 0);

	VK_CHECK(vkBindBufferMemory(device, sprite_index_buffer, device_local_memory, device_local_memory_offset));
	device_local_memory_offset += memory_requirements.size;

	// Staging buffer
	buffer_info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	VK_CHECK(vkCreateBuffer(device, &buffer_info, NULL, &staging_buffer));

	vkGetBufferMemoryRequirements(device, staging_buffer, &memory_requirements);
	assert((memory_requirements.memoryTypeBits & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) != 0);

	VkDeviceSize memory_offset = host_visible_memory_offset;
	VK_CHECK(vkBindBufferMemory(device, staging_buffer, host_visible_memory, host_visible_memory_offset));
	host_visible_memory_offset += memory_requirements.size;

	// Map
	void* staging_ptr = NULL;
	VK_CHECK(vkMapMemory(device, host_visible_memory, memory_offset, memory_requirements.size, 0, &staging_ptr));

	memcpy(staging_ptr, sprite_indices, memory_requirements.size);

	VkMappedMemoryRange memory_range = {
		.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE,
		.memory = host_visible_memory,
		.offset = memory_offset,
		.size = memory_requirements.size
	};
	VK_CHECK(vkFlushMappedMemoryRanges(device, 1, &memory_range));

	vkUnmapMemory(device, host_visible_memory);

	// Copy
	VkCommandBufferBeginInfo begin_info = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
	};
	VK_CHECK(vkBeginCommandBuffer(transfer_command_buffer, &begin_info));

	VkBufferCopy buffer_copy = {
		.srcOffset = 0,
		.dstOffset = memory_offset,
		.size = index_buffer_size
	};
	vkCmdCopyBuffer(transfer_command_buffer, staging_buffer, sprite_index_buffer, 1, &buffer_copy);

	VkBufferMemoryBarrier memory_barrier = {
		.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
		.srcAccessMask = VK_ACCESS_MEMORY_WRITE_BIT,
		.dstAccessMask = VK_ACCESS_INDEX_READ_BIT,
		.srcQueueFamilyIndex = transfer_queue_index,
		.dstQueueFamilyIndex = graphics_queue_index,
		.buffer = sprite_index_buffer,
		.offset = memory_offset,
		.size = index_buffer_size
	};
	vkCmdPipelineBarrier(transfer_command_buffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_VERTEX_INPUT_BIT,
		0, 0, NULL, 1, &memory_barrier, 0, NULL);

	VK_CHECK(vkEndCommandBuffer(transfer_command_buffer));

	// Submit
	VkSubmitInfo submit_info = {
		.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
		.commandBufferCount = 1,
		.pCommandBuffers = &transfer_command_buffer
	};
	VK_CHECK(vkQueueSubmit(transfer_queue, 1, &submit_info, VK_NULL_HANDLE));

	// TODO: Use a fence in submit instead
	VK_CHECK(vkDeviceWaitIdle(device));

	log_info("GPU initialized");
}

static void destroy_pipeline_cache()
{
	u64 pipeline_cache_size = 0;
	VK_CHECK(vkGetPipelineCacheData(device, pipeline_cache, &pipeline_cache_size, NULL));
	u8* pipeline_cache_data = malloc(pipeline_cache_size);
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

void render_quit()
{
	vkDeviceWaitIdle(device);

	vkDestroyBuffer(device, sprite_index_buffer, NULL);
	vkDestroyBuffer(device, staging_buffer, NULL);

	vkFreeMemory(device, host_visible_memory, NULL);
	vkFreeMemory(device, device_local_memory, NULL);

	destroy_pipeline_cache();

	vkDestroyPipeline(device, sprite_pipeline, NULL);
	vkDestroyPipelineLayout(device, sprite_pipeline_layout, NULL);
	vkDestroyShaderModule(device, sprite_fragment_shader, NULL);
	vkDestroyShaderModule(device, sprite_vertex_shader, NULL);

	for (u32 i = 0; i < swapchain_image_count; i++)
	{
		vkDestroyFramebuffer(device, framebuffers[i], NULL);
	}
	vkDestroyRenderPass(device, draw_render_pass, NULL);

	vkDestroyDescriptorPool(device, descriptor_pool, NULL);
	vkDestroyDescriptorSetLayout(device, descriptor_set_layout, NULL);

	vkDestroySampler(device, nearest_clamp_sampler, NULL);
	vkDestroySampler(device, linear_clamp_sampler, NULL);

	vkDestroyCommandPool(device, transfer_command_pool, NULL);
	for (u32 i = 0; i < FRAME_COUNT; i++)
	{
		vkDestroyCommandPool(device, graphics_command_pools[i], NULL);
	}

	vkDestroySemaphore(device, image_acquired_semaphore, NULL);
	for (u32 i = 0; i < FRAME_COUNT; ++i)
	{
		vkDestroySemaphore(device, render_complete_semaphores[i], NULL);
		vkDestroyFence(device, render_complete_fences[i], NULL);
	}

	for (u32 i = 0; i < swapchain_image_count; i++)
	{
		vkDestroyImageView(device, swapchain_image_views[i], NULL);
	}
	vkDestroySwapchainKHR(device, swapchain, NULL);

	vkDestroyDevice(device, NULL);

	vkDestroySurfaceKHR(instance, surface, NULL);
	if (debug_messenger != VK_NULL_HANDLE)
		vkDestroyDebugUtilsMessengerEXT(instance, debug_messenger, NULL);
	vkDestroyInstance(instance, NULL);
}

void render_frame(void)
{
	VkResult result;

	// Acquire swapchain image
	result = vkAcquireNextImageKHR(device, swapchain, UINT64_MAX, image_acquired_semaphore, VK_NULL_HANDLE, &swapchain_image_index);
	if (result == VK_ERROR_OUT_OF_DATE_KHR)
	{
		log_info("Swapchain incompatible with the surface, recreating...");
		recreate_swapchain();
	}
	else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
	{
		log_error("Failed to acquire swapchain image");
		return;
	}

	// Wait until GPU is done rendering
	VkFence render_complete_fence = render_complete_fences[frame_index];
	VK_CHECK(vkWaitForFences(device, 1, &render_complete_fence, VK_TRUE, UINT64_MAX));
	VK_CHECK(vkResetFences(device, 1, &render_complete_fence));

	// Reset commands
	VK_CHECK(vkResetCommandPool(device, graphics_command_pools[frame_index], 0));

	// Record commands
	VkCommandBuffer command_buffer = graphics_command_buffers[frame_index];
	VkCommandBufferBeginInfo command_buffer_begin_info = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
	};
	VK_CHECK(vkBeginCommandBuffer(command_buffer, &command_buffer_begin_info));

	VkRenderPassBeginInfo render_pass_begin_info = {
		.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
		.renderPass = draw_render_pass,
		.framebuffer = framebuffers[swapchain_image_index],
		.renderArea.extent = (VkExtent2D) { width, height },
		.clearValueCount = 1,
		.pClearValues = &clear_value
	};
	vkCmdBeginRenderPass(command_buffer, &render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);

	vkCmdEndRenderPass(command_buffer);

	VK_CHECK(vkEndCommandBuffer(command_buffer));

	// Submit commands
	VkPipelineStageFlags wait_stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

	VkSubmitInfo submit_info = {
		.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
		.pWaitDstStageMask = &wait_stage,
		.waitSemaphoreCount = 1,
		.pWaitSemaphores = &image_acquired_semaphore,
		.signalSemaphoreCount = 1,
		.pSignalSemaphores = &render_complete_semaphores[frame_index],
		.commandBufferCount = 1,
		.pCommandBuffers = &command_buffer
	};
	VK_CHECK(vkQueueSubmit(graphics_queue, 1, &submit_info, render_complete_fence));

	// Present 
	VkPresentInfoKHR present_info = {
		.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
		.waitSemaphoreCount = 1,
		.pWaitSemaphores = &render_complete_semaphores[frame_index],
		.swapchainCount = 1,
		.pSwapchains = &swapchain,
		.pImageIndices = &swapchain_image_index
	};

	result = vkQueuePresentKHR(graphics_queue, &present_info);
	if (result == VK_ERROR_OUT_OF_DATE_KHR)
	{
		log_info("Swapchain incompatible with the surface, recreating...");
		recreate_swapchain();
	}
	else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
	{
		log_error("Failed to present swapchain image");
		return;
	}

	// Advance frame index
	frame_index = (frame_index + 1) % FRAME_COUNT;
}