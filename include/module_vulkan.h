// module_vulkan.h
#ifndef MODULE_VULKAN_H
#define MODULE_VULKAN_H

#include <lua.h>
#include <lauxlib.h>
#include <vulkan/vulkan.h>

typedef struct {
    VkApplicationInfo* app_info;
} lua_VkApplicationInfo;

typedef struct {
    VkInstance instance;
} lua_VkInstance;

typedef struct {
    VkInstanceCreateInfo* create_info;
} lua_VkInstanceCreateInfo;

typedef struct {
    VkSurfaceKHR surface;
    VkInstance instance;
} lua_VkSurfaceKHR;

typedef struct {
    VkPhysicalDevice physical_device;
} lua_VkPhysicalDevice;

typedef struct {
    VkDevice device;
} lua_VkDevice;

typedef struct {
    VkDeviceCreateInfo* create_info;
} lua_VkDeviceCreateInfo;

typedef struct {
    VkQueue queue;
} lua_VkQueue;

typedef struct {
    VkSwapchainKHR swapchain;
    VkDevice device;
} lua_VkSwapchainKHR;

typedef struct {
    VkImageView image_view;
    VkDevice device;
} lua_VkImageView;

typedef struct {
    VkRenderPass render_pass;
    VkDevice device;
} lua_VkRenderPass;

typedef struct {
    VkFramebuffer framebuffer;
    VkDevice device;
} lua_VkFramebuffer;

typedef struct {
    VkShaderModule shader_module;
    VkDevice device;
} lua_VkShaderModule;

typedef struct {
    VkPipelineLayout pipeline_layout;
    VkDevice device;
} lua_VkPipelineLayout;

typedef struct {
    VkPipeline pipeline;
    VkDevice device;
} lua_VkPipeline;

typedef struct {
    VkSemaphore semaphore;
    VkDevice device;
} lua_VkSemaphore;

typedef struct {
    VkFence fence;
    VkDevice device;
} lua_VkFence;

typedef struct {
    VkCommandPool command_pool;
    VkDevice device;
} lua_VkCommandPool;

typedef struct {
    VkCommandBuffer command_buffer;
    VkDevice device;
} lua_VkCommandBuffer;

// Function prototypes for pushing/checking userdata
void lua_push_VkApplicationInfo(lua_State* L, VkApplicationInfo* app_info);
lua_VkApplicationInfo* lua_check_VkApplicationInfo(lua_State* L, int idx);
void lua_push_VkInstance(lua_State* L, VkInstance instance);
lua_VkInstance* lua_check_VkInstance(lua_State* L, int idx);
void lua_push_VkInstanceCreateInfo(lua_State* L, VkInstanceCreateInfo* create_info);
lua_VkInstanceCreateInfo* lua_check_VkInstanceCreateInfo(lua_State* L, int idx);
void lua_push_VkSurfaceKHR(lua_State* L, VkSurfaceKHR surface, VkInstance instance);
lua_VkSurfaceKHR* lua_check_VkSurfaceKHR(lua_State* L, int idx);
void lua_push_VkPhysicalDevice(lua_State* L, VkPhysicalDevice physical_device);
lua_VkPhysicalDevice* lua_check_VkPhysicalDevice(lua_State* L, int idx);
void lua_push_VkDevice(lua_State* L, VkDevice device);
lua_VkDevice* lua_check_VkDevice(lua_State* L, int idx);
void lua_push_VkDeviceCreateInfo(lua_State* L, VkDeviceCreateInfo* create_info);
lua_VkDeviceCreateInfo* lua_check_VkDeviceCreateInfo(lua_State* L, int idx);
void lua_push_VkQueue(lua_State* L, VkQueue queue);
lua_VkQueue* lua_check_VkQueue(lua_State* L, int idx);
void lua_push_VkSwapchainKHR(lua_State* L, VkSwapchainKHR swapchain, VkDevice device);
lua_VkSwapchainKHR* lua_check_VkSwapchainKHR(lua_State* L, int idx);
void lua_push_VkImageView(lua_State* L, VkImageView image_view, VkDevice device);
lua_VkImageView* lua_check_VkImageView(lua_State* L, int idx);
void lua_push_VkRenderPass(lua_State* L, VkRenderPass render_pass, VkDevice device);
lua_VkRenderPass* lua_check_VkRenderPass(lua_State* L, int idx);
void lua_push_VkFramebuffer(lua_State* L, VkFramebuffer framebuffer, VkDevice device);
lua_VkFramebuffer* lua_check_VkFramebuffer(lua_State* L, int idx);
void lua_push_VkShaderModule(lua_State* L, VkShaderModule shader_module, VkDevice device);
lua_VkShaderModule* lua_check_VkShaderModule(lua_State* L, int idx);
void lua_push_VkPipelineLayout(lua_State* L, VkPipelineLayout pipeline_layout, VkDevice device);
lua_VkPipelineLayout* lua_check_VkPipelineLayout(lua_State* L, int idx);
void lua_push_VkPipeline(lua_State* L, VkPipeline pipeline, VkDevice device);
lua_VkPipeline* lua_check_VkPipeline(lua_State* L, int idx);
void lua_push_VkSemaphore(lua_State* L, VkSemaphore semaphore, VkDevice device);
lua_VkSemaphore* lua_check_VkSemaphore(lua_State* L, int idx);
void lua_push_VkFence(lua_State* L, VkFence fence, VkDevice device);
lua_VkFence* lua_check_VkFence(lua_State* L, int idx);
void lua_push_VkCommandPool(lua_State* L, VkCommandPool command_pool, VkDevice device);
lua_VkCommandPool* lua_check_VkCommandPool(lua_State* L, int idx);
void lua_push_VkCommandBuffer(lua_State* L, VkCommandBuffer command_buffer, VkDevice device);
lua_VkCommandBuffer* lua_check_VkCommandBuffer(lua_State* L, int idx);

static int l_vulkan_create_shader_module_str(lua_State* L);

// Module entry point
int luaopen_vulkan(lua_State* L);

#endif