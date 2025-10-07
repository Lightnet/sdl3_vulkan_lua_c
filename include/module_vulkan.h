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

// typedef struct {
//     VkSurfaceKHR surface;
//     VkInstance instance; // For cleanup
// } lua_VkSurface;


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

typedef struct {
    VkSemaphoreCreateInfo create_info;
} lua_VkSemaphoreCreateInfo;

typedef struct {
    VkSemaphore semaphore;
    VkDevice device; // For cleanup
} lua_VkSemaphore;

// Add struct for VkSubmitInfo
typedef struct {
    VkSubmitInfo submit_info;
    VkSemaphore* pWaitSemaphores; // Managed array
    VkPipelineStageFlags* pWaitDstStageMask; // Managed array
    VkCommandBuffer* pCommandBuffers; // Managed array
    VkSemaphore* pSignalSemaphores; // Managed array
} lua_VkSubmitInfo;

// Add struct for VkPresentInfoKHR
typedef struct {
    VkPresentInfoKHR present_info;
    VkSemaphore* pWaitSemaphores; // Managed array
    VkSwapchainKHR* pSwapchains; // Managed array
    uint32_t* pImageIndices; // Managed array
} lua_VkPresentInfoKHR;


int luaopen_vulkan(lua_State* L);
#endif