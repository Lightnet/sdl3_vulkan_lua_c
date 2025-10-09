# Lua Vulkan Module API Documentation (Lua 5.4)

This documentation covers the Lua interface for the Vulkan API as implemented in the provided module_vulkan.c file. The module integrates Vulkan functionality with Lua 5.4, allowing developers to create and manage Vulkan resources such as instances, devices, swapchains, and pipelines. The module uses SDL for window and surface creation and shaderc for shader compilation. Below is a detailed description of each function exposed by the module, including parameters, return values, potential errors, and example usage.To use the module, load it in your Lua script:

lua
```lua
local vulkan = require 'vulkan'
```

Table of Contents

1. [Instance Creation](#instance-creation)
    - [create_vk_application_info](#vulkan.create_vk_application_info)
    - [make_version](vulkan.make_version)
    - sdl_vulkan_get_instance_extensions
    - create_info
    - create_instance
2. Surface and Physical Device (#surface-and-physical-device)
    - sdl_vulkan_create_surface
    - create_physical_devices
3. Device and Queue (#device-and-queue)
    - get_physical_devices_properties
    - create_device_info
    - create_device
    - get_device_queue
4. Swapchain (#swapchain)
    - get_physical_device_surface_capabilities_KHR
    - create_swap_chain_KHR
    - get_swapchain_images_KHR
    - create_image_view
5. Render Pass (#render-pass)
    - create_render_pass
6. Framebuffer (#framebuffer)
    - create_framebuffer
7. Shader and Pipeline (#shader-and-pipeline)
    - create_shader_module
    - create_shader_module_str
    - create_pipeline_layout
    - create_graphics_pipelines
8. Synchronization and Command Buffers (#synchronization-and-command-buffers)
    - create_semaphore
    - create_fence
    - create_command_pool
    - create_allocate_command_buffers
    - wait_for_fences
    - reset_fences
    - acquire_next_image_KHR
    - reset_command_buffer
    - begin_command_buffer
    - cmd_begin_renderpass
    - cmd_bind_pipeline
    - cmd_draw
    - cmd_end_renderpass
    - end_commandbuffer
    - queue_submit
    - queue_present_KHR
9. Utility and Cleanup (#utility-and-cleanup)
    - device_wait_idle
    - destroy_framebuffer
    - destroy_image_view
    - destroy_swapchain_khr
    - get_physical_device_surface_support_KHR
    - cmd_set_viewport
    - cmd_set_scissor
    - destroy_semaphore
    - destroy_fence
    - destroy_command_pool
    - destroy_pipeline
    - destroy_pipeline_layout
    - destroy_shader_module
    - destroy_render_pass
    - destroy_device
    - destroy_surface_KHR
    - destroy_instance

---

# Instance Creation

## vulkan.create_vk_application_info

Description: Creates a VkApplicationInfo structure for Vulkan instance creation, specifying application and engine details.

- Parameters:
    - table (table): A table containing:
        - application_name (string, optional): Name of the application.
        - application_version (integer): Version of the application (use make_version).
        - engine_name (string, optional): Name of the engine.
        - engine_version (integer): Version of the engine (use make_version).
        - api_version (integer): Vulkan API version (e.g., vulkan.VK_API_VERSION_1_3).
- Return:
    - Userdata (lua_VkApplicationInfo): A userdata object containing the VkApplicationInfo structure.
- Error:
    - Throws an error if memory allocation fails or if required fields are invalid.
- Example:

lua

```lua
local app_info = vulkan.create_vk_application_info({
    application_name = "MyApp",
    application_version = vulkan.make_version(1, 0, 0),
    engine_name = "MyEngine",
    engine_version = vulkan.make_version(1, 0, 0),
    api_version = vulkan.VK_API_VERSION_1_3
})
```

---

## vulkan.make_version

Description: Constructs a Vulkan version number from major, minor, and patch components.

- Parameters:
    - major (integer): Major version number.
    - minor (integer): Minor version number.
    - patch (integer): Patch version number.
- Return:
    - Integer: The combined version number using Vulkan's VK_MAKE_API_VERSION.
- Error:
    - Throws an error if parameters are not integers.
- Example:

lua

```lua
local version = vulkan.make_version(1, 3, 0) -- Vulkan 1.3.0
```

---

## vulkan.sdl_vulkan_get_instance_extensions

Description: Retrieves the list of Vulkan instance extensions required by SDL.

- Parameters: None
- Return:
    - Table: A Lua table containing the names of required Vulkan instance extensions (1-based indices).
- Error:
    - Throws an error if SDL fails to retrieve extensions (SDL_GetError).
- Example:

lua

```lua
local extensions = vulkan.sdl_vulkan_get_instance_extensions()
for i, ext in ipairs(extensions) do
    print("Extension " .. i .. ": " .. ext)
end
```

---

## vulkan.create_info

Description: Creates a VkInstanceCreateInfo structure for Vulkan instance creation.

- Parameters:
    - table (table): A table containing:
        - app_info (lua_VkApplicationInfo, optional): Application info userdata.
        - extensions (table, optional): List of extension names.
        - layers (table, optional): List of validation layer names.
- Return:
    - Userdata (lua_VkInstanceCreateInfo): A userdata object containing the VkInstanceCreateInfo structure.
- Error:
    - Throws an error if memory allocation fails or if the input table is invalid.
- Example:

lua

```lua
local app_info = vulkan.create_vk_application_info({...})
local extensions = vulkan.sdl_vulkan_get_instance_extensions()
local create_info = vulkan.create_info({
    app_info = app_info,
    extensions = extensions,
    layers = {"VK_LAYER_KHRONOS_validation"}
})
```

---

## vulkan.create_instance

Description: Creates a Vulkan instance using the provided VkInstanceCreateInfo.
- Parameters:
    - create_info (lua_VkInstanceCreateInfo): The instance creation info userdata.
- Return:
    - Userdata (lua_VkInstance): A userdata object containing the VkInstance handle.
- Error:
    - Throws an error if instance creation fails (VkResult).
- Example:

lua

```lua
local create_info = vulkan.create_info({...})
local instance = vulkan.create_instance(create_info)
```

---

## Surface and Physical Device

## vulkan.sdl_vulkan_create_surface

Description: Creates a Vulkan surface for an SDL window.

- Parameters:
    - window (lua_SDL_Window): SDL window userdata.
    - instance (lua_VkInstance): Vulkan instance userdata.
- Return:
    - Userdata (lua_VkSurfaceKHR): A userdata object containing the VkSurfaceKHR handle.
- Error:
    - Throws an error if surface creation fails (SDL_GetError).
- Example:

lua

```lua
local sdl = require 'sdl'
local window = sdl.create_window({...})
local instance = vulkan.create_instance(create_info)
local surface = vulkan.sdl_vulkan_create_surface(window, instance)
```

---

## vulkan.create_physical_devices

Description: Enumerates available physical devices for a Vulkan instance.

- Parameters:
    - instance (lua_VkInstance): Vulkan instance userdata.
- Return:
    - Table: A Lua table of physical device information, each entry containing:
        - device (lua_VkPhysicalDevice): Physical device userdata.
        - name (string): Device name.
        - type (integer): Device type (e.g., vulkan.DEVICE_TYPE_DISCRETE_GPU).
        - api_version (integer): Supported Vulkan API version.
- Error:
    - Throws an error if enumeration fails or memory allocation fails.
- Example:

lua

```lua
local instance = vulkan.create_instance(create_info)
local devices = vulkan.create_physical_devices(instance)
for i, dev in ipairs(devices) do
    print("Device " .. i .. ": " .. dev.name .. ", Type: " .. dev.type)
end
```

---

## Device and Queue

## vulkan.get_physical_devices_properties

Description: Retrieves queue family properties for a physical device, optionally checking for surface presentation support.

- Parameters:
    - physical_device (lua_VkPhysicalDevice): Physical device userdata.
    - surface (lua_VkSurfaceKHR, optional): Surface to check for presentation support.
- Return:
    - Table: A Lua table of queue family properties, each entry containing:
        - queue_count (integer): Number of queues in the family.
        - graphics (boolean): Supports graphics operations.
        - compute (boolean): Supports compute operations.
        - transfer (boolean): Supports transfer operations.
        - present (boolean): Supports presentation (if surface provided).
- Error:
    - Throws an error if memory allocation fails or if surface support check fails.
- Example:

lua

```lua
local devices = vulkan.create_physical_devices(instance)
local surface = vulkan.sdl_vulkan_create_surface(window, instance)
local props = vulkan.get_physical_devices_properties(devices[1].device, surface)
for i, family in ipairs(props) do
    print("Family " .. i .. ": Graphics=" .. tostring(family.graphics) .. ", Present=" .. tostring(family.present))
end
```

---

## vulkan.create_device_info

Description: Creates a VkDeviceCreateInfo structure for logical device creation.

- Parameters:
    - table (table): A table containing:
        - queue_families (table): List of tables with family_index and queue_count.
        - extensions (table, optional): List of device extension names.
- Return:
    - Userdata (lua_VkDeviceCreateInfo): A userdata object containing the VkDeviceCreateInfo structure.
- Error:
    - Throws an error if memory allocation fails or if the input table is invalid.
- Example:

lua

```lua
local device_info = vulkan.create_device_info({
    queue_families = {{family_index = 0, queue_count = 1}},
    extensions = {"VK_KHR_swapchain"}
})
```

---

## vulkan.create_device

Description: Creates a Vulkan logical device.

- Parameters:
    - physical_device (lua_VkPhysicalDevice): Physical device userdata.
    - device_create_info (lua_VkDeviceCreateInfo): Device creation info userdata.
- Return:
    - Userdata (lua_VkDevice): A userdata object containing the VkDevice handle.
- Error:
    - Throws an error if device creation fails (VkResult).
- Example:

lua

```lua
local devices = vulkan.create_physical_devices(instance)
local device_info = vulkan.create_device_info({...})
local device = vulkan.create_device(devices[1].device, device_info)
```

---

## vulkan.get_device_queue

Description: Retrieves a queue from a logical device.

- Parameters:
    - device (lua_VkDevice): Logical device userdata.
    - family_index (integer): Queue family index.
    - queue_index (integer): Queue index within the family.
- Return:
    - Userdata (lua_VkQueue): A userdata object containing the VkQueue handle.
- Error:
    - Throws an error if the queue cannot be retrieved.
- Example:

lua

```lua
local queue = vulkan.get_device_queue(device, 0, 0)
```

---

# Swapchain
## vulkan.get_physical_device_surface_capabilities_KHR

Description: Retrieves surface capabilities for a physical device.

- Parameters:
    - physical_device (lua_VkPhysicalDevice): Physical device userdata.
    - surface (lua_VkSurfaceKHR): Surface userdata.
- Return:
    - Table: A table containing surface capabilities, including:
        - min_image_count, max_image_count, current_extent_width, current_extent_height, etc.
- Error:
    - Throws an error if capability retrieval fails (VkResult).
- Example:

lua

```lua
local caps = vulkan.get_physical_device_surface_capabilities_KHR(physical_device, surface)
print("Min Image Count: " .. caps.min_image_count)
```

---

## vulkan.create_swap_chain_KHR

Description: Creates a Vulkan swapchain.

- Parameters:
    - device (lua_VkDevice): Logical device userdata.
    - table (table): A table containing:
        - surface (lua_VkSurfaceKHR): Surface userdata.
        - min_image_count (integer): Minimum number of images.
        - format or image_format (integer): Image format (e.g., vulkan.FORMAT_B8G8R8A8_SRGB).
        - color_space or image_color_space (integer): Color space (e.g., vulkan.COLOR_SPACE_SRGB_NONLINEAR_KHR).
        - extent (table) or image_extent_width, image_extent_height (integers): Swapchain extent.
        - image_array_layers (integer): Number of array layers.
        - image_usage (integer): Image usage flags (e.g., vulkan.IMAGE_USAGE_COLOR_ATTACHMENT).
        - sharing_mode or image_sharing_mode (integer): Sharing mode (e.g., vulkan.SHARING_MODE_EXCLUSIVE).
        - queue_family_indices (table): List of queue family indices.
        - pre_transform (integer): Surface transform (e.g., vulkan.SURFACE_TRANSFORM_IDENTITY).
        - composite_alpha (integer): Composite alpha mode (e.g., vulkan.COMPOSITE_ALPHA_OPAQUE).
        - present_mode (integer): Present mode (e.g., vulkan.PRESENT_MODE_FIFO_KHR).
        - clipped (boolean): Whether to clip obscured pixels.
        - old_swapchain (lua_VkSwapchainKHR, optional): Previous swapchain to recycle.
- Return:
    - Userdata (lua_VkSwapchainKHR): A userdata object containing the VkSwapchainKHR handle.
    - If failed: nil, error message (string).
- Error:
    - Throws an error if swapchain creation fails or memory allocation fails.
- Example:

lua

```lua
local swapchain = vulkan.create_swap_chain_KHR(device, {
    surface = surface,
    min_image_count = 2,
    format = vulkan.FORMAT_B8G8R8A8_SRGB,
    color_space = vulkan.COLOR_SPACE_SRGB_NONLINEAR_KHR,
    extent = {width = 800, height = 600},
    image_array_layers = 1,
    image_usage = vulkan.IMAGE_USAGE_COLOR_ATTACHMENT,
    sharing_mode = vulkan.SHARING_MODE_EXCLUSIVE,
    queue_family_indices = {0},
    pre_transform = vulkan.SURFACE_TRANSFORM_IDENTITY,
    composite_alpha = vulkan.COMPOSITE_ALPHA_OPAQUE,
    present_mode = vulkan.PRESENT_MODE_FIFO_KHR,
    clipped = true
})
```

---

## vulkan.get_swapchain_images_KHR

Description: Retrieves the images in a swapchain.

- Parameters:
    - device (lua_VkDevice): Logical device userdata.
    - swapchain (lua_VkSwapchainKHR): Swapchain userdata.
- Return:
    - Table: A Lua table of VkImage handles (light userdata, 1-based indices).
- Error:
    - Throws an error if image retrieval fails or memory allocation fails.
- Example:

lua

```lua
local images = vulkan.get_swapchain_images_KHR(device, swapchain)
for i, img in ipairs(images) do
    print("Image " .. i .. ": " .. tostring(img))
end
```

---

## vulkan.create_image_view

Description: Creates an image view for a Vulkan image.

- Parameters:
    - device (lua_VkDevice): Logical device userdata.
    - table (table): A table containing:
        - image (light userdata): VkImage handle (e.g., from get_swapchain_images_KHR).
        - format (integer): Image format (e.g., vulkan.FORMAT_B8G8R8A8_SRGB).
        - view_type (integer): View type (e.g., vulkan.IMAGE_VIEW_TYPE_2D).
        - components (table): Component swizzle mappings (e.g., {r = vulkan.COMPONENT_SWIZZLE_IDENTITY, ...}).
        - subresource_range (table): Subresource range (e.g., {aspect_mask = vulkan.IMAGE_ASPECT_COLOR_BIT, ...}).
- Return:
    - Userdata (lua_VkImageView): A userdata object containing the VkImageView handle.
- Error:
    - Throws an error if image view creation fails or input is invalid.
- Example:

lua

```lua
local image_view = vulkan.create_image_view(device, {
    image = images[1],
    format = vulkan.FORMAT_B8G8R8A8_SRGB,
    view_type = vulkan.IMAGE_VIEW_TYPE_2D,
    components = {
        r = vulkan.COMPONENT_SWIZZLE_IDENTITY,
        g = vulkan.COMPONENT_SWIZZLE_IDENTITY,
        b = vulkan.COMPONENT_SWIZZLE_IDENTITY,
        a = vulkan.COMPONENT_SWIZZLE_IDENTITY
    },
    subresource_range = {
        aspect_mask = vulkan.IMAGE_ASPECT_COLOR_BIT,
        base_mip_level = 0,
        level_count = 1,
        base_array_layer = 0,
        layer_count = 1
    }
})
```

---

# Render Pass

## vulkan.create_render_pass

Description: Creates a Vulkan render pass.

- Parameters:
    - device (lua_VkDevice): Logical device userdata.
    - table (table): A table containing:
        - attachments (table): List of attachment descriptions (e.g., {format, samples, load_op, store_op, ...}).
        - subpasses (table): List of subpass descriptions (e.g., {pipeline_bind_point, color_attachments, ...}).
        - dependencies (table, optional): List of subpass dependencies.
- Return:
    - Userdata (lua_VkRenderPass): A userdata object containing the VkRenderPass handle.
- Error:
    - Throws an error if render pass creation fails or input is invalid.
- Example:

lua

```lua
local render_pass = vulkan.create_render_pass(device, {
    attachments = {
        {
            format = vulkan.FORMAT_B8G8R8A8_SRGB,
            samples = vulkan.SAMPLE_COUNT_1_BIT,
            load_op = vulkan.ATTACHMENT_LOAD_OP_CLEAR,
            store_op = vulkan.ATTACHMENT_STORE_OP_STORE,
            stencil_load_op = vulkan.ATTACHMENT_LOAD_OP_DONT_CARE,
            stencil_store_op = vulkan.ATTACHMENT_STORE_OP_DONT_CARE,
            initial_layout = vulkan.IMAGE_LAYOUT_UNDEFINED,
            final_layout = vulkan.IMAGE_LAYOUT_PRESENT_SRC_KHR
        }
    },
    subpasses = {
        {
            pipeline_bind_point = vulkan.PIPELINE_BIND_POINT_GRAPHICS,
            color_attachments = {{attachment = 0, layout = vulkan.IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL}}
        }
    },
    dependencies = {
        {
            src_subpass = vulkan.SUBPASS_EXTERNAL,
            dst_subpass = 0,
            src_stage_mask = vulkan.PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT,
            dst_stage_mask = vulkan.PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT,
            src_access_mask = 0,
            dst_access_mask = vulkan.ACCESS_COLOR_ATTACHMENT_WRITE
        }
    }
})
```

---

## Framebuffer

## vulkan.create_framebuffer

Description: Creates a Vulkan framebuffer.

- Parameters:
    - device (lua_VkDevice): Logical device userdata.
    - table (table): A table containing:
        - render_pass (lua_VkRenderPass): Render pass userdata.
        - attachments (table): List of lua_VkImageView userdata.
        - width, height, layers (integer): Framebuffer dimensions and layer count.
- Return:
    - Userdata (lua_VkFramebuffer): A userdata object containing the VkFramebuffer handle.
- Error:
    - Throws an error if framebuffer creation fails or input is invalid.
- Example:

lua

```lua
local framebuffer = vulkan.create_framebuffer(device, {
    render_pass = render_pass,
    attachments = {image_view},
    width = 800,
    height = 600,
    layers = 1
})
```

---

# Shader and Pipeline

## vulkan.create_shader_module

Description: Creates a Vulkan shader module from SPIR-V binary code.

- Parameters:
    - device (lua_VkDevice): Logical device userdata.
    - code (string): SPIR-V binary code as a string.
- Return:
    - Userdata (lua_VkShaderModule): A userdata object containing the VkShaderModule handle.
- Error:
    - Throws an error if shader module creation fails.
- Example:

lua

```lua
local spv_code = io.open("shader.spv", "rb"):read("*all")
local shader_module = vulkan.create_shader_module(device, spv_code)
```

---

## vulkan.create_shader_module_str

Description: Creates a Vulkan shader module from GLSL source code, compiling it to SPIR-V using shaderc.

- Parameters:
    - device (lua_VkDevice): Logical device userdata.
    - source (string): GLSL source code.
    - kind (integer): Shader type (e.g., vulkan.shaderc_vertex_shader or vulkan.shaderc_fragment_shader).
- Return:
    - Userdata (lua_VkShaderModule): A userdata object containing the VkShaderModule handle.
- Error:
    - Throws an error if compilation fails or shader module creation fails.
- Example:

lua

```lua
local vertex_shader_code = [[
#version 450
layout(location = 0) out vec4 outColor;
void main() {
    outColor = vec4(1.0, 0.0, 0.0, 1.0);
}
]]
local shader_module = vulkan.create_shader_module_str(device, vertex_shader_code, vulkan.shaderc_vertex_shader)
```

---

## vulkan.create_pipeline_layout

Description: Creates a Vulkan pipeline layout.

- Parameters:
    - device (lua_VkDevice): Logical device userdata.
    - table (table): A table containing:
        - set_layouts (table, optional): List of descriptor set layouts (not implemented in this module).
        - push_constant_ranges (table, optional): List of push constant ranges.
- Return:
    - Userdata (lua_VkPipelineLayout): A userdata object containing the VkPipelineLayout handle.
- Error:
    - Throws an error if pipeline layout creation fails.
- Example:

lua

```lua
local pipeline_layout = vulkan.create_pipeline_layout(device, {})
```

---

## vulkan.create_graphics_pipelines

Description: Creates multiple Vulkan graphics pipelines.

- Parameters:
    - device (lua_VkDevice): Logical device userdata.
    - table (table): A table containing:
        - pipelines (table): List of pipeline configurations, each with:
            - stages (table): Shader stages (e.g., {shader_module, stage, name}).
            - vertex_input, input_assembly, viewport, rasterization, multisample, color_blend, dynamic_state (tables): Pipeline state configurations.
            - layout (lua_VkPipelineLayout): Pipeline layout userdata.
            - render_pass (lua_VkRenderPass): Render pass userdata.
            - subpass (integer): Subpass index.
- Return:
    - Table: A Lua table of lua_VkPipeline userdata (1-based indices).
- Error:
    - Throws an error if pipeline creation fails or input is invalid.
- Example:

lua

```lua
local pipelines = vulkan.create_graphics_pipelines(device, {
    pipelines = {
        {
            stages = {
                {shader_module = vertex_shader, stage = vulkan.SHADER_STAGE_VERTEX, name = "main"},
                {shader_module = fragment_shader, stage = vulkan.SHADER_STAGE_FRAGMENT, name = "main"}
            },
            vertex_input = {bindings = {}, attributes = {}},
            input_assembly = {topology = 3, primitive_restart = false},
            viewport = {viewports = {{x = 0, y = 0, width = 800, height = 600, min_depth = 0, max_depth = 1}}, scissors = {{x = 0, y = 0, width = 800, height = 600}}},
            rasterization = {depth_clamp = false, rasterizer_discard = false, polygon_mode = 2, cull_mode = 0, front_face = 0, depth_bias = false},
            multisample = {rasterization_samples = vulkan.SAMPLE_COUNT_1_BIT},
            color_blend = {logic_op_enable = false, attachments = {{blend_enable = false, color_write_mask = 15}}},
            dynamic_state = {dynamic_states = {}},
            layout = pipeline_layout,
            render_pass = render_pass,
            subpass = 0
        }
    }
})
```

---

# Synchronization and Command Buffers

## vulkan.create_semaphore

Description: Creates a Vulkan semaphore.

- Parameters:
    - device (lua_VkDevice): Logical device userdata.
- Return:
    - Userdata (lua_VkSemaphore): A userdata object containing the VkSemaphore handle.
- Error:
    - Throws an error if semaphore creation fails.
- Example:

lua

```lua
local semaphore = vulkan.create_semaphore(device)
```

---

## vulkan.create_fence

Description: Creates a Vulkan fence.

- Parameters:
    - device (lua_VkDevice): Logical device userdata.
    - signaled (boolean): Whether the fence is initially signaled.
- Return:
    - Userdata (lua_VkFence): A userdata object containing the VkFence handle.
- Error:
    - Throws an error if fence creation fails.
- Example:

lua

```lua
local fence = vulkan.create_fence(device, true)
```

---

## vulkan.create_command_pool

Description: Creates a Vulkan command pool.

- Parameters:
    - device (lua_VkDevice): Logical device userdata.
    - queue_family_index (integer): Queue family index.
- Return:
    - Userdata (lua_VkCommandPool): A userdata object containing the VkCommandPool handle.
    - If failed: nil, error message (string).
- Error:
    - Throws an error if command pool creation fails.
- Example:

lua

```lua
local command_pool = vulkan.create_command_pool(device, 0)
```

---

## vulkan.create_allocate_command_buffers

Description: Allocates command buffers from a command pool.

- Parameters:
    - device (lua_VkDevice): Logical device userdata.
    - command_pool (lua_VkCommandPool): Command pool userdata.
    - count (integer): Number of command buffers to allocate.
- Return:
    - Table: A Lua table of lua_VkCommandBuffer userdata (1-based indices).
- Error:
    - Throws an error if allocation fails or memory allocation fails.
- Example:

lua

```lua
local command_buffers = vulkan.create_allocate_command_buffers(device, command_pool, 2)
```

---

## vulkan.wait_for_fences

Description: Waits for a fence to be signaled.

- Parameters:
    - device (lua_VkDevice): Logical device userdata.
    - fence (lua_VkFence): Fence userdata.
- Return: None
- Error:
    - Throws an error if waiting fails (VkResult).
- Example:

lua

```lua
vulkan.wait_for_fences(device, fence)
```

---

## vulkan.reset_fences

Description: Resets a fence to the unsignaled state.

- Parameters:
    - device (lua_VkDevice): Logical device userdata.
    - fence (lua_VkFence): Fence userdata.
- Return: None
- Error:
    - Throws an error if resetting fails (VkResult).
- Example:

lua

```lua
vulkan.reset_fences(device, fence)
```

---

## vulkan.acquire_next_image_KHR

Description: Acquires the next image from a swapchain.

- Parameters:
    - device (lua_VkDevice): Logical device userdata.
    - swapchain (lua_VkSwapchainKHR): Swapchain userdata.
    - timeout (number): Timeout in nanoseconds (e.g., vulkan.UINT64_MAX).
    - semaphore (lua_VkSemaphore): Semaphore to signal when the image is acquired.
    - fence (lua_VkFence, optional): Fence to signal when the image is acquired.
- Return:
    - Integer: The index of the acquired image, or nil if the swapchain is out of date or suboptimal.
- Error:
    - Throws an error if acquisition fails (except for out-of-date/suboptimal cases).
- Example:

lua

```lua
local image_index = vulkan.acquire_next_image_KHR(device, swapchain, vulkan.UINT64_MAX, semaphore, fence)
if image_index then
    print("Acquired image index: " .. image_index)
end
```

---

## vulkan.reset_command_buffer

Description: Resets a command buffer to its initial state.

- Parameters:
    - command_buffer (lua_VkCommandBuffer): Command buffer userdata.
- Return: None
- Error:
    - Throws an error if resetting fails (VkResult).
- Example:

lua

```lua
vulkan.reset_command_buffer(command_buffers[1])
```

---

## vulkan.begin_command_buffer

Description: Begins recording commands to a command buffer.

- Parameters:
    - command_buffer (lua_VkCommandBuffer): Command buffer userdata.
- Return: None
- Error:
    - Throws an error if beginning recording fails (VkResult).
- Example:

lua

```lua
vulkan.begin_command_buffer(command_buffers[1])
```

---

## vulkan.cmd_begin_renderpass

Description: Begins a render pass in a command buffer.

- Parameters:
    - command_buffer (lua_VkCommandBuffer): Command buffer userdata.
    - render_pass (lua_VkRenderPass): Render pass userdata.
    - framebuffer (lua_VkFramebuffer): Framebuffer userdata.
- Return: None
- Error: None (errors are handled internally by Vulkan).
- Example:

lua

```lua
vulkan.cmd_begin_renderpass(command_buffers[1], render_pass, framebuffer)
```

---

## vulkan.cmd_bind_pipeline

Description: Binds a graphics pipeline to a command buffer.

- Parameters:
    - command_buffer (lua_VkCommandBuffer): Command buffer userdata.
    - pipeline (lua_VkPipeline): Graphics pipeline userdata.
- Return: None
- Error: None (errors are handled internally by Vulkan).
- Example:

lua

```lua
vulkan.cmd_bind_pipeline(command_buffers[1], pipelines[1])
```

---

## vulkan.cmd_draw

Description: Issues a draw command in a command buffer.

- Parameters:
    - command_buffer (lua_VkCommandBuffer): Command buffer userdata.
    - vertex_count (integer): Number of vertices to draw.
    - instance_count (integer): Number of instances.
    - first_vertex (integer): First vertex index.
    - first_instance (integer): First instance index.
- Return: None
- Error: None (errors are handled internally by Vulkan).
- Example:

lua

```lua
vulkan.cmd_draw(command_buffers[1], 3, 1, 0, 0) -- Draw a triangle
```

---

## vulkan.cmd_end_renderpass

Description: Ends a render pass in a command buffer.

- Parameters:
    - command_buffer (lua_VkCommandBuffer): Command buffer userdata.
- Return: None
- Error: None (errors are handled internally by Vulkan).
- Example:

lua

```lua
vulkan.cmd_end_renderpass(command_buffers[1])
```

---

## vulkan.end_commandbuffer

Description: Ends recording commands in a command buffer.

- Parameters:
    - command_buffer (lua_VkCommandBuffer): Command buffer userdata.
- Return: None
- Error:
    - Throws an error if ending recording fails (VkResult).
- Example:

lua

```lua
vulkan.end_commandbuffer(command_buffers[1])
```

---

## vulkan.queue_submit

Description: Submits command buffers to a queue for execution.

- Parameters:
    - queue (lua_VkQueue): Queue userdata.
    - table (table): A table containing:
        - wait_semaphores (table): List of lua_VkSemaphore userdata.
        - wait_dst_stage_mask (table): List of pipeline stage flags.
        - command_buffers (table or lua_VkCommandBuffer): List of or single command buffer userdata.
        - signal_semaphores (table): List of lua_VkSemaphore userdata to signal.
    - fence (lua_VkFence, optional): Fence to signal when submission is complete.
- Return:
    - Boolean: true on success.
    - If failed: false, error message (string).
- Error:
    - Throws an error if submission fails or input is invalid.
- Example:

lua

```lua
local success = vulkan.queue_submit(queue, {
    wait_semaphores = {semaphore},
    wait_dst_stage_mask = {vulkan.PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT},
    command_buffers = {command_buffers[1]},
    signal_semaphores = {semaphore}
}, fence)
```

---

## vulkan.queue_present_KHR

Description: Presents swapchain images to the queue.

- Parameters:
    - queue (lua_VkQueue): Queue userdata.
    - table (table): A table containing:
        - wait_semaphores (table): List of lua_VkSemaphore userdata.
        - swapchains (table or lua_VkSwapchainKHR): List of or single swapchain userdata.
        - image_indices (table): List of image indices to present.
- Return:
    - Boolean: true on success or if suboptimal.
    - If failed: false, error message (string).
- Error:
    - Throws an error if presentation fails (except for suboptimal cases) or input is invalid.
- Example:

lua

```lua
local success = vulkan.queue_present_KHR(queue, {
    wait_semaphores = {semaphore},
    swapchains = {swapchain},
    image_indices = {image_index}
})
```

---

# Utility and Cleanup

## vulkan.device_wait_idle

Description: Waits for a device to become idle.

- Parameters:
    - device (lua_VkDevice): Logical device userdata.
- Return:
    - Boolean: true on success.
    - If failed: false, error message (string).
- Error:
    - Throws an error if waiting fails (VkResult).
- Example:

lua

```lua
local success = vulkan.device_wait_idle(device)
```

---

## vulkan.destroy_framebuffer

Description: Destroys a framebuffer.

- Parameters:
    - device (lua_VkDevice): Logical device userdata.
    - framebuffer (lua_VkFramebuffer): Framebuffer userdata.
- Return: None
- Error: None (double destruction is prevented).
- Example:

lua

```lua
vulkan.destroy_framebuffer(device, framebuffer)
```

---

## vulkan.destroy_image_view

Description: Destroys an image view.

- Parameters:
    - device (lua_VkDevice): Logical device userdata.
    - image_view (lua_VkImageView): Image view userdata.
- Return: None
- Error: None (double destruction is prevented).
- Example:

lua

```lua
vulkan.destroy_image_view(device, image_view)
```

---

## vulkan.destroy_swapchain_khr

Description: Destroys a swapchain.

- Parameters:
    - device (lua_VkDevice): Logical device userdata.
    - swapchain (lua_VkSwapchainKHR): Swapchain userdata.
- Return: None
- Error: None (double destruction is prevented).
- Example:

lua

```lua
vulkan.destroy_swapchain_khr(device, swapchain)
```

---

## vulkan.get_physical_device_surface_support_KHR

Description: Checks if a queue family supports presentation to a surface.

- Parameters:
    - physical_device (lua_VkPhysicalDevice): Physical device userdata.
    - queue_family_index (integer): Queue family index.
    - surface (lua_VkSurfaceKHR): Surface userdata.
- Return:
    - Boolean: true if presentation is supported.
- Error: None (errors are handled internally).
- Example:

lua

```lua
local supported = vulkan.get_physical_device_surface_support_KHR(physical_device, 0, surface)
```

---

## vulkan.cmd_set_viewport

Description: Sets viewports for a command buffer.

- Parameters:
    - command_buffer (lua_VkCommandBuffer): Command buffer userdata.
    - viewports (table): List of viewports, each with x, y, width, height, min_depth, max_depth.
- Return: None
- Error:
    - Throws an error if input is invalid.
- Example:

lua

```lua
vulkan.cmd_set_viewport(command_buffers[1], {
    {x = 0, y = 0, width = 800, height = 600, min_depth = 0, max_depth = 1}
})
```

---

## vulkan.cmd_set_scissor

Description: Sets scissor rectangles for a command buffer.

- Parameters:
    - command_buffer (lua_VkCommandBuffer): Command buffer userdata.
    - scissors (table): List of scissors, each with x, y, width, height.
- Return: None
- Error:
    - Throws an error if input is invalid.
- Example:

lua

```lua
vulkan.cmd_set_scissor(command_buffers[1], {
    {x = 0, y = 0, width = 800, height = 600}
})
```

---

## vulkan.destroy_semaphore

Description: Destroys a semaphore.

- Parameters:
    - device (lua_VkDevice): Logical device userdata.
    - semaphore (lua_VkSemaphore): Semaphore userdata.
- Return: None
- Error: None (double destruction is prevented).
- Example:

lua

```lua
vulkan.destroy_semaphore(device, semaphore)
```

---

## vulkan.destroy_fence

Description: Destroys a fence.

- Parameters:
    - device (lua_VkDevice): Logical device userdata.
    - fence (lua_VkFence): Fence userdata.
- Return: None
- Error: None (double destruction is prevented).
- Example:

lua

```lua
vulkan.destroy_fence(device, fence)
```

---

## vulkan.destroy_command_pool

Description: Destroys a command pool.

- Parameters:
    - device (lua_VkDevice): Logical device userdata.
    - command_pool (lua_VkCommandPool): Command pool userdata.
- Return: None
- Error: None (double destruction is prevented).
- Example:

lua

```lua
vulkan.destroy_command_pool(device, command_pool)
```

---

## vulkan.destroy_pipeline

Description: Destroys a graphics pipeline.

- Parameters:
    - device (lua_VkDevice): Logical device userdata.
    - pipeline (lua_VkPipeline): Pipeline userdata.
- Return: None
- Error: None (double destruction is prevented).
- Example:

lua

```lua
vulkan.destroy_pipeline(device, pipelines[1])
```

---

## vulkan.destroy_pipeline_layout

Description: Destroys a pipeline layout.

- Parameters:
    - device (lua_VkDevice): Logical device userdata.
    - pipeline_layout (lua_VkPipelineLayout): Pipeline layout userdata.
- Return: None
- Error: None (double destruction is prevented).
- Example:

lua

```lua
vulkan.destroy_pipeline_layout(device, pipeline_layout)
```

---

## vulkan.destroy_shader_module

Description: Destroys a shader module.

- Parameters:
    - device (lua_VkDevice): Logical device userdata.
    - shader_module (lua_VkShaderModule): Shader module userdata.
- Return: None
- Error: None (double destruction is prevented).
- Example:

lua

```lua
vulkan.destroy_shader_module(device, shader_module)
```

---

# vulkan.destroy_render_pass

Description: Destroys a render pass.

- Parameters:
    - device (lua_VkDevice): Logical device userdata.
    - render_pass (lua_VkRenderPass): Render pass userdata.
- Return: None
- Error: None (double destruction is prevented).
- Example:

lua

```lua
vulkan.destroy_render_pass(device, render_pass)
```

---

## vulkan.destroy_device

Description: Destroys a logical device.

- Parameters:
    - device (lua_VkDevice): Logical device userdata.
- Return: None
- Error: None (double destruction is prevented).
- Example:

lua

```lua
vulkan.destroy_device(device)
```

---

## vulkan.destroy_surface_KHR

Description: Destroys a Vulkan surface.

- Parameters:
    - instance (lua_VkInstance): Vulkan instance userdata.
    - surface (lua_VkSurfaceKHR): Surface userdata.
- Return: None
- Error: None (double destruction is prevented).
- Example:

lua

```lua
vulkan.destroy_surface_KHR(instance, surface)
```

---

## vulkan.destroy_instance

Description: Destroys a Vulkan instance.

- Parameters:
    - instance (lua_VkInstance): Vulkan instance userdata.
- Return: None
- Error: None (double destruction is prevented).
- Example:

lua

```lua
vulkan.destroy_instance(instance)
```

---

Notes

- Memory Management: The module uses Lua's garbage collector to clean up Vulkan resources. Ensure resources are properly released by letting userdata go out of scope or calling explicit destroy functions.
- Error Handling: Most functions throw Lua errors on failure, except where noted (e.g., create_swap_chain_KHR may return nil for specific cases).
- Constants: The module provides Vulkan constants (e.g., vulkan.VK_API_VERSION_1_3, vulkan.FORMAT_B8G8R8A8_SRGB) for use in configuration tables.
- SDL Integration: Functions like sdl_vulkan_create_surface require an SDL window, provided by a separate SDL module (module_sdl.h).
- Shader Compilation: The create_shader_module_str function uses shaderc to compile GLSL to SPIR-V, supporting shaderc_vertex_shader and shaderc_fragment_shader.

This documentation provides a comprehensive guide to using the Vulkan Lua module for creating and managing Vulkan resources in a Lua 5.4 environment.