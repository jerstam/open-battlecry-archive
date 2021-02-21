#pragma once

#include "../macros.h"

#ifdef _DEBUG
#define VK_CHECK(expr) do { VkResult _res = (expr); assert(_res == VK_SUCCESS); } while(0)
#else
#define VK_CHECK(expr) do { (expr); } while(0)
#endif

typedef struct pass_t pass_t;
typedef struct image_t image_t;
typedef struct buffer_t buffer_t;
typedef struct shader_t shader_t;
typedef struct pipeline_t pipeline_t;
typedef struct descriptor_set_t descriptor_set_t;

typedef struct VkInstance_T* VkInstance;
typedef struct VkPhysicalDevice_T* VkPhysicalDevice;
typedef struct VkSurfaceKHR_T* VkSurfaceKHR;
typedef struct VkDevice_T* VkDevice;
typedef struct VkDebugUtilsMessengerEXT_T* VkDebugUtilsMessengerEXT;
typedef struct VkQueue_T* VkQueue;
typedef struct VkDescriptorPool_T* VkDescriptorPool;
typedef struct VkPipelineCache_T* VkPipelineCache;
typedef struct VkCommandPool_T* VkCommandPool;
typedef struct VkCommandBuffer_T* VkCommandBuffer;
typedef struct VkSwapchainKHR_T* VkSwapchainKHR;
typedef struct VkImage_T* VkImage;
typedef struct VkImageView_T* VkImageView;
typedef struct VkFence_T* VkFence;
typedef struct VkSemaphore_T* VkSemaphore;
typedef struct VkRenderPass_T* VkRenderPass;
typedef struct VkFramebuffer_T* VkFramebuffer;
typedef struct VkSampler_T* VkSampler;
typedef struct VkBuffer_T* VkBuffer;
typedef struct VkDescriptorSetLayout_T* VkDescriptorSetLayout;
typedef struct VkDescriptorSet_T* VkDescriptorSet;
typedef struct VkDeviceMemory_T* VkDeviceMemory;

//
// GPU
//
enum
{
    FRAME_COUNT = 2,
    MAX_SWAPCHAIN_IMAGES = 3,
    MAX_PHYSICAL_DEVICES = 4,
};

typedef union clear_value_t
{
    float color[4];
    struct
    {
        float depth;
        u32 stencil;
    };
} clear_value_t;

typedef struct memory_type_t
{
    u32 property_flags;
    u32 heap_index;
} memory_type_t;

typedef struct memory_heap_t
{
    u64 size;
    u32 flags;
} memory_heap_t;

typedef struct gpu_properties_t
{
    char name[256];
    u64 buffer_image_granularity;
    u64 non_coherent_atom_size;
    u32 memory_type_count;
    memory_type_t memory_types[32];
    u32 memory_heap_count;
    memory_heap_t memory_heaps[16];
    bool is_integrated;
} gpu_properties_t;

typedef struct gpu_t
{
    VkInstance instance;
    u32 physical_device_index;
    u32 physical_device_count;
    VkPhysicalDevice physical_devices[MAX_PHYSICAL_DEVICES];
    gpu_properties_t gpu_properties[MAX_PHYSICAL_DEVICES];
    VkDevice device;
    VkDebugUtilsMessengerEXT debug_messenger;

    VkSurfaceKHR surface;
    u16 width;
    u16 height;

    u32 queue_family_count;
    u8 graphics_queue_index;
    u8 transfer_queue_index;
    u8 compute_queue_index;
    VkQueue graphics_queue;
    VkQueue transfer_queue;
    VkQueue compute_queue;

    VkDescriptorPool descriptor_pools[MAX_THREADS];
    VkPipelineCache pipeline_cache;

    VkSwapchainKHR swapchain;
    VkImage swapchain_images[MAX_SWAPCHAIN_IMAGES];
    VkImageView swapchain_image_views[MAX_SWAPCHAIN_IMAGES];

    VkCommandPool graphics_command_pools[FRAME_COUNT];
    VkCommandPool transfer_command_pool;
    VkCommandBuffer graphics_command_buffers[FRAME_COUNT];
    VkCommandBuffer transfer_command_buffer;

    VkFence render_complete_fences[FRAME_COUNT];
    VkSemaphore image_acquired_semaphore;
    VkSemaphore render_complete_semaphores[FRAME_COUNT];

    u32 frame_count;
    u32 frame_index;
    u32 swapchain_image_count;
    u32 swapchain_image_index;

    VkRenderPass default_render_pass;
    VkRenderPass post_process_render_pass;
    VkFramebuffer framebuffers[FRAME_COUNT];
    clear_value_t clear_value;

    VkDescriptorSetLayout textures_set_layout;
    VkDescriptorSet textures_set;

    VkDescriptorSetLayout storage_set_layout;
    VkDescriptorSet storage_set;

    VkSampler linear_clamp_sampler;
    VkSampler nearest_clamp_sampler;
} gpu_t;

extern gpu_t gpu;

void gpu_init(void);
void gpu_quit(void);

void gpu_begin_frame(void);
void gpu_end_frame(void);

// 
// Buffer
//
typedef enum
{
    BUFFER_VERTEX = 1,
    BUFFER_INDEX = 2,
    BUFFER_UNIFORM = 4,
    BUFFER_STORAGE = 8,
    BUFFER_INDIRECT = 16,

    BUFFER_CPU = 32,
    BUFFER_CPU_TO_GPU = 64,
    BUFFER_PERSISTENT = 128
} buffer_flags_t;

struct buffer_t
{
    VkBuffer buffer;
    u64 memory_size;
    u64 memory_offset;
    u8* data;
    u32 size;
    bool persistent;
};

void gpu_create_buffer(u32 size, buffer_flags_t buffer_flags, buffer_t* buffer);
void gpu_destroy_buffer(buffer_t* buffer);

// 
// Image
//
typedef enum
{
    IMAGE_TYPE_SAMPLED,
    IMAGE_TYPE_STORAGE,
    IMAGE_TYPE_RENDER_TARGET,
} image_type_t;

typedef struct image_desc_t
{
    u64 memory_size;
    u64 memory_offset;
    image_type_t image_type;
    u32 width;
    u32 height;
    u32 mip_levels;
    u32 sample_count;
    clear_value_t clear_value;
    const char* tag;
} image_desc_t;


// 
// Shader
//
typedef enum
{
    SHADER_TYPE_VERTEX,
    SHADER_TYPE_FRAGMENT,
    SHADER_TYPE_COMPUTE
} shader_type_t;

typedef struct shader_desc_t
{
    shader_type_t shader_type;
    const u32* code;
    u32 code_size;
} shader_desc_t;


// 
// Pass
//
typedef struct compute_pass_desc_t
{
    u32 push_constant_size;
    u32 shader_count;
    shader_t* shaders;
} compute_pass_desc_t;

typedef struct graphics_pass_desc_t
{
    u32 push_constant_size;
    u32 shader_count;
    shader_t* shaders;
    image_t* color_attachment;
    image_t* depth_attachment;
} graphics_pass_desc_t;


// 
// Pipeline
//
typedef enum
{
    FILL_MODE_SOLID,
    FILL_MODE_WIREFRAME
} fill_mode_t;

typedef enum
{
    BLEND_MODE_ZERO,
    BLEND_MODE_ONE,
    BLEND_MODE_SRC_COLOR,
    BLEND_MODE_INV_SRC_COLOR,
    BLEND_MODE_SRC_ALPHA,
    BLEND_MODE_INV_SRC_ALPHA,
    BLEND_MODE_DEST_ALPHA,
    BLEND_MODE_INV_DEST_ALPHA,
    BLEND_MODE_DEST_COLOR,
    BLEND_MODE_INV_DEST_COLOR,
    BLEND_MODE_SRC_ALPHA_SAT,
    BLEND_MODE_BLEND_FACTOR,
    BLEND_MODE_INV_BLEND_FACTOR,
    BLEND_MODE_SRC1_COLOR,
    BLEND_MODE_INV_SRC1_COLOR,
    BLEND_MODE_SRC1_ALPHA,
    BLEND_MODE_INV_SRC1_ALPHA
} blend_mode_t;

typedef enum
{
    BLEND_OP_ADD,
    BLEND_OP_SUBTRACT,
    BLEND_OP_REV_SUBTRACT,
    BLEND_OP_MIN,
    BLEND_OP_MAX
} blend_op_t;

typedef enum
{
    LOGIC_OP_CLEAR,
    LOGIC_OP_SET,
    LOGIC_OP_COPY,
    LOGIC_OP_COPY_INVERTED,
    LOGIC_OP_NOOP,
    LOGIC_OP_INVERT,
    LOGIC_OP_AND,
    LOGIC_OP_NAND,
    LOGIC_OP_OR,
    LOGIC_OP_NOR,
    LOGIC_OP_XOR,
    LOGIC_OP_EQUIV,
    LOGIC_OP_AND_REVERSE,
    LOGIC_OP_AND_INVERTED,
    LOGIC_OP_OR_REVERSE,
    LOGIC_OP_OR_INVERTED
} logic_op_t;

typedef struct blend_state_t
{
    bool blend_enable;
    bool logic_op_enable;

    blend_mode_t src_blend;
    blend_mode_t dst_blend;
    blend_op_t blend_op;

    blend_mode_t src_blend_alpha;
    blend_mode_t dst_blend_alpha;
    blend_op_t blend_op_alpha;

    logic_op_t logic_op;
} blend_state_t;

typedef enum
{
    CULL_MODE_BACK,
    CULL_MODE_FRONT,
    CULL_MODE_NONE
} cull_mode_t;

typedef enum
{
    FACE_WINDING_CLOCKWISE,
    FACE_WINDING_COUNTER_CLOCKWISE
} face_winding_t;

typedef enum
{
    PRIMTIVE_TOPOLOGY_TRIANGLES,
    PRIMTIVE_TOPOLOGY_TRIANGLE_STRIP,
    PRIMTIVE_TOPOLOGY_LINES,
    PRIMTIVE_TOPOLOGY_LINE_STRIP
} primitive_topology_t;

typedef enum
{
    SAMPLE_COUNT_1,
    SAMPLE_COUNT_2,
    SAMPLE_COUNT_4,
    SAMPLE_COUNT_8
} sample_count_t;

typedef struct rasterizer_state_t
{
    bool enable_alpha_to_coverage;
    cull_mode_t cull_mode;
    face_winding_t face_winding;
    primitive_topology_t primitive_topology;
    sample_count_t sample_count;
} rasterizer_state_t;

typedef struct pipeline_desc_t
{
    fill_mode_t fill_mode;
    blend_state_t blend_state;
    rasterizer_state_t rasterizer_state;
    shader_t* vertex_shader;
    shader_t* fragment_shader;
} pipeline_desc_t;
