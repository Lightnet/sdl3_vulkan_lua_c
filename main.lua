-- main.lua
local sdl = require 'sdl'
local vulkan = require 'vulkan'

sdl.init()

local window = sdl.create_window("SDL3 Vulkan Demo", 800, 600, sdl.WINDOW_VULKAN | sdl.WINDOW_RESIZABLE)
local window_id = window.windowID

-- Create VkApplicationInfo
local appinfo = vulkan.create_vk_application_info({pApplicationName = "Vulkan SDL3 Test"})
print("Created VkApplicationInfo with app name:", tostring(appinfo))

local extensions = vulkan.get_instance_extensions()
for i, ext in ipairs(extensions) do
    print("Extension", i, ext)
end

-- Create VkInstanceCreateInfo
local createinfo = vulkan.create_vk_create_info({
    pApplicationInfo = appinfo,
    enabledExtensionCount = #extensions,
    ppEnabledExtensionNames = extensions
})
print("Created VkInstanceCreateInfo:", tostring(createinfo))

-- Create VkInstance
local instance = vulkan.vk_create_instance(createinfo, nil)
print("Created VkInstance:", tostring(instance))

-- Clean up appinfo, extensions, and createinfo
print("Cleaning up appinfo, extensions, and createinfo...")
appinfo = nil
extensions = nil
createinfo = nil

-- Create VkSurfaceKHR
local surface = vulkan.create_surface(window, instance, nil)
print("Created VkSurfaceKHR:", tostring(surface))

-- Get physical device count
local device_count = vulkan.get_device_count(instance)
print("Physical device count:", device_count)

-- Get physical devices
local devices = vulkan.get_devices(instance, device_count)
print("Physical devices:")
local physical_device = nil
for i, device in ipairs(devices) do
    local props = vulkan.get_physical_device(device)
    print(string.format("  Device %d: %s (Type: %d)", i, props.deviceName, props.deviceType))
    -- Select discrete GPU (type 2) or integrated GPU (type 1) if no discrete GPU is found
    if props.deviceType == 2 then -- VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU
        physical_device = device
    elseif props.deviceType == 1 and not physical_device then -- VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU
        physical_device = device
    end
end

-- Check if a suitable GPU was found
if not physical_device then
    print("No suitable GPU found")
    return
end
print("Selected physical device:", tostring(physical_device))

-- Check device extensions
local device_extensions_supported = vulkan.get_device_extensions(physical_device)
local swapchain_supported = false
for _, ext in ipairs(device_extensions_supported) do
    if ext == "VK_KHR_swapchain" then
        swapchain_supported = true
        break
    end
end
if not swapchain_supported then
    print("Error: Selected device does not support VK_KHR_swapchain extension")
    return
end
print("VK_KHR_swapchain is supported")

-- Force garbage collection to trigger __gc metamethods
collectgarbage()
print("Garbage collection triggered")

-- Verify instance, surface, and selected device are still valid
print("VkInstance after cleanup:", tostring(instance))
print("VkSurfaceKHR after cleanup:", tostring(surface))
if physical_device then
    print("Selected physical device after cleanup:", tostring(physical_device))
end

-- Get queue family count
local queue_family_count = vulkan.get_family_count(physical_device)
print("Queue family count:", queue_family_count)

-- Get queue family properties
local queue_families = vulkan.queue_families(physical_device, queue_family_count, surface)
print("Queue families:")
local graphics_family, present_family = nil, nil
for i, family in ipairs(queue_families) do
    local flags = {}
    if family.queueFlags & vulkan.QUEUE_GRAPHICS_BIT ~= 0 then
        table.insert(flags, "Graphics")
    end
    if family.queueFlags & vulkan.QUEUE_COMPUTE_BIT ~= 0 then
        table.insert(flags, "Compute")
    end
    if family.queueFlags & vulkan.QUEUE_TRANSFER_BIT ~= 0 then
        table.insert(flags, "Transfer")
    end
    print(string.format("  Family %d: %s, Queue Count: %d, Present Support: %s",
        i, table.concat(flags, ", "), family.queueCount, family.presentSupport and "Yes" or "No"))
    if family.queueFlags & vulkan.QUEUE_GRAPHICS_BIT ~= 0 and not graphics_family then
        graphics_family = i
    end
    if family.presentSupport and not present_family then
        present_family = i
    end
    if graphics_family and present_family then
        break
    end
end

if not graphics_family or not present_family then
    print("No suitable queue families found")
    return
end
print("Selected graphics family:", graphics_family)
print("Selected present family:", present_family)

-- Create VkDeviceQueueCreateInfo
local queue_create_infos = vulkan.queue_create_infos(graphics_family, present_family)

-- Create VkDeviceCreateInfo
local device_extensions = {"VK_KHR_swapchain"}
local device_create_info = vulkan.device_create_info({
    pQueueCreateInfos = queue_create_infos,
    enabledExtensionCount = #device_extensions,
    ppEnabledExtensionNames = device_extensions
})

-- Create VkDevice
local device = vulkan.create_device(physical_device, device_create_info, nil)
print("Created VkDevice:", tostring(device))

-- Get queues
local graphics_queue = vulkan.get_device_queue(device, graphics_family, 0)
local present_queue = vulkan.get_device_queue(device, present_family, 0)
print("Graphics Queue:", tostring(graphics_queue))
print("Present Queue:", tostring(present_queue))

-- Get surface capabilities
local capabilities = vulkan.get_surface_capabilities(physical_device, surface)
print("Surface Capabilities:")
print("  Min Image Count:", capabilities.minImageCount)
print("  Max Image Count:", capabilities.maxImageCount)
print("  Current Extent:", capabilities.currentExtentWidth, "x", capabilities.currentExtentHeight)
print("  Min Extent:", capabilities.minImageExtentWidth, "x", capabilities.minImageExtentHeight)
print("  Max Extent:", capabilities.maxImageExtentWidth, "x", capabilities.maxImageExtentHeight)
print("  Max Image Array Layers:", capabilities.maxImageArrayLayers)
print("  Supported Transforms:", capabilities.supportedTransforms)
print("  Current Transform:", capabilities.currentTransform)
print("  Supported Composite Alpha:", capabilities.supportedCompositeAlpha)
print("  Supported Usage Flags:", capabilities.supportedUsageFlags)

-- Get surface formats
local surface_formats = vulkan.get_surface_formats(physical_device, surface)
print("Surface Formats:", #surface_formats)

-- Select surface format (prefer VK_FORMAT_B8G8R8A8_SRGB with SRGB nonlinear color space)
local selected_format = surface_formats[1]
for i, fmt in ipairs(surface_formats) do
    if fmt.format == vulkan.FORMAT_B8G8R8A8_SRGB and fmt.colorSpace == vulkan.COLOR_SPACE_SRGB_NONLINEAR_KHR then
        selected_format = fmt
        break
    end
end
print("Selected Surface Format: format =", selected_format.format, "colorSpace =", selected_format.colorSpace)

-- Get present modes
local present_modes = vulkan.get_surface_present_modes(physical_device, surface)
print("Present Modes:", #present_modes)

-- Select present mode (default to VK_PRESENT_MODE_FIFO_KHR)
local selected_present_mode = vulkan.PRESENT_MODE_FIFO_KHR
print("Selected Present Mode:", selected_present_mode)

-- Create VkSwapchainKHR
local function create_swapchain(device, surface, capabilities, selected_format, selected_present_mode)
    local swapchain_create_info = vulkan.create_swapchain_create_info({
        surface = surface,
        minImageCount = capabilities.minImageCount,
        imageFormat = selected_format.format,
        imageColorSpace = selected_format.colorSpace,
        imageExtent = { width = capabilities.currentExtentWidth, height = capabilities.currentExtentHeight },
        imageArrayLayers = 1,
        imageUsage = vulkan.IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        presentMode = selected_present_mode
    })
    return vulkan.create_swapchain(device, swapchain_create_info, nil), swapchain_create_info
end

local swapchain, swapchain_create_info = create_swapchain(device, surface, capabilities, selected_format, selected_present_mode)
print("Created VkSwapchainKHR:", tostring(swapchain))

-- Get swapchain images
local swapchain_images = vulkan.get_swapchain_images(device, swapchain)
print("Swapchain Images:", #swapchain_images)

for i, img in ipairs(swapchain_images) do
    print("Image", i, ":", tostring(img))
end

-- Create image views for swapchain images
local function create_image_views(device, swapchain_images, selected_format)
    local image_views = {}
    for i, img in ipairs(swapchain_images) do
        local image_view_create_info = vulkan.create_image_view_create_info({
            image = img,
            format = selected_format.format
        })
        local image_view = vulkan.create_image_view(device, image_view_create_info, nil)
        image_views[i] = image_view
        print("Created VkImageView", i, ":", tostring(image_view))
    end
    return image_views
end

local image_views = create_image_views(device, swapchain_images, selected_format)

-- Create render pass
local color_attachment = vulkan.create_attachment_description({
    format = selected_format.format
})

local color_attachment_ref = {
    attachment = 0,
    layout = vulkan.IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
}

local subpass = vulkan.create_subpass_description({
    pColorAttachments = { color_attachment_ref }
})

local dependency = vulkan.create_subpass_dependency({})

local render_pass_create_info = vulkan.create_render_pass_create_info({
    pAttachments = { color_attachment },
    pSubpasses = { subpass },
    pDependencies = { dependency }
})

local render_pass = vulkan.create_render_pass(device, render_pass_create_info, nil)
print("Created VkRenderPass:", tostring(render_pass))

-- Create framebuffers
local function create_framebuffers(device, render_pass, image_views, width, height)
    local framebuffers = {}
    for i, view in ipairs(image_views) do
        local framebuffer_create_info = vulkan.create_framebuffer_create_info({
            renderPass = render_pass,
            pAttachments = { view },
            width = width,
            height = height
        })
        local framebuffer = vulkan.create_framebuffer(device, framebuffer_create_info, nil)
        framebuffers[i] = framebuffer
        print("Created VkFramebuffer", i, ":", tostring(framebuffer))
    end
    return framebuffers
end

local framebuffers = create_framebuffers(device, render_pass, image_views, capabilities.currentExtentWidth, capabilities.currentExtentHeight)

-- Create extent
local extent = vulkan.create_extent_2d({
    width = capabilities.currentExtentWidth,
    height = capabilities.currentExtentHeight
})

-- Vertex shader source
local vertex_shader_source = [[
#version 450
layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 0) out vec3 fragColor;
void main() {
    gl_Position = vec4(inPosition, 0.0, 1.0);
    fragColor = inColor;
}
]]

-- Fragment shader source
local fragment_shader_source = [[
#version 450
layout(location = 0) in vec3 fragColor;
layout(location = 0) out vec4 outColor;
void main() {
    outColor = vec4(fragColor, 1.0);
}
]]

-- Create shader modules
local vert_shader_module = vulkan.create_shader_module(device, vertex_shader_source, vulkan.SHADERC_VERTEX_SHADER)
print("Created VkShaderModule (vertex):", tostring(vert_shader_module))
local frag_shader_module = vulkan.create_shader_module(device, fragment_shader_source, vulkan.SHADERC_FRAGMENT_SHADER)
print("Created VkShaderModule (fragment):", tostring(frag_shader_module))

-- Create shader stages
local shader_stages = {
    vulkan.create_pipeline_shader_stage_create_info({
        stage = vulkan.SHADER_STAGE_VERTEX_BIT,
        module = vert_shader_module
    }),
    vulkan.create_pipeline_shader_stage_create_info({
        stage = vulkan.SHADER_STAGE_FRAGMENT_BIT,
        module = frag_shader_module
    })
}

-- Create vertex input binding description
local binding_desc = vulkan.create_vertex_input_binding_description({
    binding = 0,
    stride = 5 * 4, -- sizeof(float) * 5 (2 for position, 3 for color)
    inputRate = vulkan.VERTEX_INPUT_RATE_VERTEX
})

-- Create vertex input attribute descriptions
local attrib_descs = {
    vulkan.create_vertex_input_attribute_description({
        location = 0,
        binding = 0,
        format = vulkan.FORMAT_R32G32_SFLOAT,
        offset = 0
    }),
    vulkan.create_vertex_input_attribute_description({
        location = 1,
        binding = 0,
        format = vulkan.FORMAT_R32G32B32_SFLOAT,
        offset = 2 * 4
    })
}

-- Create vertex input state
local vertex_input_info = vulkan.create_pipeline_vertex_input_state_create_info({
    pVertexBindingDescriptions = { binding_desc },
    pVertexAttributeDescriptions = attrib_descs
})

-- Create input assembly state
local input_assembly = vulkan.create_pipeline_input_assembly_state_create_info({
    topology = vulkan.PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
    primitiveRestartEnable = false
})

-- Create viewport
local viewport = vulkan.create_viewport({
    x = 0.0,
    y = 0.0,
    width = extent.width,
    height = extent.height,
    minDepth = 0.0,
    maxDepth = 1.0
})

-- Create scissor
local scissor = vulkan.create_rect_2d({
    offset = { x = 0, y = 0 },
    extent = extent
})

-- Create viewport state
local viewport_state = vulkan.create_pipeline_viewport_state_create_info({
    pViewports = { viewport },
    pScissors = { scissor }
})

-- Create rasterization state
local rasterizer = vulkan.create_pipeline_rasterization_state_create_info({
    polygonMode = vulkan.POLYGON_MODE_FILL,
    cullMode = vulkan.CULL_MODE_NONE,
    frontFace = vulkan.FRONT_FACE_CLOCKWISE,
    lineWidth = 1.0
})

-- Create multisample state
local multisampling = vulkan.create_pipeline_multisample_state_create_info({
    rasterizationSamples = vulkan.SAMPLE_COUNT_1_BIT,
    sampleShadingEnable = false
})

-- Create color blend attachment
local color_blend_attachment = vulkan.create_pipeline_color_blend_attachment_state({
    blendEnable = false,
    colorWriteMask = vulkan.COLOR_WRITE_MASK_RGBA
})

-- Create color blend state
local color_blending = vulkan.create_pipeline_color_blend_state_create_info({
    pAttachments = { color_blend_attachment }
})

-- Create pipeline layout
local pipeline_layout_info = vulkan.create_pipeline_layout_create_info({})
local pipeline_layout = vulkan.create_pipeline_layout(device, pipeline_layout_info, nil)
print("Created VkPipelineLayout:", tostring(pipeline_layout))

-- Create graphics pipeline
local pipeline_info = vulkan.create_graphics_pipeline_create_info({
    pStages = shader_stages,
    pVertexInputState = vertex_input_info,
    pInputAssemblyState = input_assembly,
    pViewportState = viewport_state,
    pRasterizationState = rasterizer,
    pMultisampleState = multisampling,
    pColorBlendState = color_blending,
    layout = pipeline_layout,
    renderPass = render_pass,
    subpass = 0
})

local graphics_pipeline = vulkan.create_graphics_pipeline(device, nil, nil, pipeline_info)
print("Created VkPipeline:", tostring(graphics_pipeline))

-- Vertex data for a triangle (position: vec2, color: vec3)
local vertices = {
    0.0, -0.5, 1.0, 0.0, 0.0, -- Top, red
    0.5, 0.5, 0.0, 1.0, 0.0,  -- Bottom-right, green
    -0.5, 0.5, 0.0, 0.0, 1.0   -- Bottom-left, blue
}

-- Create vertex buffer
local vertex_buffer_create_info = vulkan.create_buffer_create_info({
    size = #vertices * 4, -- 3 vertices * 5 floats * sizeof(float)
    usage = vulkan.BUFFER_USAGE_VERTEX_BUFFER_BIT
})
local vertex_buffer = vulkan.create_buffer(device, vertex_buffer_create_info, nil)
print("Created VkBuffer:", tostring(vertex_buffer))

-- Get memory requirements
local mem_requirements = vulkan.get_buffer_memory_requirements(device, vertex_buffer)
print("Memory requirements: size =", mem_requirements.size, "alignment =", mem_requirements.alignment, "memoryTypeBits =", mem_requirements.memoryTypeBits)

-- Get physical device memory properties
local mem_properties = vulkan.get_physical_device_memory_properties(physical_device)

-- Find suitable memory type
local memory_type_index = -1
for i = 1, mem_properties.memoryTypeCount do
    local memory_type = mem_properties.memoryTypes[i]
    if (mem_requirements.memoryTypeBits & (1 << (i - 1))) ~= 0 then
        if (memory_type.propertyFlags & (vulkan.MEMORY_PROPERTY_HOST_VISIBLE_BIT | vulkan.MEMORY_PROPERTY_HOST_COHERENT_BIT)) ==
           (vulkan.MEMORY_PROPERTY_HOST_VISIBLE_BIT | vulkan.MEMORY_PROPERTY_HOST_COHERENT_BIT) then
            memory_type_index = i - 1
            break
        end
    end
end
if memory_type_index == -1 then
    error("Failed to find suitable memory type")
end
print("Selected memory type index:", memory_type_index)

-- Allocate memory
local memory_allocate_info = vulkan.create_memory_allocate_info({
    allocationSize = mem_requirements.size,
    memoryTypeIndex = memory_type_index
})
local vertex_memory = vulkan.allocate_memory(device, memory_allocate_info, nil)
print("Allocated VkDeviceMemory:", tostring(vertex_memory))

-- Bind memory to buffer
vulkan.bind_buffer_memory(device, vertex_buffer, vertex_memory, 0)

-- Map memory and copy vertex data
local data = vulkan.map_memory(device, vertex_memory, 0, mem_requirements.size)
vulkan.copy_to_memory(device, data, vertices, mem_requirements.size)
vulkan.unmap_memory(device, vertex_memory)

-- Create command pool
local command_pool_create_info = vulkan.create_command_pool_create_info({
    queueFamilyIndex = graphics_family
})
local command_pool = vulkan.create_command_pool(device, command_pool_create_info, nil)
print("Created VkCommandPool:", tostring(command_pool))

-- Create command buffers
local function create_command_buffers(device, command_pool, framebuffers, render_pass, extent, graphics_pipeline, vertex_buffer)
    local command_buffer_allocate_info = vulkan.create_command_buffer_allocate_info({
        commandPool = command_pool,
        commandBufferCount = #framebuffers
    })
    local command_buffers = vulkan.allocate_command_buffers(device, command_buffer_allocate_info)
    print("Allocated", #command_buffers, "VkCommandBuffers")

    for i, framebuffer in ipairs(framebuffers) do
        local cmd = command_buffers[i]
        vulkan.begin_command_buffer(cmd)
        vulkan.cmd_begin_render_pass(cmd, render_pass, framebuffer, { renderArea = { extent = extent } })
        vulkan.cmd_bind_pipeline(cmd, graphics_pipeline)
        vulkan.cmd_bind_vertex_buffers(cmd, 0, { vertex_buffer }, { 0 })
        vulkan.cmd_draw(cmd, 3, 1, 0, 0)
        vulkan.cmd_end_render_pass(cmd)
        vulkan.end_command_buffer(cmd)
        print("Recorded VkCommandBuffer", i, ":", tostring(cmd))
    end
    return command_buffers
end

local command_buffers = create_command_buffers(device, command_pool, framebuffers, render_pass, extent, graphics_pipeline, vertex_buffer)

-- Create semaphores
local semaphore_info = vulkan.create_semaphore_create_info({})
local image_available_semaphore = vulkan.create_semaphore(device, semaphore_info, nil)
local render_finished_semaphore = vulkan.create_semaphore(device, semaphore_info, nil)
print("Created imageAvailableSemaphore:", tostring(image_available_semaphore))
print("Created renderFinishedSemaphore:", tostring(render_finished_semaphore))

-- Function to recreate swapchain and dependent resources
local function recreate_swapchain_and_resources()
    -- Wait for device to be idle
    vulkan.queue_wait_idle(graphics_queue)
    vulkan.queue_wait_idle(present_queue)

    -- Clean up old resources
    for i, fb in ipairs(framebuffers) do
        framebuffers[i] = nil
    end
    for i, view in ipairs(image_views) do
        image_views[i] = nil
    end
    collectgarbage() -- Trigger cleanup of old framebuffers and image views

    -- Get updated surface capabilities
    capabilities = vulkan.get_surface_capabilities(physical_device, surface)

    -- Update extent
    extent.width = capabilities.currentExtentWidth
    extent.height = capabilities.currentExtentHeight

    -- Recreate swapchain
    swapchain_create_info.imageExtent = { width = capabilities.currentExtentWidth, height = capabilities.currentExtentHeight }
    local new_swapchain = vulkan.recreate_swapchain(device, swapchain, swapchain_create_info)
    swapchain = new_swapchain

    -- Recreate swapchain images and image views
    swapchain_images = vulkan.get_swapchain_images(device, swapchain)
    print("Recreated Swapchain Images:", #swapchain_images)
    image_views = create_image_views(device, swapchain_images, selected_format)

    -- Recreate framebuffers
    framebuffers = create_framebuffers(device, render_pass, image_views, capabilities.currentExtentWidth, capabilities.currentExtentHeight)

    -- Recreate command buffers
    command_buffers = create_command_buffers(device, command_pool, framebuffers, render_pass, extent, graphics_pipeline, vertex_buffer)
end

print("Window and vulkan created. Press ESC or close to exit.")

while true do
    local events = sdl.poll_events()
    for i, event in ipairs(events) do
        if event.type == sdl.QUIT or (event.type == sdl.WINDOW_CLOSE and event.window_id == window_id) then
            print("Window closed.")
            return
        elseif event.type == sdl.KEY_DOWN and event.keycode == sdl.KEY_ESCAPE then
            print("ESC pressed.")
            return
        elseif event.type == sdl.WINDOW_RESIZED or event.type == sdl.WINDOW_SIZE_CHANGED then
            print("Window resized, recreating swapchain...")
            recreate_swapchain_and_resources()
        end
    end

    -- Acquire next image
    local success, image_index = pcall(vulkan.acquire_next_image_khr, device, swapchain, vulkan.UINT64_MAX, image_available_semaphore, nil)
    if not success then
        if image_index == vulkan.VK_ERROR_OUT_OF_DATE_KHR then
            print("Swapchain out of date, recreating...")
            recreate_swapchain_and_resources()
            image_index = vulkan.acquire_next_image_khr(device, swapchain, vulkan.UINT64_MAX, image_available_semaphore, nil)
        else
            error("Failed to acquire next image: " .. tostring(image_index))
        end
    end

    -- Create submit info
    local wait_stages = { vulkan.PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT }
    local submit_info = vulkan.create_submit_info({
        pWaitSemaphores = { image_available_semaphore },
        pWaitDstStageMask = wait_stages,
        pCommandBuffers = { command_buffers[image_index] },
        pSignalSemaphores = { render_finished_semaphore }
    })

    -- Submit to queue
    local result = vulkan.queue_submit(graphics_queue, { submit_info }, nil)
    if result ~= vulkan.VK_SUCCESS then
        print("Failed to submit draw command buffer: ", result)
    end

    -- Create present info
    local present_info = vulkan.create_present_info_khr({
        pWaitSemaphores = { render_finished_semaphore },
        pSwapchains = { swapchain },
        pImageIndices = { image_index }
    })

    -- Present to queue
    local present_result = vulkan.queue_present_khr(present_queue, present_info)
    if present_result == vulkan.VK_ERROR_OUT_OF_DATE_KHR then
        print("Swapchain out of date during present, recreating...")
        recreate_swapchain_and_resources()
    elseif present_result ~= vulkan.VK_SUCCESS then
        print("Failed to present: ", present_result)
    end

    -- Wait for queue to be idle
    vulkan.queue_wait_idle(present_queue)
end