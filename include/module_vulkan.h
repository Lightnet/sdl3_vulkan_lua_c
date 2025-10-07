// module_vulkan.h
#ifndef MODULE_VULKAN_H
#define MODULE_VULKAN_H

#include <lua.h>
#include <lauxlib.h>
#include <vulkan/vulkan.h>
#include <SDL3/SDL_vulkan.h>
#include "module_sdl.h"

// Existing structs...
typedef struct {
    VkApplicationInfo app_info;
    char* pApplicationName;
} lua_VkApplicationInfo;

typedef struct {
    VkInstanceCreateInfo create_info;
    char** ppEnabledExtensionNames;
    uint32_t enabledExtensionCount;
} lua_VkInstanceCreateInfo;

typedef struct {
    VkInstance instance;
} lua_VkInstance;

typedef struct {
    VkSurfaceKHR surface;
    VkInstance instance;
} lua_VkSurfaceKHR;

typedef struct {
    VkPhysicalDevice device;
} lua_VkPhysicalDevice;

typedef struct {
    VkDeviceQueueCreateInfo queue_create_info;
    float* pQueuePriorities;
} lua_VkDeviceQueueCreateInfo;

typedef struct {
    VkDeviceCreateInfo device_create_info;
    char** ppEnabledExtensionNames;
    uint32_t enabledExtensionCount;
} lua_VkDeviceCreateInfo;

typedef struct {
    VkDevice device;
} lua_VkDevice;

typedef struct {
    VkQueue queue;
} lua_VkQueue;

typedef struct {
    VkSurfaceFormatKHR format;
} lua_VkSurfaceFormatKHR;

typedef struct {
    VkSwapchainCreateInfoKHR swapchain_create_info;
} lua_VkSwapchainCreateInfoKHR;

typedef struct {
    VkSwapchainKHR swapchain;
    VkDevice device; // For cleanup
} lua_VkSwapchainKHR;

typedef struct {
    VkImageViewCreateInfo create_info;
} lua_VkImageViewCreateInfo;

typedef struct {
    VkImageView image_view;
    VkDevice device; // For cleanup
} lua_VkImageView;

typedef struct {
    VkAttachmentDescription desc;
} lua_VkAttachmentDescription;

typedef struct {
    VkSubpassDescription desc;
    VkAttachmentReference* pColorAttachments; // Managed array
} lua_VkSubpassDescription;

typedef struct {
    VkSubpassDependency dep;
} lua_VkSubpassDependency;

typedef struct {
    VkRenderPassCreateInfo create_info;
    VkAttachmentDescription* pAttachments; // Managed array
    VkSubpassDescription* pSubpasses; // Managed array
    VkSubpassDependency* pDependencies; // Managed array
} lua_VkRenderPassCreateInfo;

typedef struct {
    VkRenderPass render_pass;
    VkDevice device; // For cleanup
} lua_VkRenderPass;

typedef struct {
    VkFramebufferCreateInfo create_info;
    VkImageView* pAttachments; // Managed array
} lua_VkFramebufferCreateInfo;

typedef struct {
    VkFramebuffer framebuffer;
    VkDevice device; // For cleanup
} lua_VkFramebuffer;

typedef struct {
    VkExtent2D extent;
} lua_VkExtent2D;

typedef struct {
    VkShaderModule shader_module;
    VkDevice device; // For cleanup
} lua_VkShaderModule;

typedef struct {
    VkPipelineShaderStageCreateInfo stage_info;
} lua_VkPipelineShaderStageCreateInfo;

typedef struct {
    VkVertexInputBindingDescription binding;
} lua_VkVertexInputBindingDescription;

typedef struct {
    VkVertexInputAttributeDescription attrib;
} lua_VkVertexInputAttributeDescription;

typedef struct {
    VkPipelineVertexInputStateCreateInfo vertex_input_info;
    VkVertexInputBindingDescription* pBindings; // Managed array
    VkVertexInputAttributeDescription* pAttributes; // Managed array
} lua_VkPipelineVertexInputStateCreateInfo;

typedef struct {
    VkPipelineInputAssemblyStateCreateInfo input_assembly;
} lua_VkPipelineInputAssemblyStateCreateInfo;

typedef struct {
    VkViewport viewport;
} lua_VkViewport;

typedef struct {
    VkRect2D scissor;
} lua_VkRect2D;

typedef struct {
    VkPipelineViewportStateCreateInfo viewport_state;
    VkViewport* pViewports; // Managed array
    VkRect2D* pScissors; // Managed array
} lua_VkPipelineViewportStateCreateInfo;

typedef struct {
    VkPipelineRasterizationStateCreateInfo rasterizer;
} lua_VkPipelineRasterizationStateCreateInfo;

typedef struct {
    VkPipelineMultisampleStateCreateInfo multisampling;
} lua_VkPipelineMultisampleStateCreateInfo;

typedef struct {
    VkPipelineColorBlendAttachmentState blend_attachment;
} lua_VkPipelineColorBlendAttachmentState;

typedef struct {
    VkPipelineColorBlendStateCreateInfo blend_state;
    VkPipelineColorBlendAttachmentState* pAttachments; // Managed array
} lua_VkPipelineColorBlendStateCreateInfo;

typedef struct {
    VkPipelineLayoutCreateInfo layout_info;
} lua_VkPipelineLayoutCreateInfo;

typedef struct {
    VkPipelineLayout pipeline_layout;
    VkDevice device; // For cleanup
} lua_VkPipelineLayout;

typedef struct {
    VkGraphicsPipelineCreateInfo pipeline_info;
    VkPipelineShaderStageCreateInfo* pStages; // Managed array
} lua_VkGraphicsPipelineCreateInfo;

typedef struct {
    VkPipeline pipeline;
    VkDevice device; // For cleanup
} lua_VkPipeline;

typedef struct {
    VkBufferCreateInfo create_info;
} lua_VkBufferCreateInfo;

typedef struct {
    VkBuffer buffer;
    VkDevice device; // For cleanup
} lua_VkBuffer;

typedef struct {
    VkMemoryAllocateInfo alloc_info;
} lua_VkMemoryAllocateInfo;

typedef struct {
    VkDeviceMemory memory;
    VkDevice device; // For cleanup
} lua_VkDeviceMemory;

typedef struct {
    VkCommandPoolCreateInfo create_info;
} lua_VkCommandPoolCreateInfo;

typedef struct {
    VkCommandPool command_pool;
    VkDevice device; // For cleanup
} lua_VkCommandPool;

typedef struct {
    VkCommandBufferAllocateInfo alloc_info;
} lua_VkCommandBufferAllocateInfo;

typedef struct {
    VkCommandBuffer command_buffer;
    VkDevice device; // For cleanup
} lua_VkCommandBuffer;

typedef struct {
    VkMemoryRequirements mem_requirements;
} lua_VkMemoryRequirements;

typedef struct {
    VkPhysicalDeviceMemoryProperties mem_properties;
} lua_VkPhysicalDeviceMemoryProperties;


// Existing function declarations...
void lua_push_VkApplicationInfo(lua_State* L, VkApplicationInfo* app_info, const char* app_name);
lua_VkApplicationInfo* lua_check_VkApplicationInfo(lua_State* L, int idx);
void lua_push_VkInstanceCreateInfo(lua_State* L, VkInstanceCreateInfo* create_info, char** extensions, uint32_t extension_count);
lua_VkInstanceCreateInfo* lua_check_VkInstanceCreateInfo(lua_State* L, int idx);
void lua_push_VkInstance(lua_State* L, VkInstance instance);
lua_VkInstance* lua_check_VkInstance(lua_State* L, int idx);
void lua_push_VkSurfaceKHR(lua_State* L, VkSurfaceKHR surface, VkInstance instance);
lua_VkSurfaceKHR* lua_check_VkSurfaceKHR(lua_State* L, int idx);
void lua_push_VkPhysicalDevice(lua_State* L, VkPhysicalDevice device);
lua_VkPhysicalDevice* lua_check_VkPhysicalDevice(lua_State* L, int idx);
void lua_push_VkDeviceQueueCreateInfo(lua_State* L, VkDeviceQueueCreateInfo* queue_create_info, float* priorities);
lua_VkDeviceQueueCreateInfo* lua_check_VkDeviceQueueCreateInfo(lua_State* L, int idx);
void lua_push_VkDeviceCreateInfo(lua_State* L, VkDeviceCreateInfo* device_create_info, char** extensions, uint32_t extension_count);
lua_VkDeviceCreateInfo* lua_check_VkDeviceCreateInfo(lua_State* L, int idx);
void lua_push_VkDevice(lua_State* L, VkDevice device);
lua_VkDevice* lua_check_VkDevice(lua_State* L, int idx);
void lua_push_VkQueue(lua_State* L, VkQueue queue);
lua_VkQueue* lua_check_VkQueue(lua_State* L, int idx);
void lua_push_VkSurfaceFormatKHR(lua_State* L, VkSurfaceFormatKHR format);
lua_VkSurfaceFormatKHR* lua_check_VkSurfaceFormatKHR(lua_State* L, int idx);

// Updated function declarations with static
static int l_vulkan_get_physical_device_surface_capabilities(lua_State* L);
static int l_vulkan_get_physical_device_surface_formats(lua_State* L);
static int l_vulkan_get_physical_device_surface_present_modes(lua_State* L);
int luaopen_vulkan(lua_State* L);

// swap chain
void lua_push_VkSwapchainCreateInfoKHR(lua_State* L, VkSwapchainCreateInfoKHR* swapchain_create_info);
lua_VkSwapchainCreateInfoKHR* lua_check_VkSwapchainCreateInfoKHR(lua_State* L, int idx);
void lua_push_VkSwapchainKHR(lua_State* L, VkSwapchainKHR swapchain, VkDevice device);
lua_VkSwapchainKHR* lua_check_VkSwapchainKHR(lua_State* L, int idx);
static int l_vulkan_create_swapchain_create_info(lua_State* L);
static int l_vulkan_create_swapchain(lua_State* L);
static int l_vulkan_get_swapchain_images(lua_State* L);

// image and render pass
static int l_vulkan_create_image_view_create_info(lua_State* L);
static int l_vulkan_create_image_view(lua_State* L);
static int l_vulkan_create_attachment_description(lua_State* L);
static int l_vulkan_create_subpass_description(lua_State* L);
static int l_vulkan_create_subpass_dependency(lua_State* L);
static int l_vulkan_create_render_pass_create_info(lua_State* L);
static int l_vulkan_create_render_pass(lua_State* L);


// Frame buffer
void lua_push_VkFramebufferCreateInfo(lua_State* L, VkFramebufferCreateInfo* create_info, VkImageView* attachments);
lua_VkFramebufferCreateInfo* lua_check_VkFramebufferCreateInfo(lua_State* L, int idx);
void lua_push_VkFramebuffer(lua_State* L, VkFramebuffer framebuffer, VkDevice device);
lua_VkFramebuffer* lua_check_VkFramebuffer(lua_State* L, int idx);
static int l_vulkan_create_framebuffer_create_info(lua_State* L);
static int l_vulkan_create_framebuffer(lua_State* L);


// pipeline
static int l_vulkan_create_extent_2d(lua_State* L);
static int l_vulkan_create_shader_module(lua_State* L);
static int l_vulkan_create_pipeline_shader_stage_create_info(lua_State* L);
static int l_vulkan_create_vertex_input_binding_description(lua_State* L);
static int l_vulkan_create_vertex_input_attribute_description(lua_State* L);
static int l_vulkan_create_pipeline_vertex_input_state_create_info(lua_State* L);
static int l_vulkan_create_pipeline_input_assembly_state_create_info(lua_State* L);
static int l_vulkan_create_viewport(lua_State* L);
static int l_vulkan_create_rect_2d(lua_State* L);
static int l_vulkan_create_pipeline_viewport_state_create_info(lua_State* L);
static int l_vulkan_create_pipeline_rasterization_state_create_info(lua_State* L);
static int l_vulkan_create_pipeline_multisample_state_create_info(lua_State* L);
static int l_vulkan_create_pipeline_color_blend_attachment_state(lua_State* L);
static int l_vulkan_create_pipeline_color_blend_state_create_info(lua_State* L);
static int l_vulkan_create_pipeline_layout_create_info(lua_State* L);
static int l_vulkan_create_pipeline_layout(lua_State* L);
static int l_vulkan_create_graphics_pipeline_create_info(lua_State* L);
static int l_vulkan_create_graphics_pipeline(lua_State* L);

// command
void lua_push_VkBufferCreateInfo(lua_State* L, VkBufferCreateInfo* create_info);
lua_VkBufferCreateInfo* lua_check_VkBufferCreateInfo(lua_State* L, int idx);
void lua_push_VkBuffer(lua_State* L, VkBuffer buffer, VkDevice device);
lua_VkBuffer* lua_check_VkBuffer(lua_State* L, int idx);
void lua_push_VkMemoryAllocateInfo(lua_State* L, VkMemoryAllocateInfo* alloc_info);
lua_VkMemoryAllocateInfo* lua_check_VkMemoryAllocateInfo(lua_State* L, int idx);
void lua_push_VkDeviceMemory(lua_State* L, VkDeviceMemory memory, VkDevice device);
lua_VkDeviceMemory* lua_check_VkDeviceMemory(lua_State* L, int idx);
void lua_push_VkCommandPoolCreateInfo(lua_State* L, VkCommandPoolCreateInfo* create_info);
lua_VkCommandPoolCreateInfo* lua_check_VkCommandPoolCreateInfo(lua_State* L, int idx);
void lua_push_VkCommandPool(lua_State* L, VkCommandPool command_pool, VkDevice device);
lua_VkCommandPool* lua_check_VkCommandPool(lua_State* L, int idx);
void lua_push_VkCommandBufferAllocateInfo(lua_State* L, VkCommandBufferAllocateInfo* alloc_info);
lua_VkCommandBufferAllocateInfo* lua_check_VkCommandBufferAllocateInfo(lua_State* L, int idx);
void lua_push_VkCommandBuffer(lua_State* L, VkCommandBuffer command_buffer, VkDevice device);
lua_VkCommandBuffer* lua_check_VkCommandBuffer(lua_State* L, int idx);
static int l_vulkan_create_buffer_create_info(lua_State* L);
static int l_vulkan_create_buffer(lua_State* L);
static int l_vulkan_allocate_memory(lua_State* L);
static int l_vulkan_bind_buffer_memory(lua_State* L);
static int l_vulkan_map_memory(lua_State* L);
static int l_vulkan_create_command_pool_create_info(lua_State* L);
static int l_vulkan_create_command_pool(lua_State* L);
static int l_vulkan_create_command_buffer_allocate_info(lua_State* L);
static int l_vulkan_allocate_command_buffers(lua_State* L);
static int l_vulkan_begin_command_buffer(lua_State* L);
static int l_vulkan_cmd_begin_render_pass(lua_State* L);
static int l_vulkan_cmd_bind_pipeline(lua_State* L);
static int l_vulkan_cmd_bind_vertex_buffers(lua_State* L);
static int l_vulkan_cmd_draw(lua_State* L);
static int l_vulkan_cmd_end_render_pass(lua_State* L);
static int l_vulkan_end_command_buffer(lua_State* L);

void lua_push_VkMemoryAllocateInfo(lua_State* L, VkMemoryAllocateInfo* alloc_info);
lua_VkMemoryAllocateInfo* lua_check_VkMemoryAllocateInfo(lua_State* L, int idx);
void lua_push_VkMemoryRequirements(lua_State* L, VkMemoryRequirements* mem_requirements);
lua_VkMemoryRequirements* lua_check_VkMemoryRequirements(lua_State* L, int idx);
static int l_vulkan_create_memory_allocate_info(lua_State* L);
static int l_vulkan_get_buffer_memory_requirements(lua_State* L);


int luaopen_vulkan(lua_State* L);
#endif