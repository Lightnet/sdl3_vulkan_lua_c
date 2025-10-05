// module_vulkan.h
#ifndef MODULE_VULKAN_H
#define MODULE_VULKAN_H

#include <lua.h>
#include <lauxlib.h>
#include <vulkan/vulkan.h>
#include <SDL3/SDL_vulkan.h>
#include "module_sdl.h"

typedef struct {
    VkApplicationInfo app_info;
    char* pApplicationName; // Managed string for Lua
} lua_VkApplicationInfo;

typedef struct {
    VkInstanceCreateInfo create_info;
    char** ppEnabledExtensionNames; // Managed array of extension names
    uint32_t enabledExtensionCount; // Store count for memory management
} lua_VkInstanceCreateInfo;

typedef struct {
    VkInstance instance;
} lua_VkInstance;

typedef struct {
    VkSurfaceKHR surface;
    VkInstance instance; // Store instance for vkDestroySurfaceKHR
} lua_VkSurfaceKHR;

typedef struct {
    VkPhysicalDevice device;
} lua_VkPhysicalDevice;

typedef struct {
    VkDeviceQueueCreateInfo queue_create_info;
    float* pQueuePriorities; // Managed array for queue priorities
} lua_VkDeviceQueueCreateInfo;

typedef struct {
    VkDeviceCreateInfo device_create_info;
    char** ppEnabledExtensionNames; // Managed array of extension names
    uint32_t enabledExtensionCount; // Store count for memory management
} lua_VkDeviceCreateInfo;

typedef struct {
    VkDevice device;
} lua_VkDevice;

typedef struct {
    VkQueue queue;
} lua_VkQueue;

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

// New function declarations
void lua_push_VkDeviceQueueCreateInfo(lua_State* L, VkDeviceQueueCreateInfo* queue_create_info, float* priorities);
lua_VkDeviceQueueCreateInfo* lua_check_VkDeviceQueueCreateInfo(lua_State* L, int idx);
void lua_push_VkDeviceCreateInfo(lua_State* L, VkDeviceCreateInfo* device_create_info, char** extensions, uint32_t extension_count);
lua_VkDeviceCreateInfo* lua_check_VkDeviceCreateInfo(lua_State* L, int idx);
void lua_push_VkDevice(lua_State* L, VkDevice device);
lua_VkDevice* lua_check_VkDevice(lua_State* L, int idx);
void lua_push_VkQueue(lua_State* L, VkQueue queue);
lua_VkQueue* lua_check_VkQueue(lua_State* L, int idx);
int luaopen_vulkan(lua_State* L);

#endif