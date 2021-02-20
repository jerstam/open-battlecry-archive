//#include "graphics.h"
//#include "shaders.h"
//#include "jobs.h"
//
//#include <SDL2/SDL_vulkan.h>
//#include <SDL2/SDL_timer.h>
//#include <SDL2/SDL_rwops.h>
//#include <SDL2/SDL_log.h>
//#include <volk.h>
//#define VMA_STATIC_VULKAN_FUNCTIONS 0
//#define VULKAN_H_ 1
//#include <vk_mem_alloc.h>
//#include <string.h>
//#include <stdlib.h>
//#include <assert.h>
//
//enum
//{
//	MAX_BUFFERING = 3,
//
//	MAX_PHYSICAL_DEVICES = 4,
//
//	MAX_LINES = 10000,
//	MAX_SPRITES = 10000,
//
//	// Descriptors
//	MAX_DESCRIPTOR_SETS = 10,
//	MAX_SAMPLERS = 2,
//	MAX_SAMPLED_IMAGES = 8192,
//	MAX_STORAGE_IMAGES = 1,
//	MAX_UNIFORM_TEXEL_BUFFERS = 1,
//	MAX_STORAGE_TEXEL_BUFFERS = 1,
//	MAX_UNIFORM_BUFFERS = 10,
//	MAX_STORAGE_BUFFERS = 10,
//	MAX_UNIFORM_BUFFER_DYNAMIC = 1
//};
//
//typedef struct line_vertex_t
//{
//	float x, y;
//} line_vertex_t;
//
//typedef struct SpriteInstanceData
//{
//	float x, y;
//	float scale;
//	float depth;
//} SpriteInstanceData;
//
//typedef struct CameraConstants
//{
//	float camera_position[2];
//	float camera_size;
//	float data;
//} CameraConstants;
//
//typedef struct LineConstants
//{
//	uint32_t line_index;
//} LineConstants;
//
//
//
//typedef struct Texture
//{
//	VkImage image;
//	VkImageView image_view;
//	VmaAllocation allocation;
//	VkFormat format;
//	uint16_t width;
//	uint16_t height;
//} Texture;
//
//typedef struct GraphicsPipelineDesc
//{
//	const uint32_t* vert_code;
//	size_t vert_code_size;
//	const uint32_t* frag_code;
//	size_t frag_code_size;
//	VkPrimitiveTopology topology;
//	VkPolygonMode polygon_mode;
//	VkPipelineLayout pipeline_layout;
//} GraphicsPipelineDesc;
//
//typedef struct GraphicsPipeline
//{
//	VkShaderModule vertex_module;
//	VkShaderModule fragment_module;
//	VkPipelineLayout pipeline_layout;
//	VkPipeline pipeline;
//} GraphicsPipeline;
//
//struct GraphicsContext
//{
//	SDL_Window* window;
//	VkExtent2D extent;
//
//	VkInstance instance;
//	uint32_t physical_device_index;
//	uint32_t physical_device_count;
//	VkPhysicalDevice physical_devices[MAX_PHYSICAL_DEVICES];
//	VkPhysicalDeviceProperties physical_device_properties[MAX_PHYSICAL_DEVICES];
//	VkPhysicalDeviceProperties2 physical_device_properties2[MAX_PHYSICAL_DEVICES];
//	VkSurfaceKHR surface;
//	VkDevice device;
//	VkDebugUtilsMessengerEXT debug_messenger;
//	VmaAllocator allocator;
//
//	uint32_t queue_family_count;
//	uint8_t graphics_queue_index;
//	uint8_t transfer_queue_index;
//	uint8_t compute_queue_index;
//	VkQueue graphics_queue;
//	VkQueue transfer_queue;
//	VkQueue compute_queue;
//
//	VkDescriptorPool descriptor_pools[MAX_THREADS];
//	VkPipelineCache pipeline_cache;
//
//	uint32_t buffering_count;
//
//	VkCommandPool main_command_pool;
//	VkCommandPool command_pools[MAX_BUFFERING];
//	VkCommandBuffer command_buffers[MAX_BUFFERING];
//
//	VkSwapchainKHR swapchain;
//	VkFormat swapchain_format;
//	uint32_t swapchain_image_count;
//	VkImage swapchain_images[MAX_BUFFERING];
//	VkImageView swapchain_image_views[MAX_BUFFERING];
//	VkFence render_complete_fences[MAX_BUFFERING];
//	VkSemaphore image_acquired_semaphore;
//	VkSemaphore render_complete_semaphores[MAX_BUFFERING];
//
//	uint32_t frame_index;
//
//	uint32_t line_count;
//	Buffer line_vertex_buffer;
//	VkDescriptorSetLayout line_descriptor_set_layout;
//	VkDescriptorSet line_descriptor_set;
//	GraphicsPipeline line_pipeline;
//
//	uint32_t sprite_count;
//	GraphicsPipeline sprite_pipeline;
//
//	VkRenderPass default_render_pass;
//	VkRenderPass post_process_render_pass;
//	VkFramebuffer framebuffers[MAX_BUFFERING];
//	VkClearValue clear_value;
//
//	VkSampler linear_clamp_sampler;
//};
//
//static void create_texture(VkExtent2D extent, VkImageUsageFlagBits image_usage, VmaMemoryUsage memory_usage, Texture* texture)
//{
//	assert(texture);
//
//	VkImageCreateInfo image_info = {
//		.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
//		.imageType = VK_IMAGE_TYPE_2D,
//		.format = VK_FORMAT_R8G8B8A8_UNORM,
//		.extent = (VkExtent3D) { extent.width, extent.height, 1 },
//		.mipLevels = 1,
//		.arrayLayers = 1,
//		.samples = VK_SAMPLE_COUNT_1_BIT,
//		.tiling = VK_IMAGE_TILING_OPTIMAL,
//		.usage = image_usage
//	};
//
//	VmaAllocationCreateInfo allocation_create_info = {
//		.usage = memory_usage
//	};
//
//	if (image_usage & VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT)
//	{
//		allocation_create_info.preferredFlags = VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT;
//	}
//
//	VK_CHECK(vmaCreateImage(context->allocator, &image_info, &allocation_create_info, &texture->image, &texture->allocation, NULL));
//
//	VkImageViewCreateInfo image_view_info = {
//		.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
//		.image = texture->image,
//		.viewType = VK_IMAGE_VIEW_TYPE_2D,
//		.format = VK_FORMAT_R8G8B8A8_UNORM,
//		.subresourceRange.levelCount = 1,
//		.subresourceRange.layerCount = 1
//	};
//
//	VK_CHECK(vkCreateImageView(context->device, &image_view_info, NULL, &texture->image_view));
//}
//
//static void create_graphics_pipeline(const GraphicsPipelineDesc* desc, GraphicsPipeline* pipeline)
//{
//	VkShaderModuleCreateInfo shader_info = { VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO };
//	VkPipelineShaderStageCreateInfo pipeline_shader_info = { VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO };
//	pipeline_shader_info.pName = "main";
//
//	const char* name = "name";
//	shader_info.codeSize = desc->vert_code_size;
//	shader_info.pCode = desc->vert_code;
//	VK_CHECK(vkCreateShaderModule(context->device, &shader_info, NULL, &pipeline->vertex_module));
//	shader_info.codeSize = desc->frag_code_size;
//	shader_info.pCode = desc->frag_code;
//	VK_CHECK(vkCreateShaderModule(context->device, &shader_info, NULL, &pipeline->fragment_module));
//
//	VkPipelineShaderStageCreateInfo shader_stages[2] = { 0 };
//	shader_stages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
//	shader_stages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
//	shader_stages[0].module = pipeline->vertex_module;
//	shader_stages[0].pName = "main";
//	shader_stages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
//	shader_stages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
//	shader_stages[1].module = pipeline->fragment_module;
//	shader_stages[1].pName = "main";
//
//	VkPipelineVertexInputStateCreateInfo input_state = {
//		.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO
//	};
//
//	VkPipelineInputAssemblyStateCreateInfo input_assembly = {
//		.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
//		.topology = desc->topology
//	};
//
//	VkPipelineRasterizationStateCreateInfo rasterization_state = {
//		.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
//		.polygonMode = desc->polygon_mode,
//		.lineWidth = 1.0f,
//		.cullMode = VK_CULL_MODE_BACK_BIT,
//		.frontFace = VK_FRONT_FACE_CLOCKWISE
//	};
//
//	VkPipelineMultisampleStateCreateInfo multisample_state = {
//		.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
//		.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
//		.minSampleShading = 1.0f
//	};
//
//	VkViewport viewport = {
//		.width = (float)context->extent.width,
//		.height = (float)context->extent.height,
//		.minDepth = 0.0f,
//		.maxDepth = 1.0f
//	};
//
//	VkRect2D scissor = { 0 };
//	scissor.extent = context->extent;
//
//	VkPipelineViewportStateCreateInfo viewport_state = {
//		.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
//		.viewportCount = 1,
//		.pViewports = &viewport,
//		.scissorCount = 1,
//		.pScissors = &scissor
//	};
//
//	VkPipelineColorBlendAttachmentState color_blend_attachment_state = {
//		.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
//			VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT
//	};
//
//	VkPipelineColorBlendStateCreateInfo color_blend_state = {
//		.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
//		.logicOp = VK_LOGIC_OP_COPY,
//		.attachmentCount = 1,
//		.pAttachments = &color_blend_attachment_state
//	};
//
//	VkGraphicsPipelineCreateInfo pipeline_info = {
//		.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
//		.stageCount = 2,
//		.pStages = shader_stages,
//		.pVertexInputState = &input_state,
//		.pInputAssemblyState = &input_assembly,
//		.pRasterizationState = &rasterization_state,
//		.pMultisampleState = &multisample_state,
//		.pColorBlendState = &color_blend_state,
//		.pViewportState = &viewport_state,
//		.layout = desc->pipeline_layout,
//		.renderPass = context->default_render_pass,
//	};
//	VK_CHECK(vkCreateGraphicsPipelines(context->device, context->pipeline_cache, 1, &pipeline_info, NULL, &pipeline->pipeline));
//}
//
//void graphics_init(uint16_t width, uint16_t height, bool fullscreen)
//{
//	// Command buffers
//	/*{
//		VkCommandBufferAllocateInfo command_buffer_info = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
//		command_buffer_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
//		command_buffer_info.commandBufferCount = 1;
//
//		for (uint32_t i = 0; i < context->buffering_count; ++i)
//		{
//			command_buffer_info.commandPool = context->command_pools[i];
//			VK_CHECK(vkAllocateCommandBuffers(context->device, &command_buffer_info, &context->command_buffers[i]));
//		}
//	}*/
//
//
//	// Render pass
//	{
//		VkAttachmentDescription color_attachment = {
//			.format = context->swapchain_format,
//			.samples = VK_SAMPLE_COUNT_1_BIT, // TODO: MSAA
//			.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
//			.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
//			.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
//			.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
//			.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
//			.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
//		};
//
//		VkAttachmentReference color_attachment_ref = {
//			color_attachment_ref.attachment = 0,
//			color_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
//		};
//
//		VkSubpassDescription subpass = {
//			.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
//			.colorAttachmentCount = 1,
//			.pColorAttachments = &color_attachment_ref
//		};
//
//		VkRenderPassCreateInfo render_pass_info = {
//			.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
//			.attachmentCount = 1,
//			.pAttachments = &color_attachment,
//			.subpassCount = 1,
//			.pSubpasses = &subpass
//		};
//
//		VK_CHECK(vkCreateRenderPass(context->device, &render_pass_info, NULL, &context->default_render_pass));
//
//		VkFramebufferCreateInfo framebuffer_info = {
//			.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
//			.renderPass = context->default_render_pass,
//			.attachmentCount = 1,
//			.width = width,
//			.height = height,
//			.layers = 1
//		};
//
//		for (uint32_t i = 0; i < context->swapchain_image_count; i++) {
//
//			framebuffer_info.pAttachments = &context->swapchain_image_views[i];
//			VK_CHECK(vkCreateFramebuffer(context->device, &framebuffer_info, NULL, &context->framebuffers[i]));
//		}
//	}
//
//	// Descriptor pools
//	for (uint32_t i = 0; i < jobs_thread_count; ++i)
//	{
//		VkDescriptorPoolSize descriptor_pool_sizes[] =
//		{
//			{ VK_DESCRIPTOR_TYPE_SAMPLER, MAX_SAMPLERS },
//			{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1 },
//			{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, MAX_SAMPLED_IMAGES },
//			{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, MAX_STORAGE_IMAGES },
//			{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, MAX_UNIFORM_TEXEL_BUFFERS },
//			{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, MAX_STORAGE_TEXEL_BUFFERS },
//			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, MAX_UNIFORM_BUFFERS },
//			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, MAX_STORAGE_BUFFERS },
//			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, MAX_UNIFORM_BUFFER_DYNAMIC },
//			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1 },
//			{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1 }
//		};
//
//		VkDescriptorPoolCreateInfo descriptor_pool_info = { VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO };
//		descriptor_pool_info.poolSizeCount = sizeof(descriptor_pool_sizes) / sizeof(descriptor_pool_sizes[0]);
//		descriptor_pool_info.pPoolSizes = descriptor_pool_sizes;
//		descriptor_pool_info.maxSets = MAX_DESCRIPTOR_SETS;
//		VK_CHECK(vkCreateDescriptorPool(context->device, &descriptor_pool_info, NULL, &context->descriptor_pools[i]));
//	}
//
//	// Pipelines
//	{
//		// Line
//		for (uint32_t i = 0; i < context->buffering_count; ++i)
//		{
//			create_buffer(MAX_LINES * sizeof(float) * 2,
//				VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
//				VMA_MEMORY_USAGE_CPU_TO_GPU, false,
//				&context->line_position_buffers[i]);
//		}
//
//		VkDescriptorSetLayoutBinding position_binding = {
//			.binding = 0,
//			.descriptorCount = 1,
//			.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
//			.stageFlags = VK_SHADER_STAGE_VERTEX_BIT
//		};
//
//		VkDescriptorSetLayoutBinding color_binding = {
//			.binding = 1,
//			.descriptorCount = 1,
//			.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
//			.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT
//		};
//
//		VkDescriptorSetLayoutCreateInfo set_layout_info = {
//			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
//			.bindingCount = 2,
//			.pBindings = (const VkDescriptorSetLayoutBinding[]) { position_binding, color_binding }
//		};
//		VK_CHECK(vkCreateDescriptorSetLayout(context->device, &set_layout_info, NULL, &context->line_descriptor_set_layout));
//
//		VkDescriptorSetAllocateInfo set_allocate_info = {
//			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
//			.descriptorPool = context->descriptor_pool,
//			.descriptorSetCount = 1,
//			.pSetLayouts = &context->line_descriptor_set_layout
//		};
//
//		VK_CHECK(vkAllocateDescriptorSets(context->device, &set_allocate_info, &context->line_descriptor_set));
//
//		VkPushConstantRange fragment_push_constant_range = {
//			.size = sizeof(uint32_t),
//			.offset = 0,
//			.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT
//		};
//
//		VkPipelineLayoutCreateInfo layout_info = {
//			.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
//			.setLayoutCount = 1,
//			.pSetLayouts = &context->line_descriptor_set_layout,
//			.pushConstantRangeCount = 1,
//			.pPushConstantRanges = (const VkPushConstantRange[]) { fragment_push_constant_range }
//		};
//
//		VK_CHECK(vkCreatePipelineLayout(context->device, &layout_info, NULL, &context->line_pipeline.pipeline_layout));
//
//		GraphicsPipelineDesc pipeline_desc = {
//			.vert_code = line_vert_spirv,
//			.vert_code_size = sizeof(line_vert_spirv),
//			.frag_code = line_frag_spirv,
//			.frag_code_size = sizeof(line_frag_spirv),
//			.topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST,
//			.polygon_mode = VK_POLYGON_MODE_LINE,
//			.pipeline_layout = context->line_pipeline.pipeline_layout
//		};
//
//		create_graphics_pipeline(&pipeline_desc, &context->line_pipeline);
//	}
//}
//
//void graphics_quit(void)
//{
//	vkDeviceWaitIdle(context->device);
//
//	vkDestroyPipeline(context->device, context->sprite_pipeline.pipeline, NULL);
//	vkDestroyShaderModule(context->device, context->sprite_pipeline.vertex_module, NULL);
//	vkDestroyShaderModule(context->device, context->sprite_pipeline.fragment_module, NULL);
//	vkDestroyPipelineLayout(context->device, context->sprite_pipeline.pipeline_layout, NULL);
//
//	vkDestroyPipeline(context->device, context->line_pipeline.pipeline, NULL);
//	vkDestroyShaderModule(context->device, context->line_pipeline.vertex_module, NULL);
//	vkDestroyShaderModule(context->device, context->line_pipeline.fragment_module, NULL);
//	vkDestroyPipelineLayout(context->device, context->line_pipeline.pipeline_layout, NULL);
//	vkDestroyDescriptorSetLayout(context->device, context->line_descriptor_set_layout, NULL);
//
//	vkDestroySampler(context->device, context->linear_clamp_sampler, NULL);
//	vkDestroyRenderPass(context->device, context->default_render_pass, NULL);
//	vkDestroyPipelineCache(context->device, context->pipeline_cache, NULL);
//	vkDestroySemaphore(context->device, context->image_acquired_semaphore, NULL);
//	for (uint32_t i = 0; i < context->buffering_count; ++i)
//	{
//		vmaDestroyBuffer(context->allocator, context->line_position_buffers[i].buffer, context->line_position_buffers[i].allocation);
//		vmaDestroyBuffer(context->allocator, context->line_color_buffers[i].buffer, context->line_color_buffers[i].allocation);
//
//		vkDestroySemaphore(context->device, context->render_complete_semaphores[i], NULL);
//		vkDestroyFence(context->device, context->render_complete_fences[i], NULL);
//		//vkDestroyCommandPool(context->device, context->command_pools[i], NULL);
//	}
//	for (uint32_t i = 0; i < context->swapchain_image_count; ++i)
//	{
//		vkDestroyFramebuffer(context->device, context->framebuffers[i], NULL);
//		vkDestroyImageView(context->device, context->swapchain_image_views[i], NULL);
//	}
//
//	for (uint32_t i = 0; i < jobs_thread_count; ++i)
//	{
//		vkDestroyDescriptorPool(context->device, context->descriptor_pools[i], NULL);
//	}
//	vkDestroyCommandPool(context->device, context->main_command_pool, NULL);
//
//	
//	
//	vkDestroySwapchainKHR(context->device, context->swapchain, NULL);
//	vkDestroySurfaceKHR(context->instance, context->surface, NULL);
//
//
//	SDL_DestroyWindow(context->window);
//
//	free(renderer);
//}
