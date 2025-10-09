
# Vulkan Constants in the Lua Module

The constants are grouped by their functional category, reflecting their usage in Vulkan API calls. Each constant is prefixed with vulkan. when accessed in Lua (e.g., vulkan.VK_API_VERSION_1_3). The values are derived from the Vulkan API and are typically integers or bitmasks used to configure structures like VkApplicationInfo, VkSwapchainCreateInfoKHR, or pipeline states.

1. API Versions
	These constants specify Vulkan API versions for use in VkApplicationInfo or other structures requiring version information.

- vulkan.VK_API_VERSION_1_0: Specifies Vulkan API version 1.0.0.
    - Value: VK_MAKE_API_VERSION(0, 1, 0, 0)
    - Usage: Used in create_vk_application_info to set the api_version field.
    - Example: app_info.api_version = vulkan.VK_API_VERSION_1_0
- vulkan.VK_API_VERSION_1_1: Specifies Vulkan API version 1.1.0.
    - Value: VK_MAKE_API_VERSION(0, 1, 1, 0)
    - Usage: Same as above, for Vulkan 1.1 features.
    - Example: app_info.api_version = vulkan.VK_API_VERSION_1_1
- vulkan.VK_API_VERSION_1_2: Specifies Vulkan API version 1.2.0.
    - Value: VK_MAKE_API_VERSION(0, 1, 2, 0)
    - Usage: Same as above, for Vulkan 1.2 features.
    - Example: app_info.api_version = vulkan.VK_API_VERSION_1_2
- vulkan.VK_API_VERSION_1_3: Specifies Vulkan API version 1.3.0.
    - Value: VK_MAKE_API_VERSION(0, 1, 3, 0)
    - Usage: Same as above, for Vulkan 1.3 features.
    - Example: app_info.api_version = vulkan.VK_API_VERSION_1_3

2. Physical Device Types
	These constants represent types of physical devices returned by create_physical_devices.

- vulkan.DEVICE_TYPE_OTHER: Unknown or unspecified device type.
    - Value: VK_PHYSICAL_DEVICE_TYPE_OTHER
    - Usage: Indicates a non-standard or unspecified GPU type.
- vulkan.DEVICE_TYPE_INTEGRATED_GPU: Integrated GPU (e.g., on-chip with CPU).
    - Value: VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU
    - Usage: Identifies integrated GPUs, typically with shared memory.
- vulkan.DEVICE_TYPE_DISCRETE_GPU: Discrete (dedicated) GPU.
    - Value: VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU
    - Usage: Identifies high-performance, standalone GPUs.
- vulkan.DEVICE_TYPE_VIRTUAL_GPU: Virtual GPU (e.g., for virtualization).
    - Value: VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU
    - Usage: Identifies virtualized GPU devices.
- vulkan.DEVICE_TYPE_CPU: CPU-based device (software rendering).
    - Value: VK_PHYSICAL_DEVICE_TYPE_CPU
    - Usage: Indicates a CPU used as a Vulkan device.
- Example Usage:
    
    lua
    ```lua
    local devices = vulkan.create_physical_devices(instance)
    for i, dev in ipairs(devices) do
        if dev.type == vulkan.DEVICE_TYPE_DISCRETE_GPU then
            print("Found discrete GPU: " .. dev.name)
        end
    end
    ```
    

3. Image Formats
	These constants specify image formats, used in create_swap_chain_KHR, create_image_view, or create_render_pass.

- vulkan.FORMAT_B8G8R8A8_SRGB: 32-bit BGRA format with sRGB color space.
    - Value: VK_FORMAT_B8G8R8A8_SRGB
    - Usage: Commonly used for swapchain images and render targets.
    - Example: swapchain_info.format = vulkan.FORMAT_B8G8R8A8_SRGB
- vulkan.FORMAT_R8G8B8A8_SRGB: 32-bit RGBA format with sRGB color space.
    - Value: VK_FORMAT_R8G8B8A8_SRGB
    - Usage: Alternative format for swapchain images or textures.
- Example Usage:
    
    lua
    ```lua
    local swapchain = vulkan.create_swap_chain_KHR(device, {
        surface = surface,
        format = vulkan.FORMAT_B8G8R8A8_SRGB,
        -- other fields
    })
    ```
    

4. Color Spaces
   These constants specify color spaces for swapchains, used in create_swap_chain_KHR.

- vulkan.COLOR_SPACE_SRGB_NONLINEAR_KHR: sRGB non-linear color space.
    - Value: VK_COLOR_SPACE_SRGB_NONLINEAR_KHR
    - Usage: Standard color space for most displays.
    - Example: swapchain_info.color_space = vulkan.COLOR_SPACE_SRGB_NONLINEAR_KHR

5. Image Usage Flags
	These bitmasks specify how images (e.g., swapchain images) can be used, for create_swap_chain_KHR.

- vulkan.IMAGE_USAGE_COLOR_ATTACHMENT: Image can be used as a color attachment.
    - Value: VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT
    - Usage: Indicates the image is used as a render target.
    - Example: swapchain_info.image_usage = vulkan.IMAGE_USAGE_COLOR_ATTACHMENT

6. Sharing Modes
	These constants define how resources are shared between queue families, used in create_swap_chain_KHR.

- vulkan.SHARING_MODE_EXCLUSIVE: Resource is exclusive to one queue family.
    - Value: VK_SHARING_MODE_EXCLUSIVE
    - Usage: Common for single-queue setups.
- vulkan.SHARING_MODE_CONCURRENT: Resource can be accessed by multiple queue families.
    - Value: VK_SHARING_MODE_CONCURRENT
    - Usage: Used when multiple queues need access (requires queue_family_indices).
- Example Usage:
    
    lua
    ```lua
    local swapchain = vulkan.create_swap_chain_KHR(device, {
        sharing_mode = vulkan.SHARING_MODE_EXCLUSIVE,
        queue_family_indices = {0},
        -- other fields
    })
    ```
    

7. Surface Transforms 
	These constants specify transformations applied to swapchain images, used in create_swap_chain_KHR.

- vulkan.SURFACE_TRANSFORM_IDENTITY: No transformation (default orientation).
    - Value: VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR
    - Usage: Images are presented as-is.
    - Example: swapchain_info.pre_transform = vulkan.SURFACE_TRANSFORM_IDENTITY

8. Composite Alpha Modes
	These constants define how alpha blending is handled during presentation, used in create_swap_chain_KHR.

- vulkan.COMPOSITE_ALPHA_OPAQUE: Alpha is ignored (opaque rendering).
    - Value: VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR
    - Usage: Common for most applications.
    - Example: swapchain_info.composite_alpha = vulkan.COMPOSITE_ALPHA_OPAQUE

9. Present Modes
	These constants specify how swapchain images are presented, used in create_swap_chain_KHR.

- vulkan.PRESENT_MODE_IMMEDIATE_KHR: Images are presented immediately (may cause tearing).
    - Value: VK_PRESENT_MODE_IMMEDIATE_KHR
    - Usage: Low latency, suitable for fast-paced applications.
- vulkan.PRESENT_MODE_MAILBOX_KHR: Uses a single-entry queue, replacing old images.
    - Value: VK_PRESENT_MODE_MAILBOX_KHR
    - Usage: Smooth rendering with low latency.
- vulkan.PRESENT_MODE_FIFO_KHR: First-in, first-out queue (V-Sync).
    - Value: VK_PRESENT_MODE_FIFO_KHR
    - Usage: Standard V-Sync mode, power-efficient.
- vulkan.PRESENT_MODE_FIFO_RELAXED_KHR: FIFO with relaxed timing (allows tearing if late).
    - Value: VK_PRESENT_MODE_FIFO_RELAXED_KHR
    - Usage: V-Sync with some flexibility.
- Example Usage:
    
    lua
    ```lua
    local swapchain = vulkan.create_swap_chain_KHR(device, {
        present_mode = vulkan.PRESENT_MODE_FIFO_KHR,
        -- other fields
    })
    ```
    

10. Image View Types
	These constants specify the type of image view, used in create_image_view.

- vulkan.IMAGE_VIEW_TYPE_2D: 2D image view.
    - Value: VK_IMAGE_VIEW_TYPE_2D
    - Usage: Common for swapchain images and textures.
    - Example: image_view_info.view_type = vulkan.IMAGE_VIEW_TYPE_2D

11. Component Swizzle
	These constants define how image components are mapped, used in create_image_view.

- vulkan.COMPONENT_SWIZZLE_IDENTITY: No swizzling (default mapping).
    - Value: VK_COMPONENT_SWIZZLE_IDENTITY
    - Usage: Maps components (R, G, B, A) to themselves.
- Example Usage:
    
    lua
    ```lua
    local image_view = vulkan.create_image_view(device, {
        components = {
            r = vulkan.COMPONENT_SWIZZLE_IDENTITY,
            g = vulkan.COMPONENT_SWIZZLE_IDENTITY,
            b = vulkan.COMPONENT_SWIZZLE_IDENTITY,
            a = vulkan.COMPONENT_SWIZZLE_IDENTITY
        },
        -- other fields
    })
    ```
    

12. Image Aspect Flags
	These bitmasks specify which aspects of an image are accessed, used in create_image_view.

- vulkan.IMAGE_ASPECT_COLOR_BIT: Color aspect of the image.
    - Value: VK_IMAGE_ASPECT_COLOR_BIT
    - Usage: Used for color attachments.
    - Example: image_view_info.subresource_range.aspect_mask = vulkan.IMAGE_ASPECT_COLOR_BIT

13. Sample Counts
	These constants specify multisampling levels, used in create_render_pass or pipeline creation.

- vulkan.SAMPLE_COUNT_1_BIT: No multisampling (1 sample per pixel).
    - Value: VK_SAMPLE_COUNT_1_BIT
    - Usage: Standard for non-multisampled rendering.
    - Example: render_pass_info.attachments[1].samples = vulkan.SAMPLE_COUNT_1_BIT

14. Attachment Operations
	These constants define how attachments are loaded or stored, used in create_render_pass.

- vulkan.ATTACHMENT_LOAD_OP_CLEAR: Clear the attachment before rendering.
    - Value: VK_ATTACHMENT_LOAD_OP_CLEAR
    - Usage: Initializes the attachment with a clear value.
- vulkan.ATTACHMENT_LOAD_OP_DONT_CARE: No specific load operation.
    - Value: VK_ATTACHMENT_LOAD_OP_DONT_CARE
    - Usage: Ignores initial attachment content.
- vulkan.ATTACHMENT_STORE_OP_STORE: Store the attachment after rendering.
    - Value: VK_ATTACHMENT_STORE_OP_STORE
    - Usage: Saves the rendered content.
- vulkan.ATTACHMENT_STORE_OP_DONT_CARE: No specific store operation.
    - Value: VK_ATTACHMENT_STORE_OP_DONT_CARE
    - Usage: Discards rendered content if not needed.
- Example Usage:
    
    lua
    ```lua
    local render_pass = vulkan.create_render_pass(device, {
        attachments = {
            {
                format = vulkan.FORMAT_B8G8R8A8_SRGB,
                load_op = vulkan.ATTACHMENT_LOAD_OP_CLEAR,
                store_op = vulkan.ATTACHMENT_STORE_OP_STORE,
                -- other fields
            }
        },
        -- other fields
    })
    ```
    

15. Image Layouts
	These constants specify the layout of images during rendering, used in create_render_pass or command buffers.

- vulkan.IMAGE_LAYOUT_UNDEFINED: Undefined layout (initial state).
    - Value: VK_IMAGE_LAYOUT_UNDEFINED
    - Usage: Indicates the image layout is unknown or not initialized.
- vulkan.IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL: Optimal layout for color attachments.
    - Value: VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
    - Usage: Used during rendering to color attachments.
- vulkan.IMAGE_LAYOUT_PRESENT_SRC_KHR: Layout for presenting images to the swapchain.
    - Value: VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
    - Usage: Final layout for swapchain images.
- Example Usage:
    
    lua
    ```lua
    local render_pass = vulkan.create_render_pass(device, {
        attachments = {
            {
                initial_layout = vulkan.IMAGE_LAYOUT_UNDEFINED,
                final_layout = vulkan.IMAGE_LAYOUT_PRESENT_SRC_KHR,
                -- other fields
            }
        },
        -- other fields
    })
    ```
    

16. Pipeline Bind Points
	These constants specify the type of pipeline to bind, used in create_render_pass or cmd_bind_pipeline.

- vulkan.PIPELINE_BIND_POINT_GRAPHICS: Graphics pipeline.
    - Value: VK_PIPELINE_BIND_POINT_GRAPHICS
    - Usage: Used for graphics rendering pipelines.
    - Example: render_pass_info.subpasses[1].pipeline_bind_point = vulkan.PIPELINE_BIND_POINT_GRAPHICS

17. Pipeline Stages
	These constants specify stages in the Vulkan pipeline, used in queue_submit or create_render_pass.

- vulkan.PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT: Stage for color attachment output.
    - Value: VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
    - Usage: Used in synchronization (e.g., semaphores or dependencies).
- Example Usage:
    
    lua
    ```lua
    local success = vulkan.queue_submit(queue, {
        wait_dst_stage_mask = {vulkan.PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT},
        -- other fields
    }, fence)
    ```
    

18. Access Flags
	These bitmasks specify how resources are accessed, used in create_render_pass or queue_submit.

- vulkan.ACCESS_COLOR_ATTACHMENT_WRITE: Write access to a color attachment.
    - Value: VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT
    - Usage: Specifies write operations during rendering.
    - Example: render_pass_info.dependencies[1].dst_access_mask = vulkan.ACCESS_COLOR_ATTACHMENT_WRITE

19. Subpass Special Values
     These constants are used to specify special subpass indices, used in create_render_pass.

- vulkan.SUBPASS_EXTERNAL: Refers to operations outside the render pass.
    - Value: VK_SUBPASS_EXTERNAL
    - Usage: Used in subpass dependencies.
    - Example: render_pass_info.dependencies[1].src_subpass = vulkan.SUBPASS_EXTERNAL

20. Shader Stages
     These constants specify shader stages, used in create_graphics_pipelines.

- vulkan.SHADER_STAGE_VERTEX: Vertex shader stage.
    - Value: VK_SHADER_STAGE_VERTEX_BIT
    - Usage: Specifies a vertex shader in pipeline creation.
- vulkan.SHADER_STAGE_FRAGMENT: Fragment shader stage.
    - Value: VK_SHADER_STAGE_FRAGMENT_BIT
    - Usage: Specifies a fragment shader in pipeline creation.
- Example Usage:
    
    lua
    ```lua
    local pipelines = vulkan.create_graphics_pipelines(device, {
        pipelines = {
            {
                stages = {
                    {shader_module = vertex_shader, stage = vulkan.SHADER_STAGE_VERTEX, name = "main"},
                    {shader_module = fragment_shader, stage = vulkan.SHADER_STAGE_FRAGMENT, name = "main"}
                },
                -- other fields
            }
        }
    })
    ```
    

21. Shaderc Shader Kinds
	These constants specify the type of shader for compilation, used in create_shader_module_str.

- vulkan.shaderc_vertex_shader: Vertex shader type.
    - Value: shaderc_vertex_shader
    - Usage: Indicates the shader is a vertex shader for shaderc compilation.
- vulkan.shaderc_fragment_shader: Fragment shader type.
    - Value: shaderc_fragment_shader
    - Usage: Indicates the shader is a fragment shader for shaderc compilation.
- Example Usage:
    
    lua
    ```lua
    local shader_module = vulkan.create_shader_module_str(device, vertex_shader_code, vulkan.shaderc_vertex_shader)
    ```
    

22. Miscellaneous
     These constants are used in various pipeline configurations, such as create_graphics_pipelines.

- vulkan.UINT64_MAX: Maximum 64-bit unsigned integer value.
    - Value: 0xFFFFFFFFFFFFFFFF
    - Usage: Used as a timeout value (e.g., in acquire_next_image_KHR).
    - Example: vulkan.acquire_next_image_KHR(device, swapchain, vulkan.UINT64_MAX, semaphore)

Notes

- Accessing Constants: All constants are accessed via the vulkan table (e.g., vulkan.FORMAT_B8G8R8A8_SRGB). They are registered in the Lua environment during module initialization (luaopen_vulkan in module_vulkan.c).
- Usage Context: Each constant is tied to specific Vulkan API structures or functions. Ensure the correct constant is used for the intended field (e.g., format in VkSwapchainCreateInfoKHR uses FORMAT_* constants).
- Completeness: The list above includes constants explicitly defined in the module's luaopen_vulkan function. Additional Vulkan constants may exist in the Vulkan API but are not exposed if not registered in the module.
- Bitmasks: Some constants (e.g., IMAGE_USAGE_COLOR_ATTACHMENT, ACCESS_COLOR_ATTACHMENT_WRITE) are bitmasks and can be combined using bitwise operations if supported by the API call.

ExampleHere’s a brief example showing how some constants are used together to set up a Vulkan swapchain and render pass:

lua
```lua
local vulkan = require 'vulkan'

-- Create application info
local app_info = vulkan.create_vk_application_info({
    application_name = "MyApp",
    application_version = vulkan.make_version(1, 0, 0),
    engine_name = "MyEngine",
    engine_version = vulkan.make_version(1, 0, 0),
    api_version = vulkan.VK_API_VERSION_1_3
})

-- Create swapchain
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

-- Create render pass
local render_pass = vulkan.create_render_pass(device, {
    attachments = {
        {
            format = vulkan.FORMAT_B8G8R8A8_SRGB,
            samples = vulkan.SAMPLE_COUNT_1_BIT,
            load_op = vulkan.ATTACHMENT_LOAD_OP_CLEAR,
            store_op = vulkan.ATTACHMENT_STORE_OP_STORE,
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

This documentation covers all Vulkan constants exposed by the Lua module, based on the provided module_vulkan.c implementation.