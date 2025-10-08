// module_vulkan.c
#include "module_sdl.h"
#include <SDL3/SDL_vulkan.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "module_vulkan.h" // For lua_SDL_Window
#include <shaderc/shaderc.h>

// Metatable names
static const char* APP_INFO_MT = "vulkan.app_info";
static const char* INSTANCE_MT = "vulkan.instance";
static const char* CREATE_INFO_MT = "vulkan.create_info";
static const char* SURFACE_MT = "vulkan.surface";
static const char* PHYSICAL_DEVICE_MT = "vulkan.physical_device";
static const char* DEVICE_MT = "vulkan.device";
static const char* DEVICE_CREATE_INFO_MT = "vulkan.device_create_info";
static const char* QUEUE_MT = "vulkan.queue";
static const char* SWAPCHAIN_MT = "vulkan.swapchain";
static const char* IMAGE_VIEW_MT = "vulkan.image_view";
static const char* RENDER_PASS_MT = "vulkan.render_pass";
static const char* FRAMEBUFFER_MT = "vulkan.framebuffer";
static const char* SHADER_MODULE_MT = "vulkan.shader_module";
static const char* PIPELINE_LAYOUT_MT = "vulkan.pipeline_layout";
static const char* PIPELINE_MT = "vulkan.pipeline";
static const char* SEMAPHORE_MT = "vulkan.semaphore";
static const char* FENCE_MT = "vulkan.fence";
static const char* COMMAND_POOL_MT = "vulkan.command_pool";
static const char* COMMAND_BUFFER_MT = "vulkan.command_buffer";

// Garbage collection for VkApplicationInfo
static int app_info_gc(lua_State* L) {
    lua_VkApplicationInfo* ud = (lua_VkApplicationInfo*)luaL_checkudata(L, 1, APP_INFO_MT);
    if (ud->app_info) {
        free(ud->app_info);
        ud->app_info = NULL;
    }
    return 0;
}

// Garbage collection for VkInstance
static int instance_gc(lua_State* L) {
    lua_VkInstance* ud = (lua_VkInstance*)luaL_checkudata(L, 1, INSTANCE_MT);
    if (ud->instance) {
        vkDestroyInstance(ud->instance, NULL);
        ud->instance = VK_NULL_HANDLE;
    }
    return 0;
}

// Garbage collection for VkInstanceCreateInfo
static int create_info_gc(lua_State* L) {
    lua_VkInstanceCreateInfo* ud = (lua_VkInstanceCreateInfo*)luaL_checkudata(L, 1, CREATE_INFO_MT);
    if (ud->create_info) {
        // Free extension names
        if (ud->create_info->ppEnabledExtensionNames) {
            for (uint32_t i = 0; i < ud->create_info->enabledExtensionCount; i++) {
                free((char*)ud->create_info->ppEnabledExtensionNames[i]);
            }
            free((char**)ud->create_info->ppEnabledExtensionNames);
        }
        // Free layer names
        if (ud->create_info->ppEnabledLayerNames) {
            for (uint32_t i = 0; i < ud->create_info->enabledLayerCount; i++) {
                free((char*)ud->create_info->ppEnabledLayerNames[i]);
            }
            free((char**)ud->create_info->ppEnabledLayerNames);
        }
        free(ud->create_info);
        ud->create_info = NULL;
    }
    return 0;
}

// Push VkApplicationInfo userdata
void lua_push_VkApplicationInfo(lua_State* L, VkApplicationInfo* app_info) {
    if (!app_info) {
        luaL_error(L, "Cannot create userdata for null VkApplicationInfo");
    }
    lua_VkApplicationInfo* ud = (lua_VkApplicationInfo*)lua_newuserdata(L, sizeof(lua_VkApplicationInfo));
    ud->app_info = app_info;
    luaL_setmetatable(L, APP_INFO_MT);
}

// Check VkApplicationInfo userdata
lua_VkApplicationInfo* lua_check_VkApplicationInfo(lua_State* L, int idx) {
    lua_VkApplicationInfo* ud = (lua_VkApplicationInfo*)luaL_checkudata(L, idx, APP_INFO_MT);
    if (!ud->app_info) {
        luaL_error(L, "Invalid VkApplicationInfo (already destroyed)");
    }
    return ud;
}

// Push VkInstance userdata
void lua_push_VkInstance(lua_State* L, VkInstance instance) {
    if (!instance) {
        luaL_error(L, "Cannot create userdata for null VkInstance");
    }
    lua_VkInstance* ud = (lua_VkInstance*)lua_newuserdata(L, sizeof(lua_VkInstance));
    ud->instance = instance;
    luaL_setmetatable(L, INSTANCE_MT);
}

// Check VkInstance userdata
lua_VkInstance* lua_check_VkInstance(lua_State* L, int idx) {
    lua_VkInstance* ud = (lua_VkInstance*)luaL_checkudata(L, idx, INSTANCE_MT);
    if (!ud->instance) {
        luaL_error(L, "Invalid VkInstance (already destroyed)");
    }
    return ud;
}

// Push VkInstanceCreateInfo userdata
void lua_push_VkInstanceCreateInfo(lua_State* L, VkInstanceCreateInfo* create_info) {
    if (!create_info) {
        luaL_error(L, "Cannot create userdata for null VkInstanceCreateInfo");
    }
    lua_VkInstanceCreateInfo* ud = (lua_VkInstanceCreateInfo*)lua_newuserdata(L, sizeof(lua_VkInstanceCreateInfo));
    ud->create_info = create_info;
    luaL_setmetatable(L, CREATE_INFO_MT);
}

// Check VkInstanceCreateInfo userdata
lua_VkInstanceCreateInfo* lua_check_VkInstanceCreateInfo(lua_State* L, int idx) {
    lua_VkInstanceCreateInfo* ud = (lua_VkInstanceCreateInfo*)luaL_checkudata(L, idx, CREATE_INFO_MT);
    if (!ud->create_info) {
        luaL_error(L, "Invalid VkInstanceCreateInfo (already destroyed)");
    }
    return ud;
}

// Create VkApplicationInfo: vulkan.create_vk_application_info({application_name, application_version, engine_name, engine_version, api_version})
static int l_vulkan_create_vk_application_info(lua_State* L) {
    luaL_checktype(L, 1, LUA_TTABLE);

    VkApplicationInfo* app_info = (VkApplicationInfo*)malloc(sizeof(VkApplicationInfo));
    if (!app_info) {
        luaL_error(L, "Failed to allocate memory for VkApplicationInfo");
    }
    memset(app_info, 0, sizeof(VkApplicationInfo));
    app_info->sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;

    lua_getfield(L, 1, "application_name");
    app_info->pApplicationName = luaL_optstring(L, -1, NULL) ? strdup(luaL_optstring(L, -1, NULL)) : NULL;
    lua_pop(L, 1);

    lua_getfield(L, 1, "application_version");
    app_info->applicationVersion = luaL_checkinteger(L, -1);
    lua_pop(L, 1);

    lua_getfield(L, 1, "engine_name");
    app_info->pEngineName = luaL_optstring(L, -1, NULL) ? strdup(luaL_optstring(L, -1, NULL)) : NULL;
    lua_pop(L, 1);

    lua_getfield(L, 1, "engine_version");
    app_info->engineVersion = luaL_checkinteger(L, -1);
    lua_pop(L, 1);

    lua_getfield(L, 1, "api_version");
    app_info->apiVersion = luaL_checkinteger(L, -1);
    lua_pop(L, 1);

    lua_push_VkApplicationInfo(L, app_info);
    return 1;
}

// Helper function to create version number: vulkan.make_version(major, minor, patch)
static int l_vulkan_make_version(lua_State* L) {
    int major = luaL_checkinteger(L, 1);
    int minor = luaL_checkinteger(L, 2);
    int patch = luaL_checkinteger(L, 3);
    uint32_t version = VK_MAKE_API_VERSION(0, major, minor, patch);
    lua_pushinteger(L, version);
    return 1;
}

// Get SDL Vulkan instance extensions: vulkan.sdl_vulkan_get_instance_extensions()
static int l_vulkan_sdl_vulkan_get_instance_extensions(lua_State* L) {
    Uint32 extension_count = 0;
    const char *const *extensions = SDL_Vulkan_GetInstanceExtensions(&extension_count);
    if (!extensions) {
        luaL_error(L, "Failed to get Vulkan extensions: %s", SDL_GetError());
    }

    lua_newtable(L);
    for (uint32_t i = 0; i < extension_count; i++) {
        lua_pushstring(L, extensions[i]);
        lua_rawseti(L, -2, i + 1); // Lua indices are 1-based
    }

    return 1;
}

// Create VkInstanceCreateInfo: vulkan.create_info({app_info, extensions, layers})
static int l_vulkan_create_info(lua_State* L) {
    luaL_checktype(L, 1, LUA_TTABLE);

    VkInstanceCreateInfo* create_info = (VkInstanceCreateInfo*)malloc(sizeof(VkInstanceCreateInfo));
    if (!create_info) {
        luaL_error(L, "Failed to allocate memory for VkInstanceCreateInfo");
    }
    memset(create_info, 0, sizeof(VkInstanceCreateInfo));
    create_info->sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;

    // Get application info
    lua_getfield(L, 1, "app_info");
    if (!lua_isnil(L, -1)) {
        lua_VkApplicationInfo* app_info_ud = lua_check_VkApplicationInfo(L, -1);
        create_info->pApplicationInfo = app_info_ud->app_info;
    }
    lua_pop(L, 1);

    // Get extensions
    char** extension_names = NULL;
    lua_getfield(L, 1, "extensions");
    if (!lua_isnil(L, -1)) {
        luaL_checktype(L, -1, LUA_TTABLE);
        create_info->enabledExtensionCount = lua_rawlen(L, -1);
        if (create_info->enabledExtensionCount > 0) {
            extension_names = (char**)malloc(create_info->enabledExtensionCount * sizeof(char*));
            if (!extension_names) {
                free(create_info);
                luaL_error(L, "Failed to allocate memory for extension names");
            }
            for (uint32_t i = 1; i <= create_info->enabledExtensionCount; i++) {
                lua_rawgeti(L, -1, i);
                extension_names[i-1] = strdup(luaL_checkstring(L, -1));
                if (!extension_names[i-1]) {
                    for (uint32_t j = 0; j < i-1; j++) {
                        free(extension_names[j]);
                    }
                    free(extension_names);
                    free(create_info);
                    luaL_error(L, "Failed to allocate memory for extension name");
                }
                lua_pop(L, 1);
            }
            create_info->ppEnabledExtensionNames = (const char* const*)extension_names;
        }
    } else {
        create_info->enabledExtensionCount = 0;
        create_info->ppEnabledExtensionNames = NULL;
    }
    lua_pop(L, 1);

    // Get validation layers
    char** layer_names = NULL;
    lua_getfield(L, 1, "layers");
    if (!lua_isnil(L, -1)) {
        luaL_checktype(L, -1, LUA_TTABLE);
        create_info->enabledLayerCount = lua_rawlen(L, -1);
        if (create_info->enabledLayerCount > 0) {
            layer_names = (char**)malloc(create_info->enabledLayerCount * sizeof(char*));
            if (!layer_names) {
                if (extension_names) {
                    for (uint32_t i = 0; i < create_info->enabledExtensionCount; i++) {
                        free(extension_names[i]);
                    }
                    free(extension_names);
                }
                free(create_info);
                luaL_error(L, "Failed to allocate memory for layer names");
            }
            for (uint32_t i = 1; i <= create_info->enabledLayerCount; i++) {
                lua_rawgeti(L, -1, i);
                layer_names[i-1] = strdup(luaL_checkstring(L, -1));
                if (!layer_names[i-1]) {
                    for (uint32_t j = 0; j < i-1; j++) {
                        free(layer_names[j]);
                    }
                    free(layer_names);
                    if (extension_names) {
                        for (uint32_t j = 0; j < create_info->enabledExtensionCount; j++) {
                            free(extension_names[j]);
                        }
                        free(extension_names);
                    }
                    free(create_info);
                    luaL_error(L, "Failed to allocate memory for layer name");
                }
                lua_pop(L, 1);
            }
            create_info->ppEnabledLayerNames = (const char* const*)layer_names;
        }
    } else {
        create_info->enabledLayerCount = 0;
        create_info->ppEnabledLayerNames = NULL;
    }
    lua_pop(L, 1);

    lua_push_VkInstanceCreateInfo(L, create_info);
    return 1;
}

// Create Vulkan instance: vulkan.create_instance(create_info)
static int l_vulkan_create_instance(lua_State* L) {
    lua_VkInstanceCreateInfo* create_info_ud = lua_check_VkInstanceCreateInfo(L, 1);
    VkInstance instance;
    VkResult result = vkCreateInstance(create_info_ud->create_info, NULL, &instance);
    if (result != VK_SUCCESS) {
        luaL_error(L, "Failed to create Vulkan instance: VkResult %d", result);
    }
    lua_push_VkInstance(L, instance);
    return 1;
}

// Metatable setup
static void app_info_metatable(lua_State* L) {
    luaL_newmetatable(L, APP_INFO_MT);
    lua_pushcfunction(L, app_info_gc);
    lua_setfield(L, -2, "__gc");
    lua_pop(L, 1);
}

static void instance_metatable(lua_State* L) {
    luaL_newmetatable(L, INSTANCE_MT);
    lua_pushcfunction(L, instance_gc);
    lua_setfield(L, -2, "__gc");
    lua_pop(L, 1);
}

static void create_info_metatable(lua_State* L) {
    luaL_newmetatable(L, CREATE_INFO_MT);
    lua_pushcfunction(L, create_info_gc);
    lua_setfield(L, -2, "__gc");
    lua_pop(L, 1);
}

//===============================================
// surface
//===============================================

// Garbage collection for VkSurfaceKHR
static int surface_gc(lua_State* L) {
    lua_VkSurfaceKHR* ud = (lua_VkSurfaceKHR*)luaL_checkudata(L, 1, SURFACE_MT);
    if (ud->surface && ud->instance) {
        vkDestroySurfaceKHR(ud->instance, ud->surface, NULL);
        ud->surface = VK_NULL_HANDLE;
        ud->instance = VK_NULL_HANDLE;
    }
    return 0;
}

// Garbage collection for VkPhysicalDevice
static int physical_device_gc(lua_State* L) {
    lua_VkPhysicalDevice* ud = (lua_VkPhysicalDevice*)luaL_checkudata(L, 1, PHYSICAL_DEVICE_MT);
    if (ud->physical_device) {
        // Physical devices are not destroyed, just nullified
        ud->physical_device = VK_NULL_HANDLE;
    }
    return 0;
}

// Push VkSurfaceKHR userdata
void lua_push_VkSurfaceKHR(lua_State* L, VkSurfaceKHR surface, VkInstance instance) {
    if (!surface) {
        luaL_error(L, "Cannot create userdata for null VkSurfaceKHR");
    }
    lua_VkSurfaceKHR* ud = (lua_VkSurfaceKHR*)lua_newuserdata(L, sizeof(lua_VkSurfaceKHR));
    ud->surface = surface;
    ud->instance = instance;
    luaL_setmetatable(L, SURFACE_MT);
}

// Check VkSurfaceKHR userdata
lua_VkSurfaceKHR* lua_check_VkSurfaceKHR(lua_State* L, int idx) {
    lua_VkSurfaceKHR* ud = (lua_VkSurfaceKHR*)luaL_checkudata(L, idx, SURFACE_MT);
    if (!ud->surface) {
        luaL_error(L, "Invalid VkSurfaceKHR (already destroyed)");
    }
    if (!ud->instance) {
        luaL_error(L, "Invalid VkInstance for VkSurfaceKHR (already destroyed)");
    }
    return ud;
}

// Push VkPhysicalDevice userdata
void lua_push_VkPhysicalDevice(lua_State* L, VkPhysicalDevice physical_device) {
    if (!physical_device) {
        luaL_error(L, "Cannot create userdata for null VkPhysicalDevice");
    }
    lua_VkPhysicalDevice* ud = (lua_VkPhysicalDevice*)lua_newuserdata(L, sizeof(lua_VkPhysicalDevice));
    ud->physical_device = physical_device;
    luaL_setmetatable(L, PHYSICAL_DEVICE_MT);
}

// Check VkPhysicalDevice userdata
lua_VkPhysicalDevice* lua_check_VkPhysicalDevice(lua_State* L, int idx) {
    lua_VkPhysicalDevice* ud = (lua_VkPhysicalDevice*)luaL_checkudata(L, idx, PHYSICAL_DEVICE_MT);
    if (!ud->physical_device) {
        luaL_error(L, "Invalid VkPhysicalDevice (already destroyed)");
    }
    return ud;
}

// Create Vulkan surface: vulkan.sdl_vulkan_create_surface(window, instance)
static int l_vulkan_sdl_vulkan_create_surface(lua_State* L) {
    lua_SDL_Window* window_ud = lua_check_SDL_Window(L, 1);
    lua_VkInstance* instance_ud = lua_check_VkInstance(L, 2);

    VkSurfaceKHR surface;
    if (!SDL_Vulkan_CreateSurface(window_ud->window, instance_ud->instance, NULL, &surface)) {
        luaL_error(L, "Failed to create Vulkan surface: %s", SDL_GetError());
    }

    lua_push_VkSurfaceKHR(L, surface, instance_ud->instance);
    return 1;
}

// Enumerate physical devices: vulkan.create_physical_devices(instance)
static int l_vulkan_create_physical_devices(lua_State* L) {
    lua_VkInstance* instance_ud = lua_check_VkInstance(L, 1);

    uint32_t device_count = 0;
    VkResult result = vkEnumeratePhysicalDevices(instance_ud->instance, &device_count, NULL);
    if (result != VK_SUCCESS) {
        luaL_error(L, "Failed to get physical device count: VkResult %d", result);
    }

    if (device_count == 0) {
        lua_newtable(L); // Return empty table if no devices
        return 1;
    }

    VkPhysicalDevice* devices = (VkPhysicalDevice*)malloc(device_count * sizeof(VkPhysicalDevice));
    if (!devices) {
        luaL_error(L, "Failed to allocate memory for physical devices");
    }

    result = vkEnumeratePhysicalDevices(instance_ud->instance, &device_count, devices);
    if (result != VK_SUCCESS) {
        free(devices);
        luaL_error(L, "Failed to enumerate physical devices: VkResult %d", result);
    }

    lua_newtable(L);
    for (uint32_t i = 0; i < device_count; i++) {
        // Create a table for each device with its properties
        lua_newtable(L);

        // Push physical device userdata
        lua_push_VkPhysicalDevice(L, devices[i]);
        lua_setfield(L, -2, "device");

        // Get device properties
        VkPhysicalDeviceProperties props;
        vkGetPhysicalDeviceProperties(devices[i], &props);
        lua_pushstring(L, props.deviceName);
        lua_setfield(L, -2, "name");
        lua_pushinteger(L, props.deviceType);
        lua_setfield(L, -2, "type");
        lua_pushinteger(L, VK_MAKE_API_VERSION(0, props.apiVersion >> 22, (props.apiVersion >> 12) & 0x3FF, props.apiVersion & 0xFFF));
        lua_setfield(L, -2, "api_version");

        lua_rawseti(L, -2, i + 1); // Lua indices are 1-based
    }

    free(devices);
    return 1;
}

// Metatable setup
static void surface_metatable(lua_State* L) {
    luaL_newmetatable(L, SURFACE_MT);
    lua_pushcfunction(L, surface_gc);
    lua_setfield(L, -2, "__gc");
    lua_pop(L, 1);
}

static void physical_device_metatable(lua_State* L) {
    luaL_newmetatable(L, PHYSICAL_DEVICE_MT);
    lua_pushcfunction(L, physical_device_gc);
    lua_setfield(L, -2, "__gc");
    lua_pop(L, 1);
}

//===============================================
// family and device
//===============================================
// Garbage collection for VkDevice
static int device_gc(lua_State* L) {
    lua_VkDevice* ud = (lua_VkDevice*)luaL_checkudata(L, 1, DEVICE_MT);
    if (ud->device) {
        vkDestroyDevice(ud->device, NULL);
        ud->device = VK_NULL_HANDLE;
    }
    return 0;
}

// Garbage collection for VkDeviceCreateInfo
static int device_create_info_gc(lua_State* L) {
    lua_VkDeviceCreateInfo* ud = (lua_VkDeviceCreateInfo*)luaL_checkudata(L, 1, DEVICE_CREATE_INFO_MT);
    if (ud->create_info) {
        // Free queue create infos
        if (ud->create_info->pQueueCreateInfos) {
            for (uint32_t i = 0; i < ud->create_info->queueCreateInfoCount; i++) {
                if (ud->create_info->pQueueCreateInfos[i].pQueuePriorities) {
                    free((float*)ud->create_info->pQueueCreateInfos[i].pQueuePriorities);
                }
            }
            free((VkDeviceQueueCreateInfo*)ud->create_info->pQueueCreateInfos);
        }
        // Free enabled extensions
        if (ud->create_info->ppEnabledExtensionNames) {
            for (uint32_t i = 0; i < ud->create_info->enabledExtensionCount; i++) {
                free((char*)ud->create_info->ppEnabledExtensionNames[i]);
            }
            free((char**)ud->create_info->ppEnabledExtensionNames);
        }
        free(ud->create_info);
        ud->create_info = NULL;
    }
    return 0;
}

// Push VkDevice userdata
void lua_push_VkDevice(lua_State* L, VkDevice device) {
    if (!device) {
        luaL_error(L, "Cannot create userdata for null VkDevice");
    }
    lua_VkDevice* ud = (lua_VkDevice*)lua_newuserdata(L, sizeof(lua_VkDevice));
    ud->device = device;
    luaL_setmetatable(L, DEVICE_MT);
}

// Check VkDevice userdata
lua_VkDevice* lua_check_VkDevice(lua_State* L, int idx) {
    lua_VkDevice* ud = (lua_VkDevice*)luaL_checkudata(L, idx, DEVICE_MT);
    if (!ud->device) {
        luaL_error(L, "Invalid VkDevice (already destroyed)");
    }
    return ud;
}

// Push VkDeviceCreateInfo userdata
void lua_push_VkDeviceCreateInfo(lua_State* L, VkDeviceCreateInfo* create_info) {
    if (!create_info) {
        luaL_error(L, "Cannot create userdata for null VkDeviceCreateInfo");
    }
    lua_VkDeviceCreateInfo* ud = (lua_VkDeviceCreateInfo*)lua_newuserdata(L, sizeof(lua_VkDeviceCreateInfo));
    ud->create_info = create_info;
    luaL_setmetatable(L, DEVICE_CREATE_INFO_MT);
}

// Check VkDeviceCreateInfo userdata
lua_VkDeviceCreateInfo* lua_check_VkDeviceCreateInfo(lua_State* L, int idx) {
    lua_VkDeviceCreateInfo* ud = (lua_VkDeviceCreateInfo*)luaL_checkudata(L, idx, DEVICE_CREATE_INFO_MT);
    if (!ud->create_info) {
        luaL_error(L, "Invalid VkDeviceCreateInfo (already destroyed)");
    }
    return ud;
}

// Get queue family properties: vulkan.get_physical_devices_properties(physical_device)
static int l_vulkan_get_physical_devices_properties(lua_State* L) {
    lua_VkPhysicalDevice* physical_device_ud = lua_check_VkPhysicalDevice(L, 1);
    lua_VkSurfaceKHR* surface_ud = lua_isnoneornil(L, 2) ? NULL : lua_check_VkSurfaceKHR(L, 2);

    uint32_t queue_family_count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physical_device_ud->physical_device, &queue_family_count, NULL);
    if (queue_family_count == 0) {
        fprintf(stderr, "[Vulkan] No queue families found for physical device\n");
        lua_newtable(L);
        return 1;
    }

    VkQueueFamilyProperties* queue_families = (VkQueueFamilyProperties*)malloc(queue_family_count * sizeof(VkQueueFamilyProperties));
    if (!queue_families) {
        luaL_error(L, "Failed to allocate memory for queue family properties");
    }

    vkGetPhysicalDeviceQueueFamilyProperties(physical_device_ud->physical_device, &queue_family_count, queue_families);

    lua_newtable(L);
    for (uint32_t i = 0; i < queue_family_count; i++) {
        fprintf(stderr, "[Vulkan] Queue Family %u: queueFlags=0x%x (graphics=%d, compute=%d, transfer=%d, sparse=%d), queueCount=%u\n",
                i,
                queue_families[i].queueFlags,
                (queue_families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0,
                (queue_families[i].queueFlags & VK_QUEUE_COMPUTE_BIT) != 0,
                (queue_families[i].queueFlags & VK_QUEUE_TRANSFER_BIT) != 0,
                (queue_families[i].queueFlags & VK_QUEUE_SPARSE_BINDING_BIT) != 0,
                queue_families[i].queueCount);
        lua_newtable(L);
        lua_pushinteger(L, queue_families[i].queueCount);
        lua_setfield(L, -2, "queue_count");
        lua_pushboolean(L, (queue_families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0);
        lua_setfield(L, -2, "graphics");
        lua_pushboolean(L, (queue_families[i].queueFlags & VK_QUEUE_COMPUTE_BIT) != 0);
        lua_setfield(L, -2, "compute");
        lua_pushboolean(L, (queue_families[i].queueFlags & VK_QUEUE_TRANSFER_BIT) != 0);
        lua_setfield(L, -2, "transfer");

        if (surface_ud) {
            VkBool32 present_support = VK_FALSE;
            VkResult result = vkGetPhysicalDeviceSurfaceSupportKHR(physical_device_ud->physical_device, i, surface_ud->surface, &present_support);
            fprintf(stderr, "[Vulkan] Queue Family %u: present_support=%d, vkGetPhysicalDeviceSurfaceSupportKHR result=%d\n",
                    i, present_support, result);
            if (result != VK_SUCCESS) {
                fprintf(stderr, "[Vulkan] Error: vkGetPhysicalDeviceSurfaceSupportKHR failed for queue family %u: VkResult %d\n", i, result);
                lua_pushboolean(L, false);
            } else {
                lua_pushboolean(L, present_support);
            }
            lua_setfield(L, -2, "present");
        } else {
            lua_pushboolean(L, false);
            lua_setfield(L, -2, "present");
        }

        lua_rawseti(L, -2, i + 1); // Lua indices are 1-based
    }

    free(queue_families);
    return 1;
}

// Create VkDeviceCreateInfo: vulkan.create_device_info({queue_families, extensions})
static int l_vulkan_create_device_info(lua_State* L) {
    luaL_checktype(L, 1, LUA_TTABLE);

    VkDeviceCreateInfo* create_info = (VkDeviceCreateInfo*)malloc(sizeof(VkDeviceCreateInfo));
    if (!create_info) {
        luaL_error(L, "Failed to allocate memory for VkDeviceCreateInfo");
    }
    memset(create_info, 0, sizeof(VkDeviceCreateInfo));
    create_info->sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

    // Get queue family indices and priorities
    lua_getfield(L, 1, "queue_families");
    if (!lua_isnil(L, -1)) {
        luaL_checktype(L, -1, LUA_TTABLE);
        create_info->queueCreateInfoCount = lua_rawlen(L, -1);
        if (create_info->queueCreateInfoCount > 0) {
            VkDeviceQueueCreateInfo* queue_create_infos = (VkDeviceQueueCreateInfo*)malloc(create_info->queueCreateInfoCount * sizeof(VkDeviceQueueCreateInfo));
            if (!queue_create_infos) {
                free(create_info);
                luaL_error(L, "Failed to allocate memory for queue create infos");
            }
            memset(queue_create_infos, 0, create_info->queueCreateInfoCount * sizeof(VkDeviceQueueCreateInfo));

            for (uint32_t i = 1; i <= create_info->queueCreateInfoCount; i++) {
                lua_rawgeti(L, -1, i);
                luaL_checktype(L, -1, LUA_TTABLE);

                queue_create_infos[i-1].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
                lua_getfield(L, -1, "family_index");
                queue_create_infos[i-1].queueFamilyIndex = luaL_checkinteger(L, -1);
                lua_pop(L, 1);

                lua_getfield(L, -1, "queue_count");
                queue_create_infos[i-1].queueCount = luaL_checkinteger(L, -1);
                lua_pop(L, 1);

                float* priorities = (float*)malloc(queue_create_infos[i-1].queueCount * sizeof(float));
                if (!priorities) {
                    for (uint32_t j = 0; j < i-1; j++) {
                        free((float*)queue_create_infos[j].pQueuePriorities);
                    }
                    free(queue_create_infos);
                    free(create_info);
                    luaL_error(L, "Failed to allocate memory for queue priorities");
                }
                for (uint32_t j = 0; j < queue_create_infos[i-1].queueCount; j++) {
                    priorities[j] = 1.0f; // Default priority
                }
                queue_create_infos[i-1].pQueuePriorities = priorities;

                lua_pop(L, 1); // Pop queue family table
            }
            create_info->pQueueCreateInfos = queue_create_infos;
        }
    }
    lua_pop(L, 1);

    // Get device extensions
    char** extension_names = NULL;
    lua_getfield(L, 1, "extensions");
    if (!lua_isnil(L, -1)) {
        luaL_checktype(L, -1, LUA_TTABLE);
        create_info->enabledExtensionCount = lua_rawlen(L, -1);
        if (create_info->enabledExtensionCount > 0) {
            extension_names = (char**)malloc(create_info->enabledExtensionCount * sizeof(char*));
            if (!extension_names) {
                if (create_info->pQueueCreateInfos) {
                    for (uint32_t i = 0; i < create_info->queueCreateInfoCount; i++) {
                        free((float*)create_info->pQueueCreateInfos[i].pQueuePriorities);
                    }
                    free((VkDeviceQueueCreateInfo*)create_info->pQueueCreateInfos);
                }
                free(create_info);
                luaL_error(L, "Failed to allocate memory for extension names");
            }
            for (uint32_t i = 1; i <= create_info->enabledExtensionCount; i++) {
                lua_rawgeti(L, -1, i);
                extension_names[i-1] = strdup(luaL_checkstring(L, -1));
                if (!extension_names[i-1]) {
                    for (uint32_t j = 0; j < i-1; j++) {
                        free(extension_names[j]);
                    }
                    free(extension_names);
                    if (create_info->pQueueCreateInfos) {
                        for (uint32_t j = 0; j < create_info->queueCreateInfoCount; j++) {
                            free((float*)create_info->pQueueCreateInfos[j].pQueuePriorities);
                        }
                        free((VkDeviceQueueCreateInfo*)create_info->pQueueCreateInfos);
                    }
                    free(create_info);
                    luaL_error(L, "Failed to allocate memory for extension name");
                }
                lua_pop(L, 1);
            }
            create_info->ppEnabledExtensionNames = (const char* const*)extension_names;
        }
    } else {
        create_info->enabledExtensionCount = 0;
        create_info->ppEnabledExtensionNames = NULL;
    }
    lua_pop(L, 1);

    lua_push_VkDeviceCreateInfo(L, create_info);
    return 1;
}

// Create VkDevice: vulkan.create_device(physical_device, device_create_info)
static int l_vulkan_create_device(lua_State* L) {
    lua_VkPhysicalDevice* physical_device_ud = lua_check_VkPhysicalDevice(L, 1);
    lua_VkDeviceCreateInfo* create_info_ud = lua_check_VkDeviceCreateInfo(L, 2);

    VkDevice device;
    VkResult result = vkCreateDevice(physical_device_ud->physical_device, create_info_ud->create_info, NULL, &device);
    if (result != VK_SUCCESS) {
        luaL_error(L, "Failed to create Vulkan device: VkResult %d", result);
    }

    lua_push_VkDevice(L, device);
    return 1;
}

// Metatable setup
static void device_metatable(lua_State* L) {
    luaL_newmetatable(L, DEVICE_MT);
    lua_pushcfunction(L, device_gc);
    lua_setfield(L, -2, "__gc");
    lua_pop(L, 1);
}

static void device_create_info_metatable(lua_State* L) {
    luaL_newmetatable(L, DEVICE_CREATE_INFO_MT);
    lua_pushcfunction(L, device_create_info_gc);
    lua_setfield(L, -2, "__gc");
    lua_pop(L, 1);
}

//===============================================
// queue
//===============================================
// Garbage collection for VkQueue
static int queue_gc(lua_State* L) {
    lua_VkQueue* ud = (lua_VkQueue*)luaL_checkudata(L, 1, QUEUE_MT);
    if (ud->queue) {
        // Queues are not destroyed explicitly; nullify handle
        ud->queue = VK_NULL_HANDLE;
    }
    return 0;
}

// Garbage collection for VkSwapchainKHR
static int swapchain_gc(lua_State* L) {
    lua_VkSwapchainKHR* ud = (lua_VkSwapchainKHR*)luaL_checkudata(L, 1, SWAPCHAIN_MT);
    if (ud->swapchain && ud->device) {
        vkDestroySwapchainKHR(ud->device, ud->swapchain, NULL);
        ud->swapchain = VK_NULL_HANDLE;
        ud->device = VK_NULL_HANDLE;
    }
    return 0;
}

// Push VkQueue userdata
void lua_push_VkQueue(lua_State* L, VkQueue queue) {
    if (!queue) {
        luaL_error(L, "Cannot create userdata for null VkQueue");
    }
    lua_VkQueue* ud = (lua_VkQueue*)lua_newuserdata(L, sizeof(lua_VkQueue));
    ud->queue = queue;
    luaL_setmetatable(L, QUEUE_MT);
}

// Check VkQueue userdata
lua_VkQueue* lua_check_VkQueue(lua_State* L, int idx) {
    lua_VkQueue* ud = (lua_VkQueue*)luaL_checkudata(L, idx, QUEUE_MT);
    if (!ud->queue) {
        luaL_error(L, "Invalid VkQueue (already destroyed)");
    }
    return ud;
}

// Push VkSwapchainKHR userdata
void lua_push_VkSwapchainKHR(lua_State* L, VkSwapchainKHR swapchain, VkDevice device) {
    if (!swapchain) {
        luaL_error(L, "Cannot create userdata for null VkSwapchainKHR");
    }
    lua_VkSwapchainKHR* ud = (lua_VkSwapchainKHR*)lua_newuserdata(L, sizeof(lua_VkSwapchainKHR));
    ud->swapchain = swapchain;
    ud->device = device;
    luaL_setmetatable(L, SWAPCHAIN_MT);
}

// Check VkSwapchainKHR userdata
lua_VkSwapchainKHR* lua_check_VkSwapchainKHR(lua_State* L, int idx) {
    lua_VkSwapchainKHR* ud = (lua_VkSwapchainKHR*)luaL_checkudata(L, idx, SWAPCHAIN_MT);
    if (!ud->swapchain) {
        luaL_error(L, "Invalid VkSwapchainKHR (already destroyed)");
    }
    if (!ud->device) {
        luaL_error(L, "Invalid VkDevice for VkSwapchainKHR (already destroyed)");
    }
    return ud;
}

// Get device queue: vulkan.get_device_queue(device, family_index, queue_index)
static int l_vulkan_get_device_queue(lua_State* L) {
    lua_VkDevice* device_ud = lua_check_VkDevice(L, 1);
    uint32_t family_index = luaL_checkinteger(L, 2);
    uint32_t queue_index = luaL_checkinteger(L, 3);

    VkQueue queue;
    vkGetDeviceQueue(device_ud->device, family_index, queue_index, &queue);
    if (!queue) {
        luaL_error(L, "Failed to get device queue for family %d, index %d", family_index, queue_index);
    }

    lua_push_VkQueue(L, queue);
    return 1;
}

// Get surface capabilities: vulkan.get_physical_device_surface_capabilities_KHR(physical_device, surface)
static int l_vulkan_get_physical_device_surface_capabilities_KHR(lua_State* L) {
    lua_VkPhysicalDevice* physical_device_ud = lua_check_VkPhysicalDevice(L, 1);
    lua_VkSurfaceKHR* surface_ud = lua_check_VkSurfaceKHR(L, 2);

    VkSurfaceCapabilitiesKHR capabilities;
    VkResult result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device_ud->physical_device, surface_ud->surface, &capabilities);
    if (result != VK_SUCCESS) {
        luaL_error(L, "Failed to get surface capabilities: VkResult %d", result);
    }

    lua_newtable(L);
    lua_pushinteger(L, capabilities.minImageCount);
    lua_setfield(L, -2, "min_image_count");
    lua_pushinteger(L, capabilities.maxImageCount);
    lua_setfield(L, -2, "max_image_count");
    lua_pushinteger(L, capabilities.currentExtent.width);
    lua_setfield(L, -2, "current_extent_width");
    lua_pushinteger(L, capabilities.currentExtent.height);
    lua_setfield(L, -2, "current_extent_height");
    lua_pushinteger(L, capabilities.minImageExtent.width);
    lua_setfield(L, -2, "min_image_extent_width");
    lua_pushinteger(L, capabilities.minImageExtent.height);
    lua_setfield(L, -2, "min_image_extent_height");
    lua_pushinteger(L, capabilities.maxImageExtent.width);
    lua_setfield(L, -2, "max_image_extent_width");
    lua_pushinteger(L, capabilities.maxImageExtent.height);
    lua_setfield(L, -2, "max_image_extent_height");
    lua_pushinteger(L, capabilities.maxImageArrayLayers);
    lua_setfield(L, -2, "max_image_array_layers");
    lua_pushinteger(L, capabilities.supportedTransforms);
    lua_setfield(L, -2, "supported_transforms");
    lua_pushinteger(L, capabilities.currentTransform);
    lua_setfield(L, -2, "current_transform");
    lua_pushinteger(L, capabilities.supportedCompositeAlpha);
    lua_setfield(L, -2, "supported_composite_alpha");
    lua_pushinteger(L, capabilities.supportedUsageFlags);
    lua_setfield(L, -2, "supported_usage_flags");

    return 1;
}

// Helper function to get integer field with fallback
static int lua_getfield_integer(lua_State* L, int index, const char* field) {
    lua_getfield(L, index, field);
    int value = luaL_checkinteger(L, -1);
    lua_pop(L, 1);
    return value;
}

//===============================================
// SWAPCHAIN
//===============================================
// Create swapchain: vulkan.create_swap_chain_KHR(device, {surface, min_image_count, format, color_space, extent, present_mode, ...})
static int l_vulkan_create_swap_chain_KHR(lua_State* L) {
    lua_VkDevice* device_ud = lua_check_VkDevice(L, 1);
    luaL_checktype(L, 2, LUA_TTABLE);

    VkSwapchainCreateInfoKHR create_info = {0};
    create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;

    lua_getfield(L, 2, "surface");
    lua_VkSurfaceKHR* surface_ud = lua_check_VkSurfaceKHR(L, -1);
    create_info.surface = surface_ud->surface;
    lua_pop(L, 1);

    lua_getfield(L, 2, "min_image_count");
    create_info.minImageCount = luaL_checkinteger(L, -1);
    lua_pop(L, 1);

    lua_getfield(L, 2, "format");
    if (lua_isnil(L, -1)) {
        lua_pop(L, 1);
        lua_getfield(L, 2, "image_format");
    }
    create_info.imageFormat = luaL_checkinteger(L, -1);
    lua_pop(L, 1);

    lua_getfield(L, 2, "color_space");
    if (lua_isnil(L, -1)) {
        lua_pop(L, 1);
        lua_getfield(L, 2, "image_color_space");
    }
    create_info.imageColorSpace = luaL_checkinteger(L, -1);
    lua_pop(L, 1);

    lua_getfield(L, 2, "extent");
    if (lua_isnil(L, -1)) {
        lua_pop(L, 1);
        create_info.imageExtent.width = lua_getfield_integer(L, 2, "image_extent_width");
        create_info.imageExtent.height = lua_getfield_integer(L, 2, "image_extent_height");
    } else {
        lua_getfield(L, -1, "width");
        create_info.imageExtent.width = luaL_checkinteger(L, -1);
        lua_pop(L, 1);
        lua_getfield(L, -1, "height");
        create_info.imageExtent.height = luaL_checkinteger(L, -1);
        lua_pop(L, 1);
        lua_pop(L, 1);
    }

    lua_getfield(L, 2, "image_array_layers");
    create_info.imageArrayLayers = luaL_checkinteger(L, -1);
    lua_pop(L, 1);

    lua_getfield(L, 2, "image_usage");
    create_info.imageUsage = luaL_checkinteger(L, -1);
    lua_pop(L, 1);

    lua_getfield(L, 2, "sharing_mode");
    if (lua_isnil(L, -1)) {
        lua_pop(L, 1);
        lua_getfield(L, 2, "image_sharing_mode");
    }
    create_info.imageSharingMode = luaL_checkinteger(L, -1);
    lua_pop(L, 1);

    lua_getfield(L, 2, "queue_family_indices");
    luaL_checktype(L, -1, LUA_TTABLE);
    create_info.queueFamilyIndexCount = lua_rawlen(L, -1);
    uint32_t* queue_family_indices = NULL;
    if (create_info.queueFamilyIndexCount > 0) {
        queue_family_indices = (uint32_t*)malloc(create_info.queueFamilyIndexCount * sizeof(uint32_t));
        if (!queue_family_indices) {
            luaL_error(L, "Failed to allocate memory for queue family indices");
        }
        for (uint32_t i = 1; i <= create_info.queueFamilyIndexCount; i++) {
            lua_rawgeti(L, -1, i);
            queue_family_indices[i-1] = luaL_checkinteger(L, -1);
            lua_pop(L, 1);
        }
        create_info.pQueueFamilyIndices = queue_family_indices;
    }
    lua_pop(L, 1);

    lua_getfield(L, 2, "pre_transform");
    create_info.preTransform = luaL_checkinteger(L, -1);
    lua_pop(L, 1);

    lua_getfield(L, 2, "composite_alpha");
    create_info.compositeAlpha = luaL_checkinteger(L, -1);
    lua_pop(L, 1);

    lua_getfield(L, 2, "present_mode");
    create_info.presentMode = luaL_checkinteger(L, -1);
    lua_pop(L, 1);

    lua_getfield(L, 2, "clipped");
    create_info.clipped = lua_toboolean(L, -1);
    lua_pop(L, 1);

    lua_getfield(L, 2, "old_swapchain");
    lua_VkSwapchainKHR* old_swapchain_ud = lua_isnil(L, -1) ? NULL : lua_check_VkSwapchainKHR(L, -1);
    create_info.oldSwapchain = old_swapchain_ud ? old_swapchain_ud->swapchain : VK_NULL_HANDLE;
    lua_pop(L, 1);

    lua_VkSwapchainKHR* swapchain_ud = (lua_VkSwapchainKHR*)lua_newuserdata(L, sizeof(lua_VkSwapchainKHR));
    swapchain_ud->swapchain = VK_NULL_HANDLE;
    swapchain_ud->device = device_ud->device;

    VkResult result = vkCreateSwapchainKHR(device_ud->device, &create_info, NULL, &swapchain_ud->swapchain);
    free(queue_family_indices);

    if (old_swapchain_ud && old_swapchain_ud->swapchain != VK_NULL_HANDLE) {
        vkDestroySwapchainKHR(device_ud->device, old_swapchain_ud->swapchain, NULL);
        old_swapchain_ud->swapchain = VK_NULL_HANDLE;
    }

    if (result != VK_SUCCESS) {
        lua_pushnil(L);
        lua_pushfstring(L, "Failed to create swapchain: VkResult %d", result);
        return 2;
    }

    luaL_getmetatable(L, SWAPCHAIN_MT);
    lua_setmetatable(L, -2);
    return 1;
}

// Metatable setup
static void queue_metatable(lua_State* L) {
    luaL_newmetatable(L, QUEUE_MT);
    lua_pushcfunction(L, queue_gc);
    lua_setfield(L, -2, "__gc");
    lua_pop(L, 1);
}

static void swapchain_metatable(lua_State* L) {
    luaL_newmetatable(L, SWAPCHAIN_MT);
    lua_pushcfunction(L, swapchain_gc);
    lua_setfield(L, -2, "__gc");
    lua_pushstring(L, "vulkan.swapchain");
    lua_setfield(L, -2, "__name"); // Set metatable name
    lua_pop(L, 1);
}

//===============================================
// images 
//===============================================
// Garbage collection for VkImageView
static int image_view_gc(lua_State* L) {
    lua_VkImageView* ud = (lua_VkImageView*)luaL_checkudata(L, 1, IMAGE_VIEW_MT);
    if (ud->image_view && ud->device) {
        vkDestroyImageView(ud->device, ud->image_view, NULL);
        ud->image_view = VK_NULL_HANDLE;
        ud->device = VK_NULL_HANDLE;
    }
    return 0;
}

// Push VkImageView userdata
void lua_push_VkImageView(lua_State* L, VkImageView image_view, VkDevice device) {
    if (!image_view) {
        luaL_error(L, "Cannot create userdata for null VkImageView");
    }
    lua_VkImageView* ud = (lua_VkImageView*)lua_newuserdata(L, sizeof(lua_VkImageView));
    ud->image_view = image_view;
    ud->device = device;
    luaL_setmetatable(L, IMAGE_VIEW_MT);
}

// Check VkImageView userdata
lua_VkImageView* lua_check_VkImageView(lua_State* L, int idx) {
    lua_VkImageView* ud = (lua_VkImageView*)luaL_checkudata(L, idx, IMAGE_VIEW_MT);
    if (!ud->image_view) {
        luaL_error(L, "Invalid VkImageView (already destroyed)");
    }
    if (!ud->device) {
        luaL_error(L, "Invalid VkDevice for VkImageView (already destroyed)");
    }
    return ud;
}

// Get swapchain images: vulkan.get_swapchain_images_KHR(device, swapchain)
static int l_vulkan_get_swapchain_images_KHR(lua_State* L) {
    lua_VkDevice* device_ud = lua_check_VkDevice(L, 1);
    lua_VkSwapchainKHR* swapchain_ud = lua_check_VkSwapchainKHR(L, 2);

    uint32_t image_count = 0;
    VkResult result = vkGetSwapchainImagesKHR(device_ud->device, swapchain_ud->swapchain, &image_count, NULL);
    if (result != VK_SUCCESS) {
        luaL_error(L, "Failed to get swapchain image count: VkResult %d", result);
    }

    VkImage* images = (VkImage*)malloc(image_count * sizeof(VkImage));
    if (!images) {
        luaL_error(L, "Failed to allocate memory for swapchain images");
    }

    result = vkGetSwapchainImagesKHR(device_ud->device, swapchain_ud->swapchain, &image_count, images);
    if (result != VK_SUCCESS) {
        free(images);
        luaL_error(L, "Failed to get swapchain images: VkResult %d", result);
    }

    lua_newtable(L);
    for (uint32_t i = 0; i < image_count; i++) {
        lua_pushlightuserdata(L, (void*)images[i]);
        lua_rawseti(L, -2, i + 1); // Lua indices are 1-based
    }

    free(images);
    return 1;
}

// Create image view: vulkan.create_image_view(device, {image, format, view_type, components, subresource_range})
static int l_vulkan_create_image_view(lua_State* L) {
    lua_VkDevice* device_ud = lua_check_VkDevice(L, 1);
    luaL_checktype(L, 2, LUA_TTABLE);

    VkImageViewCreateInfo create_info = {0};
    create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;

    // Get image
    lua_getfield(L, 2, "image");
    if (!lua_isuserdata(L, -1)) {
        luaL_error(L, "image must be a userdata (VkImage)");
    }
    // Assuming swapchain_images[i] is pushed as light userdata (VkImage)
    create_info.image = (VkImage)lua_touserdata(L, -1);
    lua_pop(L, 1);

    // Get format
    lua_getfield(L, 2, "format");
    create_info.format = (VkFormat)luaL_checkinteger(L, -1);
    lua_pop(L, 1);

    // Get view_type
    lua_getfield(L, 2, "view_type");
    if (lua_isnil(L, -1)) {
        luaL_error(L, "view_type is required");
    }
    create_info.viewType = (VkImageViewType)luaL_checkinteger(L, -1);
    lua_pop(L, 1);

    // Get components
    lua_getfield(L, 2, "components");
    if (lua_istable(L, -1)) {
        lua_getfield(L, -1, "r");
        create_info.components.r = (VkComponentSwizzle)luaL_checkinteger(L, -1);
        lua_pop(L, 1);

        lua_getfield(L, -1, "g");
        create_info.components.g = (VkComponentSwizzle)luaL_checkinteger(L, -1);
        lua_pop(L, 1);

        lua_getfield(L, -1, "b");
        create_info.components.b = (VkComponentSwizzle)luaL_checkinteger(L, -1);
        lua_pop(L, 1);

        lua_getfield(L, -1, "a");
        create_info.components.a = (VkComponentSwizzle)luaL_checkinteger(L, -1);
        lua_pop(L, 1);
    } else {
        luaL_error(L, "components must be a table");
    }
    lua_pop(L, 1);

    // Get subresource_range
    lua_getfield(L, 2, "subresource_range");
    if (lua_istable(L, -1)) {
        lua_getfield(L, -1, "aspect_mask");
        create_info.subresourceRange.aspectMask = (VkImageAspectFlags)luaL_checkinteger(L, -1);
        lua_pop(L, 1);

        lua_getfield(L, -1, "base_mip_level");
        create_info.subresourceRange.baseMipLevel = (uint32_t)luaL_checkinteger(L, -1);
        lua_pop(L, 1);

        lua_getfield(L, -1, "level_count");
        create_info.subresourceRange.levelCount = (uint32_t)luaL_checkinteger(L, -1);
        lua_pop(L, 1);

        lua_getfield(L, -1, "base_array_layer");
        create_info.subresourceRange.baseArrayLayer = (uint32_t)luaL_checkinteger(L, -1);
        lua_pop(L, 1);

        lua_getfield(L, -1, "layer_count");
        create_info.subresourceRange.layerCount = (uint32_t)luaL_checkinteger(L, -1);
        lua_pop(L, 1);
    } else {
        luaL_error(L, "subresource_range must be a table");
    }
    lua_pop(L, 1);

    VkImageView image_view;
    VkResult result = vkCreateImageView(device_ud->device, &create_info, NULL, &image_view);
    if (result != VK_SUCCESS) {
        luaL_error(L, "Failed to create image view: VkResult %d", result);
    }

    lua_push_VkImageView(L, image_view, device_ud->device);
    return 1;
}

// Metatable setup
static void image_view_metatable(lua_State* L) {
    luaL_newmetatable(L, IMAGE_VIEW_MT);
    lua_pushcfunction(L, image_view_gc);
    lua_setfield(L, -2, "__gc");
    lua_pop(L, 1);
}

//===============================================
// render pass
//===============================================
// Garbage collection for VkRenderPass
static int render_pass_gc(lua_State* L) {
    lua_VkRenderPass* ud = (lua_VkRenderPass*)luaL_checkudata(L, 1, RENDER_PASS_MT);
    if (ud->render_pass && ud->device) {
        vkDestroyRenderPass(ud->device, ud->render_pass, NULL);
        ud->render_pass = VK_NULL_HANDLE;
        ud->device = VK_NULL_HANDLE;
    }
    return 0;
}

// Push VkRenderPass userdata
void lua_push_VkRenderPass(lua_State* L, VkRenderPass render_pass, VkDevice device) {
    if (!render_pass) {
        luaL_error(L, "Cannot create userdata for null VkRenderPass");
    }
    lua_VkRenderPass* ud = (lua_VkRenderPass*)lua_newuserdata(L, sizeof(lua_VkRenderPass));
    ud->render_pass = render_pass;
    ud->device = device;
    luaL_setmetatable(L, RENDER_PASS_MT);
}

// Check VkRenderPass userdata
lua_VkRenderPass* lua_check_VkRenderPass(lua_State* L, int idx) {
    lua_VkRenderPass* ud = (lua_VkRenderPass*)luaL_checkudata(L, idx, RENDER_PASS_MT);
    if (!ud->render_pass) {
        luaL_error(L, "Invalid VkRenderPass (already destroyed)");
    }
    if (!ud->device) {
        luaL_error(L, "Invalid VkDevice for VkRenderPass (already destroyed)");
    }
    return ud;
}

// Create render pass: vulkan.create_render_pass(device, {attachments, subpasses, dependencies})
static int l_vulkan_create_render_pass(lua_State* L) {
    lua_VkDevice* device_ud = lua_check_VkDevice(L, 1);
    luaL_checktype(L, 2, LUA_TTABLE);

    VkRenderPassCreateInfo create_info = {0};
    create_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;

    // Get attachments
    lua_getfield(L, 2, "attachments");
    luaL_checktype(L, -1, LUA_TTABLE);
    create_info.attachmentCount = lua_rawlen(L, -1);
    VkAttachmentDescription* attachments = NULL;
    if (create_info.attachmentCount > 0) {
        attachments = (VkAttachmentDescription*)malloc(create_info.attachmentCount * sizeof(VkAttachmentDescription));
        if (!attachments) {
            luaL_error(L, "Failed to allocate memory for attachments");
        }
        memset(attachments, 0, create_info.attachmentCount * sizeof(VkAttachmentDescription));
        for (uint32_t i = 1; i <= create_info.attachmentCount; i++) {
            lua_rawgeti(L, -1, i);
            luaL_checktype(L, -1, LUA_TTABLE);

            lua_getfield(L, -1, "format");
            attachments[i-1].format = luaL_checkinteger(L, -1);
            lua_pop(L, 1);

            lua_getfield(L, -1, "samples");
            attachments[i-1].samples = luaL_checkinteger(L, -1);
            lua_pop(L, 1);

            lua_getfield(L, -1, "load_op");
            attachments[i-1].loadOp = luaL_checkinteger(L, -1);
            lua_pop(L, 1);

            lua_getfield(L, -1, "store_op");
            attachments[i-1].storeOp = luaL_checkinteger(L, -1);
            lua_pop(L, 1);

            lua_getfield(L, -1, "stencil_load_op");
            attachments[i-1].stencilLoadOp = luaL_checkinteger(L, -1);
            lua_pop(L, 1);

            lua_getfield(L, -1, "stencil_store_op");
            attachments[i-1].stencilStoreOp = luaL_checkinteger(L, -1);
            lua_pop(L, 1);

            lua_getfield(L, -1, "initial_layout");
            attachments[i-1].initialLayout = luaL_checkinteger(L, -1);
            lua_pop(L, 1);

            lua_getfield(L, -1, "final_layout");
            attachments[i-1].finalLayout = luaL_checkinteger(L, -1);
            lua_pop(L, 1);

            lua_pop(L, 1); // Pop attachment table
        }
        create_info.pAttachments = attachments;
    }
    lua_pop(L, 1);

    // Get subpasses
    lua_getfield(L, 2, "subpasses");
    luaL_checktype(L, -1, LUA_TTABLE);
    create_info.subpassCount = lua_rawlen(L, -1);
    VkSubpassDescription* subpasses = NULL;
    VkAttachmentReference* color_references = NULL;
    if (create_info.subpassCount > 0) {
        subpasses = (VkSubpassDescription*)malloc(create_info.subpassCount * sizeof(VkSubpassDescription));
        if (!subpasses) {
            free(attachments);
            luaL_error(L, "Failed to allocate memory for subpasses");
        }
        memset(subpasses, 0, create_info.subpassCount * sizeof(VkSubpassDescription));

        uint32_t total_color_refs = 0;
        for (uint32_t i = 1; i <= create_info.subpassCount; i++) {
            lua_rawgeti(L, -1, i);
            lua_getfield(L, -1, "color_attachments");
            total_color_refs += lua_rawlen(L, -1);
            lua_pop(L, 2);
        }

        if (total_color_refs > 0) {
            color_references = (VkAttachmentReference*)malloc(total_color_refs * sizeof(VkAttachmentReference));
            if (!color_references) {
                free(subpasses);
                free(attachments);
                luaL_error(L, "Failed to allocate memory for color attachment references");
            }
        }

        uint32_t color_ref_index = 0;
        for (uint32_t i = 1; i <= create_info.subpassCount; i++) {
            lua_rawgeti(L, -1, i);
            luaL_checktype(L, -1, LUA_TTABLE);

            subpasses[i-1].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

            lua_getfield(L, -1, "color_attachments");
            subpasses[i-1].colorAttachmentCount = lua_rawlen(L, -1);
            if (subpasses[i-1].colorAttachmentCount > 0) {
                subpasses[i-1].pColorAttachments = &color_references[color_ref_index];
                for (uint32_t j = 1; j <= subpasses[i-1].colorAttachmentCount; j++) {
                    lua_rawgeti(L, -1, j);
                    luaL_checktype(L, -1, LUA_TTABLE);
                    lua_getfield(L, -1, "attachment");
                    color_references[color_ref_index].attachment = luaL_checkinteger(L, -1);
                    lua_pop(L, 1);
                    lua_getfield(L, -1, "layout");
                    color_references[color_ref_index].layout = luaL_checkinteger(L, -1);
                    lua_pop(L, 1);
                    color_ref_index++;
                    lua_pop(L, 1); // Pop color attachment table
                }
            }
            lua_pop(L, 1); // Pop color_attachments table

            lua_pop(L, 1); // Pop subpass table
        }
        create_info.pSubpasses = subpasses;
    }
    lua_pop(L, 1);

    // Get dependencies
    lua_getfield(L, 2, "dependencies");
    luaL_checktype(L, -1, LUA_TTABLE);
    create_info.dependencyCount = lua_rawlen(L, -1);
    VkSubpassDependency* dependencies = NULL;
    if (create_info.dependencyCount > 0) {
        dependencies = (VkSubpassDependency*)malloc(create_info.dependencyCount * sizeof(VkSubpassDependency));
        if (!dependencies) {
            free(color_references);
            free(subpasses);
            free(attachments);
            luaL_error(L, "Failed to allocate memory for dependencies");
        }
        memset(dependencies, 0, create_info.dependencyCount * sizeof(VkSubpassDependency));
        for (uint32_t i = 1; i <= create_info.dependencyCount; i++) {
            lua_rawgeti(L, -1, i);
            luaL_checktype(L, -1, LUA_TTABLE);

            lua_getfield(L, -1, "src_subpass");
            dependencies[i-1].srcSubpass = luaL_checkinteger(L, -1);
            lua_pop(L, 1);

            lua_getfield(L, -1, "dst_subpass");
            dependencies[i-1].dstSubpass = luaL_checkinteger(L, -1);
            lua_pop(L, 1);

            lua_getfield(L, -1, "src_stage_mask");
            dependencies[i-1].srcStageMask = luaL_checkinteger(L, -1);
            lua_pop(L, 1);

            lua_getfield(L, -1, "dst_stage_mask");
            dependencies[i-1].dstStageMask = luaL_checkinteger(L, -1);
            lua_pop(L, 1);

            lua_getfield(L, -1, "src_access_mask");
            dependencies[i-1].srcAccessMask = luaL_checkinteger(L, -1);
            lua_pop(L, 1);

            lua_getfield(L, -1, "dst_access_mask");
            dependencies[i-1].dstAccessMask = luaL_checkinteger(L, -1);
            lua_pop(L, 1);

            lua_getfield(L, -1, "dependency_flags");
            dependencies[i-1].dependencyFlags = luaL_checkinteger(L, -1);
            lua_pop(L, 1);

            lua_pop(L, 1); // Pop dependency table
        }
        create_info.pDependencies = dependencies;
    }
    lua_pop(L, 1);

    VkRenderPass render_pass;
    VkResult result = vkCreateRenderPass(device_ud->device, &create_info, NULL, &render_pass);
    free(dependencies);
    free(color_references);
    free(subpasses);
    free(attachments);
    if (result != VK_SUCCESS) {
        luaL_error(L, "Failed to create render pass: VkResult %d", result);
    }

    lua_push_VkRenderPass(L, render_pass, device_ud->device);
    return 1;
}

// Metatable setup
static void render_pass_metatable(lua_State* L) {
    luaL_newmetatable(L, RENDER_PASS_MT);
    lua_pushcfunction(L, render_pass_gc);
    lua_setfield(L, -2, "__gc");
    lua_pop(L, 1);
}

//===============================================
// Framebuffer
//===============================================
// Garbage collection for VkFramebuffer
static int framebuffer_gc(lua_State* L) {
    lua_VkFramebuffer* ud = (lua_VkFramebuffer*)luaL_checkudata(L, 1, FRAMEBUFFER_MT);
    if (ud->framebuffer && ud->device) {
        vkDestroyFramebuffer(ud->device, ud->framebuffer, NULL);
        ud->framebuffer = VK_NULL_HANDLE;
        ud->device = VK_NULL_HANDLE;
    }
    return 0;
}

// Push VkFramebuffer userdata
void lua_push_VkFramebuffer(lua_State* L, VkFramebuffer framebuffer, VkDevice device) {
    if (!framebuffer) {
        luaL_error(L, "Cannot create userdata for null VkFramebuffer");
    }
    lua_VkFramebuffer* ud = (lua_VkFramebuffer*)lua_newuserdata(L, sizeof(lua_VkFramebuffer));
    ud->framebuffer = framebuffer;
    ud->device = device;
    luaL_setmetatable(L, FRAMEBUFFER_MT);
}

// Check VkFramebuffer userdata
lua_VkFramebuffer* lua_check_VkFramebuffer(lua_State* L, int idx) {
    lua_VkFramebuffer* ud = (lua_VkFramebuffer*)luaL_checkudata(L, idx, FRAMEBUFFER_MT);
    if (!ud->framebuffer) {
        luaL_error(L, "Invalid VkFramebuffer (already destroyed)");
    }
    if (!ud->device) {
        luaL_error(L, "Invalid VkDevice for VkFramebuffer (already destroyed)");
    }
    return ud;
}

// Create framebuffer: vulkan.create_framebuffer(device, {render_pass, attachments, width, height, layers})
static int l_vulkan_create_framebuffer(lua_State* L) {
    lua_VkDevice* device_ud = lua_check_VkDevice(L, 1);
    luaL_checktype(L, 2, LUA_TTABLE);

    VkFramebufferCreateInfo create_info = {0};
    create_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;

    // Get render pass
    lua_getfield(L, 2, "render_pass");
    lua_VkRenderPass* render_pass_ud = lua_check_VkRenderPass(L, -1);
    create_info.renderPass = render_pass_ud->render_pass;
    lua_pop(L, 1);

    // Get attachments (image views)
    lua_getfield(L, 2, "attachments");
    luaL_checktype(L, -1, LUA_TTABLE);
    create_info.attachmentCount = lua_rawlen(L, -1);
    VkImageView* attachments = NULL;
    if (create_info.attachmentCount > 0) {
        attachments = (VkImageView*)malloc(create_info.attachmentCount * sizeof(VkImageView));
        if (!attachments) {
            luaL_error(L, "Failed to allocate memory for framebuffer attachments");
        }
        for (uint32_t i = 1; i <= create_info.attachmentCount; i++) {
            lua_rawgeti(L, -1, i);
            lua_VkImageView* image_view_ud = lua_check_VkImageView(L, -1);
            attachments[i-1] = image_view_ud->image_view;
            lua_pop(L, 1);
        }
        create_info.pAttachments = attachments;
    }
    lua_pop(L, 1);

    // Get width, height, and layers
    lua_getfield(L, 2, "width");
    create_info.width = luaL_checkinteger(L, -1);
    lua_pop(L, 1);

    lua_getfield(L, 2, "height");
    create_info.height = luaL_checkinteger(L, -1);
    lua_pop(L, 1);

    lua_getfield(L, 2, "layers");
    create_info.layers = luaL_checkinteger(L, -1);
    lua_pop(L, 1);

    VkFramebuffer framebuffer;
    VkResult result = vkCreateFramebuffer(device_ud->device, &create_info, NULL, &framebuffer);
    free(attachments);
    if (result != VK_SUCCESS) {
        luaL_error(L, "Failed to create framebuffer: VkResult %d", result);
    }

    lua_push_VkFramebuffer(L, framebuffer, device_ud->device);
    return 1;
}

// Metatable setup
static void framebuffer_metatable(lua_State* L) {
    luaL_newmetatable(L, FRAMEBUFFER_MT);
    lua_pushcfunction(L, framebuffer_gc);
    lua_setfield(L, -2, "__gc");
    lua_pop(L, 1);
}

//===============================================
// shader
//===============================================
// Garbage collection for VkShaderModule
static int shader_module_gc(lua_State* L) {
    lua_VkShaderModule* ud = (lua_VkShaderModule*)luaL_checkudata(L, 1, SHADER_MODULE_MT);
    if (ud->shader_module && ud->device) {
        vkDestroyShaderModule(ud->device, ud->shader_module, NULL);
        ud->shader_module = VK_NULL_HANDLE;
        ud->device = VK_NULL_HANDLE;
    }
    return 0;
}

// Garbage collection for VkPipelineLayout
static int pipeline_layout_gc(lua_State* L) {
    lua_VkPipelineLayout* ud = (lua_VkPipelineLayout*)luaL_checkudata(L, 1, PIPELINE_LAYOUT_MT);
    if (ud->pipeline_layout && ud->device) {
        vkDestroyPipelineLayout(ud->device, ud->pipeline_layout, NULL);
        ud->pipeline_layout = VK_NULL_HANDLE;
        ud->device = VK_NULL_HANDLE;
    }
    return 0;
}

// Garbage collection for VkPipeline
static int pipeline_gc(lua_State* L) {
    lua_VkPipeline* ud = (lua_VkPipeline*)luaL_checkudata(L, 1, PIPELINE_MT);
    if (ud->pipeline && ud->device) {
        vkDestroyPipeline(ud->device, ud->pipeline, NULL);
        ud->pipeline = VK_NULL_HANDLE;
        ud->device = VK_NULL_HANDLE;
    }
    return 0;
}

// Push VkShaderModule userdata
void lua_push_VkShaderModule(lua_State* L, VkShaderModule shader_module, VkDevice device) {
    if (!shader_module) {
        luaL_error(L, "Cannot create userdata for null VkShaderModule");
    }
    lua_VkShaderModule* ud = (lua_VkShaderModule*)lua_newuserdata(L, sizeof(lua_VkShaderModule));
    ud->shader_module = shader_module;
    ud->device = device;
    luaL_setmetatable(L, SHADER_MODULE_MT);
}

// Check VkShaderModule userdata
lua_VkShaderModule* lua_check_VkShaderModule(lua_State* L, int idx) {
    lua_VkShaderModule* ud = (lua_VkShaderModule*)luaL_checkudata(L, idx, SHADER_MODULE_MT);
    if (!ud->shader_module) {
        luaL_error(L, "Invalid VkShaderModule (already destroyed)");
    }
    if (!ud->device) {
        luaL_error(L, "Invalid VkDevice for VkShaderModule (already destroyed)");
    }
    return ud;
}

// Push VkPipelineLayout userdata
void lua_push_VkPipelineLayout(lua_State* L, VkPipelineLayout pipeline_layout, VkDevice device) {
    if (!pipeline_layout) {
        luaL_error(L, "Cannot create userdata for null VkPipelineLayout");
    }
    lua_VkPipelineLayout* ud = (lua_VkPipelineLayout*)lua_newuserdata(L, sizeof(lua_VkPipelineLayout));
    ud->pipeline_layout = pipeline_layout;
    ud->device = device;
    luaL_setmetatable(L, PIPELINE_LAYOUT_MT);
}

// Check VkPipelineLayout userdata
lua_VkPipelineLayout* lua_check_VkPipelineLayout(lua_State* L, int idx) {
    lua_VkPipelineLayout* ud = (lua_VkPipelineLayout*)luaL_checkudata(L, idx, PIPELINE_LAYOUT_MT);
    if (!ud->pipeline_layout) {
        luaL_error(L, "Invalid VkPipelineLayout (already destroyed)");
    }
    if (!ud->device) {
        luaL_error(L, "Invalid VkDevice for VkPipelineLayout (already destroyed)");
    }
    return ud;
}

// Push VkPipeline userdata
void lua_push_VkPipeline(lua_State* L, VkPipeline pipeline, VkDevice device) {
    if (!pipeline) {
        luaL_error(L, "Cannot create userdata for null VkPipeline");
    }
    lua_VkPipeline* ud = (lua_VkPipeline*)lua_newuserdata(L, sizeof(lua_VkPipeline));
    ud->pipeline = pipeline;
    ud->device = device;
    luaL_setmetatable(L, PIPELINE_MT);
}

// Check VkPipeline userdata
lua_VkPipeline* lua_check_VkPipeline(lua_State* L, int idx) {
    lua_VkPipeline* ud = (lua_VkPipeline*)luaL_checkudata(L, idx, PIPELINE_MT);
    if (!ud->pipeline) {
        luaL_error(L, "Invalid VkPipeline (already destroyed)");
    }
    if (!ud->device) {
        luaL_error(L, "Invalid VkDevice for VkPipeline (already destroyed)");
    }
    return ud;
}

// Create shader module: vulkan.create_shader_module(device, code)
static int l_vulkan_create_shader_module(lua_State* L) {
    lua_VkDevice* device_ud = lua_check_VkDevice(L, 1);
    size_t code_size;
    const char* code = luaL_checklstring(L, 2, &code_size);

    VkShaderModuleCreateInfo create_info = {0};
    create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    create_info.codeSize = code_size;
    create_info.pCode = (const uint32_t*)code;

    VkShaderModule shader_module;
    VkResult result = vkCreateShaderModule(device_ud->device, &create_info, NULL, &shader_module);
    if (result != VK_SUCCESS) {
        luaL_error(L, "Failed to create shader module: VkResult %d", result);
    }

    lua_push_VkShaderModule(L, shader_module, device_ud->device);
    return 1;
}

// Create pipeline layout: vulkan.create_pipeline_layout(device, {set_layouts, push_constant_ranges})
static int l_vulkan_create_pipeline_layout(lua_State* L) {
    lua_VkDevice* device_ud = lua_check_VkDevice(L, 1);
    luaL_checktype(L, 2, LUA_TTABLE);

    VkPipelineLayoutCreateInfo create_info = {0};
    create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;

    // Get descriptor set layouts (optional)
    lua_getfield(L, 2, "set_layouts");
    if (!lua_isnil(L, -1)) {
        luaL_checktype(L, -1, LUA_TTABLE);
        create_info.setLayoutCount = lua_rawlen(L, -1);
        if (create_info.setLayoutCount > 0) {
            VkDescriptorSetLayout* set_layouts = (VkDescriptorSetLayout*)malloc(create_info.setLayoutCount * sizeof(VkDescriptorSetLayout));
            if (!set_layouts) {
                luaL_error(L, "Failed to allocate memory for descriptor set layouts");
            }
            for (uint32_t i = 1; i <= create_info.setLayoutCount; i++) {
                lua_rawgeti(L, -1, i);
                set_layouts[i-1] = (VkDescriptorSetLayout)lua_touserdata(L, -1); // Simplified; assumes raw handles
                lua_pop(L, 1);
            }
            create_info.pSetLayouts = set_layouts;
        }
    }
    lua_pop(L, 1);

    // Get push constant ranges (optional)
    lua_getfield(L, 2, "push_constant_ranges");
    if (!lua_isnil(L, -1)) {
        luaL_checktype(L, -1, LUA_TTABLE);
        create_info.pushConstantRangeCount = lua_rawlen(L, -1);
        if (create_info.pushConstantRangeCount > 0) {
            VkPushConstantRange* ranges = (VkPushConstantRange*)malloc(create_info.pushConstantRangeCount * sizeof(VkPushConstantRange));
            if (!ranges) {
                free((void*)create_info.pSetLayouts);
                luaL_error(L, "Failed to allocate memory for push constant ranges");
            }
            for (uint32_t i = 1; i <= create_info.pushConstantRangeCount; i++) {
                lua_rawgeti(L, -1, i);
                lua_getfield(L, -1, "stage_flags");
                ranges[i-1].stageFlags = luaL_checkinteger(L, -1);
                lua_pop(L, 1);
                lua_getfield(L, -1, "offset");
                ranges[i-1].offset = luaL_checkinteger(L, -1);
                lua_pop(L, 1);
                lua_getfield(L, -1, "size");
                ranges[i-1].size = luaL_checkinteger(L, -1);
                lua_pop(L, 1);
                lua_pop(L, 1);
            }
            create_info.pPushConstantRanges = ranges;
        }
    }
    lua_pop(L, 1);

    VkPipelineLayout pipeline_layout;
    VkResult result = vkCreatePipelineLayout(device_ud->device, &create_info, NULL, &pipeline_layout);
    free((void*)create_info.pSetLayouts);
    free((void*)create_info.pPushConstantRanges);
    if (result != VK_SUCCESS) {
        luaL_error(L, "Failed to create pipeline layout: VkResult %d", result);
    }

    lua_push_VkPipelineLayout(L, pipeline_layout, device_ud->device);
    return 1;
}

// Create graphics pipeline: vulkan.create_graphics_pipelines(device, {pipelines = {...}})
static int l_vulkan_create_graphics_pipelines(lua_State* L) {
    lua_VkDevice* device_ud = lua_check_VkDevice(L, 1);
    luaL_checktype(L, 2, LUA_TTABLE);

    lua_getfield(L, 2, "pipelines");
    luaL_checktype(L, -1, LUA_TTABLE);
    uint32_t pipeline_count = lua_rawlen(L, -1);
    if (pipeline_count == 0) {
        luaL_error(L, "No pipelines specified");
    }

    VkGraphicsPipelineCreateInfo* pipeline_infos = (VkGraphicsPipelineCreateInfo*)malloc(pipeline_count * sizeof(VkGraphicsPipelineCreateInfo));
    if (!pipeline_infos) {
        luaL_error(L, "Failed to allocate memory for pipeline create infos");
    }
    memset(pipeline_infos, 0, pipeline_count * sizeof(VkGraphicsPipelineCreateInfo));

    // Count total shader stages across all pipelines
    uint32_t total_shader_stages = 0;
    for (uint32_t i = 1; i <= pipeline_count; i++) {
        lua_rawgeti(L, -1, i);
        lua_getfield(L, -1, "stages");
        luaL_checktype(L, -1, LUA_TTABLE);
        total_shader_stages += lua_rawlen(L, -1);
        lua_pop(L, 2); // Pop stages and pipeline
    }

    VkPipelineShaderStageCreateInfo* shader_stages = NULL;
    if (total_shader_stages > 0) {
        shader_stages = (VkPipelineShaderStageCreateInfo*)malloc(total_shader_stages * sizeof(VkPipelineShaderStageCreateInfo));
        if (!shader_stages) {
            free(pipeline_infos);
            luaL_error(L, "Failed to allocate memory for shader stages");
        }
        memset(shader_stages, 0, total_shader_stages * sizeof(VkPipelineShaderStageCreateInfo));
    }

    uint32_t stage_index = 0;
    for (uint32_t i = 1; i <= pipeline_count; i++) {
        lua_rawgeti(L, -1, i);
        luaL_checktype(L, -1, LUA_TTABLE);

        pipeline_infos[i-1].sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;

        // Get shader stages
        lua_getfield(L, -1, "stages");
        uint32_t stage_count = lua_rawlen(L, -1);
        if (stage_count > 0) {
            pipeline_infos[i-1].stageCount = stage_count;
            pipeline_infos[i-1].pStages = &shader_stages[stage_index];
            for (uint32_t j = 1; j <= stage_count; j++) {
                lua_rawgeti(L, -1, j);
                shader_stages[stage_index].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
                lua_getfield(L, -1, "stage");
                shader_stages[stage_index].stage = luaL_checkinteger(L, -1);
                lua_pop(L, 1);
                lua_getfield(L, -1, "module");
                lua_VkShaderModule* module_ud = lua_check_VkShaderModule(L, -1);
                shader_stages[stage_index].module = module_ud->shader_module;
                lua_pop(L, 1);
                lua_getfield(L, -1, "name");
                const char* name = luaL_checkstring(L, -1);
                shader_stages[stage_index].pName = name; // Note: Assumes string persists
                lua_pop(L, 1);
                stage_index++;
                lua_pop(L, 1); // Pop stage table
            }
        }
        lua_pop(L, 1); // Pop stages

        // Get render pass
        lua_getfield(L, -1, "render_pass");
        lua_VkRenderPass* render_pass_ud = lua_check_VkRenderPass(L, -1);
        pipeline_infos[i-1].renderPass = render_pass_ud->render_pass;
        lua_pop(L, 1);

        // Get pipeline layout
        lua_getfield(L, -1, "layout");
        lua_VkPipelineLayout* layout_ud = lua_check_VkPipelineLayout(L, -1);
        pipeline_infos[i-1].layout = layout_ud->pipeline_layout;
        lua_pop(L, 1);

        // Get subpass
        lua_getfield(L, -1, "subpass");
        pipeline_infos[i-1].subpass = luaL_checkinteger(L, -1);
        lua_pop(L, 1);

        // Set up minimal pipeline state
        VkPipelineVertexInputStateCreateInfo vertex_input_info = {0};
        vertex_input_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

        VkPipelineInputAssemblyStateCreateInfo input_assembly = {0};
        input_assembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        input_assembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        input_assembly.primitiveRestartEnable = VK_FALSE;

        VkPipelineViewportStateCreateInfo viewport_state = {0};
        viewport_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewport_state.viewportCount = 1;
        viewport_state.scissorCount = 1;

        VkPipelineRasterizationStateCreateInfo rasterizer = {0};
        rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizer.depthClampEnable = VK_FALSE;
        rasterizer.rasterizerDiscardEnable = VK_FALSE;
        rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
        rasterizer.lineWidth = 1.0f;
        rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
        rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
        rasterizer.depthBiasEnable = VK_FALSE;

        VkPipelineMultisampleStateCreateInfo multisampling = {0};
        multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisampling.sampleShadingEnable = VK_FALSE;
        multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

        VkPipelineColorBlendAttachmentState color_blend_attachment = {0};
        color_blend_attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        color_blend_attachment.blendEnable = VK_FALSE;

        VkPipelineColorBlendStateCreateInfo color_blending = {0};
        color_blending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        color_blending.logicOpEnable = VK_FALSE;
        color_blending.attachmentCount = 1;
        color_blending.pAttachments = &color_blend_attachment;

        VkDynamicState dynamic_states[] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
        VkPipelineDynamicStateCreateInfo dynamic_state = {0};
        dynamic_state.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamic_state.dynamicStateCount = 2;
        dynamic_state.pDynamicStates = dynamic_states;

        pipeline_infos[i-1].pVertexInputState = &vertex_input_info;
        pipeline_infos[i-1].pInputAssemblyState = &input_assembly;
        pipeline_infos[i-1].pViewportState = &viewport_state;
        pipeline_infos[i-1].pRasterizationState = &rasterizer;
        pipeline_infos[i-1].pMultisampleState = &multisampling;
        pipeline_infos[i-1].pColorBlendState = &color_blending;
        pipeline_infos[i-1].pDynamicState = &dynamic_state;

        lua_pop(L, 1); // Pop pipeline table
    }
    lua_pop(L, 1); // Pop pipelines table

    VkPipeline* pipelines = (VkPipeline*)malloc(pipeline_count * sizeof(VkPipeline));
    if (!pipelines) {
        free(shader_stages);
        free(pipeline_infos);
        luaL_error(L, "Failed to allocate memory for pipelines");
    }

    VkResult result = vkCreateGraphicsPipelines(device_ud->device, VK_NULL_HANDLE, pipeline_count, pipeline_infos, NULL, pipelines);
    if (result != VK_SUCCESS) {
        free(pipelines);
        free(shader_stages);
        free(pipeline_infos);
        luaL_error(L, "Failed to create graphics pipelines: VkResult %d", result);
    }

    lua_newtable(L);
    for (uint32_t i = 0; i < pipeline_count; i++) {
        lua_push_VkPipeline(L, pipelines[i], device_ud->device);
        lua_rawseti(L, -2, i + 1);
    }
    free(pipelines);
    free(shader_stages);
    free(pipeline_infos);

    return 1;
}

// Metatable setup
static void shader_module_metatable(lua_State* L) {
    luaL_newmetatable(L, SHADER_MODULE_MT);
    lua_pushcfunction(L, shader_module_gc);
    lua_setfield(L, -2, "__gc");
    lua_pop(L, 1);
}

static void pipeline_layout_metatable(lua_State* L) {
    luaL_newmetatable(L, PIPELINE_LAYOUT_MT);
    lua_pushcfunction(L, pipeline_layout_gc);
    lua_setfield(L, -2, "__gc");
    lua_pop(L, 1);
}

static void pipeline_metatable(lua_State* L) {
    luaL_newmetatable(L, PIPELINE_MT);
    lua_pushcfunction(L, pipeline_gc);
    lua_setfield(L, -2, "__gc");
    lua_pop(L, 1);
}


//===============================================
// semaphore, fence, command
//===============================================
// Garbage collection for VkSemaphore
static int semaphore_gc(lua_State* L) {
    lua_VkSemaphore* ud = (lua_VkSemaphore*)luaL_checkudata(L, 1, SEMAPHORE_MT);
    if (ud->semaphore && ud->device) {
        vkDestroySemaphore(ud->device, ud->semaphore, NULL);
        ud->semaphore = VK_NULL_HANDLE;
        ud->device = VK_NULL_HANDLE;
    }
    return 0;
}

// Garbage collection for VkFence
static int fence_gc(lua_State* L) {
    lua_VkFence* ud = (lua_VkFence*)luaL_checkudata(L, 1, FENCE_MT);
    if (ud->fence && ud->device) {
        vkDestroyFence(ud->device, ud->fence, NULL);
        ud->fence = VK_NULL_HANDLE;
        ud->device = VK_NULL_HANDLE;
    }
    return 0;
}

// Garbage collection for VkCommandPool
static int command_pool_gc(lua_State* L) {
    lua_VkCommandPool* ud = (lua_VkCommandPool*)luaL_checkudata(L, 1, COMMAND_POOL_MT);
    if (ud->command_pool && ud->device) {
        vkDestroyCommandPool(ud->device, ud->command_pool, NULL);
        ud->command_pool = VK_NULL_HANDLE;
        ud->device = VK_NULL_HANDLE;
    }
    return 0;
}

// Garbage collection for VkCommandBuffer
static int command_buffer_gc(lua_State* L) {
    lua_VkCommandBuffer* ud = (lua_VkCommandBuffer*)luaL_checkudata(L, 1, COMMAND_BUFFER_MT);
    // Note: Command buffers are freed with the command pool, so no vkFreeCommandBuffers here
    ud->command_buffer = VK_NULL_HANDLE;
    ud->device = VK_NULL_HANDLE;
    return 0;
}

// Push VkSemaphore userdata
void lua_push_VkSemaphore(lua_State* L, VkSemaphore semaphore, VkDevice device) {
    if (!semaphore) {
        luaL_error(L, "Cannot create userdata for null VkSemaphore");
    }
    lua_VkSemaphore* ud = (lua_VkSemaphore*)lua_newuserdata(L, sizeof(lua_VkSemaphore));
    ud->semaphore = semaphore;
    ud->device = device;
    luaL_setmetatable(L, SEMAPHORE_MT);
}

// Check VkSemaphore userdata
lua_VkSemaphore* lua_check_VkSemaphore(lua_State* L, int idx) {
    lua_VkSemaphore* ud = (lua_VkSemaphore*)luaL_checkudata(L, idx, SEMAPHORE_MT);
    if (!ud->semaphore) {
        luaL_error(L, "Invalid VkSemaphore (already destroyed)");
    }
    if (!ud->device) {
        luaL_error(L, "Invalid VkDevice for VkSemaphore (already destroyed)");
    }
    return ud;
}

// Push VkFence userdata
void lua_push_VkFence(lua_State* L, VkFence fence, VkDevice device) {
    if (!fence) {
        luaL_error(L, "Cannot create userdata for null VkFence");
    }
    lua_VkFence* ud = (lua_VkFence*)lua_newuserdata(L, sizeof(lua_VkFence));
    ud->fence = fence;
    ud->device = device;
    luaL_setmetatable(L, FENCE_MT);
}

// Check VkFence userdata
lua_VkFence* lua_check_VkFence(lua_State* L, int idx) {
    lua_VkFence* ud = (lua_VkFence*)luaL_checkudata(L, idx, FENCE_MT);
    if (!ud->fence) {
        luaL_error(L, "Invalid VkFence (already destroyed)");
    }
    if (!ud->device) {
        luaL_error(L, "Invalid VkDevice for VkFence (already destroyed)");
    }
    return ud;
}

// Push VkCommandPool userdata
void lua_push_VkCommandPool(lua_State* L, VkCommandPool command_pool, VkDevice device) {
    if (!command_pool) {
        luaL_error(L, "Cannot create userdata for null VkCommandPool");
    }
    lua_VkCommandPool* ud = (lua_VkCommandPool*)lua_newuserdata(L, sizeof(lua_VkCommandPool));
    ud->command_pool = command_pool;
    ud->device = device;
    luaL_setmetatable(L, COMMAND_POOL_MT);
}

// Check VkCommandPool userdata
lua_VkCommandPool* lua_check_VkCommandPool(lua_State* L, int idx) {
    lua_VkCommandPool* ud = (lua_VkCommandPool*)luaL_checkudata(L, idx, COMMAND_POOL_MT);
    if (!ud->command_pool) {
        luaL_error(L, "Invalid VkCommandPool (already destroyed)");
    }
    if (!ud->device) {
        luaL_error(L, "Invalid VkDevice for VkCommandPool (already destroyed)");
    }
    return ud;
}

// Push VkCommandBuffer userdata
void lua_push_VkCommandBuffer(lua_State* L, VkCommandBuffer command_buffer, VkDevice device) {
    if (!command_buffer) {
        luaL_error(L, "Cannot create userdata for null VkCommandBuffer");
    }
    lua_VkCommandBuffer* ud = (lua_VkCommandBuffer*)lua_newuserdata(L, sizeof(lua_VkCommandBuffer));
    ud->command_buffer = command_buffer;
    ud->device = device;
    luaL_setmetatable(L, COMMAND_BUFFER_MT);
}

// Check VkCommandBuffer userdata
lua_VkCommandBuffer* lua_check_VkCommandBuffer(lua_State* L, int idx) {
    lua_VkCommandBuffer* ud = (lua_VkCommandBuffer*)luaL_checkudata(L, idx, COMMAND_BUFFER_MT);
    if (!ud->command_buffer) {
        luaL_error(L, "Invalid VkCommandBuffer (already destroyed)");
    }
    if (!ud->device) {
        luaL_error(L, "Invalid VkDevice for VkCommandBuffer (already destroyed)");
    }
    return ud;
}

// Create semaphore: vulkan.create_semaphore(device)
static int l_vulkan_create_semaphore(lua_State* L) {
    lua_VkDevice* device_ud = lua_check_VkDevice(L, 1);

    VkSemaphoreCreateInfo create_info = {0};
    create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkSemaphore semaphore;
    VkResult result = vkCreateSemaphore(device_ud->device, &create_info, NULL, &semaphore);
    if (result != VK_SUCCESS) {
        luaL_error(L, "Failed to create semaphore: VkResult %d", result);
    }

    lua_push_VkSemaphore(L, semaphore, device_ud->device);
    return 1;
}

// Create fence: vulkan.create_fence(device, signaled)
static int l_vulkan_create_fence(lua_State* L) {
    lua_VkDevice* device_ud = lua_check_VkDevice(L, 1);
    int signaled = lua_toboolean(L, 2);

    VkFenceCreateInfo create_info = {0};
    create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    if (signaled) {
        create_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    }

    VkFence fence;
    VkResult result = vkCreateFence(device_ud->device, &create_info, NULL, &fence);
    if (result != VK_SUCCESS) {
        luaL_error(L, "Failed to create fence: VkResult %d", result);
    }

    lua_push_VkFence(L, fence, device_ud->device);
    return 1;
}

// Create command pool: vulkan.create_command_pool(device, queue_family_index)
static int l_vulkan_create_command_pool(lua_State* L) {
    lua_VkDevice* device_ud = lua_check_VkDevice(L, 1);
    uint32_t queue_family_index = (uint32_t)luaL_checkinteger(L, 2);

    VkCommandPoolCreateInfo create_info = {0};
    create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    create_info.queueFamilyIndex = queue_family_index;
    create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

    VkCommandPool command_pool;
    VkResult result = vkCreateCommandPool(device_ud->device, &create_info, NULL, &command_pool);
    if (result != VK_SUCCESS) {
        char error_msg[128];
        snprintf(error_msg, sizeof(error_msg), "Failed to create command pool: VkResult %d", result);
        lua_pushnil(L);
        lua_pushstring(L, error_msg);
        return 2;
    }

    lua_push_VkCommandPool(L, command_pool, device_ud->device);
    return 1;
}

// Allocate command buffers: vulkan.create_allocate_command_buffers(device, command_pool, count)
static int l_vulkan_create_allocate_command_buffers(lua_State* L) {
    lua_VkDevice* device_ud = lua_check_VkDevice(L, 1);
    lua_VkCommandPool* pool_ud = lua_check_VkCommandPool(L, 2);
    uint32_t count = luaL_checkinteger(L, 3);

    VkCommandBufferAllocateInfo alloc_info = {0};
    alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    alloc_info.commandPool = pool_ud->command_pool;
    alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    alloc_info.commandBufferCount = count;

    VkCommandBuffer* command_buffers = (VkCommandBuffer*)malloc(count * sizeof(VkCommandBuffer));
    if (!command_buffers) {
        luaL_error(L, "Failed to allocate memory for command buffers");
    }

    VkResult result = vkAllocateCommandBuffers(device_ud->device, &alloc_info, command_buffers);
    if (result != VK_SUCCESS) {
        free(command_buffers);
        luaL_error(L, "Failed to allocate command buffers: VkResult %d", result);
    }

    lua_newtable(L);
    for (uint32_t i = 0; i < count; i++) {
        lua_push_VkCommandBuffer(L, command_buffers[i], device_ud->device);
        lua_rawseti(L, -2, i + 1);
    }
    free(command_buffers);

    return 1;
}

// Metatable setup
static void semaphore_metatable(lua_State* L) {
    luaL_newmetatable(L, SEMAPHORE_MT);
    lua_pushcfunction(L, semaphore_gc);
    lua_setfield(L, -2, "__gc");
    lua_pop(L, 1);
}

static void fence_metatable(lua_State* L) {
    luaL_newmetatable(L, FENCE_MT);
    lua_pushcfunction(L, fence_gc);
    lua_setfield(L, -2, "__gc");
    lua_pop(L, 1);
}

static void command_pool_metatable(lua_State* L) {
    luaL_newmetatable(L, COMMAND_POOL_MT);
    lua_pushcfunction(L, command_pool_gc);
    lua_setfield(L, -2, "__gc");
    lua_pop(L, 1);
}

static void command_buffer_metatable(lua_State* L) {
    luaL_newmetatable(L, COMMAND_BUFFER_MT);
    lua_pushcfunction(L, command_buffer_gc);
    lua_setfield(L, -2, "__gc");
    lua_pushstring(L, "vulkan.command_buffer");
    lua_setfield(L, -2, "__name"); // Set metatable name
    lua_pop(L, 1);
}

// Wait for fences: vulkan.wait_for_fences(device, fence)
static int l_vulkan_wait_for_fences(lua_State* L) {
    lua_VkDevice* device_ud = lua_check_VkDevice(L, 1);
    lua_VkFence* fence_ud = lua_check_VkFence(L, 2);

    VkResult result = vkWaitForFences(device_ud->device, 1, &fence_ud->fence, VK_TRUE, UINT64_MAX);
    if (result != VK_SUCCESS) {
        luaL_error(L, "Failed to wait for fences: VkResult %d", result);
    }

    return 0;
}

// Reset fences: vulkan.reset_fences(device, fence)
static int l_vulkan_reset_fences(lua_State* L) {
    lua_VkDevice* device_ud = lua_check_VkDevice(L, 1);
    lua_VkFence* fence_ud = lua_check_VkFence(L, 2);

    VkResult result = vkResetFences(device_ud->device, 1, &fence_ud->fence);
    if (result != VK_SUCCESS) {
        luaL_error(L, "Failed to reset fences: VkResult %d", result);
    }

    return 0;
}

// Acquire next image: vulkan.acquire_next_image_KHR(device, swapchain, timeout, semaphore, fence)
static int l_vulkan_acquire_next_image_KHR(lua_State* L) {
    lua_VkDevice* device_ud = lua_check_VkDevice(L, 1);
    lua_VkSwapchainKHR* swapchain_ud = lua_check_VkSwapchainKHR(L, 2);
    uint64_t timeout = luaL_checknumber(L, 3);
    lua_VkSemaphore* semaphore_ud = lua_check_VkSemaphore(L, 4);
    lua_VkFence* fence_ud = lua_isnil(L, 5) ? NULL : lua_check_VkFence(L, 5);

    uint32_t image_index;
    VkResult result = vkAcquireNextImageKHR(device_ud->device, swapchain_ud->swapchain, timeout,
                                            semaphore_ud->semaphore, fence_ud ? fence_ud->fence : VK_NULL_HANDLE, &image_index);
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
        lua_pushnil(L);
        return 1; // Swapchain needs recreation, return nil
    }
    if (result != VK_SUCCESS) {
        luaL_error(L, "Failed to acquire next image: VkResult %d", result);
    }

    lua_pushinteger(L, image_index);
    return 1;
}

// Reset command buffer: vulkan.reset_command_buffer(command_buffer)
static int l_vulkan_reset_command_buffer(lua_State* L) {
    lua_VkCommandBuffer* cmd_buffer_ud = lua_check_VkCommandBuffer(L, 1);

    VkResult result = vkResetCommandBuffer(cmd_buffer_ud->command_buffer, 0);
    if (result != VK_SUCCESS) {
        luaL_error(L, "Failed to reset command buffer: VkResult %d", result);
    }

    return 0;
}

// Begin command buffer: vulkan.begin_command_buffer(command_buffer)
static int l_vulkan_begin_command_buffer(lua_State* L) {
    lua_VkCommandBuffer* cmd_buffer_ud = lua_check_VkCommandBuffer(L, 1);

    VkCommandBufferBeginInfo begin_info = {0};
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    VkResult result = vkBeginCommandBuffer(cmd_buffer_ud->command_buffer, &begin_info);
    if (result != VK_SUCCESS) {
        luaL_error(L, "Failed to begin command buffer: VkResult %d", result);
    }

    return 0;
}

// Begin render pass: vulkan.cmd_begin_renderpass(command_buffer, render_pass, framebuffer)
static int l_vulkan_cmd_begin_renderpass(lua_State* L) {
    lua_VkCommandBuffer* cmd_buffer_ud = lua_check_VkCommandBuffer(L, 1);
    lua_VkRenderPass* render_pass_ud = lua_check_VkRenderPass(L, 2);
    lua_VkFramebuffer* framebuffer_ud = lua_check_VkFramebuffer(L, 3);

    VkClearValue clear_color = {{{0.0f, 0.0f, 0.0f, 1.0f}}}; // Black background
    VkRenderPassBeginInfo render_pass_info = {0};
    render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    render_pass_info.renderPass = render_pass_ud->render_pass;
    render_pass_info.framebuffer = framebuffer_ud->framebuffer;
    render_pass_info.renderArea.offset = (VkOffset2D){0, 0};
    render_pass_info.renderArea.extent.width = 800; // Hardcoded for simplicity
    render_pass_info.renderArea.extent.height = 600;
    render_pass_info.clearValueCount = 1;
    render_pass_info.pClearValues = &clear_color;

    vkCmdBeginRenderPass(cmd_buffer_ud->command_buffer, &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);
    return 0;
}

// Bind pipeline: vulkan.cmd_bind_pipeline(command_buffer, pipeline)
static int l_vulkan_cmd_bind_pipeline(lua_State* L) {
    lua_VkCommandBuffer* cmd_buffer_ud = lua_check_VkCommandBuffer(L, 1);
    lua_VkPipeline* pipeline_ud = lua_check_VkPipeline(L, 2);

    vkCmdBindPipeline(cmd_buffer_ud->command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_ud->pipeline);
    return 0;
}

// Draw command: vulkan.cmd_draw(command_buffer, vertex_count, instance_count, first_vertex, first_instance)
static int l_vulkan_cmd_draw(lua_State* L) {
    lua_VkCommandBuffer* cmd_buffer_ud = lua_check_VkCommandBuffer(L, 1);
    uint32_t vertex_count = luaL_checkinteger(L, 2);
    uint32_t instance_count = luaL_checkinteger(L, 3);
    uint32_t first_vertex = luaL_checkinteger(L, 4);
    uint32_t first_instance = luaL_checkinteger(L, 5);

    vkCmdDraw(cmd_buffer_ud->command_buffer, vertex_count, instance_count, first_vertex, first_instance);
    return 0;
}

// End render pass: vulkan.cmd_end_renderpass(command_buffer)
static int l_vulkan_cmd_end_renderpass(lua_State* L) {
    lua_VkCommandBuffer* cmd_buffer_ud = lua_check_VkCommandBuffer(L, 1);
    vkCmdEndRenderPass(cmd_buffer_ud->command_buffer);
    return 0;
}

// End command buffer: vulkan.end_commandbuffer(command_buffer)
static int l_vulkan_end_commandbuffer(lua_State* L) {
    lua_VkCommandBuffer* cmd_buffer_ud = lua_check_VkCommandBuffer(L, 1);

    VkResult result = vkEndCommandBuffer(cmd_buffer_ud->command_buffer);
    if (result != VK_SUCCESS) {
        luaL_error(L, "Failed to end command buffer: VkResult %d", result);
    }

    return 0;
}

// Submit queue: vulkan.queue_submit(queue, command_buffer, wait_semaphore, signal_semaphore, fence)
// Queue submit: vulkan.queue_submit(queue, {wait_semaphores, wait_dst_stage_mask, command_buffers, signal_semaphores}, fence)
static int l_vulkan_queue_submit(lua_State* L) {
    lua_VkQueue* queue_ud = lua_check_VkQueue(L, 1);
    luaL_checktype(L, 2, LUA_TTABLE);
    lua_VkFence* fence_ud = lua_isnil(L, 3) ? NULL : lua_check_VkFence(L, 3);

    VkSubmitInfo submit_info = {0};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    // Get wait semaphores
    lua_getfield(L, 2, "wait_semaphores");
    luaL_checktype(L, -1, LUA_TTABLE);
    submit_info.waitSemaphoreCount = lua_rawlen(L, -1);
    VkSemaphore* wait_semaphores = NULL;
    if (submit_info.waitSemaphoreCount > 0) {
        wait_semaphores = (VkSemaphore*)malloc(submit_info.waitSemaphoreCount * sizeof(VkSemaphore));
        if (!wait_semaphores) {
            luaL_error(L, "Failed to allocate memory for wait semaphores");
        }
        for (uint32_t i = 1; i <= submit_info.waitSemaphoreCount; i++) {
            lua_rawgeti(L, -1, i);
            lua_VkSemaphore* sem_ud = lua_check_VkSemaphore(L, -1);
            wait_semaphores[i-1] = sem_ud->semaphore;
            lua_pop(L, 1);
        }
        submit_info.pWaitSemaphores = wait_semaphores;
    }
    lua_pop(L, 1);

    // Get wait destination stage mask
    lua_getfield(L, 2, "wait_dst_stage_mask");
    luaL_checktype(L, -1, LUA_TTABLE);
    if (lua_rawlen(L, -1) != submit_info.waitSemaphoreCount) {
        free(wait_semaphores);
        luaL_error(L, "Mismatch between wait semaphores and stage mask count");
    }
    VkPipelineStageFlags* wait_stages = NULL;
    if (submit_info.waitSemaphoreCount > 0) {
        wait_stages = (VkPipelineStageFlags*)malloc(submit_info.waitSemaphoreCount * sizeof(VkPipelineStageFlags));
        if (!wait_stages) {
            free(wait_semaphores);
            luaL_error(L, "Failed to allocate memory for wait stages");
        }
        for (uint32_t i = 1; i <= submit_info.waitSemaphoreCount; i++) {
            lua_rawgeti(L, -1, i);
            wait_stages[i-1] = luaL_checkinteger(L, -1);
            lua_pop(L, 1);
        }
        submit_info.pWaitDstStageMask = wait_stages;
    }
    lua_pop(L, 1);

    // Get command buffers
    VkCommandBuffer* command_buffers = NULL;
    lua_getfield(L, 2, "command_buffers");
    if (luaL_testudata(L, -1, COMMAND_BUFFER_MT)) {
        // Single command buffer
        submit_info.commandBufferCount = 1;
        command_buffers = (VkCommandBuffer*)malloc(sizeof(VkCommandBuffer));
        if (!command_buffers) {
            free(wait_semaphores);
            free(wait_stages);
            luaL_error(L, "Failed to allocate memory for command buffer");
        }
        lua_VkCommandBuffer* cmd_ud = lua_check_VkCommandBuffer(L, -1);
        command_buffers[0] = cmd_ud->command_buffer;
    } else {
        // Table of command buffers
        luaL_checktype(L, -1, LUA_TTABLE);
        submit_info.commandBufferCount = lua_rawlen(L, -1);
        if (submit_info.commandBufferCount > 0) {
            command_buffers = (VkCommandBuffer*)malloc(submit_info.commandBufferCount * sizeof(VkCommandBuffer));
            if (!command_buffers) {
                free(wait_semaphores);
                free(wait_stages);
                luaL_error(L, "Failed to allocate memory for command buffers");
            }
            for (uint32_t i = 1; i <= submit_info.commandBufferCount; i++) {
                lua_rawgeti(L, -1, i);
                if (!luaL_testudata(L, -1, COMMAND_BUFFER_MT)) {
                    free(wait_semaphores);
                    free(wait_stages);
                    free(command_buffers);
                    luaL_error(L, "Expected vulkan.command_buffer at index %d, got %s", i, lua_typename(L, lua_type(L, -1)));
                }
                lua_VkCommandBuffer* cmd_ud = lua_check_VkCommandBuffer(L, -1);
                command_buffers[i-1] = cmd_ud->command_buffer;
                lua_pop(L, 1);
            }
        }
    }
    submit_info.pCommandBuffers = command_buffers;
    lua_pop(L, 1);

    // Get signal semaphores
    lua_getfield(L, 2, "signal_semaphores");
    luaL_checktype(L, -1, LUA_TTABLE);
    submit_info.signalSemaphoreCount = lua_rawlen(L, -1);
    VkSemaphore* signal_semaphores = NULL;
    if (submit_info.signalSemaphoreCount > 0) {
        signal_semaphores = (VkSemaphore*)malloc(submit_info.signalSemaphoreCount * sizeof(VkSemaphore));
        if (!signal_semaphores) {
            free(wait_semaphores);
            free(wait_stages);
            free(command_buffers);
            luaL_error(L, "Failed to allocate memory for signal semaphores");
        }
        for (uint32_t i = 1; i <= submit_info.signalSemaphoreCount; i++) {
            lua_rawgeti(L, -1, i);
            lua_VkSemaphore* sem_ud = lua_check_VkSemaphore(L, -1);
            signal_semaphores[i-1] = sem_ud->semaphore;
            lua_pop(L, 1);
        }
        submit_info.pSignalSemaphores = signal_semaphores;
    }
    lua_pop(L, 1);

    VkResult result = vkQueueSubmit(queue_ud->queue, 1, &submit_info, fence_ud ? fence_ud->fence : VK_NULL_HANDLE);
    free(wait_semaphores);
    free(wait_stages);
    free(command_buffers);
    free(signal_semaphores);

    if (result != VK_SUCCESS) {
        lua_pushboolean(L, false);
        lua_pushfstring(L, "Failed to submit queue: VkResult %d", result);
        return 2;
    }

    lua_pushboolean(L, true);
    return 1;
}

// Present queue: vulkan.queue_present_KHR(queue, {wait_semaphores, swapchains, image_indices})
static int l_vulkan_queue_present_KHR(lua_State* L) {
    lua_VkQueue* queue_ud = lua_check_VkQueue(L, 1);
    luaL_checktype(L, 2, LUA_TTABLE);

    VkPresentInfoKHR present_info = {0};
    present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    // Get wait semaphores
    lua_getfield(L, 2, "wait_semaphores");
    luaL_checktype(L, -1, LUA_TTABLE);
    present_info.waitSemaphoreCount = lua_rawlen(L, -1);
    VkSemaphore* wait_semaphores = NULL;
    if (present_info.waitSemaphoreCount > 0) {
        wait_semaphores = (VkSemaphore*)malloc(present_info.waitSemaphoreCount * sizeof(VkSemaphore));
        if (!wait_semaphores) {
            luaL_error(L, "Failed to allocate memory for wait semaphores");
        }
        for (uint32_t i = 1; i <= present_info.waitSemaphoreCount; i++) {
            lua_rawgeti(L, -1, i);
            lua_VkSemaphore* sem_ud = lua_check_VkSemaphore(L, -1);
            wait_semaphores[i-1] = sem_ud->semaphore;
            lua_pop(L, 1);
        }
        present_info.pWaitSemaphores = wait_semaphores;
    }
    lua_pop(L, 1);

    // Get swapchains
    VkSwapchainKHR* swapchains = NULL;
    lua_getfield(L, 2, "swapchains");
    if (luaL_testudata(L, -1, SWAPCHAIN_MT)) {
        // Single swapchain
        present_info.swapchainCount = 1;
        swapchains = (VkSwapchainKHR*)malloc(sizeof(VkSwapchainKHR));
        if (!swapchains) {
            free(wait_semaphores);
            luaL_error(L, "Failed to allocate memory for swapchain");
        }
        lua_VkSwapchainKHR* swapchain_ud = lua_check_VkSwapchainKHR(L, -1);
        swapchains[0] = swapchain_ud->swapchain;
    } else {
        // Table of swapchains
        luaL_checktype(L, -1, LUA_TTABLE);
        present_info.swapchainCount = lua_rawlen(L, -1);
        if (present_info.swapchainCount > 0) {
            swapchains = (VkSwapchainKHR*)malloc(present_info.swapchainCount * sizeof(VkSwapchainKHR));
            if (!swapchains) {
                free(wait_semaphores);
                luaL_error(L, "Failed to allocate memory for swapchains");
            }
            for (uint32_t i = 1; i <= present_info.swapchainCount; i++) {
                lua_rawgeti(L, -1, i);
                if (!luaL_testudata(L, -1, SWAPCHAIN_MT)) {
                    free(wait_semaphores);
                    free(swapchains);
                    luaL_error(L, "Expected vulkan.swapchain at index %d, got %s", i, lua_typename(L, lua_type(L, -1)));
                }
                lua_VkSwapchainKHR* swapchain_ud = lua_check_VkSwapchainKHR(L, -1);
                swapchains[i-1] = swapchain_ud->swapchain;
                lua_pop(L, 1);
            }
        }
    }
    present_info.pSwapchains = swapchains;
    lua_pop(L, 1);

    // Get image indices
    lua_getfield(L, 2, "image_indices");
    luaL_checktype(L, -1, LUA_TTABLE);
    if (lua_rawlen(L, -1) != present_info.swapchainCount) {
        free(wait_semaphores);
        free(swapchains);
        luaL_error(L, "Mismatch between swapchains and image indices count");
    }
    uint32_t* image_indices = NULL;
    if (present_info.swapchainCount > 0) {
        image_indices = (uint32_t*)malloc(present_info.swapchainCount * sizeof(uint32_t));
        if (!image_indices) {
            free(wait_semaphores);
            free(swapchains);
            luaL_error(L, "Failed to allocate memory for image indices");
        }
        for (uint32_t i = 1; i <= present_info.swapchainCount; i++) {
            lua_rawgeti(L, -1, i);
            image_indices[i-1] = luaL_checkinteger(L, -1);
            lua_pop(L, 1);
        }
        present_info.pImageIndices = image_indices;
    }
    lua_pop(L, 1);

    VkResult result = vkQueuePresentKHR(queue_ud->queue, &present_info);
    free(wait_semaphores);
    free(swapchains);
    free(image_indices);

    if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        lua_pushboolean(L, false);
        lua_pushfstring(L, "Failed to present queue: VkResult %d", result);
        return 2;
    }

    lua_pushboolean(L, true);
    return 1;
}

static int l_vulkan_destroy_framebuffer(lua_State* L) {
    lua_VkDevice* device_ud = lua_check_VkDevice(L, 1);
    lua_VkFramebuffer* framebuffer_ud = lua_check_VkFramebuffer(L, 2);
    if (framebuffer_ud->framebuffer != VK_NULL_HANDLE) {
        vkDestroyFramebuffer(device_ud->device, framebuffer_ud->framebuffer, NULL);
        framebuffer_ud->framebuffer = VK_NULL_HANDLE;
    }
    return 0;
}

static int l_vulkan_destroy_image_view(lua_State* L) {
    lua_VkDevice* device_ud = lua_check_VkDevice(L, 1);
    lua_VkImageView* image_view_ud = lua_check_VkImageView(L, 2);
    if (image_view_ud->image_view != VK_NULL_HANDLE) {
        vkDestroyImageView(device_ud->device, image_view_ud->image_view, NULL);
        image_view_ud->image_view = VK_NULL_HANDLE;
    }
    return 0;
}

static int l_vulkan_device_wait_idle(lua_State* L) {
    lua_VkDevice* device_ud = lua_check_VkDevice(L, 1);
    VkResult result = vkDeviceWaitIdle(device_ud->device);
    if (result != VK_SUCCESS) {
        lua_pushboolean(L, false);
        lua_pushfstring(L, "Failed to wait for device idle: VkResult %d", result);
        return 2;
    }
    lua_pushboolean(L, true);
    return 1;
}

static int l_vulkan_cmd_set_viewport(lua_State* L) {
    lua_VkCommandBuffer* cmd_buffer_ud = lua_check_VkCommandBuffer(L, 1);
    luaL_checktype(L, 2, LUA_TTABLE);

    uint32_t viewport_count = lua_rawlen(L, 2);
    VkViewport* viewports = (VkViewport*)malloc(viewport_count * sizeof(VkViewport));
    for (uint32_t i = 0; i < viewport_count; i++) {
        lua_rawgeti(L, 2, i + 1);
        lua_getfield(L, -1, "x");
        viewports[i].x = (float)luaL_checknumber(L, -1);
        lua_pop(L, 1);
        lua_getfield(L, -1, "y");
        viewports[i].y = (float)luaL_checknumber(L, -1);
        lua_pop(L, 1);
        lua_getfield(L, -1, "width");
        viewports[i].width = (float)luaL_checknumber(L, -1);
        lua_pop(L, 1);
        lua_getfield(L, -1, "height");
        viewports[i].height = (float)luaL_checknumber(L, -1);
        lua_pop(L, 1);
        lua_getfield(L, -1, "min_depth");
        viewports[i].minDepth = (float)luaL_checknumber(L, -1);
        lua_pop(L, 1);
        lua_getfield(L, -1, "max_depth");
        viewports[i].maxDepth = (float)luaL_checknumber(L, -1);
        lua_pop(L, 1);
        lua_pop(L, 1);
    }
    vkCmdSetViewport(cmd_buffer_ud->command_buffer, 0, viewport_count, viewports);
    free(viewports);
    return 0;
}

static int l_vulkan_cmd_set_scissor(lua_State* L) {
    lua_VkCommandBuffer* cmd_buffer_ud = lua_check_VkCommandBuffer(L, 1);
    luaL_checktype(L, 2, LUA_TTABLE);

    uint32_t scissor_count = lua_rawlen(L, 2);
    VkRect2D* scissors = (VkRect2D*)malloc(scissor_count * sizeof(VkRect2D));
    for (uint32_t i = 0; i < scissor_count; i++) {
        lua_rawgeti(L, 2, i + 1);
        lua_getfield(L, -1, "x");
        scissors[i].offset.x = (int32_t)luaL_checkinteger(L, -1);
        lua_pop(L, 1);
        lua_getfield(L, -1, "y");
        scissors[i].offset.y = (int32_t)luaL_checkinteger(L, -1);
        lua_pop(L, 1);
        lua_getfield(L, -1, "width");
        scissors[i].extent.width = (uint32_t)luaL_checkinteger(L, -1);
        lua_pop(L, 1);
        lua_getfield(L, -1, "height");
        scissors[i].extent.height = (uint32_t)luaL_checkinteger(L, -1);
        lua_pop(L, 1);
        lua_pop(L, 1);
    }
    vkCmdSetScissor(cmd_buffer_ud->command_buffer, 0, scissor_count, scissors);
    free(scissors);
    return 0;
}

static int l_vulkan_get_physical_device_surface_support_KHR(lua_State* L) {
    lua_VkPhysicalDevice* phys_device_ud = lua_check_VkPhysicalDevice(L, 1);
    uint32_t queue_family_index = (uint32_t)luaL_checkinteger(L, 2);
    lua_VkSurfaceKHR* surface_ud = lua_check_VkSurfaceKHR(L, 3);
    VkBool32 supported;
    vkGetPhysicalDeviceSurfaceSupportKHR(phys_device_ud->physical_device, queue_family_index, surface_ud->surface, &supported);
    lua_pushboolean(L, supported);
    return 1;
}

static int l_vulkan_destroy_swapchain_khr(lua_State* L) {
    lua_VkDevice* device_ud = lua_check_VkDevice(L, 1);
    lua_VkSwapchainKHR* swapchain_ud = lua_check_VkSwapchainKHR(L, 2);
    vkDestroySwapchainKHR(device_ud->device, swapchain_ud->swapchain, NULL);
    swapchain_ud->swapchain = VK_NULL_HANDLE;
    return 0;
}


// Destroy semaphore
static int l_vulkan_destroy_semaphore(lua_State* L) {
    lua_VkDevice* device_ud = lua_check_VkDevice(L, 1);
    lua_VkSemaphore* semaphore_ud = lua_check_VkSemaphore(L, 2);
    if (semaphore_ud->semaphore != VK_NULL_HANDLE) {
        vkDestroySemaphore(device_ud->device, semaphore_ud->semaphore, NULL);
        semaphore_ud->semaphore = VK_NULL_HANDLE; // Prevent double destruction
    }
    return 0;
}

// Destroy fence
static int l_vulkan_destroy_fence(lua_State* L) {
    lua_VkDevice* device_ud = lua_check_VkDevice(L, 1);
    lua_VkFence* fence_ud = lua_check_VkFence(L, 2);
    if (fence_ud->fence != VK_NULL_HANDLE) {
        vkDestroyFence(device_ud->device, fence_ud->fence, NULL);
        fence_ud->fence = VK_NULL_HANDLE; // Prevent double destruction
    }
    return 0;
}

// Destroy command pool
static int l_vulkan_destroy_command_pool(lua_State* L) {
    lua_VkDevice* device_ud = lua_check_VkDevice(L, 1);
    lua_VkCommandPool* command_pool_ud = lua_check_VkCommandPool(L, 2);
    if (command_pool_ud->command_pool != VK_NULL_HANDLE) {
        vkDestroyCommandPool(device_ud->device, command_pool_ud->command_pool, NULL);
        command_pool_ud->command_pool = VK_NULL_HANDLE; // Prevent double destruction
    }
    return 0;
}

// Destroy pipeline
static int l_vulkan_destroy_pipeline(lua_State* L) {
    lua_VkDevice* device_ud = lua_check_VkDevice(L, 1);
    lua_VkPipeline* pipeline_ud = lua_check_VkPipeline(L, 2);
    if (pipeline_ud->pipeline != VK_NULL_HANDLE) {
        vkDestroyPipeline(device_ud->device, pipeline_ud->pipeline, NULL);
        pipeline_ud->pipeline = VK_NULL_HANDLE; // Prevent double destruction
    }
    return 0;
}

// Destroy pipeline layout
static int l_vulkan_destroy_pipeline_layout(lua_State* L) {
    lua_VkDevice* device_ud = lua_check_VkDevice(L, 1);
    lua_VkPipelineLayout* pipeline_layout_ud = lua_check_VkPipelineLayout(L, 2);
    if (pipeline_layout_ud->pipeline_layout != VK_NULL_HANDLE) {
        vkDestroyPipelineLayout(device_ud->device, pipeline_layout_ud->pipeline_layout, NULL);
        pipeline_layout_ud->pipeline_layout = VK_NULL_HANDLE; // Prevent double destruction
    }
    return 0;
}

// Destroy shader module
static int l_vulkan_destroy_shader_module(lua_State* L) {
    lua_VkDevice* device_ud = lua_check_VkDevice(L, 1);
    lua_VkShaderModule* shader_module_ud = lua_check_VkShaderModule(L, 2);
    if (shader_module_ud->shader_module != VK_NULL_HANDLE) {
        vkDestroyShaderModule(device_ud->device, shader_module_ud->shader_module, NULL);
        shader_module_ud->shader_module = VK_NULL_HANDLE; // Prevent double destruction
    }
    return 0;
}

// Destroy render pass
static int l_vulkan_destroy_render_pass(lua_State* L) {
    lua_VkDevice* device_ud = lua_check_VkDevice(L, 1);
    lua_VkRenderPass* render_pass_ud = lua_check_VkRenderPass(L, 2);
    if (render_pass_ud->render_pass != VK_NULL_HANDLE) {
        vkDestroyRenderPass(device_ud->device, render_pass_ud->render_pass, NULL);
        render_pass_ud->render_pass = VK_NULL_HANDLE; // Prevent double destruction
    }
    return 0;
}

// Destroy device
static int l_vulkan_destroy_device(lua_State* L) {
    lua_VkDevice* device_ud = lua_check_VkDevice(L, 1);
    if (device_ud->device != VK_NULL_HANDLE) {
        vkDestroyDevice(device_ud->device, NULL);
        device_ud->device = VK_NULL_HANDLE; // Prevent double destruction
    }
    return 0;
}

// Destroy surface (VK_KHR_surface)
static int l_vulkan_destroy_surface_KHR(lua_State* L) {
    lua_VkInstance* instance_ud = lua_check_VkInstance(L, 1);
    lua_VkSurfaceKHR* surface_ud = lua_check_VkSurfaceKHR(L, 2);
    if (surface_ud->surface != VK_NULL_HANDLE) {
        vkDestroySurfaceKHR(instance_ud->instance, surface_ud->surface, NULL);
        surface_ud->surface = VK_NULL_HANDLE; // Prevent double destruction
    }
    return 0;
}

// Destroy instance
static int l_vulkan_destroy_instance(lua_State* L) {
    lua_VkInstance* instance_ud = lua_check_VkInstance(L, 1);
    if (instance_ud->instance != VK_NULL_HANDLE) {
        vkDestroyInstance(instance_ud->instance, NULL);
        instance_ud->instance = VK_NULL_HANDLE; // Prevent double destruction
    }
    return 0;
}


// Create VkShaderModule from GLSL source string: vulkan.create_shader_module_str(device, source, kind)
static int l_vulkan_create_shader_module_str(lua_State* L) {
    lua_VkDevice* device_ud = lua_check_VkDevice(L, 1);
    const char* source = luaL_checkstring(L, 2); // GLSL source code
    int shader_kind = luaL_checkinteger(L, 3);   // Shader kind (e.g., shaderc_glsl_vertex_shader)

    if (!device_ud->device) {
        luaL_error(L, "Invalid Vulkan device (already destroyed)");
    }

    // Initialize shaderc compiler
    shaderc_compiler_t compiler = shaderc_compiler_initialize();
    if (!compiler) {
        luaL_error(L, "Failed to initialize shaderc compiler");
    }

    // Create compilation options
    shaderc_compile_options_t options = shaderc_compile_options_initialize();
    if (!options) {
        shaderc_compiler_release(compiler);
        luaL_error(L, "Failed to initialize shaderc compilation options");
    }

    // Set source language to GLSL
    shaderc_compile_options_set_source_language(options, shaderc_source_language_glsl);

    // Compile GLSL to SPIR-V
    shaderc_compilation_result_t result = shaderc_compile_into_spv(
        compiler,
        source,
        strlen(source),
        (shaderc_shader_kind)shader_kind,
        "shader", // Input file name (arbitrary for string input)
        "main",  // Entry point name
        options
    );

    // Check compilation status
    if (shaderc_result_get_compilation_status(result) != shaderc_compilation_status_success) {
        const char* error_msg = shaderc_result_get_error_message(result);
        shaderc_result_release(result);
        shaderc_compile_options_release(options);
        shaderc_compiler_release(compiler);
        luaL_error(L, "Shader compilation failed: %s", error_msg ? error_msg : "Unknown error");
    }

    // Get SPIR-V binary
    size_t spirv_size = shaderc_result_get_length(result);
    const char* spirv_data = shaderc_result_get_bytes(result);

    // Create VkShaderModule
    VkShaderModuleCreateInfo create_info = {
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .codeSize = spirv_size,
        .pCode = (const uint32_t*)spirv_data
    };

    VkShaderModule shader_module;
    VkResult res = vkCreateShaderModule(device_ud->device, &create_info, NULL, &shader_module);
    if (res != VK_SUCCESS) {
        shaderc_result_release(result);
        shaderc_compile_options_release(options);
        shaderc_compiler_release(compiler);
        luaL_error(L, "Failed to create shader module: VkResult %d", res);
    }

    // Clean up shaderc resources
    shaderc_result_release(result);
    shaderc_compile_options_release(options);
    shaderc_compiler_release(compiler);

    // Push shader module as userdata
    lua_push_VkShaderModule(L, shader_module, device_ud->device);
    return 1;
}


//===============================================
// Module loader
//===============================================
static const struct luaL_Reg vulkan_lib[] = {
    {"create_vk_application_info", l_vulkan_create_vk_application_info},
    {"make_version", l_vulkan_make_version},
    {"sdl_vulkan_get_instance_extensions", l_vulkan_sdl_vulkan_get_instance_extensions},
    {"create_info", l_vulkan_create_info},
    {"create_instance", l_vulkan_create_instance},

    {"sdl_vulkan_create_surface", l_vulkan_sdl_vulkan_create_surface},
    {"create_physical_devices", l_vulkan_create_physical_devices},

    {"get_physical_devices_properties", l_vulkan_get_physical_devices_properties},
    {"create_device_info", l_vulkan_create_device_info},
    {"create_device", l_vulkan_create_device},

    {"get_device_queue", l_vulkan_get_device_queue},
    {"get_physical_device_surface_capabilities_KHR", l_vulkan_get_physical_device_surface_capabilities_KHR},
    {"create_swap_chain_KHR", l_vulkan_create_swap_chain_KHR},

    {"get_swapchain_images_KHR", l_vulkan_get_swapchain_images_KHR},
    {"create_image_view", l_vulkan_create_image_view},

    {"create_render_pass", l_vulkan_create_render_pass},

    {"create_framebuffer", l_vulkan_create_framebuffer},

    {"create_shader_module", l_vulkan_create_shader_module},        // shader file spv
    {"create_shader_module_str", l_vulkan_create_shader_module_str}, // string

    {"create_pipeline_layout", l_vulkan_create_pipeline_layout},
    {"create_graphics_pipelines", l_vulkan_create_graphics_pipelines},

    {"create_semaphore", l_vulkan_create_semaphore},
    {"create_fence", l_vulkan_create_fence},
    {"create_command_pool", l_vulkan_create_command_pool},
    {"create_allocate_command_buffers", l_vulkan_create_allocate_command_buffers},

    {"wait_for_fences", l_vulkan_wait_for_fences},
    {"reset_fences", l_vulkan_reset_fences},
    {"acquire_next_image_KHR", l_vulkan_acquire_next_image_KHR},
    {"reset_command_buffer", l_vulkan_reset_command_buffer},
    {"begin_command_buffer", l_vulkan_begin_command_buffer},
    {"cmd_begin_renderpass", l_vulkan_cmd_begin_renderpass},
    {"cmd_bind_pipeline", l_vulkan_cmd_bind_pipeline},
    {"cmd_draw", l_vulkan_cmd_draw},
    {"cmd_end_renderpass", l_vulkan_cmd_end_renderpass},
    {"end_commandbuffer", l_vulkan_end_commandbuffer},
    {"queue_submit", l_vulkan_queue_submit},
    {"queue_present_KHR", l_vulkan_queue_present_KHR},

    {"device_wait_idle", l_vulkan_device_wait_idle},
    {"destroy_framebuffer", l_vulkan_destroy_framebuffer},
    {"destroy_image_view", l_vulkan_destroy_image_view},

    {"destroy_swapchain_khr", l_vulkan_destroy_swapchain_khr},

    {"get_physical_device_surface_support_KHR", l_vulkan_get_physical_device_surface_support_KHR},
    {"cmd_set_viewport", l_vulkan_cmd_set_viewport},
    {"cmd_set_scissor", l_vulkan_cmd_set_scissor},

    {"destroy_semaphore", l_vulkan_destroy_semaphore},
    {"destroy_fence", l_vulkan_destroy_fence},
    {"destroy_command_pool", l_vulkan_destroy_command_pool},
    {"destroy_pipeline", l_vulkan_destroy_pipeline},
    {"destroy_pipeline_layout", l_vulkan_destroy_pipeline_layout},
    {"destroy_shader_module", l_vulkan_destroy_shader_module},
    {"destroy_render_pass", l_vulkan_destroy_render_pass},
    {"destroy_device", l_vulkan_destroy_device},
    {"destroy_surface_KHR", l_vulkan_destroy_surface_KHR},
    {"destroy_instance", l_vulkan_destroy_instance},

    {NULL, NULL}
};

int luaopen_vulkan(lua_State* L) {
    app_info_metatable(L);
    instance_metatable(L);
    create_info_metatable(L);

    surface_metatable(L);
    physical_device_metatable(L);

    device_metatable(L);
    device_create_info_metatable(L);

    queue_metatable(L);
    swapchain_metatable(L);

    image_view_metatable(L);

    render_pass_metatable(L);

    framebuffer_metatable(L);

    shader_module_metatable(L);
    pipeline_layout_metatable(L);
    pipeline_metatable(L);

    semaphore_metatable(L);
    fence_metatable(L);
    command_pool_metatable(L);
    command_buffer_metatable(L);

    luaL_newlib(L, vulkan_lib);

    // Vulkan constants
    lua_pushinteger(L, VK_API_VERSION_1_0);
    lua_setfield(L, -2, "VK_API_VERSION_1_0");
    lua_pushinteger(L, VK_API_VERSION_1_1);
    lua_setfield(L, -2, "VK_API_VERSION_1_1");
    lua_pushinteger(L, VK_API_VERSION_1_2);
    lua_setfield(L, -2, "VK_API_VERSION_1_2");
    lua_pushinteger(L, VK_API_VERSION_1_3);
    lua_setfield(L, -2, "VK_API_VERSION_1_3");

    // Device type constants
    lua_pushinteger(L, VK_PHYSICAL_DEVICE_TYPE_OTHER);
    lua_setfield(L, -2, "DEVICE_TYPE_OTHER");
    lua_pushinteger(L, VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU);
    lua_setfield(L, -2, "DEVICE_TYPE_INTEGRATED_GPU");
    lua_pushinteger(L, VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU);
    lua_setfield(L, -2, "DEVICE_TYPE_DISCRETE_GPU");
    lua_pushinteger(L, VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU);
    lua_setfield(L, -2, "DEVICE_TYPE_VIRTUAL_GPU");
    lua_pushinteger(L, VK_PHYSICAL_DEVICE_TYPE_CPU);
    lua_setfield(L, -2, "DEVICE_TYPE_CPU");

    // Swapchain-related constants
    lua_pushinteger(L, VK_FORMAT_B8G8R8A8_SRGB);
    lua_setfield(L, -2, "FORMAT_B8G8R8A8_SRGB");
    lua_pushinteger(L, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR);
    lua_setfield(L, -2, "COLOR_SPACE_SRGB_NONLINEAR_KHR");
    lua_pushinteger(L, VK_PRESENT_MODE_FIFO_KHR);
    lua_setfield(L, -2, "PRESENT_MODE_FIFO_KHR");
    lua_pushinteger(L, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
    lua_setfield(L, -2, "IMAGE_USAGE_COLOR_ATTACHMENT");
    lua_pushinteger(L, VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR);
    lua_setfield(L, -2, "SURFACE_TRANSFORM_IDENTITY");
    lua_pushinteger(L, VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR);
    lua_setfield(L, -2, "COMPOSITE_ALPHA_OPAQUE");
    lua_pushinteger(L, VK_SHARING_MODE_EXCLUSIVE);
    lua_setfield(L, -2, "SHARING_MODE_EXCLUSIVE");

    // Image view constants
    lua_pushinteger(L, VK_IMAGE_VIEW_TYPE_2D);
    lua_setfield(L, -2, "IMAGE_VIEW_TYPE_2D");
    lua_pushinteger(L, VK_COMPONENT_SWIZZLE_IDENTITY);
    lua_setfield(L, -2, "COMPONENT_SWIZZLE_IDENTITY");
    lua_pushinteger(L, VK_IMAGE_ASPECT_COLOR_BIT);
    lua_setfield(L, -2, "IMAGE_ASPECT_COLOR_BIT");

    // Render pass constants
    lua_pushinteger(L, VK_ATTACHMENT_LOAD_OP_CLEAR);
    lua_setfield(L, -2, "ATTACHMENT_LOAD_OP_CLEAR");
    lua_pushinteger(L, VK_ATTACHMENT_LOAD_OP_DONT_CARE);
    lua_setfield(L, -2, "ATTACHMENT_LOAD_OP_DONT_CARE");
    lua_pushinteger(L, VK_ATTACHMENT_STORE_OP_STORE);
    lua_setfield(L, -2, "ATTACHMENT_STORE_OP_STORE");
    lua_pushinteger(L, VK_ATTACHMENT_STORE_OP_DONT_CARE);
    lua_setfield(L, -2, "ATTACHMENT_STORE_OP_DONT_CARE");
    lua_pushinteger(L, VK_IMAGE_LAYOUT_UNDEFINED);
    lua_setfield(L, -2, "IMAGE_LAYOUT_UNDEFINED");
    lua_pushinteger(L, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    lua_setfield(L, -2, "IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL");
    lua_pushinteger(L, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
    lua_setfield(L, -2, "IMAGE_LAYOUT_PRESENT_SRC_KHR");
    lua_pushinteger(L, VK_SAMPLE_COUNT_1_BIT);
    lua_setfield(L, -2, "SAMPLE_COUNT_1_BIT");
    lua_pushinteger(L, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
    lua_setfield(L, -2, "PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT");
    lua_pushinteger(L, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT);
    lua_setfield(L, -2, "ACCESS_COLOR_ATTACHMENT_WRITE");
    lua_pushinteger(L, VK_SUBPASS_EXTERNAL);
    lua_setfield(L, -2, "SUBPASS_EXTERNAL");

    // Pipeline constants
    lua_pushinteger(L, VK_SHADER_STAGE_VERTEX_BIT);
    lua_setfield(L, -2, "SHADER_STAGE_VERTEX");
    lua_pushinteger(L, VK_SHADER_STAGE_FRAGMENT_BIT);
    lua_setfield(L, -2, "SHADER_STAGE_FRAGMENT");

    // Additional constants
    lua_pushinteger(L, VK_PIPELINE_BIND_POINT_GRAPHICS);
    lua_setfield(L, -2, "PIPELINE_BIND_POINT_GRAPHICS");
    lua_pushnumber(L, UINT64_MAX);
    lua_setfield(L, -2, "UINT64_MAX");

    // shaders
    lua_pushinteger(L, shaderc_glsl_vertex_shader);
    lua_setfield(L, -2, "shaderc_vertex_shader");
    lua_pushinteger(L, shaderc_glsl_fragment_shader);
    lua_setfield(L, -2, "shaderc_fragment_shader");

    return 1;
}
//===============================================
// 
//===============================================
