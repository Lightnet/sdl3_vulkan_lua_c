// module_vulkan.c
#include "module_vulkan.h"
#include <stdlib.h>
#include <string.h>

// Metatable names
static const char* APPINFO_MT = "vulkan.application_info";
static const char* CREATEINFO_MT = "vulkan.instance_create_info";
static const char* INSTANCE_MT = "vulkan.instance";
static const char* SURFACE_MT = "vulkan.surface";

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
    ud->instance = instance; // Store instance for cleanup
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
    luaL_checktype(L, 2, LUA_TNIL); // Ensure pAllocator is nil

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
    luaL_checktype(L, 3, LUA_TNIL); // Ensure pAllocator is nil

    VkSurfaceKHR surface;
    if (!SDL_Vulkan_CreateSurface(window_ud->window, instance_ud->instance, NULL, &surface)) {
        luaL_error(L, "Failed to create Vulkan surface: %s", SDL_GetError());
    }

    lua_push_VkSurfaceKHR(L, surface, instance_ud->instance);
    return 1;
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

// Module loader: Register functions
static const struct luaL_Reg vulkan_lib[] = {
    {"create_vk_application_info", l_vulkan_create_vk_application_info},
    {"get_instance_extensions", l_vulkan_get_instance_extensions},
    {"create_vk_create_info", l_vulkan_create_vk_create_info},
    {"vk_create_instance", l_vulkan_vk_create_instance},
    {"create_surface", l_vulkan_create_surface},
    {NULL, NULL}
};

int luaopen_vulkan(lua_State* L) {
    appinfo_metatable(L);
    createinfo_metatable(L);
    instance_metatable(L);
    surface_metatable(L);
    luaL_newlib(L, vulkan_lib);
    return 1;
}