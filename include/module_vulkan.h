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
int luaopen_vulkan(lua_State* L);

// Frame buffer
void lua_push_VkFramebufferCreateInfo(lua_State* L, VkFramebufferCreateInfo* create_info, VkImageView* attachments);
lua_VkFramebufferCreateInfo* lua_check_VkFramebufferCreateInfo(lua_State* L, int idx);
void lua_push_VkFramebuffer(lua_State* L, VkFramebuffer framebuffer, VkDevice device);
lua_VkFramebuffer* lua_check_VkFramebuffer(lua_State* L, int idx);
static int l_vulkan_create_framebuffer_create_info(lua_State* L);
static int l_vulkan_create_framebuffer(lua_State* L);

#endif