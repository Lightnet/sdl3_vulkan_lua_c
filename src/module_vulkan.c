// module_vulkan.c
#include "module_vulkan.h"
#include <stdlib.h>
#include <string.h>
#include <shaderc/shaderc.h>

// Metatable names
static const char* APPINFO_MT = "vulkan.application_info";
static const char* CREATEINFO_MT = "vulkan.instance_create_info";
static const char* INSTANCE_MT = "vulkan.instance";
static const char* SURFACE_MT = "vulkan.surface";
static const char* PHYSICAL_DEVICE_MT = "vulkan.physical_device";

static const char* DEVICE_QUEUE_CREATE_INFO_MT = "vulkan.device_queue_create_info";
static const char* DEVICE_CREATE_INFO_MT = "vulkan.device_create_info";
static const char* DEVICE_MT = "vulkan.device";
static const char* QUEUE_MT = "vulkan.queue";

static const char* SURFACE_FORMAT_MT = "vulkan.surface_format";

static const char* SWAPCHAIN_CREATE_INFO_MT = "vulkan.swapchain_create_info";
static const char* SWAPCHAIN_MT = "vulkan.swapchain";

static const char* IMAGE_VIEW_CREATE_INFO_MT = "vulkan.image_view_create_info";
static const char* IMAGE_VIEW_MT = "vulkan.image_view";
static const char* ATTACHMENT_DESCRIPTION_MT = "vulkan.attachment_description";
static const char* SUBPASS_DESCRIPTION_MT = "vulkan.subpass_description";
static const char* SUBPASS_DEPENDENCY_MT = "vulkan.subpass_dependency";
static const char* RENDER_PASS_CREATE_INFO_MT = "vulkan.render_pass_create_info";
static const char* RENDER_PASS_MT = "vulkan.render_pass";

static const char* FRAMEBUFFER_CREATE_INFO_MT = "vulkan.framebuffer_create_info";
static const char* FRAMEBUFFER_MT = "vulkan.framebuffer";

static const char* EXTENT_2D_MT = "vulkan.extent_2d";
static const char* SHADER_MODULE_MT = "vulkan.shader_module";
static const char* PIPELINE_SHADER_STAGE_CREATE_INFO_MT = "vulkan.pipeline_shader_stage_create_info";
static const char* VERTEX_INPUT_BINDING_DESCRIPTION_MT = "vulkan.vertex_input_binding_description";
static const char* VERTEX_INPUT_ATTRIBUTE_DESCRIPTION_MT = "vulkan.vertex_input_attribute_description";
static const char* PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO_MT = "vulkan.pipeline_vertex_input_state_create_info";
static const char* PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO_MT = "vulkan.pipeline_input_assembly_state_create_info";
static const char* VIEWPORT_MT = "vulkan.viewport";
static const char* RECT_2D_MT = "vulkan.rect_2d";
static const char* PIPELINE_VIEWPORT_STATE_CREATE_INFO_MT = "vulkan.pipeline_viewport_state_create_info";
static const char* PIPELINE_RASTERIZATION_STATE_CREATE_INFO_MT = "vulkan.pipeline_rasterization_state_create_info";
static const char* PIPELINE_MULTISAMPLE_STATE_CREATE_INFO_MT = "vulkan.pipeline_multisample_state_create_info";
static const char* PIPELINE_COLOR_BLEND_ATTACHMENT_STATE_MT = "vulkan.pipeline_color_blend_attachment_state";
static const char* PIPELINE_COLOR_BLEND_STATE_CREATE_INFO_MT = "vulkan.pipeline_color_blend_state_create_info";
static const char* PIPELINE_LAYOUT_CREATE_INFO_MT = "vulkan.pipeline_layout_create_info";
static const char* PIPELINE_LAYOUT_MT = "vulkan.pipeline_layout";
static const char* GRAPHICS_PIPELINE_CREATE_INFO_MT = "vulkan.graphics_pipeline_create_info";
static const char* PIPELINE_MT = "vulkan.pipeline";

// New metatable names
static const char* BUFFER_CREATE_INFO_MT = "vulkan.buffer_create_info";
static const char* BUFFER_MT = "vulkan.buffer";
static const char* MEMORY_ALLOCATE_INFO_MT = "vulkan.memory_allocate_info";
static const char* DEVICE_MEMORY_MT = "vulkan.device_memory";
static const char* COMMAND_POOL_CREATE_INFO_MT = "vulkan.command_pool_create_info";
static const char* COMMAND_POOL_MT = "vulkan.command_pool";
static const char* COMMAND_BUFFER_ALLOCATE_INFO_MT = "vulkan.command_buffer_allocate_info";
static const char* COMMAND_BUFFER_MT = "vulkan.command_buffer";

static const char* MEMORY_REQUIREMENTS_MT = "vulkan.memory_requirements";
static const char* PHYSICAL_DEVICE_MEMORY_PROPERTIES_MT = "vulkan.physical_device_memory_properties";


// Garbage collection for VkExtent2D (no dynamic allocations)
static int extent_2d_gc(lua_State* L) {
    printf("Cleaning up VkExtent2D\n");
    return 0;
}

// Garbage collection metamethod for VkApplicationInfo
static int appinfo_gc(lua_State* L) {
    lua_VkApplicationInfo* ud = (lua_VkApplicationInfo*)luaL_checkudata(L, 1, APPINFO_MT);
    if (ud->pApplicationName) {
        printf("Cleaning up VkApplicationInfo (pApplicationName: %s)\n", ud->pApplicationName);
        free(ud->pApplicationName);
        ud->pApplicationName = NULL;
    }
    return 0;
}

// Garbage collection metamethod for VkInstanceCreateInfo
static int createinfo_gc(lua_State* L) {
    lua_VkInstanceCreateInfo* ud = (lua_VkInstanceCreateInfo*)luaL_checkudata(L, 1, CREATEINFO_MT);
    if (ud->ppEnabledExtensionNames) {
        printf("Cleaning up VkInstanceCreateInfo (%u extensions)\n", ud->enabledExtensionCount);
        for (uint32_t i = 0; i < ud->enabledExtensionCount; i++) {
            if (ud->ppEnabledExtensionNames[i]) {
                free(ud->ppEnabledExtensionNames[i]);
            }
        }
        free(ud->ppEnabledExtensionNames);
        ud->ppEnabledExtensionNames = NULL;
        ud->enabledExtensionCount = 0;
    }
    return 0;
}

// Garbage collection metamethod for VkInstance
static int instance_gc(lua_State* L) {
    lua_VkInstance* ud = (lua_VkInstance*)luaL_checkudata(L, 1, INSTANCE_MT);
    if (ud->instance) {
        printf("Cleaning up VkInstance\n");
        vkDestroyInstance(ud->instance, NULL);
        ud->instance = NULL;
    }
    return 0;
}

// Garbage collection metamethod for VkSurfaceKHR
static int surface_gc(lua_State* L) {
    lua_VkSurfaceKHR* ud = (lua_VkSurfaceKHR*)luaL_checkudata(L, 1, SURFACE_MT);
    if (ud->surface && ud->instance) {
        printf("Cleaning up VkSurfaceKHR\n");
        vkDestroySurfaceKHR(ud->instance, ud->surface, NULL);
        ud->surface = NULL;
        ud->instance = NULL;
    }
    return 0;
}

// Garbage collection for VkDeviceQueueCreateInfo
static int device_queue_create_info_gc(lua_State* L) {
    lua_VkDeviceQueueCreateInfo* ud = (lua_VkDeviceQueueCreateInfo*)luaL_checkudata(L, 1, DEVICE_QUEUE_CREATE_INFO_MT);
    if (ud->pQueuePriorities) {
        printf("Cleaning up VkDeviceQueueCreateInfo\n");
        free(ud->pQueuePriorities);
        ud->pQueuePriorities = NULL;
    }
    return 0;
}

// Garbage collection for VkDeviceCreateInfo
static int device_create_info_gc(lua_State* L) {
    lua_VkDeviceCreateInfo* ud = (lua_VkDeviceCreateInfo*)luaL_checkudata(L, 1, DEVICE_CREATE_INFO_MT);
    if (ud->ppEnabledExtensionNames) {
        printf("Cleaning up VkDeviceCreateInfo (%u extensions)\n", ud->enabledExtensionCount);
        for (uint32_t i = 0; i < ud->enabledExtensionCount; i++) {
            if (ud->ppEnabledExtensionNames[i]) {
                free(ud->ppEnabledExtensionNames[i]);
            }
        }
        free(ud->ppEnabledExtensionNames);
        ud->ppEnabledExtensionNames = NULL;
        ud->enabledExtensionCount = 0;
    }
    if (ud->device_create_info.pQueueCreateInfos) {
        free((void*)ud->device_create_info.pQueueCreateInfos);
        ud->device_create_info.pQueueCreateInfos = NULL;
    }
    if (ud->device_create_info.pEnabledFeatures) {
        free((void*)ud->device_create_info.pEnabledFeatures);
        ud->device_create_info.pEnabledFeatures = NULL;
    }
    return 0;
}

// Garbage collection for VkDevice
static int device_gc(lua_State* L) {
    lua_VkDevice* ud = (lua_VkDevice*)luaL_checkudata(L, 1, DEVICE_MT);
    if (ud->device) {
        printf("Cleaning up VkDevice\n");
        vkDestroyDevice(ud->device, NULL);
        ud->device = NULL;
    }
    return 0;
}

// Push VkApplicationInfo as userdata
void lua_push_VkApplicationInfo(lua_State* L, VkApplicationInfo* app_info, const char* app_name) {
    if (!app_info) {
        luaL_error(L, "Cannot create userdata for null VkApplicationInfo");
    }
    lua_VkApplicationInfo* ud = (lua_VkApplicationInfo*)lua_newuserdata(L, sizeof(lua_VkApplicationInfo));
    ud->app_info = *app_info;
    ud->pApplicationName = app_name ? strdup(app_name) : NULL;
    luaL_setmetatable(L, APPINFO_MT);
}

// Check VkApplicationInfo userdata
lua_VkApplicationInfo* lua_check_VkApplicationInfo(lua_State* L, int idx) {
    lua_VkApplicationInfo* ud = (lua_VkApplicationInfo*)luaL_checkudata(L, idx, APPINFO_MT);
    return ud;
}

// Push VkInstanceCreateInfo as userdata
void lua_push_VkInstanceCreateInfo(lua_State* L, VkInstanceCreateInfo* create_info, char** extensions, uint32_t extension_count) {
    if (!create_info) {
        luaL_error(L, "Cannot create userdata for null VkInstanceCreateInfo");
    }
    lua_VkInstanceCreateInfo* ud = (lua_VkInstanceCreateInfo*)lua_newuserdata(L, sizeof(lua_VkInstanceCreateInfo));
    ud->create_info = *create_info;
    ud->ppEnabledExtensionNames = extensions;
    ud->enabledExtensionCount = extension_count;
    luaL_setmetatable(L, CREATEINFO_MT);
}

// Check VkInstanceCreateInfo userdata
lua_VkInstanceCreateInfo* lua_check_VkInstanceCreateInfo(lua_State* L, int idx) {
    lua_VkInstanceCreateInfo* ud = (lua_VkInstanceCreateInfo*)luaL_checkudata(L, idx, CREATEINFO_MT);
    return ud;
}

// Push VkInstance as userdata
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

// Push VkSurfaceKHR as userdata
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
    return ud;
}

// Push VkPhysicalDevice as userdata
void lua_push_VkPhysicalDevice(lua_State* L, VkPhysicalDevice device) {
    if (!device) {
        luaL_error(L, "Cannot create userdata for null VkPhysicalDevice");
    }
    lua_VkPhysicalDevice* ud = (lua_VkPhysicalDevice*)lua_newuserdata(L, sizeof(lua_VkPhysicalDevice));
    ud->device = device;
    luaL_setmetatable(L, PHYSICAL_DEVICE_MT);
}

// Check VkPhysicalDevice userdata
lua_VkPhysicalDevice* lua_check_VkPhysicalDevice(lua_State* L, int idx) {
    lua_VkPhysicalDevice* ud = (lua_VkPhysicalDevice*)luaL_checkudata(L, idx, PHYSICAL_DEVICE_MT);
    if (!ud->device) {
        luaL_error(L, "Invalid VkPhysicalDevice");
    }
    return ud;
}

// Get physical device count: vulkan.get_device_count(instance)
static int l_vulkan_get_device_count(lua_State* L) {
    lua_VkInstance* instance_ud = lua_check_VkInstance(L, 1);
    uint32_t device_count = 0;
    VkResult result = vkEnumeratePhysicalDevices(instance_ud->instance, &device_count, NULL);
    if (result != VK_SUCCESS) {
        luaL_error(L, "Failed to get physical device count: VkResult %d", result);
    }
    lua_pushinteger(L, device_count);
    return 1;
}

// Get physical devices: vulkan.get_devices(instance, device_count)
static int l_vulkan_get_devices(lua_State* L) {
    lua_VkInstance* instance_ud = lua_check_VkInstance(L, 1);
    uint32_t device_count = (uint32_t)luaL_checkinteger(L, 2);

    VkPhysicalDevice* devices = (VkPhysicalDevice*)malloc(device_count * sizeof(VkPhysicalDevice));
    if (!devices) {
        luaL_error(L, "Failed to allocate memory for physical devices");
    }

    VkResult result = vkEnumeratePhysicalDevices(instance_ud->instance, &device_count, devices);
    if (result != VK_SUCCESS) {
        free(devices);
        luaL_error(L, "Failed to enumerate physical devices: VkResult %d", result);
    }

    lua_newtable(L);
    for (uint32_t i = 0; i < device_count; i++) {
        lua_push_VkPhysicalDevice(L, devices[i]);
        lua_rawseti(L, -2, i + 1); // Lua tables are 1-based
    }

    free(devices);
    return 1;
}

// Get physical device properties: vulkan.get_physical_device(device)
static int l_vulkan_get_physical_device(lua_State* L) {
    lua_VkPhysicalDevice* device_ud = lua_check_VkPhysicalDevice(L, 1);
    VkPhysicalDeviceProperties props;
    vkGetPhysicalDeviceProperties(device_ud->device, &props);

    lua_newtable(L);
    lua_pushstring(L, props.deviceName);
    lua_setfield(L, -2, "deviceName");
    lua_pushinteger(L, props.deviceType);
    lua_setfield(L, -2, "deviceType");

    // Optionally add more properties for detailed selection
    lua_pushinteger(L, props.apiVersion);
    lua_setfield(L, -2, "apiVersion");
    lua_pushinteger(L, props.driverVersion);
    lua_setfield(L, -2, "driverVersion");

    return 1;
}

// Get queue family count: vulkan.get_family_count(physical_device)
static int l_vulkan_get_family_count(lua_State* L) {
    lua_VkPhysicalDevice* device_ud = lua_check_VkPhysicalDevice(L, 1);
    uint32_t queue_family_count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device_ud->device, &queue_family_count, NULL);
    lua_pushinteger(L, queue_family_count);
    return 1;
}

// Get queue family properties: vulkan.queue_families(physical_device, queue_family_count, surface)
static int l_vulkan_queue_families(lua_State* L) {
    lua_VkPhysicalDevice* device_ud = lua_check_VkPhysicalDevice(L, 1);
    uint32_t queue_family_count = (uint32_t)luaL_checkinteger(L, 2);
    lua_VkSurfaceKHR* surface_ud = lua_check_VkSurfaceKHR(L, 3);

    VkQueueFamilyProperties* queue_families = (VkQueueFamilyProperties*)malloc(queue_family_count * sizeof(VkQueueFamilyProperties));
    if (!queue_families) {
        luaL_error(L, "Failed to allocate memory for queue family properties");
    }

    vkGetPhysicalDeviceQueueFamilyProperties(device_ud->device, &queue_family_count, queue_families);

    lua_newtable(L);
    for (uint32_t i = 0; i < queue_family_count; i++) {
        lua_newtable(L);
        lua_pushinteger(L, queue_families[i].queueFlags);
        lua_setfield(L, -2, "queueFlags");
        lua_pushinteger(L, queue_families[i].queueCount);
        lua_setfield(L, -2, "queueCount");
        lua_pushinteger(L, queue_families[i].timestampValidBits);
        lua_setfield(L, -2, "timestampValidBits");
        lua_pushinteger(L, queue_families[i].minImageTransferGranularity.width);
        lua_setfield(L, -2, "minImageTransferGranularityWidth");

        // print(string.format("  Family %d: %s, Queue Count: %d, Present Support: %s, Timestamp Valid Bits: %d", i, table.concat(flags, ", "), family.queueCount, family.presentSupport and "Yes" or "No", family.timestampValidBits))

        // Check present support
        VkBool32 present_support = VK_FALSE;
        VkResult result = vkGetPhysicalDeviceSurfaceSupportKHR(device_ud->device, i, surface_ud->surface, &present_support);
        if (result != VK_SUCCESS) {
            free(queue_families);
            luaL_error(L, "Failed to check surface support for queue family %u: VkResult %d", i, result);
        }
        lua_pushboolean(L, present_support);
        lua_setfield(L, -2, "presentSupport");

        lua_rawseti(L, -2, i + 1); // Lua tables are 1-based
    }

    free(queue_families);
    return 1;
}

// Create VkApplicationInfo: vulkan.create_vk_application_info(table)
static int l_vulkan_create_vk_application_info(lua_State* L) {
    luaL_checktype(L, 1, LUA_TTABLE);

    VkApplicationInfo app_info = {
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pNext = NULL,
        .pApplicationName = NULL,
        .applicationVersion = VK_MAKE_API_VERSION(0, 1, 0, 0),
        .pEngineName = "No Engine",
        .engineVersion = VK_MAKE_API_VERSION(0, 1, 0, 0),
        .apiVersion = VK_API_VERSION_1_0
    };

    lua_getfield(L, 1, "pApplicationName");
    if (lua_isstring(L, -1)) {
        app_info.pApplicationName = lua_tostring(L, -1);
    }
    lua_pop(L, 1);

    lua_push_VkApplicationInfo(L, &app_info, app_info.pApplicationName);
    return 1;
}

// Get Vulkan instance extensions: vulkan.get_instance_extensions()
static int l_vulkan_get_instance_extensions(lua_State* L) {
    Uint32 extension_count = 0;
    const char *const *extensions = SDL_Vulkan_GetInstanceExtensions(&extension_count);
    if (!extensions) {
        luaL_error(L, "Failed to get Vulkan instance extensions: %s", SDL_GetError());
    }

    lua_newtable(L);
    for (Uint32 i = 0; i < extension_count; i++) {
        lua_pushstring(L, extensions[i]);
        lua_rawseti(L, -2, i + 1);
    }

    return 1;
}

// Create VkInstanceCreateInfo: vulkan.create_vk_create_info(table)
static int l_vulkan_create_vk_create_info(lua_State* L) {
    luaL_checktype(L, 1, LUA_TTABLE);

    VkInstanceCreateInfo create_info = {
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pNext = NULL,
        .flags = 0,
        .pApplicationInfo = NULL,
        .enabledLayerCount = 0,
        .ppEnabledLayerNames = NULL,
        .enabledExtensionCount = 0,
        .ppEnabledExtensionNames = NULL
    };

    char** extensions = NULL;
    uint32_t extension_count = 0;

    lua_getfield(L, 1, "pApplicationInfo");
    if (!lua_isnil(L, -1)) {
        lua_VkApplicationInfo* app_info_ud = lua_check_VkApplicationInfo(L, -1);
        create_info.pApplicationInfo = &app_info_ud->app_info;
    }
    lua_pop(L, 1);

    lua_getfield(L, 1, "ppEnabledExtensionNames");
    if (!lua_isnil(L, -1)) {
        luaL_checktype(L, -1, LUA_TTABLE);
        extension_count = lua_rawlen(L, -1);
        if (extension_count > 0) {
            extensions = (char**)malloc(extension_count * sizeof(char*));
            if (!extensions) {
                luaL_error(L, "Failed to allocate memory for extension names");
            }
            for (uint32_t i = 0; i < extension_count; i++) {
                lua_rawgeti(L, -1, i + 1);
                extensions[i] = strdup(luaL_checkstring(L, -1));
                if (!extensions[i]) {
                    for (uint32_t j = 0; j < i; j++) {
                        free(extensions[j]);
                    }
                    free(extensions);
                    luaL_error(L, "Failed to allocate memory for extension name");
                }
                lua_pop(L, 1);
            }
            create_info.enabledExtensionCount = extension_count;
            create_info.ppEnabledExtensionNames = (const char* const*)extensions;
        }
    }
    lua_pop(L, 1);

    lua_getfield(L, 1, "enabledExtensionCount");
    if (!lua_isnil(L, -1)) {
        create_info.enabledExtensionCount = (uint32_t)luaL_checkinteger(L, -1);
        if (create_info.enabledExtensionCount != extension_count) {
            if (extensions) {
                for (uint32_t i = 0; i < extension_count; i++) {
                    free(extensions[i]);
                }
                free(extensions);
            }
            luaL_error(L, "enabledExtensionCount does not match ppEnabledExtensionNames length");
        }
    }
    lua_pop(L, 1);

    lua_push_VkInstanceCreateInfo(L, &create_info, extensions, extension_count);
    return 1;
}

// Create VkInstance: vulkan.vk_create_instance(createinfo, pAllocator)
static int l_vulkan_vk_create_instance(lua_State* L) {
    lua_VkInstanceCreateInfo* create_info_ud = lua_check_VkInstanceCreateInfo(L, 1);
    luaL_checktype(L, 2, LUA_TNIL);

    VkInstance instance;
    VkResult result = vkCreateInstance(&create_info_ud->create_info, NULL, &instance);
    if (result != VK_SUCCESS) {
        luaL_error(L, "Failed to create Vulkan instance: VkResult %d", result);
    }

    lua_push_VkInstance(L, instance);
    return 1;
}

// Create VkSurfaceKHR: vulkan.create_surface(window, instance, pAllocator)
static int l_vulkan_create_surface(lua_State* L) {
    lua_SDL_Window* window_ud = lua_check_SDL_Window(L, 1);
    lua_VkInstance* instance_ud = lua_check_VkInstance(L, 2);
    luaL_checktype(L, 3, LUA_TNIL);

    VkSurfaceKHR surface;
    if (!SDL_Vulkan_CreateSurface(window_ud->window, instance_ud->instance, NULL, &surface)) {
        luaL_error(L, "Failed to create Vulkan surface: %s", SDL_GetError());
    }

    lua_push_VkSurfaceKHR(L, surface, instance_ud->instance);
    return 1;
}

// Push VkDeviceQueueCreateInfo
void lua_push_VkDeviceQueueCreateInfo(lua_State* L, VkDeviceQueueCreateInfo* queue_create_info, float* priorities) {
    if (!queue_create_info) {
        luaL_error(L, "Cannot create userdata for null VkDeviceQueueCreateInfo");
    }
    lua_VkDeviceQueueCreateInfo* ud = (lua_VkDeviceQueueCreateInfo*)lua_newuserdata(L, sizeof(lua_VkDeviceQueueCreateInfo));
    ud->queue_create_info = *queue_create_info;
    ud->pQueuePriorities = priorities;
    luaL_setmetatable(L, DEVICE_QUEUE_CREATE_INFO_MT);
}

// Check VkDeviceQueueCreateInfo
lua_VkDeviceQueueCreateInfo* lua_check_VkDeviceQueueCreateInfo(lua_State* L, int idx) {
    lua_VkDeviceQueueCreateInfo* ud = (lua_VkDeviceQueueCreateInfo*)luaL_checkudata(L, idx, DEVICE_QUEUE_CREATE_INFO_MT);
    return ud;
}

// Push VkDeviceCreateInfo
void lua_push_VkDeviceCreateInfo(lua_State* L, VkDeviceCreateInfo* device_create_info, char** extensions, uint32_t extension_count) {
    if (!device_create_info) {
        luaL_error(L, "Cannot create userdata for null VkDeviceCreateInfo");
    }
    lua_VkDeviceCreateInfo* ud = (lua_VkDeviceCreateInfo*)lua_newuserdata(L, sizeof(lua_VkDeviceCreateInfo));
    ud->device_create_info = *device_create_info;
    ud->ppEnabledExtensionNames = extensions;
    ud->enabledExtensionCount = extension_count;
    luaL_setmetatable(L, DEVICE_CREATE_INFO_MT);
}

// Check VkDeviceCreateInfo
lua_VkDeviceCreateInfo* lua_check_VkDeviceCreateInfo(lua_State* L, int idx) {
    lua_VkDeviceCreateInfo* ud = (lua_VkDeviceCreateInfo*)luaL_checkudata(L, idx, DEVICE_CREATE_INFO_MT);
    return ud;
}

// Push VkDevice
void lua_push_VkDevice(lua_State* L, VkDevice device) {
    if (!device) {
        luaL_error(L, "Cannot create userdata for null VkDevice");
    }
    lua_VkDevice* ud = (lua_VkDevice*)lua_newuserdata(L, sizeof(lua_VkDevice));
    ud->device = device;
    luaL_setmetatable(L, DEVICE_MT);
}

// Check VkDevice
lua_VkDevice* lua_check_VkDevice(lua_State* L, int idx) {
    lua_VkDevice* ud = (lua_VkDevice*)luaL_checkudata(L, idx, DEVICE_MT);
    if (!ud->device) {
        luaL_error(L, "Invalid VkDevice (already destroyed)");
    }
    return ud;
}

// Push VkQueue
void lua_push_VkQueue(lua_State* L, VkQueue queue) {
    if (!queue) {
        luaL_error(L, "Cannot create userdata for null VkQueue");
    }
    lua_VkQueue* ud = (lua_VkQueue*)lua_newuserdata(L, sizeof(lua_VkQueue));
    ud->queue = queue;
    luaL_setmetatable(L, QUEUE_MT);
}

// Check VkQueue
lua_VkQueue* lua_check_VkQueue(lua_State* L, int idx) {
    lua_VkQueue* ud = (lua_VkQueue*)luaL_checkudata(L, idx, QUEUE_MT);
    if (!ud->queue) {
        luaL_error(L, "Invalid VkQueue");
    }
    return ud;
}

// Create VkDeviceQueueCreateInfo: vulkan.queue_create_infos(graphics_family, present_family)
static int l_vulkan_queue_create_infos(lua_State* L) {
    uint32_t graphics_family = (uint32_t)luaL_checkinteger(L, 1);
    uint32_t present_family = (uint32_t)luaL_checkinteger(L, 2);

    uint32_t unique_families[2] = {graphics_family, present_family};
    uint32_t queue_create_info_count = (graphics_family == present_family) ? 1 : 2;

    lua_newtable(L);
    for (uint32_t i = 0; i < queue_create_info_count; i++) {
        float* queue_priority = (float*)malloc(sizeof(float));
        if (!queue_priority) {
            luaL_error(L, "Failed to allocate memory for queue priority");
        }
        *queue_priority = 1.0f;

        VkDeviceQueueCreateInfo queue_create_info = {
            .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
            .pNext = NULL,
            .flags = 0,
            .queueFamilyIndex = unique_families[i],
            .queueCount = 1,
            .pQueuePriorities = queue_priority
        };

        lua_push_VkDeviceQueueCreateInfo(L, &queue_create_info, queue_priority);
        lua_rawseti(L, -2, i + 1); // Lua tables are 1-based
    }

    return 1;
}

// Create VkDeviceCreateInfo: vulkan.device_create_info(table)
// Create VkDeviceCreateInfo: vulkan.device_create_info(table)
static int l_vulkan_device_create_info(lua_State* L) {
    luaL_checktype(L, 1, LUA_TTABLE);

    VkDeviceCreateInfo device_create_info = {
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .pNext = NULL,
        .flags = 0,
        .queueCreateInfoCount = 0,
        .pQueueCreateInfos = NULL,
        .enabledLayerCount = 0,
        .ppEnabledLayerNames = NULL,
        .enabledExtensionCount = 0,
        .ppEnabledExtensionNames = NULL,
        .pEnabledFeatures = NULL
    };

    char** extensions = NULL;
    uint32_t extension_count = 0;
    VkDeviceQueueCreateInfo* queue_create_infos = NULL;
    VkPhysicalDeviceFeatures* device_features = NULL;

    // Handle pQueueCreateInfos
    lua_getfield(L, 1, "pQueueCreateInfos");
    if (!lua_isnil(L, -1)) {
        luaL_checktype(L, -1, LUA_TTABLE);
        device_create_info.queueCreateInfoCount = lua_rawlen(L, -1);
        if (device_create_info.queueCreateInfoCount > 0) {
            queue_create_infos = (VkDeviceQueueCreateInfo*)malloc(device_create_info.queueCreateInfoCount * sizeof(VkDeviceQueueCreateInfo));
            if (!queue_create_infos) {
                luaL_error(L, "Failed to allocate memory for queue create infos");
            }
            for (uint32_t i = 0; i < device_create_info.queueCreateInfoCount; i++) {
                lua_rawgeti(L, -1, i + 1);
                lua_VkDeviceQueueCreateInfo* queue_ud = lua_check_VkDeviceQueueCreateInfo(L, -1);
                queue_create_infos[i] = queue_ud->queue_create_info;
                lua_pop(L, 1);
            }
            device_create_info.pQueueCreateInfos = queue_create_infos;
        }
    }
    lua_pop(L, 1);

    // Handle device extensions
    lua_getfield(L, 1, "ppEnabledExtensionNames");
    if (!lua_isnil(L, -1)) {
        luaL_checktype(L, -1, LUA_TTABLE);
        extension_count = lua_rawlen(L, -1);
        if (extension_count > 0) {
            extensions = (char**)malloc(extension_count * sizeof(char*));
            if (!extensions) {
                if (queue_create_infos) free(queue_create_infos);
                luaL_error(L, "Failed to allocate memory for extension names");
            }
            for (uint32_t i = 0; i < extension_count; i++) {
                lua_rawgeti(L, -1, i + 1);
                extensions[i] = strdup(luaL_checkstring(L, -1));
                if (!extensions[i]) {
                    for (uint32_t j = 0; j < i; j++) free(extensions[j]);
                    free(extensions);
                    if (queue_create_infos) free(queue_create_infos);
                    luaL_error(L, "Failed to allocate memory for extension name");
                }
                lua_pop(L, 1);
            }
            device_create_info.enabledExtensionCount = extension_count;
            device_create_info.ppEnabledExtensionNames = (const char* const*)extensions;
        }
    }
    lua_pop(L, 1);

    lua_getfield(L, 1, "enabledExtensionCount");
    if (!lua_isnil(L, -1)) {
        device_create_info.enabledExtensionCount = (uint32_t)luaL_checkinteger(L, -1);
        if (device_create_info.enabledExtensionCount != extension_count) {
            if (extensions) {
                for (uint32_t i = 0; i < extension_count; i++) free(extensions[i]);
                free(extensions);
            }
            if (queue_create_infos) free(queue_create_infos);
            luaL_error(L, "enabledExtensionCount does not match ppEnabledExtensionNames length");
        }
    }
    lua_pop(L, 1);

    // Allocate device features
    device_features = (VkPhysicalDeviceFeatures*)malloc(sizeof(VkPhysicalDeviceFeatures));
    if (!device_features) {
        if (extensions) {
            for (uint32_t i = 0; i < extension_count; i++) free(extensions[i]);
            free(extensions);
        }
        if (queue_create_infos) free(queue_create_infos);
        luaL_error(L, "Failed to allocate memory for device features");
    }
    memset(device_features, 0, sizeof(VkPhysicalDeviceFeatures));
    device_create_info.pEnabledFeatures = device_features;

    lua_VkDeviceCreateInfo* ud = (lua_VkDeviceCreateInfo*)lua_newuserdata(L, sizeof(lua_VkDeviceCreateInfo));
    ud->device_create_info = device_create_info;
    ud->ppEnabledExtensionNames = extensions;
    ud->enabledExtensionCount = extension_count;
    ud->device_create_info.pQueueCreateInfos = queue_create_infos; // Store for GC
    luaL_setmetatable(L, DEVICE_CREATE_INFO_MT);
    return 1;
}


// Create VkDevice: vulkan.create_device(physical_device, device_create_info, pAllocator)
static int l_vulkan_create_device(lua_State* L) {
    lua_VkPhysicalDevice* physical_device_ud = lua_check_VkPhysicalDevice(L, 1);
    lua_VkDeviceCreateInfo* device_create_info_ud = lua_check_VkDeviceCreateInfo(L, 2);
    luaL_checktype(L, 3, LUA_TNIL);

    VkDevice device;
    VkResult result = vkCreateDevice(physical_device_ud->device, &device_create_info_ud->device_create_info, NULL, &device);
    if (result != VK_SUCCESS) {
        luaL_error(L, "Failed to create Vulkan device: VkResult %d", result);
    }

    lua_push_VkDevice(L, device);
    return 1;
}

// Get VkQueue: vulkan.get_device_queue(device, queue_family_index, queue_index)
static int l_vulkan_get_device_queue(lua_State* L) {
    lua_VkDevice* device_ud = lua_check_VkDevice(L, 1);
    uint32_t queue_family_index = (uint32_t)luaL_checkinteger(L, 2);
    uint32_t queue_index = (uint32_t)luaL_checkinteger(L, 3);

    VkQueue queue;
    vkGetDeviceQueue(device_ud->device, queue_family_index, queue_index, &queue);
    if (!queue) {
        luaL_error(L, "Failed to get device queue");
    }

    lua_push_VkQueue(L, queue);
    return 1;
}

// Get device extensions: vulkan.get_device_extensions(physical_device)
static int l_vulkan_get_device_extensions(lua_State* L) {
    lua_VkPhysicalDevice* device_ud = lua_check_VkPhysicalDevice(L, 1);
    uint32_t extension_count = 0;
    VkResult result = vkEnumerateDeviceExtensionProperties(device_ud->device, NULL, &extension_count, NULL);
    if (result != VK_SUCCESS) {
        luaL_error(L, "Failed to get device extension count: VkResult %d", result);
    }

    VkExtensionProperties* extensions = (VkExtensionProperties*)malloc(extension_count * sizeof(VkExtensionProperties));
    if (!extensions) {
        luaL_error(L, "Failed to allocate memory for device extensions");
    }

    result = vkEnumerateDeviceExtensionProperties(device_ud->device, NULL, &extension_count, extensions);
    if (result != VK_SUCCESS) {
        free(extensions);
        luaL_error(L, "Failed to enumerate device extensions: VkResult %d", result);
    }

    lua_newtable(L);
    for (uint32_t i = 0; i < extension_count; i++) {
        lua_pushstring(L, extensions[i].extensionName);
        lua_rawseti(L, -2, i + 1); // Lua tables are 1-based
    }

    free(extensions);
    return 1;
}



// __index for VkSurfaceFormatKHR
static int surface_format_index(lua_State* L) {
    lua_VkSurfaceFormatKHR* ud = (lua_VkSurfaceFormatKHR*)luaL_checkudata(L, 1, SURFACE_FORMAT_MT);
    const char* field = luaL_checkstring(L, 2);

    if (strcmp(field, "format") == 0) {
        lua_pushinteger(L, ud->format.format);
        return 1;
    } else if (strcmp(field, "colorSpace") == 0) {
        lua_pushinteger(L, ud->format.colorSpace);
        return 1;
    } else {
        luaL_error(L, "Unknown field '%s' for vulkan.surface_format", field);
        return 0;
    }
}

// No garbage collection needed for VkSurfaceFormatKHR (no dynamic allocations)
// Updated metatable setup for VkSurfaceFormatKHR
static void surface_format_metatable(lua_State* L) {
    luaL_newmetatable(L, SURFACE_FORMAT_MT);
    lua_pushcfunction(L, surface_format_index);
    lua_setfield(L, -2, "__index");
    lua_pop(L, 1);
}

// Push VkSurfaceFormatKHR
void lua_push_VkSurfaceFormatKHR(lua_State* L, VkSurfaceFormatKHR format) {
    lua_VkSurfaceFormatKHR* ud = (lua_VkSurfaceFormatKHR*)lua_newuserdata(L, sizeof(lua_VkSurfaceFormatKHR));
    ud->format = format;
    luaL_setmetatable(L, SURFACE_FORMAT_MT);
}

// Check VkSurfaceFormatKHR
lua_VkSurfaceFormatKHR* lua_check_VkSurfaceFormatKHR(lua_State* L, int idx) {
    lua_VkSurfaceFormatKHR* ud = (lua_VkSurfaceFormatKHR*)luaL_checkudata(L, idx, SURFACE_FORMAT_MT);
    return ud;
}

// Get surface capabilities: vulkan.get_surface_capabilities(physical_device, surface)
static int l_vulkan_get_physical_device_surface_capabilities(lua_State* L) {
    lua_VkPhysicalDevice* device_ud = lua_check_VkPhysicalDevice(L, 1);
    lua_VkSurfaceKHR* surface_ud = lua_check_VkSurfaceKHR(L, 2);

    VkSurfaceCapabilitiesKHR capabilities;
    VkResult result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device_ud->device, surface_ud->surface, &capabilities);
    if (result != VK_SUCCESS) {
        luaL_error(L, "Failed to get surface capabilities: VkResult %d", result);
    }

    lua_newtable(L);
    lua_pushinteger(L, capabilities.minImageCount);
    lua_setfield(L, -2, "minImageCount");
    lua_pushinteger(L, capabilities.maxImageCount);
    lua_setfield(L, -2, "maxImageCount");
    lua_pushinteger(L, capabilities.currentExtent.width);
    lua_setfield(L, -2, "currentExtentWidth");
    lua_pushinteger(L, capabilities.currentExtent.height);
    lua_setfield(L, -2, "currentExtentHeight");
    lua_pushinteger(L, capabilities.minImageExtent.width);
    lua_setfield(L, -2, "minImageExtentWidth");
    lua_pushinteger(L, capabilities.minImageExtent.height);
    lua_setfield(L, -2, "minImageExtentHeight");
    lua_pushinteger(L, capabilities.maxImageExtent.width);
    lua_setfield(L, -2, "maxImageExtentWidth");
    lua_pushinteger(L, capabilities.maxImageExtent.height);
    lua_setfield(L, -2, "maxImageExtentHeight");
    lua_pushinteger(L, capabilities.maxImageArrayLayers);
    lua_setfield(L, -2, "maxImageArrayLayers");
    lua_pushinteger(L, capabilities.supportedTransforms);
    lua_setfield(L, -2, "supportedTransforms");
    lua_pushinteger(L, capabilities.currentTransform);
    lua_setfield(L, -2, "currentTransform");
    lua_pushinteger(L, capabilities.supportedCompositeAlpha);
    lua_setfield(L, -2, "supportedCompositeAlpha");
    lua_pushinteger(L, capabilities.supportedUsageFlags);
    lua_setfield(L, -2, "supportedUsageFlags");

    return 1;
}

// Get surface formats: vulkan.get_surface_formats(physical_device, surface)
static int l_vulkan_get_physical_device_surface_formats(lua_State* L) {
    lua_VkPhysicalDevice* device_ud = lua_check_VkPhysicalDevice(L, 1);
    lua_VkSurfaceKHR* surface_ud = lua_check_VkSurfaceKHR(L, 2);

    uint32_t format_count = 0;
    VkResult result = vkGetPhysicalDeviceSurfaceFormatsKHR(device_ud->device, surface_ud->surface, &format_count, NULL);
    if (result != VK_SUCCESS) {
        luaL_error(L, "Failed to get surface format count: VkResult %d", result);
    }

    VkSurfaceFormatKHR* formats = (VkSurfaceFormatKHR*)malloc(format_count * sizeof(VkSurfaceFormatKHR));
    if (!formats) {
        luaL_error(L, "Failed to allocate memory for surface formats");
    }

    result = vkGetPhysicalDeviceSurfaceFormatsKHR(device_ud->device, surface_ud->surface, &format_count, formats);
    if (result != VK_SUCCESS) {
        free(formats);
        luaL_error(L, "Failed to get surface formats: VkResult %d", result);
    }

    lua_newtable(L);
    for (uint32_t i = 0; i < format_count; i++) {
        lua_push_VkSurfaceFormatKHR(L, formats[i]);
        lua_rawseti(L, -2, i + 1); // Lua tables are 1-based
    }

    free(formats);
    return 1;
}

// Get present modes: vulkan.get_surface_present_modes(physical_device, surface)
static int l_vulkan_get_physical_device_surface_present_modes(lua_State* L) {
    lua_VkPhysicalDevice* device_ud = lua_check_VkPhysicalDevice(L, 1);
    lua_VkSurfaceKHR* surface_ud = lua_check_VkSurfaceKHR(L, 2);

    uint32_t present_mode_count = 0;
    VkResult result = vkGetPhysicalDeviceSurfacePresentModesKHR(device_ud->device, surface_ud->surface, &present_mode_count, NULL);
    if (result != VK_SUCCESS) {
        luaL_error(L, "Failed to get present mode count: VkResult %d", result);
    }

    VkPresentModeKHR* present_modes = (VkPresentModeKHR*)malloc(present_mode_count * sizeof(VkPresentModeKHR));
    if (!present_modes) {
        luaL_error(L, "Failed to allocate memory for present modes");
    }

    result = vkGetPhysicalDeviceSurfacePresentModesKHR(device_ud->device, surface_ud->surface, &present_mode_count, present_modes);
    if (result != VK_SUCCESS) {
        free(present_modes);
        luaL_error(L, "Failed to get present modes: VkResult %d", result);
    }

    lua_newtable(L);
    for (uint32_t i = 0; i < present_mode_count; i++) {
        lua_pushinteger(L, present_modes[i]);
        lua_rawseti(L, -2, i + 1); // Lua tables are 1-based
    }

    free(present_modes);
    return 1;
}


// Garbage collection for VkSwapchainCreateInfoKHR (no dynamic allocations to clean)
static int swapchain_create_info_gc(lua_State* L) {
    printf("Cleaning up VkSwapchainCreateInfoKHR\n");
    return 0;
}

// Garbage collection for VkSwapchainKHR
static int swapchain_gc(lua_State* L) {
    lua_VkSwapchainKHR* ud = (lua_VkSwapchainKHR*)luaL_checkudata(L, 1, SWAPCHAIN_MT);
    if (ud->swapchain && ud->device) {
        printf("Cleaning up VkSwapchainKHR\n");
        vkDestroySwapchainKHR(ud->device, ud->swapchain, NULL);
        ud->swapchain = NULL;
        ud->device = NULL;
    }
    return 0;
}

// Push VkSwapchainCreateInfoKHR
void lua_push_VkSwapchainCreateInfoKHR(lua_State* L, VkSwapchainCreateInfoKHR* swapchain_create_info) {
    if (!swapchain_create_info) {
        luaL_error(L, "Cannot create userdata for null VkSwapchainCreateInfoKHR");
    }
    lua_VkSwapchainCreateInfoKHR* ud = (lua_VkSwapchainCreateInfoKHR*)lua_newuserdata(L, sizeof(lua_VkSwapchainCreateInfoKHR));
    ud->swapchain_create_info = *swapchain_create_info;
    luaL_setmetatable(L, SWAPCHAIN_CREATE_INFO_MT);
}

// Check VkSwapchainCreateInfoKHR
lua_VkSwapchainCreateInfoKHR* lua_check_VkSwapchainCreateInfoKHR(lua_State* L, int idx) {
    lua_VkSwapchainCreateInfoKHR* ud = (lua_VkSwapchainCreateInfoKHR*)luaL_checkudata(L, idx, SWAPCHAIN_CREATE_INFO_MT);
    return ud;
}

// Push VkSwapchainKHR
void lua_push_VkSwapchainKHR(lua_State* L, VkSwapchainKHR swapchain, VkDevice device) {
    if (!swapchain) {
        luaL_error(L, "Cannot create userdata for null VkSwapchainKHR");
    }
    lua_VkSwapchainKHR* ud = (lua_VkSwapchainKHR*)lua_newuserdata(L, sizeof(lua_VkSwapchainKHR));
    ud->swapchain = swapchain;
    ud->device = device;
    luaL_setmetatable(L, SWAPCHAIN_MT);
}

// Check VkSwapchainKHR
lua_VkSwapchainKHR* lua_check_VkSwapchainKHR(lua_State* L, int idx) {
    lua_VkSwapchainKHR* ud = (lua_VkSwapchainKHR*)luaL_checkudata(L, idx, SWAPCHAIN_MT);
    if (!ud->swapchain) {
        luaL_error(L, "Invalid VkSwapchainKHR (already destroyed)");
    }
    return ud;
}

// Create VkSwapchainCreateInfoKHR: vulkan.create_swapchain_create_info(table)
static int l_vulkan_create_swapchain_create_info(lua_State* L) {
    luaL_checktype(L, 1, LUA_TTABLE);

    VkSwapchainCreateInfoKHR swapchain_create_info = {
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .pNext = NULL,
        .flags = 0,
        .surface = VK_NULL_HANDLE,
        .minImageCount = 0,
        .imageFormat = VK_FORMAT_UNDEFINED,
        .imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR,
        .imageExtent = {0, 0},
        .imageArrayLayers = 1,
        .imageUsage = 0,
        .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .queueFamilyIndexCount = 0,
        .pQueueFamilyIndices = NULL,
        .preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR,
        .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        .presentMode = VK_PRESENT_MODE_FIFO_KHR,
        .clipped = VK_TRUE,
        .oldSwapchain = VK_NULL_HANDLE
    };

    // Handle surface
    lua_getfield(L, 1, "surface");
    if (!lua_isnil(L, -1)) {
        lua_VkSurfaceKHR* surface_ud = lua_check_VkSurfaceKHR(L, -1);
        swapchain_create_info.surface = surface_ud->surface;
    }
    lua_pop(L, 1);

    // Handle minImageCount
    lua_getfield(L, 1, "minImageCount");
    if (!lua_isnil(L, -1)) {
        swapchain_create_info.minImageCount = (uint32_t)luaL_checkinteger(L, -1);
    }
    lua_pop(L, 1);

    // Handle imageFormat
    lua_getfield(L, 1, "imageFormat");
    if (!lua_isnil(L, -1)) {
        swapchain_create_info.imageFormat = (VkFormat)luaL_checkinteger(L, -1);
    }
    lua_pop(L, 1);

    // Handle imageColorSpace
    lua_getfield(L, 1, "imageColorSpace");
    if (!lua_isnil(L, -1)) {
        swapchain_create_info.imageColorSpace = (VkColorSpaceKHR)luaL_checkinteger(L, -1);
    }
    lua_pop(L, 1);

    // Handle imageExtent
    lua_getfield(L, 1, "imageExtent");
    if (!lua_isnil(L, -1)) {
        luaL_checktype(L, -1, LUA_TTABLE);
        lua_getfield(L, -1, "width");
        swapchain_create_info.imageExtent.width = (uint32_t)luaL_checkinteger(L, -1);
        lua_pop(L, 1);
        lua_getfield(L, -1, "height");
        swapchain_create_info.imageExtent.height = (uint32_t)luaL_checkinteger(L, -1);
        lua_pop(L, 1);
    }
    lua_pop(L, 1);

    // Handle imageArrayLayers
    lua_getfield(L, 1, "imageArrayLayers");
    if (!lua_isnil(L, -1)) {
        swapchain_create_info.imageArrayLayers = (uint32_t)luaL_checkinteger(L, -1);
    }
    lua_pop(L, 1);

    // Handle imageUsage
    lua_getfield(L, 1, "imageUsage");
    if (!lua_isnil(L, -1)) {
        swapchain_create_info.imageUsage = (VkImageUsageFlags)luaL_checkinteger(L, -1);
    }
    lua_pop(L, 1);

    // Handle presentMode
    lua_getfield(L, 1, "presentMode");
    if (!lua_isnil(L, -1)) {
        swapchain_create_info.presentMode = (VkPresentModeKHR)luaL_checkinteger(L, -1);
    }
    lua_pop(L, 1);

    lua_push_VkSwapchainCreateInfoKHR(L, &swapchain_create_info);
    return 1;
}

// Create VkSwapchainKHR: vulkan.create_swapchain(device, swapchain_create_info, pAllocator)
static int l_vulkan_create_swapchain(lua_State* L) {
    lua_VkDevice* device_ud = lua_check_VkDevice(L, 1);
    lua_VkSwapchainCreateInfoKHR* create_info_ud = lua_check_VkSwapchainCreateInfoKHR(L, 2);
    luaL_checktype(L, 3, LUA_TNIL);

    VkSwapchainKHR swapchain;
    VkResult result = vkCreateSwapchainKHR(device_ud->device, &create_info_ud->swapchain_create_info, NULL, &swapchain);
    if (result != VK_SUCCESS) {
        luaL_error(L, "Failed to create Vulkan swapchain: VkResult %d", result);
    }

    lua_push_VkSwapchainKHR(L, swapchain, device_ud->device);
    return 1;
}

// Get swapchain images: vulkan.get_swapchain_images(device, swapchain)
static int l_vulkan_get_swapchain_images(lua_State* L) {
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
        lua_rawseti(L, -2, i + 1);
    }

    free(images);
    return 1;
}


// IMAGE AND RENDER PASS

// Garbage collection for VkImageViewCreateInfo (no dynamic allocations)
static int image_view_create_info_gc(lua_State* L) {
    printf("Cleaning up VkImageViewCreateInfo\n");
    return 0;
}

// Garbage collection for VkImageView
static int image_view_gc(lua_State* L) {
    lua_VkImageView* ud = (lua_VkImageView*)luaL_checkudata(L, 1, IMAGE_VIEW_MT);
    if (ud->image_view && ud->device) {
        printf("Cleaning up VkImageView\n");
        vkDestroyImageView(ud->device, ud->image_view, NULL);
        ud->image_view = NULL;
        ud->device = NULL;
    }
    return 0;
}

// Garbage collection for VkAttachmentDescription (no dynamic allocations)
static int attachment_description_gc(lua_State* L) {
    printf("Cleaning up VkAttachmentDescription\n");
    return 0;
}

// Garbage collection for VkSubpassDescription
static int subpass_description_gc(lua_State* L) {
    lua_VkSubpassDescription* ud = (lua_VkSubpassDescription*)luaL_checkudata(L, 1, SUBPASS_DESCRIPTION_MT);
    if (ud->pColorAttachments) {
        printf("Cleaning up VkSubpassDescription\n");
        free(ud->pColorAttachments);
        ud->pColorAttachments = NULL;
    }
    return 0;
}

// Garbage collection for VkSubpassDependency (no dynamic allocations)
static int subpass_dependency_gc(lua_State* L) {
    printf("Cleaning up VkSubpassDependency\n");
    return 0;
}

// Garbage collection for VkRenderPassCreateInfo
static int render_pass_create_info_gc(lua_State* L) {
    lua_VkRenderPassCreateInfo* ud = (lua_VkRenderPassCreateInfo*)luaL_checkudata(L, 1, RENDER_PASS_CREATE_INFO_MT);
    if (ud->pAttachments) {
        printf("Cleaning up VkRenderPassCreateInfo attachments\n");
        free(ud->pAttachments);
        ud->pAttachments = NULL;
    }
    if (ud->pSubpasses) {
        printf("Cleaning up VkRenderPassCreateInfo subpasses\n");
        free(ud->pSubpasses);
        ud->pSubpasses = NULL;
    }
    if (ud->pDependencies) {
        printf("Cleaning up VkRenderPassCreateInfo dependencies\n");
        free(ud->pDependencies);
        ud->pDependencies = NULL;
    }
    return 0;
}

// Garbage collection for VkRenderPass
static int render_pass_gc(lua_State* L) {
    lua_VkRenderPass* ud = (lua_VkRenderPass*)luaL_checkudata(L, 1, RENDER_PASS_MT);
    if (ud->render_pass && ud->device) {
        printf("Cleaning up VkRenderPass\n");
        vkDestroyRenderPass(ud->device, ud->render_pass, NULL);
        ud->render_pass = NULL;
        ud->device = NULL;
    }
    return 0;
}

// Push VkImageViewCreateInfo
void lua_push_VkImageViewCreateInfo(lua_State* L, VkImageViewCreateInfo* create_info) {
    if (!create_info) {
        luaL_error(L, "Cannot create userdata for null VkImageViewCreateInfo");
    }
    lua_VkImageViewCreateInfo* ud = (lua_VkImageViewCreateInfo*)lua_newuserdata(L, sizeof(lua_VkImageViewCreateInfo));
    ud->create_info = *create_info;
    luaL_setmetatable(L, IMAGE_VIEW_CREATE_INFO_MT);
}

// Check VkImageViewCreateInfo
lua_VkImageViewCreateInfo* lua_check_VkImageViewCreateInfo(lua_State* L, int idx) {
    lua_VkImageViewCreateInfo* ud = (lua_VkImageViewCreateInfo*)luaL_checkudata(L, idx, IMAGE_VIEW_CREATE_INFO_MT);
    return ud;
}

// Push VkImageView
void lua_push_VkImageView(lua_State* L, VkImageView image_view, VkDevice device) {
    if (!image_view) {
        luaL_error(L, "Cannot create userdata for null VkImageView");
    }
    lua_VkImageView* ud = (lua_VkImageView*)lua_newuserdata(L, sizeof(lua_VkImageView));
    ud->image_view = image_view;
    ud->device = device;
    luaL_setmetatable(L, IMAGE_VIEW_MT);
}

// Check VkImageView
lua_VkImageView* lua_check_VkImageView(lua_State* L, int idx) {
    lua_VkImageView* ud = (lua_VkImageView*)luaL_checkudata(L, idx, IMAGE_VIEW_MT);
    if (!ud->image_view) {
        luaL_error(L, "Invalid VkImageView (already destroyed)");
    }
    return ud;
}

// Push VkAttachmentDescription
void lua_push_VkAttachmentDescription(lua_State* L, VkAttachmentDescription* desc) {
    if (!desc) {
        luaL_error(L, "Cannot create userdata for null VkAttachmentDescription");
    }
    lua_VkAttachmentDescription* ud = (lua_VkAttachmentDescription*)lua_newuserdata(L, sizeof(lua_VkAttachmentDescription));
    ud->desc = *desc;
    luaL_setmetatable(L, ATTACHMENT_DESCRIPTION_MT);
}

// Check VkAttachmentDescription
lua_VkAttachmentDescription* lua_check_VkAttachmentDescription(lua_State* L, int idx) {
    lua_VkAttachmentDescription* ud = (lua_VkAttachmentDescription*)luaL_checkudata(L, idx, ATTACHMENT_DESCRIPTION_MT);
    return ud;
}

// Push VkSubpassDescription
void lua_push_VkSubpassDescription(lua_State* L, VkSubpassDescription* desc, VkAttachmentReference* color_attachments) {
    if (!desc) {
        luaL_error(L, "Cannot create userdata for null VkSubpassDescription");
    }
    lua_VkSubpassDescription* ud = (lua_VkSubpassDescription*)lua_newuserdata(L, sizeof(lua_VkSubpassDescription));
    ud->desc = *desc;
    ud->pColorAttachments = color_attachments;
    luaL_setmetatable(L, SUBPASS_DESCRIPTION_MT);
}

// Check VkSubpassDescription
lua_VkSubpassDescription* lua_check_VkSubpassDescription(lua_State* L, int idx) {
    lua_VkSubpassDescription* ud = (lua_VkSubpassDescription*)luaL_checkudata(L, idx, SUBPASS_DESCRIPTION_MT);
    return ud;
}

// Push VkSubpassDependency
void lua_push_VkSubpassDependency(lua_State* L, VkSubpassDependency* dep) {
    if (!dep) {
        luaL_error(L, "Cannot create userdata for null VkSubpassDependency");
    }
    lua_VkSubpassDependency* ud = (lua_VkSubpassDependency*)lua_newuserdata(L, sizeof(lua_VkSubpassDependency));
    ud->dep = *dep;
    luaL_setmetatable(L, SUBPASS_DEPENDENCY_MT);
}

// Check VkSubpassDependency
lua_VkSubpassDependency* lua_check_VkSubpassDependency(lua_State* L, int idx) {
    lua_VkSubpassDependency* ud = (lua_VkSubpassDependency*)luaL_checkudata(L, idx, SUBPASS_DEPENDENCY_MT);
    return ud;
}

// Push VkRenderPassCreateInfo
void lua_push_VkRenderPassCreateInfo(lua_State* L, VkRenderPassCreateInfo* create_info, VkAttachmentDescription* attachments, VkSubpassDescription* subpasses, VkSubpassDependency* dependencies) {
    if (!create_info) {
        luaL_error(L, "Cannot create userdata for null VkRenderPassCreateInfo");
    }
    lua_VkRenderPassCreateInfo* ud = (lua_VkRenderPassCreateInfo*)lua_newuserdata(L, sizeof(lua_VkRenderPassCreateInfo));
    ud->create_info = *create_info;
    ud->pAttachments = attachments;
    ud->pSubpasses = subpasses;
    ud->pDependencies = dependencies;
    luaL_setmetatable(L, RENDER_PASS_CREATE_INFO_MT);
}

// Check VkRenderPassCreateInfo
lua_VkRenderPassCreateInfo* lua_check_VkRenderPassCreateInfo(lua_State* L, int idx) {
    lua_VkRenderPassCreateInfo* ud = (lua_VkRenderPassCreateInfo*)luaL_checkudata(L, idx, RENDER_PASS_CREATE_INFO_MT);
    return ud;
}

// Push VkRenderPass
void lua_push_VkRenderPass(lua_State* L, VkRenderPass render_pass, VkDevice device) {
    if (!render_pass) {
        luaL_error(L, "Cannot create userdata for null VkRenderPass");
    }
    lua_VkRenderPass* ud = (lua_VkRenderPass*)lua_newuserdata(L, sizeof(lua_VkRenderPass));
    ud->render_pass = render_pass;
    ud->device = device;
    luaL_setmetatable(L, RENDER_PASS_MT);
}

// Check VkRenderPass
lua_VkRenderPass* lua_check_VkRenderPass(lua_State* L, int idx) {
    lua_VkRenderPass* ud = (lua_VkRenderPass*)luaL_checkudata(L, idx, RENDER_PASS_MT);
    if (!ud->render_pass) {
        luaL_error(L, "Invalid VkRenderPass (already destroyed)");
    }
    return ud;
}

// Create VkImageViewCreateInfo: vulkan.create_image_view_create_info(table)
static int l_vulkan_create_image_view_create_info(lua_State* L) {
    luaL_checktype(L, 1, LUA_TTABLE);

    VkImageViewCreateInfo create_info = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .pNext = NULL,
        .flags = 0,
        .image = VK_NULL_HANDLE,
        .viewType = VK_IMAGE_VIEW_TYPE_2D,
        .format = VK_FORMAT_UNDEFINED,
        .components = {VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY},
        .subresourceRange = {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .baseMipLevel = 0,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1
        }
    };

    lua_getfield(L, 1, "image");
    if (!lua_isnil(L, -1)) {
        create_info.image = (VkImage)lua_touserdata(L, -1);
    }
    lua_pop(L, 1);

    lua_getfield(L, 1, "format");
    if (!lua_isnil(L, -1)) {
        create_info.format = (VkFormat)luaL_checkinteger(L, -1);
    }
    lua_pop(L, 1);

    lua_push_VkImageViewCreateInfo(L, &create_info);
    return 1;
}

// Create VkImageView: vulkan.create_image_view(device, create_info, pAllocator)
static int l_vulkan_create_image_view(lua_State* L) {
    lua_VkDevice* device_ud = lua_check_VkDevice(L, 1);
    lua_VkImageViewCreateInfo* create_info_ud = lua_check_VkImageViewCreateInfo(L, 2);
    luaL_checktype(L, 3, LUA_TNIL);

    VkImageView image_view;
    VkResult result = vkCreateImageView(device_ud->device, &create_info_ud->create_info, NULL, &image_view);
    if (result != VK_SUCCESS) {
        luaL_error(L, "Failed to create image view: VkResult %d", result);
    }

    lua_push_VkImageView(L, image_view, device_ud->device);
    return 1;
}

// Create VkAttachmentDescription: vulkan.create_attachment_description(table)
static int l_vulkan_create_attachment_description(lua_State* L) {
    luaL_checktype(L, 1, LUA_TTABLE);

    VkAttachmentDescription desc = {
        .flags = 0,
        .format = VK_FORMAT_UNDEFINED,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
    };

    lua_getfield(L, 1, "format");
    if (!lua_isnil(L, -1)) {
        desc.format = (VkFormat)luaL_checkinteger(L, -1);
    }
    lua_pop(L, 1);

    lua_push_VkAttachmentDescription(L, &desc);
    return 1;
}

// Create VkSubpassDescription: vulkan.create_subpass_description(table)
static int l_vulkan_create_subpass_description(lua_State* L) {
    luaL_checktype(L, 1, LUA_TTABLE);

    VkSubpassDescription desc = {
        .flags = 0,
        .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
        .inputAttachmentCount = 0,
        .pInputAttachments = NULL,
        .colorAttachmentCount = 0,
        .pColorAttachments = NULL,
        .pResolveAttachments = NULL,
        .pDepthStencilAttachment = NULL,
        .preserveAttachmentCount = 0,
        .pPreserveAttachments = NULL
    };

    VkAttachmentReference* color_attachments = NULL;
    lua_getfield(L, 1, "pColorAttachments");
    if (!lua_isnil(L, -1)) {
        luaL_checktype(L, -1, LUA_TTABLE);
        desc.colorAttachmentCount = lua_rawlen(L, -1);
        if (desc.colorAttachmentCount > 0) {
            color_attachments = (VkAttachmentReference*)malloc(desc.colorAttachmentCount * sizeof(VkAttachmentReference));
            if (!color_attachments) {
                luaL_error(L, "Failed to allocate memory for color attachments");
            }
            for (uint32_t i = 0; i < desc.colorAttachmentCount; i++) {
                lua_rawgeti(L, -1, i + 1);
                luaL_checktype(L, -1, LUA_TTABLE);
                lua_getfield(L, -1, "attachment");
                color_attachments[i].attachment = (uint32_t)luaL_checkinteger(L, -1);
                lua_pop(L, 1);
                lua_getfield(L, -1, "layout");
                color_attachments[i].layout = (VkImageLayout)luaL_checkinteger(L, -1);
                lua_pop(L, 1);
                lua_pop(L, 1);
            }
            desc.pColorAttachments = color_attachments;
        }
    }
    lua_pop(L, 1);

    lua_push_VkSubpassDescription(L, &desc, color_attachments);
    return 1;
}

// Create VkSubpassDependency: vulkan.create_subpass_dependency(table)
static int l_vulkan_create_subpass_dependency(lua_State* L) {
    luaL_checktype(L, 1, LUA_TTABLE);

    VkSubpassDependency dep = {
        .srcSubpass = VK_SUBPASS_EXTERNAL,
        .dstSubpass = 0,
        .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        .srcAccessMask = 0,
        .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
        .dependencyFlags = 0
    };

    lua_getfield(L, 1, "srcSubpass");
    if (!lua_isnil(L, -1)) {
        dep.srcSubpass = (uint32_t)luaL_checkinteger(L, -1);
    }
    lua_pop(L, 1);

    lua_getfield(L, 1, "dstSubpass");
    if (!lua_isnil(L, -1)) {
        dep.dstSubpass = (uint32_t)luaL_checkinteger(L, -1);
    }
    lua_pop(L, 1);

    lua_getfield(L, 1, "srcStageMask");
    if (!lua_isnil(L, -1)) {
        dep.srcStageMask = (VkPipelineStageFlags)luaL_checkinteger(L, -1);
    }
    lua_pop(L, 1);

    lua_getfield(L, 1, "dstStageMask");
    if (!lua_isnil(L, -1)) {
        dep.dstStageMask = (VkPipelineStageFlags)luaL_checkinteger(L, -1);
    }
    lua_pop(L, 1);

    lua_getfield(L, 1, "srcAccessMask");
    if (!lua_isnil(L, -1)) {
        dep.srcAccessMask = (VkAccessFlags)luaL_checkinteger(L, -1);
    }
    lua_pop(L, 1);

    lua_getfield(L, 1, "dstAccessMask");
    if (!lua_isnil(L, -1)) {
        dep.dstAccessMask = (VkAccessFlags)luaL_checkinteger(L, -1);
    }
    lua_pop(L, 1);

    lua_push_VkSubpassDependency(L, &dep);
    return 1;
}

// Create VkRenderPassCreateInfo: vulkan.create_render_pass_create_info(table)
static int l_vulkan_create_render_pass_create_info(lua_State* L) {
    luaL_checktype(L, 1, LUA_TTABLE);

    VkRenderPassCreateInfo create_info = {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        .pNext = NULL,
        .flags = 0,
        .attachmentCount = 0,
        .pAttachments = NULL,
        .subpassCount = 0,
        .pSubpasses = NULL,
        .dependencyCount = 0,
        .pDependencies = NULL
    };

    VkAttachmentDescription* attachments = NULL;
    VkSubpassDescription* subpasses = NULL;
    VkSubpassDependency* dependencies = NULL;

    lua_getfield(L, 1, "pAttachments");
    if (!lua_isnil(L, -1)) {
        luaL_checktype(L, -1, LUA_TTABLE);
        create_info.attachmentCount = lua_rawlen(L, -1);
        if (create_info.attachmentCount > 0) {
            attachments = (VkAttachmentDescription*)malloc(create_info.attachmentCount * sizeof(VkAttachmentDescription));
            if (!attachments) {
                luaL_error(L, "Failed to allocate memory for attachments");
            }
            for (uint32_t i = 0; i < create_info.attachmentCount; i++) {
                lua_rawgeti(L, -1, i + 1);
                lua_VkAttachmentDescription* desc_ud = lua_check_VkAttachmentDescription(L, -1);
                attachments[i] = desc_ud->desc;
                lua_pop(L, 1);
            }
            create_info.pAttachments = attachments;
        }
    }
    lua_pop(L, 1);

    lua_getfield(L, 1, "pSubpasses");
    if (!lua_isnil(L, -1)) {
        luaL_checktype(L, -1, LUA_TTABLE);
        create_info.subpassCount = lua_rawlen(L, -1);
        if (create_info.subpassCount > 0) {
            subpasses = (VkSubpassDescription*)malloc(create_info.subpassCount * sizeof(VkSubpassDescription));
            if (!subpasses) {
                if (attachments) free(attachments);
                luaL_error(L, "Failed to allocate memory for subpasses");
            }
            for (uint32_t i = 0; i < create_info.subpassCount; i++) {
                lua_rawgeti(L, -1, i + 1);
                lua_VkSubpassDescription* subpass_ud = lua_check_VkSubpassDescription(L, -1);
                subpasses[i] = subpass_ud->desc;
                lua_pop(L, 1);
            }
            create_info.pSubpasses = subpasses;
        }
    }
    lua_pop(L, 1);

    lua_getfield(L, 1, "pDependencies");
    if (!lua_isnil(L, -1)) {
        luaL_checktype(L, -1, LUA_TTABLE);
        create_info.dependencyCount = lua_rawlen(L, -1);
        if (create_info.dependencyCount > 0) {
            dependencies = (VkSubpassDependency*)malloc(create_info.dependencyCount * sizeof(VkSubpassDependency));
            if (!dependencies) {
                if (attachments) free(attachments);
                if (subpasses) free(subpasses);
                luaL_error(L, "Failed to allocate memory for dependencies");
            }
            for (uint32_t i = 0; i < create_info.dependencyCount; i++) {
                lua_rawgeti(L, -1, i + 1);
                lua_VkSubpassDependency* dep_ud = lua_check_VkSubpassDependency(L, -1);
                dependencies[i] = dep_ud->dep;
                lua_pop(L, 1);
            }
            create_info.pDependencies = dependencies;
        }
    }
    lua_pop(L, 1);

    lua_push_VkRenderPassCreateInfo(L, &create_info, attachments, subpasses, dependencies);
    return 1;
}

// Create VkRenderPass: vulkan.create_render_pass(device, create_info, pAllocator)
static int l_vulkan_create_render_pass(lua_State* L) {
    lua_VkDevice* device_ud = lua_check_VkDevice(L, 1);
    lua_VkRenderPassCreateInfo* create_info_ud = lua_check_VkRenderPassCreateInfo(L, 2);
    luaL_checktype(L, 3, LUA_TNIL);

    VkRenderPass render_pass;
    VkResult result = vkCreateRenderPass(device_ud->device, &create_info_ud->create_info, NULL, &render_pass);
    if (result != VK_SUCCESS) {
        luaL_error(L, "Failed to create render pass: VkResult %d", result);
    }

    lua_push_VkRenderPass(L, render_pass, device_ud->device);
    return 1;
}


static int framebuffer_create_info_gc(lua_State* L) {
    lua_VkFramebufferCreateInfo* ud = (lua_VkFramebufferCreateInfo*)luaL_checkudata(L, 1, FRAMEBUFFER_CREATE_INFO_MT);
    if (ud->pAttachments) {
        printf("Cleaning up VkFramebufferCreateInfo attachments\n");
        free(ud->pAttachments);
        ud->pAttachments = NULL;
    }
    return 0;
}

static int framebuffer_gc(lua_State* L) {
    lua_VkFramebuffer* ud = (lua_VkFramebuffer*)luaL_checkudata(L, 1, FRAMEBUFFER_MT);
    if (ud->framebuffer && ud->device) {
        printf("Cleaning up VkFramebuffer\n");
        vkDestroyFramebuffer(ud->device, ud->framebuffer, NULL);
        ud->framebuffer = NULL;
        ud->device = NULL;
    }
    return 0;
}

void lua_push_VkFramebufferCreateInfo(lua_State* L, VkFramebufferCreateInfo* create_info, VkImageView* attachments) {
    if (!create_info) {
        luaL_error(L, "Cannot create userdata for null VkFramebufferCreateInfo");
    }
    lua_VkFramebufferCreateInfo* ud = (lua_VkFramebufferCreateInfo*)lua_newuserdata(L, sizeof(lua_VkFramebufferCreateInfo));
    ud->create_info = *create_info;
    ud->pAttachments = attachments;
    luaL_setmetatable(L, FRAMEBUFFER_CREATE_INFO_MT);
}

lua_VkFramebufferCreateInfo* lua_check_VkFramebufferCreateInfo(lua_State* L, int idx) {
    lua_VkFramebufferCreateInfo* ud = (lua_VkFramebufferCreateInfo*)luaL_checkudata(L, idx, FRAMEBUFFER_CREATE_INFO_MT);
    return ud;
}

void lua_push_VkFramebuffer(lua_State* L, VkFramebuffer framebuffer, VkDevice device) {
    if (!framebuffer) {
        luaL_error(L, "Cannot create userdata for null VkFramebuffer");
    }
    lua_VkFramebuffer* ud = (lua_VkFramebuffer*)lua_newuserdata(L, sizeof(lua_VkFramebuffer));
    ud->framebuffer = framebuffer;
    ud->device = device;
    luaL_setmetatable(L, FRAMEBUFFER_MT);
}

lua_VkFramebuffer* lua_check_VkFramebuffer(lua_State* L, int idx) {
    lua_VkFramebuffer* ud = (lua_VkFramebuffer*)luaL_checkudata(L, idx, FRAMEBUFFER_MT);
    if (!ud->framebuffer) {
        luaL_error(L, "Invalid VkFramebuffer (already destroyed)");
    }
    return ud;
}

static int l_vulkan_create_framebuffer_create_info(lua_State* L) {
    luaL_checktype(L, 1, LUA_TTABLE);

    VkFramebufferCreateInfo create_info = {
        .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
        .pNext = NULL,
        .flags = 0,
        .renderPass = VK_NULL_HANDLE,
        .attachmentCount = 0,
        .pAttachments = NULL,
        .width = 0,
        .height = 0,
        .layers = 1
    };

    VkImageView* attachments = NULL;

    lua_getfield(L, 1, "renderPass");
    if (!lua_isnil(L, -1)) {
        lua_VkRenderPass* render_pass_ud = lua_check_VkRenderPass(L, -1);
        create_info.renderPass = render_pass_ud->render_pass;
    }
    lua_pop(L, 1);

    lua_getfield(L, 1, "pAttachments");
    if (!lua_isnil(L, -1)) {
        luaL_checktype(L, -1, LUA_TTABLE);
        create_info.attachmentCount = lua_rawlen(L, -1);
        if (create_info.attachmentCount > 0) {
            attachments = (VkImageView*)malloc(create_info.attachmentCount * sizeof(VkImageView));
            if (!attachments) {
                luaL_error(L, "Failed to allocate memory for framebuffer attachments");
            }
            for (uint32_t i = 0; i < create_info.attachmentCount; i++) {
                lua_rawgeti(L, -1, i + 1);
                lua_VkImageView* view_ud = lua_check_VkImageView(L, -1);
                attachments[i] = view_ud->image_view;
                lua_pop(L, 1);
            }
            create_info.pAttachments = attachments;
        }
    }
    lua_pop(L, 1);

    lua_getfield(L, 1, "width");
    if (!lua_isnil(L, -1)) {
        create_info.width = (uint32_t)luaL_checkinteger(L, -1);
    }
    lua_pop(L, 1);

    lua_getfield(L, 1, "height");
    if (!lua_isnil(L, -1)) {
        create_info.height = (uint32_t)luaL_checkinteger(L, -1);
    }
    lua_pop(L, 1);

    lua_push_VkFramebufferCreateInfo(L, &create_info, attachments);
    return 1;
}

static int l_vulkan_create_framebuffer(lua_State* L) {
    lua_VkDevice* device_ud = lua_check_VkDevice(L, 1);
    lua_VkFramebufferCreateInfo* create_info_ud = lua_check_VkFramebufferCreateInfo(L, 2);
    luaL_checktype(L, 3, LUA_TNIL);

    VkFramebuffer framebuffer;
    VkResult result = vkCreateFramebuffer(device_ud->device, &create_info_ud->create_info, NULL, &framebuffer);
    if (result != VK_SUCCESS) {
        luaL_error(L, "Failed to create framebuffer: VkResult %d", result);
    }

    lua_push_VkFramebuffer(L, framebuffer, device_ud->device);
    return 1;
}


// pipeline

// Garbage collection for VkShaderModule
static int shader_module_gc(lua_State* L) {
    lua_VkShaderModule* ud = (lua_VkShaderModule*)luaL_checkudata(L, 1, SHADER_MODULE_MT);
    if (ud->shader_module && ud->device) {
        printf("Cleaning up VkShaderModule\n");
        vkDestroyShaderModule(ud->device, ud->shader_module, NULL);
        ud->shader_module = NULL;
        ud->device = NULL;
    }
    return 0;
}

// Garbage collection for VkPipelineShaderStageCreateInfo (no dynamic allocations)
static int pipeline_shader_stage_create_info_gc(lua_State* L) {
    printf("Cleaning up VkPipelineShaderStageCreateInfo\n");
    return 0;
}

// Garbage collection for VkVertexInputBindingDescription (no dynamic allocations)
static int vertex_input_binding_description_gc(lua_State* L) {
    printf("Cleaning up VkVertexInputBindingDescription\n");
    return 0;
}

// Garbage collection for VkVertexInputAttributeDescription (no dynamic allocations)
static int vertex_input_attribute_description_gc(lua_State* L) {
    printf("Cleaning up VkVertexInputAttributeDescription\n");
    return 0;
}

// Garbage collection for VkPipelineVertexInputStateCreateInfo
static int pipeline_vertex_input_state_create_info_gc(lua_State* L) {
    lua_VkPipelineVertexInputStateCreateInfo* ud = (lua_VkPipelineVertexInputStateCreateInfo*)luaL_checkudata(L, 1, PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO_MT);
    if (ud->pBindings) {
        printf("Cleaning up VkPipelineVertexInputStateCreateInfo bindings\n");
        free(ud->pBindings);
        ud->pBindings = NULL;
    }
    if (ud->pAttributes) {
        printf("Cleaning up VkPipelineVertexInputStateCreateInfo attributes\n");
        free(ud->pAttributes);
        ud->pAttributes = NULL;
    }
    return 0;
}

// Garbage collection for VkPipelineInputAssemblyStateCreateInfo (no dynamic allocations)
static int pipeline_input_assembly_state_create_info_gc(lua_State* L) {
    printf("Cleaning up VkPipelineInputAssemblyStateCreateInfo\n");
    return 0;
}

// Garbage collection for VkViewport (no dynamic allocations)
static int viewport_gc(lua_State* L) {
    printf("Cleaning up VkViewport\n");
    return 0;
}

// Garbage collection for VkRect2D (no dynamic allocations)
static int rect_2d_gc(lua_State* L) {
    printf("Cleaning up VkRect2D\n");
    return 0;
}

// Garbage collection for VkPipelineViewportStateCreateInfo
static int pipeline_viewport_state_create_info_gc(lua_State* L) {
    lua_VkPipelineViewportStateCreateInfo* ud = (lua_VkPipelineViewportStateCreateInfo*)luaL_checkudata(L, 1, PIPELINE_VIEWPORT_STATE_CREATE_INFO_MT);
    if (ud->pViewports) {
        printf("Cleaning up VkPipelineViewportStateCreateInfo viewports\n");
        free(ud->pViewports);
        ud->pViewports = NULL;
    }
    if (ud->pScissors) {
        printf("Cleaning up VkPipelineViewportStateCreateInfo scissors\n");
        free(ud->pScissors);
        ud->pScissors = NULL;
    }
    return 0;
}

// Garbage collection for VkPipelineRasterizationStateCreateInfo (no dynamic allocations)
static int pipeline_rasterization_state_create_info_gc(lua_State* L) {
    printf("Cleaning up VkPipelineRasterizationStateCreateInfo\n");
    return 0;
}

// Garbage collection for VkPipelineMultisampleStateCreateInfo (no dynamic allocations)
static int pipeline_multisample_state_create_info_gc(lua_State* L) {
    printf("Cleaning up VkPipelineMultisampleStateCreateInfo\n");
    return 0;
}

// Garbage collection for VkPipelineColorBlendAttachmentState (no dynamic allocations)
static int pipeline_color_blend_attachment_state_gc(lua_State* L) {
    printf("Cleaning up VkPipelineColorBlendAttachmentState\n");
    return 0;
}

// Garbage collection for VkPipelineColorBlendStateCreateInfo
static int pipeline_color_blend_state_create_info_gc(lua_State* L) {
    lua_VkPipelineColorBlendStateCreateInfo* ud = (lua_VkPipelineColorBlendStateCreateInfo*)luaL_checkudata(L, 1, PIPELINE_COLOR_BLEND_STATE_CREATE_INFO_MT);
    if (ud->pAttachments) {
        printf("Cleaning up VkPipelineColorBlendStateCreateInfo attachments\n");
        free(ud->pAttachments);
        ud->pAttachments = NULL;
    }
    return 0;
}

// Garbage collection for VkPipelineLayoutCreateInfo (no dynamic allocations)
static int pipeline_layout_create_info_gc(lua_State* L) {
    printf("Cleaning up VkPipelineLayoutCreateInfo\n");
    return 0;
}

// Garbage collection for VkPipelineLayout
static int pipeline_layout_gc(lua_State* L) {
    lua_VkPipelineLayout* ud = (lua_VkPipelineLayout*)luaL_checkudata(L, 1, PIPELINE_LAYOUT_MT);
    if (ud->pipeline_layout && ud->device) {
        printf("Cleaning up VkPipelineLayout\n");
        vkDestroyPipelineLayout(ud->device, ud->pipeline_layout, NULL);
        ud->pipeline_layout = NULL;
        ud->device = NULL;
    }
    return 0;
}

// Garbage collection for VkGraphicsPipelineCreateInfo
static int graphics_pipeline_create_info_gc(lua_State* L) {
    lua_VkGraphicsPipelineCreateInfo* ud = (lua_VkGraphicsPipelineCreateInfo*)luaL_checkudata(L, 1, GRAPHICS_PIPELINE_CREATE_INFO_MT);
    if (ud->pStages) {
        printf("Cleaning up VkGraphicsPipelineCreateInfo stages\n");
        free(ud->pStages);
        ud->pStages = NULL;
    }
    return 0;
}

// Garbage collection for VkPipeline
static int pipeline_gc(lua_State* L) {
    lua_VkPipeline* ud = (lua_VkPipeline*)luaL_checkudata(L, 1, PIPELINE_MT);
    if (ud->pipeline && ud->device) {
        printf("Cleaning up VkPipeline\n");
        vkDestroyPipeline(ud->device, ud->pipeline, NULL);
        ud->pipeline = NULL;
        ud->device = NULL;
    }
    return 0;
}

// Push VkExtent2D
void lua_push_VkExtent2D(lua_State* L, VkExtent2D* extent) {
    if (!extent) {
        luaL_error(L, "Cannot create userdata for null VkExtent2D");
    }
    lua_VkExtent2D* ud = (lua_VkExtent2D*)lua_newuserdata(L, sizeof(lua_VkExtent2D));
    ud->extent = *extent;
    luaL_setmetatable(L, EXTENT_2D_MT);
}

// Check VkExtent2D
lua_VkExtent2D* lua_check_VkExtent2D(lua_State* L, int idx) {
    lua_VkExtent2D* ud = (lua_VkExtent2D*)luaL_checkudata(L, idx, EXTENT_2D_MT);
    return ud;
}

// Push VkShaderModule
void lua_push_VkShaderModule(lua_State* L, VkShaderModule shader_module, VkDevice device) {
    if (!shader_module) {
        luaL_error(L, "Cannot create userdata for null VkShaderModule");
    }
    lua_VkShaderModule* ud = (lua_VkShaderModule*)lua_newuserdata(L, sizeof(lua_VkShaderModule));
    ud->shader_module = shader_module;
    ud->device = device;
    luaL_setmetatable(L, SHADER_MODULE_MT);
}

// Check VkShaderModule
lua_VkShaderModule* lua_check_VkShaderModule(lua_State* L, int idx) {
    lua_VkShaderModule* ud = (lua_VkShaderModule*)luaL_checkudata(L, idx, SHADER_MODULE_MT);
    if (!ud->shader_module) {
        luaL_error(L, "Invalid VkShaderModule (already destroyed)");
    }
    return ud;
}

// Push VkPipelineShaderStageCreateInfo
void lua_push_VkPipelineShaderStageCreateInfo(lua_State* L, VkPipelineShaderStageCreateInfo* stage_info) {
    if (!stage_info) {
        luaL_error(L, "Cannot create userdata for null VkPipelineShaderStageCreateInfo");
    }
    lua_VkPipelineShaderStageCreateInfo* ud = (lua_VkPipelineShaderStageCreateInfo*)lua_newuserdata(L, sizeof(lua_VkPipelineShaderStageCreateInfo));
    ud->stage_info = *stage_info;
    luaL_setmetatable(L, PIPELINE_SHADER_STAGE_CREATE_INFO_MT);
}

// Check VkPipelineShaderStageCreateInfo
lua_VkPipelineShaderStageCreateInfo* lua_check_VkPipelineShaderStageCreateInfo(lua_State* L, int idx) {
    lua_VkPipelineShaderStageCreateInfo* ud = (lua_VkPipelineShaderStageCreateInfo*)luaL_checkudata(L, idx, PIPELINE_SHADER_STAGE_CREATE_INFO_MT);
    return ud;
}

// Push VkVertexInputBindingDescription
void lua_push_VkVertexInputBindingDescription(lua_State* L, VkVertexInputBindingDescription* binding) {
    if (!binding) {
        luaL_error(L, "Cannot create userdata for null VkVertexInputBindingDescription");
    }
    lua_VkVertexInputBindingDescription* ud = (lua_VkVertexInputBindingDescription*)lua_newuserdata(L, sizeof(lua_VkVertexInputBindingDescription));
    ud->binding = *binding;
    luaL_setmetatable(L, VERTEX_INPUT_BINDING_DESCRIPTION_MT);
}

// Check VkVertexInputBindingDescription
lua_VkVertexInputBindingDescription* lua_check_VkVertexInputBindingDescription(lua_State* L, int idx) {
    lua_VkVertexInputBindingDescription* ud = (lua_VkVertexInputBindingDescription*)luaL_checkudata(L, idx, VERTEX_INPUT_BINDING_DESCRIPTION_MT);
    return ud;
}

// Push VkVertexInputAttributeDescription
void lua_push_VkVertexInputAttributeDescription(lua_State* L, VkVertexInputAttributeDescription* attrib) {
    if (!attrib) {
        luaL_error(L, "Cannot create userdata for null VkVertexInputAttributeDescription");
    }
    lua_VkVertexInputAttributeDescription* ud = (lua_VkVertexInputAttributeDescription*)lua_newuserdata(L, sizeof(lua_VkVertexInputAttributeDescription));
    ud->attrib = *attrib;
    luaL_setmetatable(L, VERTEX_INPUT_ATTRIBUTE_DESCRIPTION_MT);
}

// Check VkVertexInputAttributeDescription
lua_VkVertexInputAttributeDescription* lua_check_VkVertexInputAttributeDescription(lua_State* L, int idx) {
    lua_VkVertexInputAttributeDescription* ud = (lua_VkVertexInputAttributeDescription*)luaL_checkudata(L, idx, VERTEX_INPUT_ATTRIBUTE_DESCRIPTION_MT);
    return ud;
}

// Push VkPipelineVertexInputStateCreateInfo
void lua_push_VkPipelineVertexInputStateCreateInfo(lua_State* L, VkPipelineVertexInputStateCreateInfo* vertex_input_info, VkVertexInputBindingDescription* bindings, VkVertexInputAttributeDescription* attributes) {
    if (!vertex_input_info) {
        luaL_error(L, "Cannot create userdata for null VkPipelineVertexInputStateCreateInfo");
    }
    lua_VkPipelineVertexInputStateCreateInfo* ud = (lua_VkPipelineVertexInputStateCreateInfo*)lua_newuserdata(L, sizeof(lua_VkPipelineVertexInputStateCreateInfo));
    ud->vertex_input_info = *vertex_input_info;
    ud->pBindings = bindings;
    ud->pAttributes = attributes;
    luaL_setmetatable(L, PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO_MT);
}

// Check VkPipelineVertexInputStateCreateInfo
lua_VkPipelineVertexInputStateCreateInfo* lua_check_VkPipelineVertexInputStateCreateInfo(lua_State* L, int idx) {
    lua_VkPipelineVertexInputStateCreateInfo* ud = (lua_VkPipelineVertexInputStateCreateInfo*)luaL_checkudata(L, idx, PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO_MT);
    return ud;
}

// Push VkPipelineInputAssemblyStateCreateInfo
void lua_push_VkPipelineInputAssemblyStateCreateInfo(lua_State* L, VkPipelineInputAssemblyStateCreateInfo* input_assembly) {
    if (!input_assembly) {
        luaL_error(L, "Cannot create userdata for null VkPipelineInputAssemblyStateCreateInfo");
    }
    lua_VkPipelineInputAssemblyStateCreateInfo* ud = (lua_VkPipelineInputAssemblyStateCreateInfo*)lua_newuserdata(L, sizeof(lua_VkPipelineInputAssemblyStateCreateInfo));
    ud->input_assembly = *input_assembly;
    luaL_setmetatable(L, PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO_MT);
}

// Check VkPipelineInputAssemblyStateCreateInfo
lua_VkPipelineInputAssemblyStateCreateInfo* lua_check_VkPipelineInputAssemblyStateCreateInfo(lua_State* L, int idx) {
    lua_VkPipelineInputAssemblyStateCreateInfo* ud = (lua_VkPipelineInputAssemblyStateCreateInfo*)luaL_checkudata(L, idx, PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO_MT);
    return ud;
}

// Push VkViewport
void lua_push_VkViewport(lua_State* L, VkViewport* viewport) {
    if (!viewport) {
        luaL_error(L, "Cannot create userdata for null VkViewport");
    }
    lua_VkViewport* ud = (lua_VkViewport*)lua_newuserdata(L, sizeof(lua_VkViewport));
    ud->viewport = *viewport;
    luaL_setmetatable(L, VIEWPORT_MT);
}

// Check VkViewport
lua_VkViewport* lua_check_VkViewport(lua_State* L, int idx) {
    lua_VkViewport* ud = (lua_VkViewport*)luaL_checkudata(L, idx, VIEWPORT_MT);
    return ud;
}

// Push VkRect2D
void lua_push_VkRect2D(lua_State* L, VkRect2D* scissor) {
    if (!scissor) {
        luaL_error(L, "Cannot create userdata for null VkRect2D");
    }
    lua_VkRect2D* ud = (lua_VkRect2D*)lua_newuserdata(L, sizeof(lua_VkRect2D));
    ud->scissor = *scissor;
    luaL_setmetatable(L, RECT_2D_MT);
}

// Check VkRect2D
lua_VkRect2D* lua_check_VkRect2D(lua_State* L, int idx) {
    lua_VkRect2D* ud = (lua_VkRect2D*)luaL_checkudata(L, idx, RECT_2D_MT);
    return ud;
}

// Push VkPipelineViewportStateCreateInfo
void lua_push_VkPipelineViewportStateCreateInfo(lua_State* L, VkPipelineViewportStateCreateInfo* viewport_state, VkViewport* viewports, VkRect2D* scissors) {
    if (!viewport_state) {
        luaL_error(L, "Cannot create userdata for null VkPipelineViewportStateCreateInfo");
    }
    lua_VkPipelineViewportStateCreateInfo* ud = (lua_VkPipelineViewportStateCreateInfo*)lua_newuserdata(L, sizeof(lua_VkPipelineViewportStateCreateInfo));
    ud->viewport_state = *viewport_state;
    ud->pViewports = viewports;
    ud->pScissors = scissors;
    luaL_setmetatable(L, PIPELINE_VIEWPORT_STATE_CREATE_INFO_MT);
}

// Check VkPipelineViewportStateCreateInfo
lua_VkPipelineViewportStateCreateInfo* lua_check_VkPipelineViewportStateCreateInfo(lua_State* L, int idx) {
    lua_VkPipelineViewportStateCreateInfo* ud = (lua_VkPipelineViewportStateCreateInfo*)luaL_checkudata(L, idx, PIPELINE_VIEWPORT_STATE_CREATE_INFO_MT);
    return ud;
}

// Push VkPipelineRasterizationStateCreateInfo
void lua_push_VkPipelineRasterizationStateCreateInfo(lua_State* L, VkPipelineRasterizationStateCreateInfo* rasterizer) {
    if (!rasterizer) {
        luaL_error(L, "Cannot create userdata for null VkPipelineRasterizationStateCreateInfo");
    }
    lua_VkPipelineRasterizationStateCreateInfo* ud = (lua_VkPipelineRasterizationStateCreateInfo*)lua_newuserdata(L, sizeof(lua_VkPipelineRasterizationStateCreateInfo));
    ud->rasterizer = *rasterizer;
    luaL_setmetatable(L, PIPELINE_RASTERIZATION_STATE_CREATE_INFO_MT);
}

// Check VkPipelineRasterizationStateCreateInfo
lua_VkPipelineRasterizationStateCreateInfo* lua_check_VkPipelineRasterizationStateCreateInfo(lua_State* L, int idx) {
    lua_VkPipelineRasterizationStateCreateInfo* ud = (lua_VkPipelineRasterizationStateCreateInfo*)luaL_checkudata(L, idx, PIPELINE_RASTERIZATION_STATE_CREATE_INFO_MT);
    return ud;
}

// Push VkPipelineMultisampleStateCreateInfo
void lua_push_VkPipelineMultisampleStateCreateInfo(lua_State* L, VkPipelineMultisampleStateCreateInfo* multisampling) {
    if (!multisampling) {
        luaL_error(L, "Cannot create userdata for null VkPipelineMultisampleStateCreateInfo");
    }
    lua_VkPipelineMultisampleStateCreateInfo* ud = (lua_VkPipelineMultisampleStateCreateInfo*)lua_newuserdata(L, sizeof(lua_VkPipelineMultisampleStateCreateInfo));
    ud->multisampling = *multisampling;
    luaL_setmetatable(L, PIPELINE_MULTISAMPLE_STATE_CREATE_INFO_MT);
}

// Check VkPipelineMultisampleStateCreateInfo
lua_VkPipelineMultisampleStateCreateInfo* lua_check_VkPipelineMultisampleStateCreateInfo(lua_State* L, int idx) {
    lua_VkPipelineMultisampleStateCreateInfo* ud = (lua_VkPipelineMultisampleStateCreateInfo*)luaL_checkudata(L, idx, PIPELINE_MULTISAMPLE_STATE_CREATE_INFO_MT);
    return ud;
}

// Push VkPipelineColorBlendAttachmentState
void lua_push_VkPipelineColorBlendAttachmentState(lua_State* L, VkPipelineColorBlendAttachmentState* blend_attachment) {
    if (!blend_attachment) {
        luaL_error(L, "Cannot create userdata for null VkPipelineColorBlendAttachmentState");
    }
    lua_VkPipelineColorBlendAttachmentState* ud = (lua_VkPipelineColorBlendAttachmentState*)lua_newuserdata(L, sizeof(lua_VkPipelineColorBlendAttachmentState));
    ud->blend_attachment = *blend_attachment;
    luaL_setmetatable(L, PIPELINE_COLOR_BLEND_ATTACHMENT_STATE_MT);
}

// Check VkPipelineColorBlendAttachmentState
lua_VkPipelineColorBlendAttachmentState* lua_check_VkPipelineColorBlendAttachmentState(lua_State* L, int idx) {
    lua_VkPipelineColorBlendAttachmentState* ud = (lua_VkPipelineColorBlendAttachmentState*)luaL_checkudata(L, idx, PIPELINE_COLOR_BLEND_ATTACHMENT_STATE_MT);
    return ud;
}

// Push VkPipelineColorBlendStateCreateInfo
void lua_push_VkPipelineColorBlendStateCreateInfo(lua_State* L, VkPipelineColorBlendStateCreateInfo* blend_state, VkPipelineColorBlendAttachmentState* attachments) {
    if (!blend_state) {
        luaL_error(L, "Cannot create userdata for null VkPipelineColorBlendStateCreateInfo");
    }
    lua_VkPipelineColorBlendStateCreateInfo* ud = (lua_VkPipelineColorBlendStateCreateInfo*)lua_newuserdata(L, sizeof(lua_VkPipelineColorBlendStateCreateInfo));
    ud->blend_state = *blend_state;
    ud->pAttachments = attachments;
    luaL_setmetatable(L, PIPELINE_COLOR_BLEND_STATE_CREATE_INFO_MT);
}

// Check VkPipelineColorBlendStateCreateInfo
lua_VkPipelineColorBlendStateCreateInfo* lua_check_VkPipelineColorBlendStateCreateInfo(lua_State* L, int idx) {
    lua_VkPipelineColorBlendStateCreateInfo* ud = (lua_VkPipelineColorBlendStateCreateInfo*)luaL_checkudata(L, idx, PIPELINE_COLOR_BLEND_STATE_CREATE_INFO_MT);
    return ud;
}

// Push VkPipelineLayoutCreateInfo
void lua_push_VkPipelineLayoutCreateInfo(lua_State* L, VkPipelineLayoutCreateInfo* layout_info) {
    if (!layout_info) {
        luaL_error(L, "Cannot create userdata for null VkPipelineLayoutCreateInfo");
    }
    lua_VkPipelineLayoutCreateInfo* ud = (lua_VkPipelineLayoutCreateInfo*)lua_newuserdata(L, sizeof(lua_VkPipelineLayoutCreateInfo));
    ud->layout_info = *layout_info;
    luaL_setmetatable(L, PIPELINE_LAYOUT_CREATE_INFO_MT);
}

// Check VkPipelineLayoutCreateInfo
lua_VkPipelineLayoutCreateInfo* lua_check_VkPipelineLayoutCreateInfo(lua_State* L, int idx) {
    lua_VkPipelineLayoutCreateInfo* ud = (lua_VkPipelineLayoutCreateInfo*)luaL_checkudata(L, idx, PIPELINE_LAYOUT_CREATE_INFO_MT);
    return ud;
}

// Push VkPipelineLayout
void lua_push_VkPipelineLayout(lua_State* L, VkPipelineLayout pipeline_layout, VkDevice device) {
    if (!pipeline_layout) {
        luaL_error(L, "Cannot create userdata for null VkPipelineLayout");
    }
    lua_VkPipelineLayout* ud = (lua_VkPipelineLayout*)lua_newuserdata(L, sizeof(lua_VkPipelineLayout));
    ud->pipeline_layout = pipeline_layout;
    ud->device = device;
    luaL_setmetatable(L, PIPELINE_LAYOUT_MT);
}

// Check VkPipelineLayout
lua_VkPipelineLayout* lua_check_VkPipelineLayout(lua_State* L, int idx) {
    lua_VkPipelineLayout* ud = (lua_VkPipelineLayout*)luaL_checkudata(L, idx, PIPELINE_LAYOUT_MT);
    if (!ud->pipeline_layout) {
        luaL_error(L, "Invalid VkPipelineLayout (already destroyed)");
    }
    return ud;
}

// Push VkGraphicsPipelineCreateInfo
void lua_push_VkGraphicsPipelineCreateInfo(lua_State* L, VkGraphicsPipelineCreateInfo* pipeline_info, VkPipelineShaderStageCreateInfo* stages) {
    if (!pipeline_info) {
        luaL_error(L, "Cannot create userdata for null VkGraphicsPipelineCreateInfo");
    }
    lua_VkGraphicsPipelineCreateInfo* ud = (lua_VkGraphicsPipelineCreateInfo*)lua_newuserdata(L, sizeof(lua_VkGraphicsPipelineCreateInfo));
    ud->pipeline_info = *pipeline_info;
    ud->pStages = stages;
    luaL_setmetatable(L, GRAPHICS_PIPELINE_CREATE_INFO_MT);
}

// Check VkGraphicsPipelineCreateInfo
lua_VkGraphicsPipelineCreateInfo* lua_check_VkGraphicsPipelineCreateInfo(lua_State* L, int idx) {
    lua_VkGraphicsPipelineCreateInfo* ud = (lua_VkGraphicsPipelineCreateInfo*)luaL_checkudata(L, idx, GRAPHICS_PIPELINE_CREATE_INFO_MT);
    return ud;
}

// Push VkPipeline
void lua_push_VkPipeline(lua_State* L, VkPipeline pipeline, VkDevice device) {
    if (!pipeline) {
        luaL_error(L, "Cannot create userdata for null VkPipeline");
    }
    lua_VkPipeline* ud = (lua_VkPipeline*)lua_newuserdata(L, sizeof(lua_VkPipeline));
    ud->pipeline = pipeline;
    ud->device = device;
    luaL_setmetatable(L, PIPELINE_MT);
}

// Check VkPipeline
lua_VkPipeline* lua_check_VkPipeline(lua_State* L, int idx) {
    lua_VkPipeline* ud = (lua_VkPipeline*)luaL_checkudata(L, idx, PIPELINE_MT);
    if (!ud->pipeline) {
        luaL_error(L, "Invalid VkPipeline (already destroyed)");
    }
    return ud;
}

// Create VkExtent2D: vulkan.create_extent_2d(table)
static int l_vulkan_create_extent_2d(lua_State* L) {
    luaL_checktype(L, 1, LUA_TTABLE);

    VkExtent2D extent = {0, 0};

    lua_getfield(L, 1, "width");
    if (!lua_isnil(L, -1)) {
        extent.width = (uint32_t)luaL_checkinteger(L, -1);
    }
    lua_pop(L, 1);

    lua_getfield(L, 1, "height");
    if (!lua_isnil(L, -1)) {
        extent.height = (uint32_t)luaL_checkinteger(L, -1);
    }
    lua_pop(L, 1);

    lua_push_VkExtent2D(L, &extent);
    return 1;
}

// Create VkShaderModule: vulkan.create_shader_module(device, code, shader_kind)
static int l_vulkan_create_shader_module(lua_State* L) {
    lua_VkDevice* device_ud = lua_check_VkDevice(L, 1);
    const char* code = luaL_checkstring(L, 2);
    shaderc_shader_kind kind = (shaderc_shader_kind)luaL_checkinteger(L, 3);

    shaderc_compiler_t compiler = shaderc_compiler_initialize();
    if (!compiler) {
        luaL_error(L, "Failed to initialize shaderc compiler");
    }

    shaderc_compilation_result_t result = shaderc_compile_into_spv(
        compiler,
        code,
        strlen(code),
        kind,
        "shader.glsl",
        "main",
        NULL
    );

    if (shaderc_result_get_compilation_status(result) != shaderc_compilation_status_success) {
        const char* error_msg = shaderc_result_get_error_message(result);
        luaL_error(L, "Shader compilation failed: %s", error_msg);
        shaderc_result_release(result);
        shaderc_compiler_release(compiler);
        return 0;
    }

    size_t code_size = shaderc_result_get_length(result);
    const char* spv_code = shaderc_result_get_bytes(result);

    VkShaderModuleCreateInfo create_info = {
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .pNext = NULL,
        .flags = 0,
        .codeSize = code_size,
        .pCode = (const uint32_t*)spv_code
    };

    VkShaderModule shader_module;
    VkResult vk_result = vkCreateShaderModule(device_ud->device, &create_info, NULL, &shader_module);
    shaderc_result_release(result);
    shaderc_compiler_release(compiler);

    if (vk_result != VK_SUCCESS) {
        luaL_error(L, "Failed to create shader module: VkResult %d", vk_result);
    }

    lua_push_VkShaderModule(L, shader_module, device_ud->device);
    return 1;
}

// Create VkPipelineShaderStageCreateInfo: vulkan.create_pipeline_shader_stage_create_info(table)
static int l_vulkan_create_pipeline_shader_stage_create_info(lua_State* L) {
    luaL_checktype(L, 1, LUA_TTABLE);

    VkPipelineShaderStageCreateInfo stage_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .pNext = NULL,
        .flags = 0,
        .stage = 0,
        .module = VK_NULL_HANDLE,
        .pName = "main",
        .pSpecializationInfo = NULL
    };

    lua_getfield(L, 1, "stage");
    if (!lua_isnil(L, -1)) {
        stage_info.stage = (VkShaderStageFlagBits)luaL_checkinteger(L, -1);
    }
    lua_pop(L, 1);

    lua_getfield(L, 1, "module");
    if (!lua_isnil(L, -1)) {
        lua_VkShaderModule* module_ud = lua_check_VkShaderModule(L, -1);
        stage_info.module = module_ud->shader_module;
    }
    lua_pop(L, 1);

    lua_push_VkPipelineShaderStageCreateInfo(L, &stage_info);
    return 1;
}

// Create VkVertexInputBindingDescription: vulkan.create_vertex_input_binding_description(table)
static int l_vulkan_create_vertex_input_binding_description(lua_State* L) {
    luaL_checktype(L, 1, LUA_TTABLE);

    VkVertexInputBindingDescription binding = {
        .binding = 0,
        .stride = 0,
        .inputRate = VK_VERTEX_INPUT_RATE_VERTEX
    };

    lua_getfield(L, 1, "binding");
    if (!lua_isnil(L, -1)) {
        binding.binding = (uint32_t)luaL_checkinteger(L, -1);
    }
    lua_pop(L, 1);

    lua_getfield(L, 1, "stride");
    if (!lua_isnil(L, -1)) {
        binding.stride = (uint32_t)luaL_checkinteger(L, -1);
    }
    lua_pop(L, 1);

    lua_getfield(L, 1, "inputRate");
    if (!lua_isnil(L, -1)) {
        binding.inputRate = (VkVertexInputRate)luaL_checkinteger(L, -1);
    }
    lua_pop(L, 1);

    lua_push_VkVertexInputBindingDescription(L, &binding);
    return 1;
}

// Create VkVertexInputAttributeDescription: vulkan.create_vertex_input_attribute_description(table)
static int l_vulkan_create_vertex_input_attribute_description(lua_State* L) {
    luaL_checktype(L, 1, LUA_TTABLE);

    VkVertexInputAttributeDescription attrib = {
        .location = 0,
        .binding = 0,
        .format = VK_FORMAT_UNDEFINED,
        .offset = 0
    };

    lua_getfield(L, 1, "location");
    if (!lua_isnil(L, -1)) {
        attrib.location = (uint32_t)luaL_checkinteger(L, -1);
    }
    lua_pop(L, 1);

    lua_getfield(L, 1, "binding");
    if (!lua_isnil(L, -1)) {
        attrib.binding = (uint32_t)luaL_checkinteger(L, -1);
    }
    lua_pop(L, 1);

    lua_getfield(L, 1, "format");
    if (!lua_isnil(L, -1)) {
        attrib.format = (VkFormat)luaL_checkinteger(L, -1);
    }
    lua_pop(L, 1);

    lua_getfield(L, 1, "offset");
    if (!lua_isnil(L, -1)) {
        attrib.offset = (uint32_t)luaL_checkinteger(L, -1);
    }
    lua_pop(L, 1);

    lua_push_VkVertexInputAttributeDescription(L, &attrib);
    return 1;
}

// Create VkPipelineVertexInputStateCreateInfo: vulkan.create_pipeline_vertex_input_state_create_info(table)
static int l_vulkan_create_pipeline_vertex_input_state_create_info(lua_State* L) {
    luaL_checktype(L, 1, LUA_TTABLE);

    VkPipelineVertexInputStateCreateInfo vertex_input_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        .pNext = NULL,
        .flags = 0,
        .vertexBindingDescriptionCount = 0,
        .pVertexBindingDescriptions = NULL,
        .vertexAttributeDescriptionCount = 0,
        .pVertexAttributeDescriptions = NULL
    };

    VkVertexInputBindingDescription* bindings = NULL;
    VkVertexInputAttributeDescription* attributes = NULL;

    lua_getfield(L, 1, "pVertexBindingDescriptions");
    if (!lua_isnil(L, -1)) {
        luaL_checktype(L, -1, LUA_TTABLE);
        vertex_input_info.vertexBindingDescriptionCount = lua_rawlen(L, -1);
        if (vertex_input_info.vertexBindingDescriptionCount > 0) {
            bindings = (VkVertexInputBindingDescription*)malloc(vertex_input_info.vertexBindingDescriptionCount * sizeof(VkVertexInputBindingDescription));
            if (!bindings) {
                luaL_error(L, "Failed to allocate memory for vertex binding descriptions");
            }
            for (uint32_t i = 0; i < vertex_input_info.vertexBindingDescriptionCount; i++) {
                lua_rawgeti(L, -1, i + 1);
                lua_VkVertexInputBindingDescription* binding_ud = lua_check_VkVertexInputBindingDescription(L, -1);
                bindings[i] = binding_ud->binding;
                lua_pop(L, 1);
            }
            vertex_input_info.pVertexBindingDescriptions = bindings;
        }
    }
    lua_pop(L, 1);

    lua_getfield(L, 1, "pVertexAttributeDescriptions");
    if (!lua_isnil(L, -1)) {
        luaL_checktype(L, -1, LUA_TTABLE);
        vertex_input_info.vertexAttributeDescriptionCount = lua_rawlen(L, -1);
        if (vertex_input_info.vertexAttributeDescriptionCount > 0) {
            attributes = (VkVertexInputAttributeDescription*)malloc(vertex_input_info.vertexAttributeDescriptionCount * sizeof(VkVertexInputAttributeDescription));
            if (!attributes) {
                if (bindings) free(bindings);
                luaL_error(L, "Failed to allocate memory for vertex attribute descriptions");
            }
            for (uint32_t i = 0; i < vertex_input_info.vertexAttributeDescriptionCount; i++) {
                lua_rawgeti(L, -1, i + 1);
                lua_VkVertexInputAttributeDescription* attrib_ud = lua_check_VkVertexInputAttributeDescription(L, -1);
                attributes[i] = attrib_ud->attrib;
                lua_pop(L, 1);
            }
            vertex_input_info.pVertexAttributeDescriptions = attributes;
        }
    }
    lua_pop(L, 1);

    lua_push_VkPipelineVertexInputStateCreateInfo(L, &vertex_input_info, bindings, attributes);
    return 1;
}

// Create VkPipelineInputAssemblyStateCreateInfo: vulkan.create_pipeline_input_assembly_state_create_info(table)
static int l_vulkan_create_pipeline_input_assembly_state_create_info(lua_State* L) {
    luaL_checktype(L, 1, LUA_TTABLE);

    VkPipelineInputAssemblyStateCreateInfo input_assembly = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        .pNext = NULL,
        .flags = 0,
        .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
        .primitiveRestartEnable = VK_FALSE
    };

    lua_getfield(L, 1, "topology");
    if (!lua_isnil(L, -1)) {
        input_assembly.topology = (VkPrimitiveTopology)luaL_checkinteger(L, -1);
    }
    lua_pop(L, 1);

    lua_getfield(L, 1, "primitiveRestartEnable");
    if (!lua_isnil(L, -1)) {
        input_assembly.primitiveRestartEnable = (VkBool32)lua_toboolean(L, -1);
    }
    lua_pop(L, 1);

    lua_push_VkPipelineInputAssemblyStateCreateInfo(L, &input_assembly);
    return 1;
}

// Create VkViewport: vulkan.create_viewport(table)
static int l_vulkan_create_viewport(lua_State* L) {
    luaL_checktype(L, 1, LUA_TTABLE);

    VkViewport viewport = {
        .x = 0.0f,
        .y = 0.0f,
        .width = 0.0f,
        .height = 0.0f,
        .minDepth = 0.0f,
        .maxDepth = 1.0f
    };

    lua_getfield(L, 1, "x");
    if (!lua_isnil(L, -1)) {
        viewport.x = (float)luaL_checknumber(L, -1);
    }
    lua_pop(L, 1);

    lua_getfield(L, 1, "y");
    if (!lua_isnil(L, -1)) {
        viewport.y = (float)luaL_checknumber(L, -1);
    }
    lua_pop(L, 1);

    lua_getfield(L, 1, "width");
    if (!lua_isnil(L, -1)) {
        viewport.width = (float)luaL_checknumber(L, -1);
    }
    lua_pop(L, 1);

    lua_getfield(L, 1, "height");
    if (!lua_isnil(L, -1)) {
        viewport.height = (float)luaL_checknumber(L, -1);
    }
    lua_pop(L, 1);

    lua_getfield(L, 1, "minDepth");
    if (!lua_isnil(L, -1)) {
        viewport.minDepth = (float)luaL_checknumber(L, -1);
    }
    lua_pop(L, 1);

    lua_getfield(L, 1, "maxDepth");
    if (!lua_isnil(L, -1)) {
        viewport.maxDepth = (float)luaL_checknumber(L, -1);
    }
    lua_pop(L, 1);

    lua_push_VkViewport(L, &viewport);
    return 1;
}

// Create VkRect2D: vulkan.create_rect_2d(table)
static int l_vulkan_create_rect_2d(lua_State* L) {
    luaL_checktype(L, 1, LUA_TTABLE);

    VkRect2D scissor = {
        .offset = {0, 0},
        .extent = {0, 0}
    };

    lua_getfield(L, 1, "offset");
    if (!lua_isnil(L, -1)) {
        luaL_checktype(L, -1, LUA_TTABLE);
        lua_getfield(L, -1, "x");
        scissor.offset.x = (int32_t)luaL_checkinteger(L, -1);
        lua_pop(L, 1);
        lua_getfield(L, -1, "y");
        scissor.offset.y = (int32_t)luaL_checkinteger(L, -1);
        lua_pop(L, 1);
    }
    lua_pop(L, 1);

    lua_getfield(L, 1, "extent");
    if (!lua_isnil(L, -1)) {
        lua_VkExtent2D* extent_ud = lua_check_VkExtent2D(L, -1);
        scissor.extent = extent_ud->extent;
    }
    lua_pop(L, 1);

    lua_push_VkRect2D(L, &scissor);
    return 1;
}

// Create VkPipelineViewportStateCreateInfo: vulkan.create_pipeline_viewport_state_create_info(table)
static int l_vulkan_create_pipeline_viewport_state_create_info(lua_State* L) {
    luaL_checktype(L, 1, LUA_TTABLE);

    VkPipelineViewportStateCreateInfo viewport_state = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        .pNext = NULL,
        .flags = 0,
        .viewportCount = 0,
        .pViewports = NULL,
        .scissorCount = 0,
        .pScissors = NULL
    };

    VkViewport* viewports = NULL;
    VkRect2D* scissors = NULL;

    lua_getfield(L, 1, "pViewports");
    if (!lua_isnil(L, -1)) {
        luaL_checktype(L, -1, LUA_TTABLE);
        viewport_state.viewportCount = lua_rawlen(L, -1);
        if (viewport_state.viewportCount > 0) {
            viewports = (VkViewport*)malloc(viewport_state.viewportCount * sizeof(VkViewport));
            if (!viewports) {
                luaL_error(L, "Failed to allocate memory for viewports");
            }
            for (uint32_t i = 0; i < viewport_state.viewportCount; i++) {
                lua_rawgeti(L, -1, i + 1);
                lua_VkViewport* viewport_ud = lua_check_VkViewport(L, -1);
                viewports[i] = viewport_ud->viewport;
                lua_pop(L, 1);
            }
            viewport_state.pViewports = viewports;
        }
    }
    lua_pop(L, 1);

    lua_getfield(L, 1, "pScissors");
    if (!lua_isnil(L, -1)) {
        luaL_checktype(L, -1, LUA_TTABLE);
        viewport_state.scissorCount = lua_rawlen(L, -1);
        if (viewport_state.scissorCount > 0) {
            scissors = (VkRect2D*)malloc(viewport_state.scissorCount * sizeof(VkRect2D));
            if (!scissors) {
                if (viewports) free(viewports);
                luaL_error(L, "Failed to allocate memory for scissors");
            }
            for (uint32_t i = 0; i < viewport_state.scissorCount; i++) {
                lua_rawgeti(L, -1, i + 1);
                lua_VkRect2D* scissor_ud = lua_check_VkRect2D(L, -1);
                scissors[i] = scissor_ud->scissor;
                lua_pop(L, 1);
            }
            viewport_state.pScissors = scissors;
        }
    }
    lua_pop(L, 1);

    lua_push_VkPipelineViewportStateCreateInfo(L, &viewport_state, viewports, scissors);
    return 1;
}

// Create VkPipelineRasterizationStateCreateInfo: vulkan.create_pipeline_rasterization_state_create_info(table)
static int l_vulkan_create_pipeline_rasterization_state_create_info(lua_State* L) {
    luaL_checktype(L, 1, LUA_TTABLE);

    VkPipelineRasterizationStateCreateInfo rasterizer = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
        .pNext = NULL,
        .flags = 0,
        .depthClampEnable = VK_FALSE,
        .rasterizerDiscardEnable = VK_FALSE,
        .polygonMode = VK_POLYGON_MODE_FILL,
        .cullMode = VK_CULL_MODE_NONE,
        .frontFace = VK_FRONT_FACE_CLOCKWISE,
        .depthBiasEnable = VK_FALSE,
        .depthBiasConstantFactor = 0.0f,
        .depthBiasClamp = 0.0f,
        .depthBiasSlopeFactor = 0.0f,
        .lineWidth = 1.0f
    };

    lua_getfield(L, 1, "polygonMode");
    if (!lua_isnil(L, -1)) {
        rasterizer.polygonMode = (VkPolygonMode)luaL_checkinteger(L, -1);
    }
    lua_pop(L, 1);

    lua_getfield(L, 1, "cullMode");
    if (!lua_isnil(L, -1)) {
        rasterizer.cullMode = (VkCullModeFlags)luaL_checkinteger(L, -1);
    }
    lua_pop(L, 1);

    lua_getfield(L, 1, "frontFace");
    if (!lua_isnil(L, -1)) {
        rasterizer.frontFace = (VkFrontFace)luaL_checkinteger(L, -1);
    }
    lua_pop(L, 1);

    lua_getfield(L, 1, "lineWidth");
    if (!lua_isnil(L, -1)) {
        rasterizer.lineWidth = (float)luaL_checknumber(L, -1);
    }
    lua_pop(L, 1);

    lua_push_VkPipelineRasterizationStateCreateInfo(L, &rasterizer);
    return 1;
}

// Create VkPipelineMultisampleStateCreateInfo: vulkan.create_pipeline_multisample_state_create_info(table)
static int l_vulkan_create_pipeline_multisample_state_create_info(lua_State* L) {
    luaL_checktype(L, 1, LUA_TTABLE);

    VkPipelineMultisampleStateCreateInfo multisampling = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
        .pNext = NULL,
        .flags = 0,
        .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
        .sampleShadingEnable = VK_FALSE,
        .minSampleShading = 0.0f,
        .pSampleMask = NULL,
        .alphaToCoverageEnable = VK_FALSE,
        .alphaToOneEnable = VK_FALSE
    };

    lua_getfield(L, 1, "rasterizationSamples");
    if (!lua_isnil(L, -1)) {
        multisampling.rasterizationSamples = (VkSampleCountFlagBits)luaL_checkinteger(L, -1);
    }
    lua_pop(L, 1);

    lua_getfield(L, 1, "sampleShadingEnable");
    if (!lua_isnil(L, -1)) {
        multisampling.sampleShadingEnable = (VkBool32)lua_toboolean(L, -1);
    }
    lua_pop(L, 1);

    lua_push_VkPipelineMultisampleStateCreateInfo(L, &multisampling);
    return 1;
}

// Create VkPipelineColorBlendAttachmentState: vulkan.create_pipeline_color_blend_attachment_state(table)
static int l_vulkan_create_pipeline_color_blend_attachment_state(lua_State* L) {
    luaL_checktype(L, 1, LUA_TTABLE);

    VkPipelineColorBlendAttachmentState blend_attachment = {
        .blendEnable = VK_FALSE,
        .srcColorBlendFactor = VK_BLEND_FACTOR_ONE,
        .dstColorBlendFactor = VK_BLEND_FACTOR_ZERO,
        .colorBlendOp = VK_BLEND_OP_ADD,
        .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
        .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
        .alphaBlendOp = VK_BLEND_OP_ADD,
        .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT
    };

    lua_getfield(L, 1, "blendEnable");
    if (!lua_isnil(L, -1)) {
        blend_attachment.blendEnable = (VkBool32)lua_toboolean(L, -1);
    }
    lua_pop(L, 1);

    lua_getfield(L, 1, "colorWriteMask");
    if (!lua_isnil(L, -1)) {
        blend_attachment.colorWriteMask = (VkColorComponentFlags)luaL_checkinteger(L, -1);
    }
    lua_pop(L, 1);

    lua_push_VkPipelineColorBlendAttachmentState(L, &blend_attachment);
    return 1;
}

// Create VkPipelineColorBlendStateCreateInfo: vulkan.create_pipeline_color_blend_state_create_info(table)
static int l_vulkan_create_pipeline_color_blend_state_create_info(lua_State* L) {
    luaL_checktype(L, 1, LUA_TTABLE);

    VkPipelineColorBlendStateCreateInfo blend_state = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
        .pNext = NULL,
        .flags = 0,
        .logicOpEnable = VK_FALSE,
        .logicOp = VK_LOGIC_OP_COPY,
        .attachmentCount = 0,
        .pAttachments = NULL,
        .blendConstants = {0.0f, 0.0f, 0.0f, 0.0f}
    };

    VkPipelineColorBlendAttachmentState* attachments = NULL;

    lua_getfield(L, 1, "pAttachments");
    if (!lua_isnil(L, -1)) {
        luaL_checktype(L, -1, LUA_TTABLE);
        blend_state.attachmentCount = lua_rawlen(L, -1);
        if (blend_state.attachmentCount > 0) {
            attachments = (VkPipelineColorBlendAttachmentState*)malloc(blend_state.attachmentCount * sizeof(VkPipelineColorBlendAttachmentState));
            if (!attachments) {
                luaL_error(L, "Failed to allocate memory for blend attachments");
            }
            for (uint32_t i = 0; i < blend_state.attachmentCount; i++) {
                lua_rawgeti(L, -1, i + 1);
                lua_VkPipelineColorBlendAttachmentState* attachment_ud = lua_check_VkPipelineColorBlendAttachmentState(L, -1);
                attachments[i] = attachment_ud->blend_attachment;
                lua_pop(L, 1);
            }
            blend_state.pAttachments = attachments;
        }
    }
    lua_pop(L, 1);

    lua_push_VkPipelineColorBlendStateCreateInfo(L, &blend_state, attachments);
    return 1;
}

// Create VkPipelineLayoutCreateInfo: vulkan.create_pipeline_layout_create_info(table)
static int l_vulkan_create_pipeline_layout_create_info(lua_State* L) {
    luaL_checktype(L, 1, LUA_TTABLE);

    VkPipelineLayoutCreateInfo layout_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .pNext = NULL,
        .flags = 0,
        .setLayoutCount = 0,
        .pSetLayouts = NULL,
        .pushConstantRangeCount = 0,
        .pPushConstantRanges = NULL
    };

    lua_push_VkPipelineLayoutCreateInfo(L, &layout_info);
    return 1;
}

// Create VkPipelineLayout: vulkan.create_pipeline_layout(device, layout_info, pAllocator)
static int l_vulkan_create_pipeline_layout(lua_State* L) {
    lua_VkDevice* device_ud = lua_check_VkDevice(L, 1);
    lua_VkPipelineLayoutCreateInfo* layout_info_ud = lua_check_VkPipelineLayoutCreateInfo(L, 2);
    luaL_checktype(L, 3, LUA_TNIL);

    VkPipelineLayout pipeline_layout;
    VkResult result = vkCreatePipelineLayout(device_ud->device, &layout_info_ud->layout_info, NULL, &pipeline_layout);
    if (result != VK_SUCCESS) {
        luaL_error(L, "Failed to create pipeline layout: VkResult %d", result);
    }

    lua_push_VkPipelineLayout(L, pipeline_layout, device_ud->device);
    return 1;
}

// Create VkGraphicsPipelineCreateInfo: vulkan.create_graphics_pipeline_create_info(table)
static int l_vulkan_create_graphics_pipeline_create_info(lua_State* L) {
    luaL_checktype(L, 1, LUA_TTABLE);

    VkGraphicsPipelineCreateInfo pipeline_info = {
        .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .pNext = NULL,
        .flags = 0,
        .stageCount = 0,
        .pStages = NULL,
        .pVertexInputState = NULL,
        .pInputAssemblyState = NULL,
        .pTessellationState = NULL,
        .pViewportState = NULL,
        .pRasterizationState = NULL,
        .pMultisampleState = NULL,
        .pDepthStencilState = NULL,
        .pColorBlendState = NULL,
        .pDynamicState = NULL,
        .layout = VK_NULL_HANDLE,
        .renderPass = VK_NULL_HANDLE,
        .subpass = 0,
        .basePipelineHandle = VK_NULL_HANDLE,
        .basePipelineIndex = -1
    };

    VkPipelineShaderStageCreateInfo* stages = NULL;

    lua_getfield(L, 1, "pStages");
    if (!lua_isnil(L, -1)) {
        luaL_checktype(L, -1, LUA_TTABLE);
        pipeline_info.stageCount = lua_rawlen(L, -1);
        if (pipeline_info.stageCount > 0) {
            stages = (VkPipelineShaderStageCreateInfo*)malloc(pipeline_info.stageCount * sizeof(VkPipelineShaderStageCreateInfo));
            if (!stages) {
                luaL_error(L, "Failed to allocate memory for pipeline stages");
            }
            for (uint32_t i = 0; i < pipeline_info.stageCount; i++) {
                lua_rawgeti(L, -1, i + 1);
                lua_VkPipelineShaderStageCreateInfo* stage_ud = lua_check_VkPipelineShaderStageCreateInfo(L, -1);
                stages[i] = stage_ud->stage_info;
                lua_pop(L, 1);
            }
            pipeline_info.pStages = stages;
        }
    }
    lua_pop(L, 1);

    lua_getfield(L, 1, "pVertexInputState");
    if (!lua_isnil(L, -1)) {
        lua_VkPipelineVertexInputStateCreateInfo* vertex_input_ud = lua_check_VkPipelineVertexInputStateCreateInfo(L, -1);
        pipeline_info.pVertexInputState = &vertex_input_ud->vertex_input_info;
    }
    lua_pop(L, 1);

    lua_getfield(L, 1, "pInputAssemblyState");
    if (!lua_isnil(L, -1)) {
        lua_VkPipelineInputAssemblyStateCreateInfo* input_assembly_ud = lua_check_VkPipelineInputAssemblyStateCreateInfo(L, -1);
        pipeline_info.pInputAssemblyState = &input_assembly_ud->input_assembly;
    }
    lua_pop(L, 1);

    lua_getfield(L, 1, "pViewportState");
    if (!lua_isnil(L, -1)) {
        lua_VkPipelineViewportStateCreateInfo* viewport_state_ud = lua_check_VkPipelineViewportStateCreateInfo(L, -1);
        pipeline_info.pViewportState = &viewport_state_ud->viewport_state;
    }
    lua_pop(L, 1);

    lua_getfield(L, 1, "pRasterizationState");
    if (!lua_isnil(L, -1)) {
        lua_VkPipelineRasterizationStateCreateInfo* rasterizer_ud = lua_check_VkPipelineRasterizationStateCreateInfo(L, -1);
        pipeline_info.pRasterizationState = &rasterizer_ud->rasterizer;
    }
    lua_pop(L, 1);

    lua_getfield(L, 1, "pMultisampleState");
    if (!lua_isnil(L, -1)) {
        lua_VkPipelineMultisampleStateCreateInfo* multisampling_ud = lua_check_VkPipelineMultisampleStateCreateInfo(L, -1);
        pipeline_info.pMultisampleState = &multisampling_ud->multisampling;
    }
    lua_pop(L, 1);

    lua_getfield(L, 1, "pColorBlendState");
    if (!lua_isnil(L, -1)) {
        lua_VkPipelineColorBlendStateCreateInfo* blend_state_ud = lua_check_VkPipelineColorBlendStateCreateInfo(L, -1);
        pipeline_info.pColorBlendState = &blend_state_ud->blend_state;
    }
    lua_pop(L, 1);

    lua_getfield(L, 1, "layout");
    if (!lua_isnil(L, -1)) {
        lua_VkPipelineLayout* layout_ud = lua_check_VkPipelineLayout(L, -1);
        pipeline_info.layout = layout_ud->pipeline_layout;
    }
    lua_pop(L, 1);

    lua_getfield(L, 1, "renderPass");
    if (!lua_isnil(L, -1)) {
        lua_VkRenderPass* render_pass_ud = lua_check_VkRenderPass(L, -1);
        pipeline_info.renderPass = render_pass_ud->render_pass;
    }
    lua_pop(L, 1);

    lua_getfield(L, 1, "subpass");
    if (!lua_isnil(L, -1)) {
        pipeline_info.subpass = (uint32_t)luaL_checkinteger(L, -1);
    }
    lua_pop(L, 1);

    lua_push_VkGraphicsPipelineCreateInfo(L, &pipeline_info, stages);
    return 1;
}

// Create VkPipeline: vulkan.create_graphics_pipeline(device, pAllocator, pipeline_cache, pipeline_info)
static int l_vulkan_create_graphics_pipeline(lua_State* L) {
    lua_VkDevice* device_ud = lua_check_VkDevice(L, 1);
    luaL_checktype(L, 2, LUA_TNIL); // pAllocator
    luaL_checktype(L, 3, LUA_TNIL); // pipeline_cache
    lua_VkGraphicsPipelineCreateInfo* pipeline_info_ud = lua_check_VkGraphicsPipelineCreateInfo(L, 4);

    VkPipeline pipeline;
    VkResult result = vkCreateGraphicsPipelines(device_ud->device, VK_NULL_HANDLE, 1, &pipeline_info_ud->pipeline_info, NULL, &pipeline);
    if (result != VK_SUCCESS) {
        luaL_error(L, "Failed to create graphics pipeline: VkResult %d", result);
    }

    lua_push_VkPipeline(L, pipeline, device_ud->device);
    return 1;
}



// Garbage collection for VkBufferCreateInfo
static int buffer_create_info_gc(lua_State* L) {
    printf("Cleaning up VkBufferCreateInfo\n");
    return 0;
}

// Garbage collection for VkBuffer
static int buffer_gc(lua_State* L) {
    lua_VkBuffer* ud = (lua_VkBuffer*)luaL_checkudata(L, 1, BUFFER_MT);
    if (ud->buffer && ud->device) {
        printf("Cleaning up VkBuffer\n");
        vkDestroyBuffer(ud->device, ud->buffer, NULL);
        ud->buffer = NULL;
        ud->device = NULL;
    }
    return 0;
}

// Garbage collection for VkMemoryAllocateInfo
static int memory_allocate_info_gc(lua_State* L) {
    printf("Cleaning up VkMemoryAllocateInfo\n");
    return 0;
}

// Garbage collection for VkDeviceMemory
static int device_memory_gc(lua_State* L) {
    lua_VkDeviceMemory* ud = (lua_VkDeviceMemory*)luaL_checkudata(L, 1, DEVICE_MEMORY_MT);
    if (ud->memory && ud->device) {
        printf("Cleaning up VkDeviceMemory\n");
        vkFreeMemory(ud->device, ud->memory, NULL);
        ud->memory = NULL;
        ud->device = NULL;
    }
    return 0;
}

// Garbage collection for VkCommandPoolCreateInfo
static int command_pool_create_info_gc(lua_State* L) {
    printf("Cleaning up VkCommandPoolCreateInfo\n");
    return 0;
}

// Garbage collection for VkCommandPool
static int command_pool_gc(lua_State* L) {
    lua_VkCommandPool* ud = (lua_VkCommandPool*)luaL_checkudata(L, 1, COMMAND_POOL_MT);
    if (ud->command_pool && ud->device) {
        printf("Cleaning up VkCommandPool\n");
        vkDestroyCommandPool(ud->device, ud->command_pool, NULL);
        ud->command_pool = NULL;
        ud->device = NULL;
    }
    return 0;
}

// Garbage collection for VkCommandBufferAllocateInfo
static int command_buffer_allocate_info_gc(lua_State* L) {
    printf("Cleaning up VkCommandBufferAllocateInfo\n");
    return 0;
}

// Garbage collection for VkCommandBuffer
static int command_buffer_gc(lua_State* L) {
    lua_VkCommandBuffer* ud = (lua_VkCommandBuffer*)luaL_checkudata(L, 1, COMMAND_BUFFER_MT);
    // Note: Command buffers are freed when their command pool is destroyed
    printf("Cleaning up VkCommandBuffer\n");
    ud->command_buffer = NULL;
    ud->device = NULL;
    return 0;
}

// Push/check functions
void lua_push_VkBufferCreateInfo(lua_State* L, VkBufferCreateInfo* create_info) {
    lua_VkBufferCreateInfo* ud = (lua_VkBufferCreateInfo*)lua_newuserdata(L, sizeof(lua_VkBufferCreateInfo));
    ud->create_info = *create_info;
    luaL_setmetatable(L, BUFFER_CREATE_INFO_MT);
}

lua_VkBufferCreateInfo* lua_check_VkBufferCreateInfo(lua_State* L, int idx) {
    return (lua_VkBufferCreateInfo*)luaL_checkudata(L, idx, BUFFER_CREATE_INFO_MT);
}

void lua_push_VkBuffer(lua_State* L, VkBuffer buffer, VkDevice device) {
    lua_VkBuffer* ud = (lua_VkBuffer*)lua_newuserdata(L, sizeof(lua_VkBuffer));
    ud->buffer = buffer;
    ud->device = device;
    luaL_setmetatable(L, BUFFER_MT);
}

lua_VkBuffer* lua_check_VkBuffer(lua_State* L, int idx) {
    lua_VkBuffer* ud = (lua_VkBuffer*)luaL_checkudata(L, idx, BUFFER_MT);
    if (!ud->buffer) luaL_error(L, "Invalid VkBuffer");
    return ud;
}

void lua_push_VkMemoryAllocateInfo(lua_State* L, VkMemoryAllocateInfo* alloc_info) {
    lua_VkMemoryAllocateInfo* ud = (lua_VkMemoryAllocateInfo*)lua_newuserdata(L, sizeof(lua_VkMemoryAllocateInfo));
    ud->alloc_info = *alloc_info;
    luaL_setmetatable(L, MEMORY_ALLOCATE_INFO_MT);
}

lua_VkMemoryAllocateInfo* lua_check_VkMemoryAllocateInfo(lua_State* L, int idx) {
    return (lua_VkMemoryAllocateInfo*)luaL_checkudata(L, idx, MEMORY_ALLOCATE_INFO_MT);
}

void lua_push_VkDeviceMemory(lua_State* L, VkDeviceMemory memory, VkDevice device) {
    lua_VkDeviceMemory* ud = (lua_VkDeviceMemory*)lua_newuserdata(L, sizeof(lua_VkDeviceMemory));
    ud->memory = memory;
    ud->device = device;
    luaL_setmetatable(L, DEVICE_MEMORY_MT);
}

lua_VkDeviceMemory* lua_check_VkDeviceMemory(lua_State* L, int idx) {
    lua_VkDeviceMemory* ud = (lua_VkDeviceMemory*)luaL_checkudata(L, idx, DEVICE_MEMORY_MT);
    if (!ud->memory) luaL_error(L, "Invalid VkDeviceMemory");
    return ud;
}

void lua_push_VkCommandPoolCreateInfo(lua_State* L, VkCommandPoolCreateInfo* create_info) {
    lua_VkCommandPoolCreateInfo* ud = (lua_VkCommandPoolCreateInfo*)lua_newuserdata(L, sizeof(lua_VkCommandPoolCreateInfo));
    ud->create_info = *create_info;
    luaL_setmetatable(L, COMMAND_POOL_CREATE_INFO_MT);
}

lua_VkCommandPoolCreateInfo* lua_check_VkCommandPoolCreateInfo(lua_State* L, int idx) {
    return (lua_VkCommandPoolCreateInfo*)luaL_checkudata(L, idx, COMMAND_POOL_CREATE_INFO_MT);
}

void lua_push_VkCommandPool(lua_State* L, VkCommandPool command_pool, VkDevice device) {
    lua_VkCommandPool* ud = (lua_VkCommandPool*)lua_newuserdata(L, sizeof(lua_VkCommandPool));
    ud->command_pool = command_pool;
    ud->device = device;
    luaL_setmetatable(L, COMMAND_POOL_MT);
}

lua_VkCommandPool* lua_check_VkCommandPool(lua_State* L, int idx) {
    lua_VkCommandPool* ud = (lua_VkCommandPool*)luaL_checkudata(L, idx, COMMAND_POOL_MT);
    if (!ud->command_pool) luaL_error(L, "Invalid VkCommandPool");
    return ud;
}

void lua_push_VkCommandBufferAllocateInfo(lua_State* L, VkCommandBufferAllocateInfo* alloc_info) {
    lua_VkCommandBufferAllocateInfo* ud = (lua_VkCommandBufferAllocateInfo*)lua_newuserdata(L, sizeof(lua_VkCommandBufferAllocateInfo));
    ud->alloc_info = *alloc_info;
    luaL_setmetatable(L, COMMAND_BUFFER_ALLOCATE_INFO_MT);
}

lua_VkCommandBufferAllocateInfo* lua_check_VkCommandBufferAllocateInfo(lua_State* L, int idx) {
    return (lua_VkCommandBufferAllocateInfo*)luaL_checkudata(L, idx, COMMAND_BUFFER_ALLOCATE_INFO_MT);
}

void lua_push_VkCommandBuffer(lua_State* L, VkCommandBuffer command_buffer, VkDevice device) {
    lua_VkCommandBuffer* ud = (lua_VkCommandBuffer*)lua_newuserdata(L, sizeof(lua_VkCommandBuffer));
    ud->command_buffer = command_buffer;
    ud->device = device;
    luaL_setmetatable(L, COMMAND_BUFFER_MT);
}

lua_VkCommandBuffer* lua_check_VkCommandBuffer(lua_State* L, int idx) {
    lua_VkCommandBuffer* ud = (lua_VkCommandBuffer*)luaL_checkudata(L, idx, COMMAND_BUFFER_MT);
    if (!ud->command_buffer) luaL_error(L, "Invalid VkCommandBuffer");
    return ud;
}



// New functions
static int l_vulkan_create_buffer_create_info(lua_State* L) {
    luaL_checktype(L, 1, LUA_TTABLE);

    VkBufferCreateInfo create_info = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .pNext = NULL,
        .flags = 0,
        .size = 0,
        .usage = 0,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .queueFamilyIndexCount = 0,
        .pQueueFamilyIndices = NULL
    };

    lua_getfield(L, 1, "size");
    if (!lua_isnil(L, -1)) {
        create_info.size = (VkDeviceSize)luaL_checkinteger(L, -1);
    }
    lua_pop(L, 1);

    lua_getfield(L, 1, "usage");
    if (!lua_isnil(L, -1)) {
        create_info.usage = (VkBufferUsageFlags)luaL_checkinteger(L, -1);
    }
    lua_pop(L, 1);

    lua_push_VkBufferCreateInfo(L, &create_info);
    return 1;
}

static int l_vulkan_create_buffer(lua_State* L) {
    lua_VkDevice* device_ud = lua_check_VkDevice(L, 1);
    lua_VkBufferCreateInfo* create_info_ud = lua_check_VkBufferCreateInfo(L, 2);
    luaL_checktype(L, 3, LUA_TNIL);

    VkBuffer buffer;
    VkResult result = vkCreateBuffer(device_ud->device, &create_info_ud->create_info, NULL, &buffer);
    if (result != VK_SUCCESS) {
        luaL_error(L, "Failed to create buffer: VkResult %d", result);
    }

    lua_push_VkBuffer(L, buffer, device_ud->device);
    return 1;
}

static int l_vulkan_allocate_memory(lua_State* L) {
    lua_VkDevice* device_ud = lua_check_VkDevice(L, 1);
    lua_VkMemoryAllocateInfo* alloc_info_ud = lua_check_VkMemoryAllocateInfo(L, 2);
    luaL_checktype(L, 3, LUA_TNIL);

    VkDeviceMemory memory;
    VkResult result = vkAllocateMemory(device_ud->device, &alloc_info_ud->alloc_info, NULL, &memory);
    if (result != VK_SUCCESS) {
        luaL_error(L, "Failed to allocate memory: VkResult %d", result);
    }

    lua_push_VkDeviceMemory(L, memory, device_ud->device);
    return 1;
}

static int l_vulkan_bind_buffer_memory(lua_State* L) {
    lua_VkDevice* device_ud = lua_check_VkDevice(L, 1);
    lua_VkBuffer* buffer_ud = lua_check_VkBuffer(L, 2);
    lua_VkDeviceMemory* memory_ud = lua_check_VkDeviceMemory(L, 3);
    VkDeviceSize offset = (VkDeviceSize)luaL_checkinteger(L, 4);

    VkResult result = vkBindBufferMemory(device_ud->device, buffer_ud->buffer, memory_ud->memory, offset);
    if (result != VK_SUCCESS) {
        luaL_error(L, "Failed to bind buffer memory: VkResult %d", result);
    }

    return 0;
}

static int l_vulkan_map_memory(lua_State* L) {
    lua_VkDevice* device_ud = lua_check_VkDevice(L, 1);
    lua_VkDeviceMemory* memory_ud = lua_check_VkDeviceMemory(L, 2);
    VkDeviceSize offset = (VkDeviceSize)luaL_checkinteger(L, 3);
    VkDeviceSize size = (VkDeviceSize)luaL_checkinteger(L, 4);

    void* data;
    VkResult result = vkMapMemory(device_ud->device, memory_ud->memory, offset, size, 0, &data);
    if (result != VK_SUCCESS) {
        luaL_error(L, "Failed to map memory: VkResult %d", result);
    }

    lua_pushlightuserdata(L, data);
    return 1;
}

static int l_vulkan_create_command_pool_create_info(lua_State* L) {
    luaL_checktype(L, 1, LUA_TTABLE);

    VkCommandPoolCreateInfo create_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .pNext = NULL,
        .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        .queueFamilyIndex = 0
    };

    lua_getfield(L, 1, "queueFamilyIndex");
    if (!lua_isnil(L, -1)) {
        create_info.queueFamilyIndex = (uint32_t)luaL_checkinteger(L, -1);
    }
    lua_pop(L, 1);

    lua_push_VkCommandPoolCreateInfo(L, &create_info);
    return 1;
}

static int l_vulkan_create_command_pool(lua_State* L) {
    lua_VkDevice* device_ud = lua_check_VkDevice(L, 1);
    lua_VkCommandPoolCreateInfo* create_info_ud = lua_check_VkCommandPoolCreateInfo(L, 2);
    luaL_checktype(L, 3, LUA_TNIL);

    VkCommandPool command_pool;
    VkResult result = vkCreateCommandPool(device_ud->device, &create_info_ud->create_info, NULL, &command_pool);
    if (result != VK_SUCCESS) {
        luaL_error(L, "Failed to create command pool: VkResult %d", result);
    }

    lua_push_VkCommandPool(L, command_pool, device_ud->device);
    return 1;
}

static int l_vulkan_create_command_buffer_allocate_info(lua_State* L) {
    luaL_checktype(L, 1, LUA_TTABLE);

    VkCommandBufferAllocateInfo alloc_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .pNext = NULL,
        .commandPool = VK_NULL_HANDLE,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 0
    };

    lua_getfield(L, 1, "commandPool");
    if (!lua_isnil(L, -1)) {
        lua_VkCommandPool* pool_ud = lua_check_VkCommandPool(L, -1);
        alloc_info.commandPool = pool_ud->command_pool;
    }
    lua_pop(L, 1);

    lua_getfield(L, 1, "commandBufferCount");
    if (!lua_isnil(L, -1)) {
        alloc_info.commandBufferCount = (uint32_t)luaL_checkinteger(L, -1);
    }
    lua_pop(L, 1);

    lua_push_VkCommandBufferAllocateInfo(L, &alloc_info);
    return 1;
}

static int l_vulkan_allocate_command_buffers(lua_State* L) {
    lua_VkDevice* device_ud = lua_check_VkDevice(L, 1);
    lua_VkCommandBufferAllocateInfo* alloc_info_ud = lua_check_VkCommandBufferAllocateInfo(L, 2);

    VkCommandBuffer* command_buffers = (VkCommandBuffer*)malloc(alloc_info_ud->alloc_info.commandBufferCount * sizeof(VkCommandBuffer));
    if (!command_buffers) {
        luaL_error(L, "Failed to allocate memory for command buffers");
    }

    VkResult result = vkAllocateCommandBuffers(device_ud->device, &alloc_info_ud->alloc_info, command_buffers);
    if (result != VK_SUCCESS) {
        free(command_buffers);
        luaL_error(L, "Failed to allocate command buffers: VkResult %d", result);
    }

    lua_newtable(L);
    for (uint32_t i = 0; i < alloc_info_ud->alloc_info.commandBufferCount; i++) {
        lua_push_VkCommandBuffer(L, command_buffers[i], device_ud->device);
        lua_rawseti(L, -2, i + 1);
    }
    free(command_buffers);
    return 1;
}

static int l_vulkan_begin_command_buffer(lua_State* L) {
    lua_VkCommandBuffer* cmd_ud = lua_check_VkCommandBuffer(L, 1);

    VkCommandBufferBeginInfo begin_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .pNext = NULL,
        .flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT,
        .pInheritanceInfo = NULL
    };

    VkResult result = vkBeginCommandBuffer(cmd_ud->command_buffer, &begin_info);
    if (result != VK_SUCCESS) {
        luaL_error(L, "Failed to begin command buffer: VkResult %d", result);
    }

    return 0;
}

static int l_vulkan_cmd_begin_render_pass(lua_State* L) {
    lua_VkCommandBuffer* cmd_ud = lua_check_VkCommandBuffer(L, 1);
    lua_VkRenderPass* render_pass_ud = lua_check_VkRenderPass(L, 2);
    lua_VkFramebuffer* framebuffer_ud = lua_check_VkFramebuffer(L, 3);

    VkRenderPassBeginInfo render_pass_info = {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        .pNext = NULL,
        .renderPass = render_pass_ud->render_pass,
        .framebuffer = framebuffer_ud->framebuffer,
        .renderArea = {{0, 0}, {800, 600}}, // From your output
        .clearValueCount = 1,
        .pClearValues = &(VkClearValue){.color = {{0.0f, 0.0f, 0.0f, 1.0f}}}
    };

    lua_getfield(L, 4, "renderArea");
    if (!lua_isnil(L, -1)) {
        lua_getfield(L, -1, "extent");
        lua_VkExtent2D* extent_ud = lua_check_VkExtent2D(L, -1);
        render_pass_info.renderArea.extent = extent_ud->extent;
        lua_pop(L, 1);
        lua_pop(L, 1);
    }

    vkCmdBeginRenderPass(cmd_ud->command_buffer, &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);
    return 0;
}

static int l_vulkan_cmd_bind_pipeline(lua_State* L) {
    lua_VkCommandBuffer* cmd_ud = lua_check_VkCommandBuffer(L, 1);
    lua_VkPipeline* pipeline_ud = lua_check_VkPipeline(L, 2);

    vkCmdBindPipeline(cmd_ud->command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_ud->pipeline);
    return 0;
}

static int l_vulkan_cmd_bind_vertex_buffers(lua_State* L) {
    lua_VkCommandBuffer* cmd_ud = lua_check_VkCommandBuffer(L, 1);
    uint32_t first_binding = (uint32_t)luaL_checkinteger(L, 2);
    luaL_checktype(L, 3, LUA_TTABLE);
    luaL_checktype(L, 4, LUA_TTABLE);

    uint32_t buffer_count = lua_rawlen(L, 3);
    VkBuffer* buffers = (VkBuffer*)malloc(buffer_count * sizeof(VkBuffer));
    VkDeviceSize* offsets = (VkDeviceSize*)malloc(buffer_count * sizeof(VkDeviceSize));
    if (!buffers || !offsets) {
        free(buffers);
        free(offsets);
        luaL_error(L, "Failed to allocate memory for buffers/offsets");
    }

    for (uint32_t i = 0; i < buffer_count; i++) {
        lua_rawgeti(L, 3, i + 1);
        lua_VkBuffer* buffer_ud = lua_check_VkBuffer(L, -1);
        buffers[i] = buffer_ud->buffer;
        lua_pop(L, 1);

        lua_rawgeti(L, 4, i + 1);
        offsets[i] = (VkDeviceSize)luaL_checkinteger(L, -1);
        lua_pop(L, 1);
    }

    vkCmdBindVertexBuffers(cmd_ud->command_buffer, first_binding, buffer_count, buffers, offsets);
    free(buffers);
    free(offsets);
    return 0;
}

static int l_vulkan_cmd_draw(lua_State* L) {
    lua_VkCommandBuffer* cmd_ud = lua_check_VkCommandBuffer(L, 1);
    uint32_t vertex_count = (uint32_t)luaL_checkinteger(L, 2);
    uint32_t instance_count = (uint32_t)luaL_checkinteger(L, 3);
    uint32_t first_vertex = (uint32_t)luaL_checkinteger(L, 4);
    uint32_t first_instance = (uint32_t)luaL_checkinteger(L, 5);

    vkCmdDraw(cmd_ud->command_buffer, vertex_count, instance_count, first_vertex, first_instance);
    return 0;
}

static int l_vulkan_cmd_end_render_pass(lua_State* L) {
    lua_VkCommandBuffer* cmd_ud = lua_check_VkCommandBuffer(L, 1);
    vkCmdEndRenderPass(cmd_ud->command_buffer);
    return 0;
}

static int l_vulkan_end_command_buffer(lua_State* L) {
    lua_VkCommandBuffer* cmd_ud = lua_check_VkCommandBuffer(L, 1);
    VkResult result = vkEndCommandBuffer(cmd_ud->command_buffer);
    if (result != VK_SUCCESS) {
        luaL_error(L, "Failed to end command buffer: VkResult %d", result);
    }
    return 0;
}

// Garbage collection for VkMemoryRequirements
static int memory_requirements_gc(lua_State* L) {
    printf("Cleaning up VkMemoryRequirements\n");
    return 0;
}

// Check VkMemoryRequirements from Lua
lua_VkMemoryRequirements* lua_check_VkMemoryRequirements(lua_State* L, int idx) {
    return (lua_VkMemoryRequirements*)luaL_checkudata(L, idx, MEMORY_REQUIREMENTS_MT);
}

// __index function for VkMemoryRequirements
static int memory_requirements_index(lua_State* L) {
    lua_VkMemoryRequirements* ud = lua_check_VkMemoryRequirements(L, 1);
    const char* field = luaL_checkstring(L, 2);
    if (strcmp(field, "size") == 0) {
        lua_pushinteger(L, ud->mem_requirements.size);
    } else if (strcmp(field, "alignment") == 0) {
        lua_pushinteger(L, ud->mem_requirements.alignment);
    } else if (strcmp(field, "memoryTypeBits") == 0) {
        lua_pushinteger(L, ud->mem_requirements.memoryTypeBits);
    } else {
        luaL_error(L, "Unknown field '%s' for vulkan.memory_requirements", field);
    }
    return 1;
}



// Push VkMemoryRequirements to Lua
void lua_push_VkMemoryRequirements(lua_State* L, VkMemoryRequirements* mem_requirements) {
    lua_VkMemoryRequirements* ud = (lua_VkMemoryRequirements*)lua_newuserdata(L, sizeof(lua_VkMemoryRequirements));
    ud->mem_requirements = *mem_requirements;
    luaL_setmetatable(L, MEMORY_REQUIREMENTS_MT);
}



// Create VkMemoryAllocateInfo
static int l_vulkan_create_memory_allocate_info(lua_State* L) {
    luaL_checktype(L, 1, LUA_TTABLE);

    VkMemoryAllocateInfo alloc_info = {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .pNext = NULL,
        .allocationSize = 0,
        .memoryTypeIndex = 0
    };

    lua_getfield(L, 1, "allocationSize");
    if (!lua_isnil(L, -1)) {
        alloc_info.allocationSize = (VkDeviceSize)luaL_checkinteger(L, -1);
    }
    lua_pop(L, 1);

    lua_getfield(L, 1, "memoryTypeIndex");
    if (!lua_isnil(L, -1)) {
        alloc_info.memoryTypeIndex = (uint32_t)luaL_checkinteger(L, -1);
    }
    lua_pop(L, 1);

    lua_push_VkMemoryAllocateInfo(L, &alloc_info);
    return 1;
}

// Get VkMemoryRequirements for a buffer
static int l_vulkan_get_buffer_memory_requirements(lua_State* L) {
    lua_VkDevice* device_ud = lua_check_VkDevice(L, 1);
    lua_VkBuffer* buffer_ud = lua_check_VkBuffer(L, 2);

    VkMemoryRequirements mem_requirements;
    vkGetBufferMemoryRequirements(device_ud->device, buffer_ud->buffer, &mem_requirements);

    lua_push_VkMemoryRequirements(L, &mem_requirements);
    return 1;
}

// Garbage collection for VkPhysicalDeviceMemoryProperties
static int physical_device_memory_properties_gc(lua_State* L) {
    printf("Cleaning up VkPhysicalDeviceMemoryProperties\n");
    return 0;
}





// Push VkPhysicalDeviceMemoryProperties
void lua_push_VkPhysicalDeviceMemoryProperties(lua_State* L, VkPhysicalDeviceMemoryProperties* mem_properties) {
    lua_VkPhysicalDeviceMemoryProperties* ud = (lua_VkPhysicalDeviceMemoryProperties*)lua_newuserdata(L, sizeof(lua_VkPhysicalDeviceMemoryProperties));
    ud->mem_properties = *mem_properties;
    luaL_setmetatable(L, PHYSICAL_DEVICE_MEMORY_PROPERTIES_MT);
}

// Check VkPhysicalDeviceMemoryProperties
lua_VkPhysicalDeviceMemoryProperties* lua_check_VkPhysicalDeviceMemoryProperties(lua_State* L, int idx) {
    return (lua_VkPhysicalDeviceMemoryProperties*)luaL_checkudata(L, idx, PHYSICAL_DEVICE_MEMORY_PROPERTIES_MT);
}

// __index function for VkPhysicalDeviceMemoryProperties
static int physical_device_memory_properties_index(lua_State* L) {
    lua_VkPhysicalDeviceMemoryProperties* ud = lua_check_VkPhysicalDeviceMemoryProperties(L, 1);
    const char* field = luaL_checkstring(L, 2);
    if (strcmp(field, "memoryTypeCount") == 0) {
        lua_pushinteger(L, ud->mem_properties.memoryTypeCount);
    } else if (strcmp(field, "memoryTypes") == 0) {
        lua_newtable(L);
        for (uint32_t i = 0; i < ud->mem_properties.memoryTypeCount; i++) {
            lua_newtable(L);
            lua_pushinteger(L, ud->mem_properties.memoryTypes[i].propertyFlags);
            lua_setfield(L, -2, "propertyFlags");
            lua_pushinteger(L, ud->mem_properties.memoryTypes[i].heapIndex);
            lua_setfield(L, -2, "heapIndex");
            lua_rawseti(L, -2, i + 1);
        }
    } else if (strcmp(field, "memoryHeapCount") == 0) {
        lua_pushinteger(L, ud->mem_properties.memoryHeapCount);
    } else if (strcmp(field, "memoryHeaps") == 0) {
        lua_newtable(L);
        for (uint32_t i = 0; i < ud->mem_properties.memoryHeapCount; i++) {
            lua_newtable(L);
            lua_pushinteger(L, ud->mem_properties.memoryHeaps[i].size);
            lua_setfield(L, -2, "size");
            lua_pushinteger(L, ud->mem_properties.memoryHeaps[i].flags);
            lua_setfield(L, -2, "flags");
            lua_rawseti(L, -2, i + 1);
        }
    } else {
        luaL_error(L, "Unknown field '%s' for vulkan.physical_device_memory_properties", field);
    }
    return 1;
}


// Get physical device memory properties
static int l_vulkan_get_physical_device_memory_properties(lua_State* L) {
    lua_VkPhysicalDevice* phys_device_ud = lua_check_VkPhysicalDevice(L, 1);

    VkPhysicalDeviceMemoryProperties mem_properties;
    vkGetPhysicalDeviceMemoryProperties(phys_device_ud->device, &mem_properties);

    lua_push_VkPhysicalDeviceMemoryProperties(L, &mem_properties);
    return 1;
}

// Metatable for VkPhysicalDeviceMemoryProperties
static void physical_device_memory_properties_metatable(lua_State* L) {
    luaL_newmetatable(L, PHYSICAL_DEVICE_MEMORY_PROPERTIES_MT);
    lua_pushcfunction(L, physical_device_memory_properties_gc);
    lua_setfield(L, -2, "__gc");
    lua_pushcfunction(L, physical_device_memory_properties_index);
    lua_setfield(L, -2, "__index");
    lua_pop(L, 1);
}


// Metatable for VkMemoryRequirements
static void memory_requirements_metatable(lua_State* L) {
    luaL_newmetatable(L, MEMORY_REQUIREMENTS_MT);
    lua_pushcfunction(L, memory_requirements_gc);
    lua_setfield(L, -2, "__gc");
    lua_pushcfunction(L, memory_requirements_index);
    lua_setfield(L, -2, "__index");
    lua_pop(L, 1);
}


static int l_vulkan_copy_to_memory(lua_State* L) {
    lua_VkDevice* device_ud = lua_check_VkDevice(L, 1);
    void* mapped_memory = lua_touserdata(L, 2); // From l_vulkan_map_memory
    luaL_checktype(L, 3, LUA_TTABLE); // Vertex data array
    VkDeviceSize size = (VkDeviceSize)luaL_checkinteger(L, 4);

    // Get vertex data from Lua table
    size_t vertex_count = lua_rawlen(L, 3);
    float* vertices = (float*)malloc(vertex_count * sizeof(float));
    if (!vertices) {
        luaL_error(L, "Failed to allocate memory for vertex data");
    }

    for (size_t i = 0; i < vertex_count; i++) {
        lua_rawgeti(L, 3, i + 1);
        vertices[i] = (float)luaL_checknumber(L, -1);
        lua_pop(L, 1);
    }

    // Copy to mapped memory
    memcpy(mapped_memory, vertices, size);
    free(vertices);

    return 0;
}

// Unmap memory
static int l_vulkan_unmap_memory(lua_State* L) {
    lua_VkDevice* device_ud = lua_check_VkDevice(L, 1);
    lua_VkDeviceMemory* memory_ud = lua_check_VkDeviceMemory(L, 2);
    vkUnmapMemory(device_ud->device, memory_ud->memory);
    return 0;
}





// Metatable setup
static int extent_2d_index(lua_State* L) {
    lua_VkExtent2D* ud = (lua_VkExtent2D*)luaL_checkudata(L, 1, EXTENT_2D_MT);
    const char* field = luaL_checkstring(L, 2);

    if (strcmp(field, "width") == 0) {
        lua_pushinteger(L, ud->extent.width);
        return 1;
    } else if (strcmp(field, "height") == 0) {
        lua_pushinteger(L, ud->extent.height);
        return 1;
    } else {
        luaL_error(L, "Unknown field '%s' for vulkan.extent_2d", field);
        return 0;
    }
}

static void extent_2d_metatable(lua_State* L) {
    luaL_newmetatable(L, EXTENT_2D_MT);
    lua_pushcfunction(L, extent_2d_gc);
    lua_setfield(L, -2, "__gc");
    lua_pushcfunction(L, extent_2d_index);
    lua_setfield(L, -2, "__index");
    lua_pop(L, 1);
}

static void shader_module_metatable(lua_State* L) {
    luaL_newmetatable(L, SHADER_MODULE_MT);
    lua_pushcfunction(L, shader_module_gc);
    lua_setfield(L, -2, "__gc");
    lua_pop(L, 1);
}

static void pipeline_shader_stage_create_info_metatable(lua_State* L) {
    luaL_newmetatable(L, PIPELINE_SHADER_STAGE_CREATE_INFO_MT);
    lua_pushcfunction(L, pipeline_shader_stage_create_info_gc);
    lua_setfield(L, -2, "__gc");
    lua_pop(L, 1);
}

static void vertex_input_binding_description_metatable(lua_State* L) {
    luaL_newmetatable(L, VERTEX_INPUT_BINDING_DESCRIPTION_MT);
    lua_pushcfunction(L, vertex_input_binding_description_gc);
    lua_setfield(L, -2, "__gc");
    lua_pop(L, 1);
}

static void vertex_input_attribute_description_metatable(lua_State* L) {
    luaL_newmetatable(L, VERTEX_INPUT_ATTRIBUTE_DESCRIPTION_MT);
    lua_pushcfunction(L, vertex_input_attribute_description_gc);
    lua_setfield(L, -2, "__gc");
    lua_pop(L, 1);
}

static void pipeline_vertex_input_state_create_info_metatable(lua_State* L) {
    luaL_newmetatable(L, PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO_MT);
    lua_pushcfunction(L, pipeline_vertex_input_state_create_info_gc);
    lua_setfield(L, -2, "__gc");
    lua_pop(L, 1);
}

static void pipeline_input_assembly_state_create_info_metatable(lua_State* L) {
    luaL_newmetatable(L, PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO_MT);
    lua_pushcfunction(L, pipeline_input_assembly_state_create_info_gc);
    lua_setfield(L, -2, "__gc");
    lua_pop(L, 1);
}



// Metatable setup
static void appinfo_metatable(lua_State* L) {
    luaL_newmetatable(L, APPINFO_MT);
    lua_pushcfunction(L, appinfo_gc);
    lua_setfield(L, -2, "__gc");
    lua_pop(L, 1);
}

static void createinfo_metatable(lua_State* L) {
    luaL_newmetatable(L, CREATEINFO_MT);
    lua_pushcfunction(L, createinfo_gc);
    lua_setfield(L, -2, "__gc");
    lua_pop(L, 1);
}

static void instance_metatable(lua_State* L) {
    luaL_newmetatable(L, INSTANCE_MT);
    lua_pushcfunction(L, instance_gc);
    lua_setfield(L, -2, "__gc");
    lua_pop(L, 1);
}

static void surface_metatable(lua_State* L) {
    luaL_newmetatable(L, SURFACE_MT);
    lua_pushcfunction(L, surface_gc);
    lua_setfield(L, -2, "__gc");
    lua_pop(L, 1);
}

static void physical_device_metatable(lua_State* L) {
    luaL_newmetatable(L, PHYSICAL_DEVICE_MT);
    // No __gc needed, as VkPhysicalDevice is not destroyed
    lua_pop(L, 1);
}

// Metatable setup
static void device_queue_create_info_metatable(lua_State* L) {
    luaL_newmetatable(L, DEVICE_QUEUE_CREATE_INFO_MT);
    lua_pushcfunction(L, device_queue_create_info_gc);
    lua_setfield(L, -2, "__gc");
    lua_pop(L, 1);
}

static void device_create_info_metatable(lua_State* L) {
    luaL_newmetatable(L, DEVICE_CREATE_INFO_MT);
    lua_pushcfunction(L, device_create_info_gc);
    lua_setfield(L, -2, "__gc");
    lua_pop(L, 1);
}

static void device_metatable(lua_State* L) {
    luaL_newmetatable(L, DEVICE_MT);
    lua_pushcfunction(L, device_gc);
    lua_setfield(L, -2, "__gc");
    lua_pop(L, 1);
}

static void queue_metatable(lua_State* L) {
    luaL_newmetatable(L, QUEUE_MT);
    // No __gc needed, as VkQueue is not destroyed explicitly
    lua_pop(L, 1);
}


// Metatable setup
static void swapchain_create_info_metatable(lua_State* L) {
    luaL_newmetatable(L, SWAPCHAIN_CREATE_INFO_MT);
    lua_pushcfunction(L, swapchain_create_info_gc);
    lua_setfield(L, -2, "__gc");
    lua_pop(L, 1);
}

static void swapchain_metatable(lua_State* L) {
    luaL_newmetatable(L, SWAPCHAIN_MT);
    lua_pushcfunction(L, swapchain_gc);
    lua_setfield(L, -2, "__gc");
    lua_pop(L, 1);
}


// Metatable setup
static void image_view_create_info_metatable(lua_State* L) {
    luaL_newmetatable(L, IMAGE_VIEW_CREATE_INFO_MT);
    lua_pushcfunction(L, image_view_create_info_gc);
    lua_setfield(L, -2, "__gc");
    lua_pop(L, 1);
}

static void image_view_metatable(lua_State* L) {
    luaL_newmetatable(L, IMAGE_VIEW_MT);
    lua_pushcfunction(L, image_view_gc);
    lua_setfield(L, -2, "__gc");
    lua_pop(L, 1);
}

static void attachment_description_metatable(lua_State* L) {
    luaL_newmetatable(L, ATTACHMENT_DESCRIPTION_MT);
    lua_pushcfunction(L, attachment_description_gc);
    lua_setfield(L, -2, "__gc");
    lua_pop(L, 1);
}

static void subpass_description_metatable(lua_State* L) {
    luaL_newmetatable(L, SUBPASS_DESCRIPTION_MT);
    lua_pushcfunction(L, subpass_description_gc);
    lua_setfield(L, -2, "__gc");
    lua_pop(L, 1);
}

static void subpass_dependency_metatable(lua_State* L) {
    luaL_newmetatable(L, SUBPASS_DEPENDENCY_MT);
    lua_pushcfunction(L, subpass_dependency_gc);
    lua_setfield(L, -2, "__gc");
    lua_pop(L, 1);
}

static void render_pass_create_info_metatable(lua_State* L) {
    luaL_newmetatable(L, RENDER_PASS_CREATE_INFO_MT);
    lua_pushcfunction(L, render_pass_create_info_gc);
    lua_setfield(L, -2, "__gc");
    lua_pop(L, 1);
}

static void render_pass_metatable(lua_State* L) {
    luaL_newmetatable(L, RENDER_PASS_MT);
    lua_pushcfunction(L, render_pass_gc);
    lua_setfield(L, -2, "__gc");
    lua_pop(L, 1);
}

// frame buffer
static void framebuffer_create_info_metatable(lua_State* L) {
    luaL_newmetatable(L, FRAMEBUFFER_CREATE_INFO_MT);
    lua_pushcfunction(L, framebuffer_create_info_gc);
    lua_setfield(L, -2, "__gc");
    lua_pop(L, 1);
}

static void framebuffer_metatable(lua_State* L) {
    luaL_newmetatable(L, FRAMEBUFFER_MT);
    lua_pushcfunction(L, framebuffer_gc);
    lua_setfield(L, -2, "__gc");
    lua_pop(L, 1);
}

// pipeline

// Metatable setup (continued)
static void viewport_metatable(lua_State* L) {
    luaL_newmetatable(L, VIEWPORT_MT);
    lua_pushcfunction(L, viewport_gc);
    lua_setfield(L, -2, "__gc");
    lua_pop(L, 1);
}

static void rect_2d_metatable(lua_State* L) {
    luaL_newmetatable(L, RECT_2D_MT);
    lua_pushcfunction(L, rect_2d_gc);
    lua_setfield(L, -2, "__gc");
    lua_pop(L, 1);
}

static void pipeline_viewport_state_create_info_metatable(lua_State* L) {
    luaL_newmetatable(L, PIPELINE_VIEWPORT_STATE_CREATE_INFO_MT);
    lua_pushcfunction(L, pipeline_viewport_state_create_info_gc);
    lua_setfield(L, -2, "__gc");
    lua_pop(L, 1);
}

static void pipeline_rasterization_state_create_info_metatable(lua_State* L) {
    luaL_newmetatable(L, PIPELINE_RASTERIZATION_STATE_CREATE_INFO_MT);
    lua_pushcfunction(L, pipeline_rasterization_state_create_info_gc);
    lua_setfield(L, -2, "__gc");
    lua_pop(L, 1);
}

static void pipeline_multisample_state_create_info_metatable(lua_State* L) {
    luaL_newmetatable(L, PIPELINE_MULTISAMPLE_STATE_CREATE_INFO_MT);
    lua_pushcfunction(L, pipeline_multisample_state_create_info_gc);
    lua_setfield(L, -2, "__gc");
    lua_pop(L, 1);
}

static void pipeline_color_blend_attachment_state_metatable(lua_State* L) {
    luaL_newmetatable(L, PIPELINE_COLOR_BLEND_ATTACHMENT_STATE_MT);
    lua_pushcfunction(L, pipeline_color_blend_attachment_state_gc);
    lua_setfield(L, -2, "__gc");
    lua_pop(L, 1);
}

static void pipeline_color_blend_state_create_info_metatable(lua_State* L) {
    luaL_newmetatable(L, PIPELINE_COLOR_BLEND_STATE_CREATE_INFO_MT);
    lua_pushcfunction(L, pipeline_color_blend_state_create_info_gc);
    lua_setfield(L, -2, "__gc");
    lua_pop(L, 1);
}

static void pipeline_layout_create_info_metatable(lua_State* L) {
    luaL_newmetatable(L, PIPELINE_LAYOUT_CREATE_INFO_MT);
    lua_pushcfunction(L, pipeline_layout_create_info_gc);
    lua_setfield(L, -2, "__gc");
    lua_pop(L, 1);
}

static void pipeline_layout_metatable(lua_State* L) {
    luaL_newmetatable(L, PIPELINE_LAYOUT_MT);
    lua_pushcfunction(L, pipeline_layout_gc);
    lua_setfield(L, -2, "__gc");
    lua_pop(L, 1);
}

static void graphics_pipeline_create_info_metatable(lua_State* L) {
    luaL_newmetatable(L, GRAPHICS_PIPELINE_CREATE_INFO_MT);
    lua_pushcfunction(L, graphics_pipeline_create_info_gc);
    lua_setfield(L, -2, "__gc");
    lua_pop(L, 1);
}

static void pipeline_metatable(lua_State* L) {
    luaL_newmetatable(L, PIPELINE_MT);
    lua_pushcfunction(L, pipeline_gc);
    lua_setfield(L, -2, "__gc");
    lua_pop(L, 1);
}

static void buffer_create_info_metatable(lua_State* L) {
    luaL_newmetatable(L, BUFFER_CREATE_INFO_MT);
    lua_pushcfunction(L, buffer_create_info_gc);
    lua_setfield(L, -2, "__gc");
    lua_pop(L, 1);
}

static void buffer_metatable(lua_State* L) {
    luaL_newmetatable(L, BUFFER_MT);
    lua_pushcfunction(L, buffer_gc);
    lua_setfield(L, -2, "__gc");
    lua_pop(L, 1);
}

static void memory_allocate_info_metatable(lua_State* L) {
    luaL_newmetatable(L, MEMORY_ALLOCATE_INFO_MT);
    lua_pushcfunction(L, memory_allocate_info_gc);
    lua_setfield(L, -2, "__gc");
    lua_pop(L, 1);
}

static void device_memory_metatable(lua_State* L) {
    luaL_newmetatable(L, DEVICE_MEMORY_MT);
    lua_pushcfunction(L, device_memory_gc);
    lua_setfield(L, -2, "__gc");
    lua_pop(L, 1);
}

static void command_pool_create_info_metatable(lua_State* L) {
    luaL_newmetatable(L, COMMAND_POOL_CREATE_INFO_MT);
    lua_pushcfunction(L, command_pool_create_info_gc);
    lua_setfield(L, -2, "__gc");
    lua_pop(L, 1);
}

static void command_pool_metatable(lua_State* L) {
    luaL_newmetatable(L, COMMAND_POOL_MT);
    lua_pushcfunction(L, command_pool_gc);
    lua_setfield(L, -2, "__gc");
    lua_pop(L, 1);
}

static void command_buffer_allocate_info_metatable(lua_State* L) {
    luaL_newmetatable(L, COMMAND_BUFFER_ALLOCATE_INFO_MT);
    lua_pushcfunction(L, command_buffer_allocate_info_gc);
    lua_setfield(L, -2, "__gc");
    lua_pop(L, 1);
}

static void command_buffer_metatable(lua_State* L) {
    luaL_newmetatable(L, COMMAND_BUFFER_MT);
    lua_pushcfunction(L, command_buffer_gc);
    lua_setfield(L, -2, "__gc");
    lua_pop(L, 1);
}













// Module loader: Register functions
static const struct luaL_Reg vulkan_lib[] = {
    {"create_vk_application_info", l_vulkan_create_vk_application_info},
    {"get_instance_extensions", l_vulkan_get_instance_extensions},
    {"create_vk_create_info", l_vulkan_create_vk_create_info},
    {"vk_create_instance", l_vulkan_vk_create_instance},
    {"create_surface", l_vulkan_create_surface},
    {"get_device_count", l_vulkan_get_device_count},
    {"get_devices", l_vulkan_get_devices},
    {"get_physical_device", l_vulkan_get_physical_device},

    {"get_family_count", l_vulkan_get_family_count},
    {"queue_families", l_vulkan_queue_families},

    {"queue_create_infos", l_vulkan_queue_create_infos},
    {"device_create_info", l_vulkan_device_create_info},
    {"create_device", l_vulkan_create_device},
    {"get_device_queue", l_vulkan_get_device_queue},
    {"get_device_extensions", l_vulkan_get_device_extensions},

    {"get_surface_capabilities", l_vulkan_get_physical_device_surface_capabilities},
    {"get_surface_formats", l_vulkan_get_physical_device_surface_formats},
    {"get_surface_present_modes", l_vulkan_get_physical_device_surface_present_modes},

    {"create_swapchain_create_info", l_vulkan_create_swapchain_create_info},
    {"create_swapchain", l_vulkan_create_swapchain},
    {"get_swapchain_images", l_vulkan_get_swapchain_images},

    {"create_image_view_create_info", l_vulkan_create_image_view_create_info},
    {"create_image_view", l_vulkan_create_image_view},
    {"create_attachment_description", l_vulkan_create_attachment_description},
    {"create_subpass_description", l_vulkan_create_subpass_description},
    {"create_subpass_dependency", l_vulkan_create_subpass_dependency},
    {"create_render_pass_create_info", l_vulkan_create_render_pass_create_info},
    {"create_render_pass", l_vulkan_create_render_pass},

    {"create_framebuffer", l_vulkan_create_framebuffer},
    {"create_framebuffer_create_info", l_vulkan_create_framebuffer_create_info},


    {"create_extent_2d", l_vulkan_create_extent_2d},
    {"create_shader_module", l_vulkan_create_shader_module},
    {"create_pipeline_shader_stage_create_info", l_vulkan_create_pipeline_shader_stage_create_info},
    {"create_vertex_input_binding_description", l_vulkan_create_vertex_input_binding_description},
    {"create_vertex_input_attribute_description", l_vulkan_create_vertex_input_attribute_description},
    {"create_pipeline_vertex_input_state_create_info", l_vulkan_create_pipeline_vertex_input_state_create_info},
    {"create_pipeline_input_assembly_state_create_info", l_vulkan_create_pipeline_input_assembly_state_create_info},
    {"create_viewport", l_vulkan_create_viewport},
    {"create_rect_2d", l_vulkan_create_rect_2d},
    {"create_pipeline_viewport_state_create_info", l_vulkan_create_pipeline_viewport_state_create_info},
    {"create_pipeline_rasterization_state_create_info", l_vulkan_create_pipeline_rasterization_state_create_info},
    {"create_pipeline_multisample_state_create_info", l_vulkan_create_pipeline_multisample_state_create_info},
    {"create_pipeline_color_blend_attachment_state", l_vulkan_create_pipeline_color_blend_attachment_state},
    {"create_pipeline_color_blend_state_create_info", l_vulkan_create_pipeline_color_blend_state_create_info},
    {"create_pipeline_layout_create_info", l_vulkan_create_pipeline_layout_create_info},
    {"create_pipeline_layout", l_vulkan_create_pipeline_layout},
    {"create_graphics_pipeline_create_info", l_vulkan_create_graphics_pipeline_create_info},
    {"create_graphics_pipeline", l_vulkan_create_graphics_pipeline},
    

    {"create_buffer_create_info", l_vulkan_create_buffer_create_info},
    {"create_buffer", l_vulkan_create_buffer},
    {"allocate_memory", l_vulkan_allocate_memory},
    {"bind_buffer_memory", l_vulkan_bind_buffer_memory},
    {"map_memory", l_vulkan_map_memory},
    {"create_command_pool_create_info", l_vulkan_create_command_pool_create_info},
    {"create_command_pool", l_vulkan_create_command_pool},
    {"create_command_buffer_allocate_info", l_vulkan_create_command_buffer_allocate_info},
    {"allocate_command_buffers", l_vulkan_allocate_command_buffers},
    {"begin_command_buffer", l_vulkan_begin_command_buffer},
    {"cmd_begin_render_pass", l_vulkan_cmd_begin_render_pass},
    {"cmd_bind_pipeline", l_vulkan_cmd_bind_pipeline},
    {"cmd_bind_vertex_buffers", l_vulkan_cmd_bind_vertex_buffers},
    {"cmd_draw", l_vulkan_cmd_draw},
    {"cmd_end_render_pass", l_vulkan_cmd_end_render_pass},
    {"end_command_buffer", l_vulkan_end_command_buffer},
    {"create_memory_allocate_info", l_vulkan_create_memory_allocate_info},
    {"get_buffer_memory_requirements", l_vulkan_get_buffer_memory_requirements},
    {"copy_to_memory", l_vulkan_copy_to_memory},
    {"create_command_pool_create_info", l_vulkan_create_command_pool_create_info},
    {"get_physical_device_memory_properties", l_vulkan_get_physical_device_memory_properties},
    {"unmap_memory", l_vulkan_unmap_memory},
    


    {NULL, NULL}
};

int luaopen_vulkan(lua_State* L) {
    appinfo_metatable(L);
    createinfo_metatable(L);
    instance_metatable(L);
    surface_metatable(L);
    physical_device_metatable(L);
    device_queue_create_info_metatable(L);
    device_create_info_metatable(L);
    device_metatable(L);
    queue_metatable(L);
    surface_format_metatable(L);
    swapchain_create_info_metatable(L);
    swapchain_metatable(L);

    image_view_create_info_metatable(L);
    image_view_metatable(L);
    attachment_description_metatable(L);
    subpass_description_metatable(L);
    subpass_dependency_metatable(L);
    render_pass_create_info_metatable(L);
    render_pass_metatable(L);

    framebuffer_create_info_metatable(L);
    framebuffer_metatable(L);

    extent_2d_metatable(L);
    shader_module_metatable(L);
    pipeline_shader_stage_create_info_metatable(L);
    vertex_input_binding_description_metatable(L);
    vertex_input_attribute_description_metatable(L);
    pipeline_vertex_input_state_create_info_metatable(L);
    pipeline_input_assembly_state_create_info_metatable(L);
    viewport_metatable(L);
    rect_2d_metatable(L);
    pipeline_viewport_state_create_info_metatable(L);
    pipeline_rasterization_state_create_info_metatable(L);
    pipeline_multisample_state_create_info_metatable(L);
    pipeline_color_blend_attachment_state_metatable(L);
    pipeline_color_blend_state_create_info_metatable(L);
    pipeline_layout_create_info_metatable(L);
    pipeline_layout_metatable(L);
    graphics_pipeline_create_info_metatable(L);
    pipeline_metatable(L);


    buffer_create_info_metatable(L);
    buffer_metatable(L);
    memory_allocate_info_metatable(L);
    device_memory_metatable(L);
    command_pool_create_info_metatable(L);
    command_pool_metatable(L);
    command_buffer_allocate_info_metatable(L);
    command_buffer_metatable(L);
    memory_requirements_metatable(L);

    physical_device_memory_properties_metatable(L);


    luaL_newlib(L, vulkan_lib);

    // Queue flag constants
    lua_pushinteger(L, VK_QUEUE_GRAPHICS_BIT);
    lua_setfield(L, -2, "QUEUE_GRAPHICS_BIT");
    lua_pushinteger(L, VK_QUEUE_COMPUTE_BIT);
    lua_setfield(L, -2, "QUEUE_COMPUTE_BIT");
    lua_pushinteger(L, VK_QUEUE_TRANSFER_BIT);
    lua_setfield(L, -2, "QUEUE_TRANSFER_BIT");

    // Add format and color space constants
    lua_pushinteger(L, VK_FORMAT_B8G8R8A8_SRGB);
    lua_setfield(L, -2, "FORMAT_B8G8R8A8_SRGB");
    lua_pushinteger(L, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR);
    lua_setfield(L, -2, "COLOR_SPACE_SRGB_NONLINEAR_KHR");

    // Add present mode constants
    lua_pushinteger(L, VK_PRESENT_MODE_FIFO_KHR);
    lua_setfield(L, -2, "PRESENT_MODE_FIFO_KHR");

    // Add image usage flags
    lua_pushinteger(L, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
    lua_setfield(L, -2, "IMAGE_USAGE_COLOR_ATTACHMENT_BIT");

    // Additional constants for render pass
    lua_pushinteger(L, VK_SAMPLE_COUNT_1_BIT);
    lua_setfield(L, -2, "SAMPLE_COUNT_1_BIT");
    lua_pushinteger(L, VK_ATTACHMENT_LOAD_OP_CLEAR);
    lua_setfield(L, -2, "ATTACHMENT_LOAD_OP_CLEAR");
    lua_pushinteger(L, VK_ATTACHMENT_LOAD_OP_DONT_CARE);
    lua_setfield(L, -2, "ATTACHMENT_LOAD_OP_DONT_CARE");
    lua_pushinteger(L, VK_ATTACHMENT_STORE_OP_STORE);
    lua_setfield(L, -2, "ATTACHMENT_STORE_OP_STORE");
    lua_pushinteger(L, VK_IMAGE_LAYOUT_UNDEFINED);
    lua_setfield(L, -2, "IMAGE_LAYOUT_UNDEFINED");
    lua_pushinteger(L, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
    lua_setfield(L, -2, "IMAGE_LAYOUT_PRESENT_SRC_KHR");
    lua_pushinteger(L, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    lua_setfield(L, -2, "IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL");
    lua_pushinteger(L, VK_PIPELINE_BIND_POINT_GRAPHICS);
    lua_setfield(L, -2, "PIPELINE_BIND_POINT_GRAPHICS");
    lua_pushinteger(L, VK_SUBPASS_EXTERNAL);
    lua_setfield(L, -2, "SUBPASS_EXTERNAL");
    lua_pushinteger(L, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
    lua_setfield(L, -2, "PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT");
    lua_pushinteger(L, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT);
    lua_setfield(L, -2, "ACCESS_COLOR_ATTACHMENT_WRITE_BIT");

    // New constants for pipeline
    lua_pushinteger(L, shaderc_vertex_shader);
    lua_setfield(L, -2, "SHADERC_VERTEX_SHADER");
    lua_pushinteger(L, shaderc_fragment_shader);
    lua_setfield(L, -2, "SHADERC_FRAGMENT_SHADER");
    lua_pushinteger(L, VK_SHADER_STAGE_VERTEX_BIT);
    lua_setfield(L, -2, "SHADER_STAGE_VERTEX_BIT");
    lua_pushinteger(L, VK_SHADER_STAGE_FRAGMENT_BIT);
    lua_setfield(L, -2, "SHADER_STAGE_FRAGMENT_BIT");
    lua_pushinteger(L, VK_VERTEX_INPUT_RATE_VERTEX);
    lua_setfield(L, -2, "VERTEX_INPUT_RATE_VERTEX");
    lua_pushinteger(L, VK_FORMAT_R32G32_SFLOAT);
    lua_setfield(L, -2, "FORMAT_R32G32_SFLOAT");
    lua_pushinteger(L, VK_FORMAT_R32G32B32_SFLOAT);
    lua_setfield(L, -2, "FORMAT_R32G32B32_SFLOAT");
    lua_pushinteger(L, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
    lua_setfield(L, -2, "PRIMITIVE_TOPOLOGY_TRIANGLE_LIST");
    lua_pushinteger(L, VK_POLYGON_MODE_FILL);
    lua_setfield(L, -2, "POLYGON_MODE_FILL");
    lua_pushinteger(L, VK_CULL_MODE_NONE);
    lua_setfield(L, -2, "CULL_MODE_NONE");
    lua_pushinteger(L, VK_FRONT_FACE_CLOCKWISE);
    lua_setfield(L, -2, "FRONT_FACE_CLOCKWISE");
    lua_pushinteger(L, VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT);
    lua_setfield(L, -2, "COLOR_WRITE_MASK_RGBA");


    lua_pushinteger(L, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
    lua_setfield(L, -2, "BUFFER_USAGE_VERTEX_BUFFER_BIT");
    lua_pushinteger(L, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
    lua_setfield(L, -2, "MEMORY_PROPERTY_HOST_VISIBLE_BIT");
    lua_pushinteger(L, VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    lua_setfield(L, -2, "MEMORY_PROPERTY_HOST_COHERENT_BIT");
    lua_pushinteger(L, VK_COMMAND_BUFFER_LEVEL_PRIMARY);
    lua_setfield(L, -2, "COMMAND_BUFFER_LEVEL_PRIMARY");
    lua_pushinteger(L, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
    lua_setfield(L, -2, "COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT");

    return 1;
}