-- simple_vulkan.lua
local sdl = require 'sdl'
local vulkan = require 'vulkan'

sdl.init(sdl.INIT_VIDEO)
local window = sdl.create_window("SDL3 Vulkan Lua 5.4 Demo", 800, 600, sdl.WINDOW_VULKAN | sdl.WINDOW_RESIZABLE)
local window_id = window.windowID

local appinfo = vulkan.create_vk_application_info({
    application_name = "Vulkan SDL3 Lua Test",
    application_version = vulkan.make_version(1, 0, 0),
    engine_name = "LuaJIT Vulkan",
    engine_version = vulkan.make_version(1, 0, 0),
    api_version = vulkan.VK_API_VERSION_1_3
})
print("appinfo:" .. tostring(appinfo))

local sdl_extensions = vulkan.sdl_vulkan_get_instance_extensions()
print("sdl_extensions:" .. tostring(sdl_extensions))
for i, ext in ipairs(sdl_extensions) do
    print("sdl_extensions: " .. ext)
end

local create_info = vulkan.create_info({
    app_info = appinfo,
    extensions = sdl_extensions,
    layers = { "VK_LAYER_KHRONOS_validation" }
})

local instance = vulkan.create_instance(create_info)
print("instance:" .. tostring(instance))
if not instance then
    error("Failed to create Vulkan instance")
end

local surface = vulkan.sdl_vulkan_create_surface(window, instance)
print("surface:" .. tostring(surface))
if not surface then
    error("Failed to create Vulkan surface")
end

local physical_devices = vulkan.create_physical_devices(instance)
print("physical_devices:" .. tostring(physical_devices))
local physical_device = nil

-- Step 1: Select a physical device
for i, device in ipairs(physical_devices) do
    print(string.format("Physical Device %d: name=%s, type=%d, api_version=%d", 
        i, device.name, device.type, device.api_version))
    if device.type == vulkan.DEVICE_TYPE_DISCRETE_GPU then
        physical_device = device.device
        print("Selected physical device: " .. device.name)
        break
    end
end

if not physical_device then
    -- Fallback to any device if no discrete GPU is found
    for i, device in ipairs(physical_devices) do
        print(string.format("Physical Device %d: name=%s, type=%d, api_version=%d", 
            i, device.name, device.type, device.api_version))
        physical_device = device.device
        print("Selected fallback physical device: " .. device.name)
        break
    end
end

if not physical_device then
    print("No physical devices found")
    return
end

-- Step 2: Select queue families
local graphics_family = nil
local present_family = nil
local queue_families = vulkan.get_physical_devices_properties(physical_device, surface)
for j, family in ipairs(queue_families) do
    local vk_queue_index = j - 1 -- Convert to 0-based index for Vulkan
    print(string.format("Queue Family %d: queue_count=%d, graphics=%s, present=%s, compute=%s, transfer=%s", 
        j, family.queue_count, tostring(family.graphics), tostring(family.present), 
        tostring(family.compute), tostring(family.transfer)))
    -- Validate presentation support directly
    local present_support = vulkan.get_physical_device_surface_support_KHR(physical_device, vk_queue_index, surface)
    print(string.format("Queue Family %d: direct present_support=%s", j, tostring(present_support)))
    if family.graphics and not graphics_family then
        graphics_family = vk_queue_index
    end
    if present_support and not present_family then
        present_family = vk_queue_index
    end
    if graphics_family and present_family then
        print(string.format("Selected graphics_family=%d, present_family=%d", graphics_family, present_family))
        break
    end
end

if not (graphics_family and present_family) then
    print("No suitable queue families found for graphics and presentation")
    return
end

local device_create_info = vulkan.create_device_info({
    queue_families = {
        { family_index = graphics_family, queue_count = 1 },
        graphics_family ~= present_family and { family_index = present_family, queue_count = 1 } or nil
    },
    extensions = { "VK_KHR_swapchain" }
})
local device = vulkan.create_device(physical_device, device_create_info)
print("device:" .. tostring(device))
if not device then
    error("Failed to create Vulkan device")
end

local graphics_queue = vulkan.get_device_queue(device, graphics_family, 0)
print("graphics_queue:" .. tostring(graphics_queue))
local present_queue = vulkan.get_device_queue(device, present_family, 0)
print("present_queue:" .. tostring(present_queue))

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
            color_attachments = {
                { attachment = 0, layout = vulkan.IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL }
            }
        }
    },
    dependencies = {
        {
            src_subpass = vulkan.SUBPASS_EXTERNAL,
            dst_subpass = 0,
            src_stage_mask = vulkan.PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT,
            dst_stage_mask = vulkan.PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT,
            src_access_mask = 0,
            dst_access_mask = vulkan.ACCESS_COLOR_ATTACHMENT_WRITE,
            dependency_flags = 0
        }
    }
})
print("render_pass:" .. tostring(render_pass))
if not render_pass then
    error("Failed to create render pass")
end

local image_views = {}
local swapchain = nil
local swapchain_images = nil
local framebuffers = nil
local surface_capabilities = nil

local function recreate_swapchain()
    local success, err = vulkan.device_wait_idle(device)
    if not success then
        print("Device wait idle failed: ", err or "No error message")
        return false
    end

    if framebuffers then
        for i = 1, #framebuffers do
            vulkan.destroy_framebuffer(device, framebuffers[i])
        end
        framebuffers = nil
    end
    if image_views then
        for i = 1, #image_views do
            if image_views[i] then
                vulkan.destroy_image_view(device, image_views[i])
            end
        end
        image_views = {}
    end
    if swapchain_images then
        swapchain_images = nil
    end
    if swapchain then
        vulkan.destroy_swapchain_khr(device, swapchain)
        swapchain = nil
    end

    surface_capabilities = vulkan.get_physical_device_surface_capabilities_KHR(physical_device, surface)
    print("surface_capabilities:" .. tostring(surface_capabilities))
    print("Surface Capabilities: min_image_count=" .. (surface_capabilities.min_image_count or "nil") ..
          ", max_image_count=" .. (surface_capabilities.max_image_count or "nil") ..
          ", current_extent=" .. (surface_capabilities.current_extent_width or "nil") .. "x" ..
          (surface_capabilities.current_extent_height or "nil"))

    local present_support = vulkan.get_physical_device_surface_support_KHR(physical_device, present_family, surface)
    print("Checking presentation support for queue family " .. present_family .. ": " .. tostring(present_support))
    if not present_support then
        print("Error: Surface does not support presentation for present queue family " .. present_family)
        return false
    end

    local create_info = {
        surface = surface,
        min_image_count = surface_capabilities.min_image_count,
        format = vulkan.FORMAT_B8G8R8A8_SRGB,
        color_space = vulkan.COLOR_SPACE_SRGB_NONLINEAR_KHR,
        extent = {
            width = surface_capabilities.current_extent_width,
            height = surface_capabilities.current_extent_height
        },
        image_array_layers = 1,
        image_usage = vulkan.IMAGE_USAGE_COLOR_ATTACHMENT,
        sharing_mode = graphics_family == present_family and vulkan.SHARING_MODE_EXCLUSIVE or vulkan.SHARING_MODE_CONCURRENT,
        queue_family_indices = graphics_family == present_family and {} or { graphics_family, present_family },
        pre_transform = surface_capabilities.current_transform or vulkan.SURFACE_TRANSFORM_IDENTITY,
        composite_alpha = vulkan.COMPOSITE_ALPHA_OPAQUE,
        present_mode = vulkan.PRESENT_MODE_FIFO_KHR,
        clipped = true,
        old_swapchain = swapchain
    }
    print("create_swap_chain_KHR fields:")
    for k, v in pairs(create_info) do
        print(k .. ": " .. tostring(v))
        if type(v) == "table" and k == "extent" then
            print("extent.width: " .. tostring(v.width))
            print("extent.height: " .. tostring(v.height))
        end
    end

    swapchain = vulkan.create_swap_chain_KHR(device, create_info)
    if not swapchain then
        print("Failed to create swapchain")
        return false
    end
    print("swapchain:" .. tostring(swapchain))
    print("swapchain type:", type(swapchain), "metatable:", getmetatable(swapchain) and getmetatable(swapchain).__name or "none")

    swapchain_images = vulkan.get_swapchain_images_KHR(device, swapchain)
    print("swapchain_images:" .. tostring(swapchain_images))
    if not swapchain_images or type(swapchain_images) ~= "table" then
        print("Failed to get swapchain images")
        return false
    end
    print("Number of swapchain images: " .. #swapchain_images)

    framebuffers = {}
    image_views = {}
    for i = 1, #swapchain_images do
        print("Processing swapchain_image[" .. i .. "]: " .. tostring(swapchain_images[i]))
        if not swapchain_images[i] then
            print("Error: swapchain_images[" .. i .. "] is nil")
            return false
        end

        local image_view_create_info = {
            image = swapchain_images[i],
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
        }
        print("image_view_create_info for image " .. i .. ":")
        for k, v in pairs(image_view_create_info) do
            print("  " .. k .. ": " .. tostring(v))
            if type(v) == "table" then
                for k2, v2 in pairs(v) do
                    print("    " .. k2 .. ": " .. tostring(v2))
                end
            end
        end

        local image_view = vulkan.create_image_view(device, image_view_create_info)
        print("image_view[" .. i .. "]: " .. tostring(image_view))
        if not image_view then
            print("Failed to create image view for swapchain image " .. i)
            return false
        end
        image_views[i] = image_view

        if not render_pass then
            print("Error: render_pass is nil")
            return false
        end

        local framebuffer_create_info = {
            render_pass = render_pass,
            attachments = { image_view },
            width = surface_capabilities.current_extent_width,
            height = surface_capabilities.current_extent_height,
            layers = 1
        }
        print("framebuffer_create_info for image " .. i .. ":")
        for k, v in pairs(framebuffer_create_info) do
            print("  " .. k .. ": " .. tostring(v))
            if type(v) == "table" then
                for k2, v2 in pairs(v) do
                    print("    " .. k2 .. ": " .. tostring(v2))
                end
            end
        end

        framebuffers[i] = vulkan.create_framebuffer(device, framebuffer_create_info)
        print("framebuffer[" .. i .. "]: " .. tostring(framebuffers[i]))
        if not framebuffers[i] then
            print("Failed to create framebuffer for swapchain image " .. i)
            return false
        end
    end
    print("Created " .. #framebuffers .. " framebuffers")
    return true
end

print("Loading shaders")
local function readFile(path)
    local file = io.open(path, "rb")
    assert(file, "Failed to open " .. path)
    local data = file:read("*all")
    file:close()
    return data
end
local vertShaderCode = readFile("triangle.vert.spv")
local fragShaderCode = readFile("triangle.frag.spv")

local vertShaderModule = vulkan.create_shader_module(device, vertShaderCode)
print("vertShaderModule:" .. tostring(vertShaderModule))
if not vertShaderModule then
    error("Failed to create vertex shader module")
end
local fragShaderModule = vulkan.create_shader_module(device, fragShaderCode)
print("fragShaderModule:" .. tostring(fragShaderModule))
if not fragShaderModule then
    error("Failed to create fragment shader module")
end

local pipelineLayout = vulkan.create_pipeline_layout(device, {
    set_layouts = {},
    push_constant_ranges = {}
})
print("pipelineLayout:" .. tostring(pipelineLayout))
if not pipelineLayout then
    error("Failed to create pipeline layout")
end

local pipelines = vulkan.create_graphics_pipelines(device, {
    pipelines = {
        {
            stages = {
                { stage = vulkan.SHADER_STAGE_VERTEX, module = vertShaderModule, name = "main" },
                { stage = vulkan.SHADER_STAGE_FRAGMENT, module = fragShaderModule, name = "main" }
            },
            render_pass = render_pass,
            layout = pipelineLayout,
            subpass = 0
        }
    }
})
if pipelines and #pipelines > 0 then
    print("pipeline:" .. tostring(pipelines[1]))
else
    print("Failed to create graphics pipeline")
    return
end

local MAX_FRAMES_IN_FLIGHT = 2
local imageAvailableSemaphores = {}
local renderFinishedSemaphores = {}
local inFlightFences = {}
for i = 1, MAX_FRAMES_IN_FLIGHT do
    imageAvailableSemaphores[i] = vulkan.create_semaphore(device)
    renderFinishedSemaphores[i] = vulkan.create_semaphore(device)
    inFlightFences[i] = vulkan.create_fence(device, true)
end
print("Created " .. #imageAvailableSemaphores .. " semaphores and fences")

local commandPool = vulkan.create_command_pool(device, graphics_family)
print("commandPool:" .. tostring(commandPool))
if not commandPool then
    error("Failed to create command pool")
end
local commandBuffers = vulkan.create_allocate_command_buffers(device, commandPool, MAX_FRAMES_IN_FLIGHT)
print("Created " .. #commandBuffers .. " command buffers")
for i = 1, #commandBuffers do
    local mt = getmetatable(commandBuffers[i])
    print("commandBuffers[" .. i .. "] type:", type(commandBuffers[i]), "metatable:", mt and (mt.__name or tostring(mt)) or "none")
end

if not recreate_swapchain() then
    error("Initial swapchain creation failed")
end

local currentFrame = 1
local function render()
    local fence = inFlightFences[currentFrame]
    vulkan.wait_for_fences(device, fence)
    vulkan.reset_fences(device, fence)

    local success, err = vulkan.device_wait_idle(device)
    if not success then
        print("Device wait idle failed: ", err or "No error message")
        return false
    end

    local imageIndex = vulkan.acquire_next_image_KHR(device, swapchain, vulkan.UINT64_MAX, imageAvailableSemaphores[currentFrame], nil)
    if not imageIndex then
        print("Failed to acquire next image")
        return recreate_swapchain()
    end

    local cmdBuffer = commandBuffers[currentFrame]
    vulkan.reset_command_buffer(cmdBuffer)
    vulkan.begin_command_buffer(cmdBuffer)
    vulkan.cmd_begin_renderpass(cmdBuffer, render_pass, framebuffers[imageIndex + 1], {
        clear_values = { { r = 1.0, g = 0.0, b = 0.0, a = 1.0 } }
    })
    vulkan.cmd_set_viewport(cmdBuffer, {
        { x = 0, y = 0, width = surface_capabilities.current_extent_width, height = surface_capabilities.current_extent_height, min_depth = 0.0, max_depth = 1.0 }
    })
    vulkan.cmd_set_scissor(cmdBuffer, {
        { x = 0, y = 0, width = surface_capabilities.current_extent_width, height = surface_capabilities.current_extent_height }
    })
    vulkan.cmd_bind_pipeline(cmdBuffer, pipelines[1])
    vulkan.cmd_draw(cmdBuffer, 3, 1, 0, 0)
    vulkan.cmd_end_renderpass(cmdBuffer)
    vulkan.end_commandbuffer(cmdBuffer)

    local submit_result = vulkan.queue_submit(graphics_queue, {
        wait_semaphores = { imageAvailableSemaphores[currentFrame] },
        wait_dst_stage_mask = { vulkan.PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT },
        command_buffers = {cmdBuffer},
        signal_semaphores = { renderFinishedSemaphores[currentFrame] }
    }, fence)
    if not submit_result then
        print("Failed to submit command buffer")
        return false
    end

    local present_result, err_msg = vulkan.queue_present_KHR(present_queue, {
        wait_semaphores = { renderFinishedSemaphores[currentFrame] },
        swapchains = {swapchain},
        image_indices = { imageIndex }
    })
    if not present_result then
        print("Failed to present image: ", err_msg or "No error message")
        if err_msg and err_msg:find("VkResult -8") then
            return recreate_swapchain()
        end
        return false
    end

    currentFrame = (currentFrame % MAX_FRAMES_IN_FLIGHT) + 1
    return true
end

local function cleanup()
    vulkan.device_wait_idle(device)
    if framebuffers then
        for i = 1, #framebuffers do
            vulkan.destroy_framebuffer(device, framebuffers[i])
        end
    end
    if image_views then
        for i = 1, #image_views do
            if image_views[i] then
                vulkan.destroy_image_view(device, image_views[i])
            end
        end
    end
    if swapchain then
        vulkan.destroy_swapchain_khr(device, swapchain)
    end
    for i = 1, #imageAvailableSemaphores do
        vulkan.destroy_semaphore(device, imageAvailableSemaphores[i])
        vulkan.destroy_semaphore(device, renderFinishedSemaphores[i])
        vulkan.destroy_fence(device, inFlightFences[i])
    end
    vulkan.destroy_command_pool(device, commandPool)
    vulkan.destroy_pipeline(device, pipelines[1])
    vulkan.destroy_pipeline_layout(device, pipelineLayout)
    vulkan.destroy_shader_module(device, vertShaderModule)
    vulkan.destroy_shader_module(device, fragShaderModule)
    vulkan.destroy_render_pass(device, render_pass)
    vulkan.destroy_device(device)
    vulkan.destroy_surface_KHR(instance, surface)
    vulkan.destroy_instance(instance)
    sdl.destroy_window(window)
end

local running = true
while running do
    local events = sdl.poll_events()
    for i, event in ipairs(events) do
        if event.type == sdl.QUIT or (event.type == sdl.WINDOW_CLOSE and event.window_id == window_id) then
            print("Window closed.")
            running = false
        end
    end
    if not render() then
        running = false
    end
end

print("finished lua")

cleanup()
print("finished...")