# Information:
  When using the lua script for vulkan. Which has module package name vulkan. Lua is a case-sensitive language

  There are two methods to load the module.
```lua
local vulkan = require 'vulkan'
```

```lua
local vulkan = require('vulkan')
```

```lua
local vk = require('vulkan')
```
  Can make shorter varaible.

  There are couple are define for sample triangle for constent varaible that they are all upper case and underscore to match vulkan but note prefixed VK_ is remove since vulkan.VK_<name>. So be less press.

  Note that it need to rework later to fixed missing constant variable to match vulkan some degree.


# lua vulkan:
  The code use snake names for lua functions.

  Note that using function c is different from lua function.
```c
Uint32 sdlExtensionCount = 0;
const char *const *sdlExtensions = SDL_Vulkan_GetInstanceExtensions(&sdlExtensionCount);
```
 Example it can return sdlExtensionCount.

```lua
local count = 0
local extensions = instance_extensions(count)
print(count) -- 0
```
  It not possible to assign count since it not pointer which does not exist lua script as it will pass as local variable. It would return 0.

```lua
function instance_extensions(value)
    value.count = 2 -- Modify the table's field
end
local sdlExtensionCount = { count = 0 } -- Use a table
local sdlExtensions = instance_extensions(sdlExtensionCount)
print(sdlExtensionCount.count) -- Prints 2
```
  This is simple example of table use to assign a variable.

# Meta table
  Vulkan functions in lua will return metatable variable has been coded.


```c
struct VulkanContext {
    VkInstance instance;
    VkPhysicalDevice physicalDevice;
    VkDevice device;
    VkQueue graphicsQueue;
    VkSurfaceKHR surface;
    VkSwapchainKHR swapchain;
    VkRenderPass renderPass;
    VkPipelineLayout pipelineLayout;
    VkPipeline graphicsPipeline; // Original pipeline
    VkPipeline minimalPipeline;  // minimal pipeline
    VkCommandPool commandPool;
    VkCommandBuffer commandBuffer;
    VkBuffer vertexBuffer;
    VkDeviceMemory vertexMemory;
    VkSemaphore imageAvailableSemaphore;
    VkSemaphore renderFinishedSemaphore;
    VkFence inFlightFence;
    uint32_t imageCount;
    VkImage* swapchainImages;
    VkImageView* swapchainImageViews;
    VkFramebuffer* swapchainFramebuffers;
} vkCtx = {0};
```
  This is reference data.

```c
#define VK_API_VERSION_1_3 VK_MAKE_API_VERSION(0, 1, 3, 0)
```

# VkApplicationInfo:

```c
VkApplicationInfo appInfo = {VK_STRUCTURE_TYPE_APPLICATION_INFO};
```

```lua
local appinfo = vulkan.create_vk_application_info({
    application_name = "SDL3 Vulkan Lua Test",
    application_version = vulkan.make_version(1, 0, 0),
    engine_name = "Lua Vulkan",
    engine_version = vulkan.make_version(1, 0, 0),
    api_version = vulkan.VK_API_VERSION_1_3
})
print("appinfo:" .. tostring(appinfo))
```

# sdl_vulkan_get_instance_extensions:

```c
Uint32 sdlExtensionCount = 0;
const char *const *sdlExtensions = SDL_Vulkan_GetInstanceExtensions(&sdlExtensionCount);
```

```lua
local sdl_extensions = vulkan.sdl_vulkan_get_instance_extensions()
print("sdl_extensions:" .. tostring(sdl_extensions))
for i, ext in ipairs(sdl_extensions) do
    print("sdl_extensions: " .. ext)
end
```

# VkInstanceCreateInfo:

```c
VkInstanceCreateInfo createInfo = {VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO};
```

```lua
local create_info = vulkan.create_info({
    app_info = appinfo,
    extensions = sdl_extensions,
    layers = { "VK_LAYER_KHRONOS_validation" }
})
```

# vkCreateInstance:

```c
vkCreateInstance(&createInfo, NULL, &instance)
```

```lua
local instance = vulkan.create_instance(create_info)
```

# SDL_Vulkan_CreateSurface:

```c
SDL_Vulkan_CreateSurface(window, instance, NULL, &surface)
```

```lua
local surface = vulkan.sdl_vulkan_create_surface(window, instance)
```


# vkEnumeratePhysicalDevices:

```c
vkEnumeratePhysicalDevices(instance, &deviceCount, devices);
```

```lua
local physical_devices = vulkan.create_physical_devices(instance)
```

# vkGetPhysicalDeviceQueueFamilyProperties:

```c
// Find queue families
uint32_t queueFamilyCount = 0;
vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, NULL);
VkQueueFamilyProperties* queueFamilies = malloc(queueFamilyCount * sizeof(VkQueueFamilyProperties));
vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilies);
uint32_t graphicsFamily = UINT32_MAX, presentFamily = UINT32_MAX;
//...
```
```lua
local queue_families = vulkan.get_physical_devices_properties(physical_device, surface)
```

# vkCreateDevice:

```c
vkCreateDevice(physicalDevice, &deviceCreateInfo, NULL, &device)
```
```lua
local device = vulkan.create_device(physical_device, device_create_info)
```

# vkCreateSwapchainKHR:
```c
vkCreateSwapchainKHR(device, &swapchainCreateInfo, NULL, &swapchain)
```
```lua
swapchain = vulkan.create_swap_chain_KHR(device, create_info)
```

# vkCreateRenderPass:
```c
vkCreateRenderPass(device, &renderPassInfo, NULL, &renderPass)
```
```lua
local render_pass = vulkan.create_render_pass(device, {})
```

# vkCreateCommandPool:
```c
vkCreateCommandPool(device, &poolInfo, NULL, &commandPool)
```
```lua
local commandPool = vulkan.create_command_pool(device, graphics_family)
```

# vkAllocateCommandBuffers:
```c
vkAllocateCommandBuffers(device, &allocInfoCmd, commandBuffers) 
```
```lua
local commandBuffers = vulkan.create_allocate_command_buffers(device, commandPool, MAX_FRAMES_IN_FLIGHT)
```

# vkBeginCommandBuffer:
```c
vkBeginCommandBuffer(commandBuffers[i], &beginInfo);
```
```lua
vulkan.begin_command_buffer(cmdBuffer)
```

# vkCmdBeginRenderPass:
```c
vkCmdBeginRenderPass(commandBuffers[i], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
```
```lua
vulkan.cmd_begin_renderpass(cmdBuffer, render_pass, framebuffers[imageIndex + 1], {
        clear_values = { { r = 1.0, g = 0.0, b = 0.0, a = 1.0 } }
    })
```

# vkCmdBindPipeline:
```c
vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);
```
```lua
vulkan.cmd_bind_pipeline(cmdBuffer, pipelines[1])
```

# vkCmdEndRenderPass:
```c
vkCmdEndRenderPass(commandBuffers[i]);
```
```lua
vulkan.cmd_end_renderpass(cmdBuffer)
```

# vkCmdDraw:
```c
vkCmdDraw(commandBuffers[i], 3, 1, 0, 0);
```
```lua
vulkan.cmd_draw(cmdBuffer, 3, 1, 0, 0)
```

# vkEndCommandBuffer:
```c
vkEndCommandBuffer(commandBuffers[i]);
```
```lua
vulkan.end_commandbuffer(cmdBuffer)
```

# vkCreateSemaphore:
```c
VkSemaphoreCreateInfo semaphoreInfo = {VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
VkSemaphore imageAvailableSemaphore, renderFinishedSemaphore;
vkCreateSemaphore(device, &semaphoreInfo, NULL, &imageAvailableSemaphore);
```
```lua
imageAvailableSemaphores[i] = vulkan.create_semaphore(device)
```

# vkAcquireNextImageKHR:
```c
uint32_t imageIndex;
vkAcquireNextImageKHR(device, swapchain, UINT64_MAX, imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);
```
```
local imageIndex = vulkan.acquire_next_image_KHR(device, swapchain, vulkan.UINT64_MAX, imageAvailableSemaphores[currentFrame], nil)
```

# vkQueueSubmit:
```c
vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE)
```
```lua
local submit_result = vulkan.queue_submit(graphics_queue, {}, fence)
```

# vkQueuePresentKHR:
```c
vkQueuePresentKHR(presentQueue, &presentInfo);
```
```lua
local present_result, err_msg = vulkan.queue_present_KHR(present_queue, {})
```

# vkQueueWaitIdle:
```c
vkQueueWaitIdle(presentQueue);
```
```lua
vulkan.device_wait_idle(device)
```

# :
```c
```
```lua
```
