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
//
//void graphics_init(uint16_t width, uint16_t height, bool fullscreen)
//{
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
//	}
//}
//
