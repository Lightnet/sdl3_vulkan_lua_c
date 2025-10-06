-- main.txt (copied to main.lua)
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
local selected_device = nil
for i, device in ipairs(devices) do
    local props = vulkan.get_physical_device(device)
    print(string.format("  Device %d: %s (Type: %d)", i, props.deviceName, props.deviceType))
    -- Select discrete GPU (type 2) or integrated GPU (type 1) if no discrete GPU is found
    if props.deviceType == 2 then -- VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU
        selected_device = device
    elseif props.deviceType == 1 and not selected_device then -- VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU
        selected_device = device
    end
end

-- Check if a suitable GPU was found
if not selected_device then
    print("No suitable GPU found")
else
    print("Selected physical device:", tostring(selected_device))
end

-- Check device extensions
local device_extensions_supported = vulkan.get_device_extensions(selected_device)
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
if selected_device then
    print("Selected physical device after cleanup:", tostring(selected_device))
end

-- Get queue family count
local queue_family_count = vulkan.get_family_count(selected_device)
print("Queue family count:", queue_family_count)

-- Get queue family properties
local queue_families = vulkan.queue_families(selected_device, queue_family_count, surface)
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
        break -- Early break to match C code
    end
end

if not graphics_family or not present_family then
    print("No suitable queue families found")
else
    print("Selected graphics family:", graphics_family)
    print("Selected present family:", present_family)
end

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
local device = vulkan.create_device(selected_device, device_create_info, nil)
print("Created VkDevice:", tostring(device))

-- Get queues
local graphics_queue = vulkan.get_device_queue(device, graphics_family, 0)
local present_queue = vulkan.get_device_queue(device, present_family, 0)
print("Graphics Queue:", tostring(graphics_queue))
print("Present Queue:", tostring(present_queue))


-- Get surface capabilities
local capabilities = vulkan.get_surface_capabilities(selected_device, surface)
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
local surface_formats = vulkan.get_surface_formats(selected_device, surface)
print("Surface Formats:", #surface_formats)

-- Select surface format (prefer VK_FORMAT_B8G8R8A8_SRGB with SRGB nonlinear color space)
local selected_format = surface_formats[1] -- Default to first format
for i, fmt in ipairs(surface_formats) do
    if fmt.format == vulkan.FORMAT_B8G8R8A8_SRGB and fmt.colorSpace == vulkan.COLOR_SPACE_SRGB_NONLINEAR_KHR then
        selected_format = fmt
        break
    end
end
print("Selected Surface Format: format =", selected_format.format, "colorSpace =", selected_format.colorSpace)

-- Get present modes
local present_modes = vulkan.get_surface_present_modes(selected_device, surface)
print("Present Modes:", #present_modes)

-- Select present mode (default to VK_PRESENT_MODE_FIFO_KHR)
local selected_present_mode = vulkan.PRESENT_MODE_FIFO_KHR
print("Selected Present Mode:", selected_present_mode)

-- Create VkSwapchainCreateInfoKHR
local swapchain_create_info = vulkan.create_swapchain_create_info({
    surface = surface,
    minImageCount = capabilities.minImageCount,
    imageFormat = selected_format.format,
    imageColorSpace = selected_format.colorSpace,
    imageExtent = { width = capabilities.currentExtentWidth, height = capabilities.currentExtentHeight },
    imageArrayLayers = 1,
    imageUsage = vulkan.IMAGE_USAGE_COLOR_ATTACHMENT_BIT, -- Common usage for rendering
    presentMode = selected_present_mode
})

-- Create VkSwapchainKHR
local swapchain = vulkan.create_swapchain(device, swapchain_create_info, nil)
print("Created VkSwapchainKHR:", tostring(swapchain))

-- Get swapchain images
local swapchain_images = vulkan.get_swapchain_images(device, swapchain)
print("Swapchain Images:", #swapchain_images)

for i, img in ipairs(swapchain_images) do
    print("Image", i, ":", tostring(img))
end

-- Create image views for swapchain images
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

-- Create color attachment
local color_attachment = vulkan.create_attachment_description({
    format = selected_format.format
})

-- Create color attachment reference
local color_attachment_ref = {
    attachment = 0,
    layout = vulkan.IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
}

-- Create subpass description
local subpass = vulkan.create_subpass_description({
    pColorAttachments = { color_attachment_ref }
})

-- Create subpass dependency
local dependency = vulkan.create_subpass_dependency({})

-- Create render pass create info
local render_pass_create_info = vulkan.create_render_pass_create_info({
    pAttachments = { color_attachment },
    pSubpasses = { subpass },
    pDependencies = { dependency }
})

-- Create render pass
local render_pass = vulkan.create_render_pass(device, render_pass_create_info, nil)
print("Created VkRenderPass:", tostring(render_pass))






















print("Window and vulkan created (wip). Press ESC or close to exit.")

while true do
    local events = sdl.poll_events()
    for i, event in ipairs(events) do
        if event.type == sdl.QUIT or (event.type == sdl.WINDOW_CLOSE and event.window_id == window_id) then
            print("Window closed.")
            return
        end
        -- if event.type == sdl.QUIT or (event.type == sdl.WINDOW_CLOSE and event.window_id == window_id) then
        --     print("Window closed.")
        --     return
        -- elseif event.type == sdl.KEY_DOWN then
        --     print(string.format("Key pressed: %s (scancode: %d, keycode: %d, is_repeat: %s, window_id: %d)",
        --         event.key_name, event.scancode, event.keycode, tostring(event.is_repeat), event.window_id))
        --     if event.keycode == sdl.KEY_ESCAPE then
        --         print("ESC pressed. Exiting.")
        --         return
        --     elseif event.keycode == sdl.KEY_A then
        --         print("A key pressed! Changing to blue.")
        --         sdl.set_render_draw_color(renderer, 0, 0, 255, 255) -- Blue
        --     elseif event.keycode == sdl.KEY_B then
        --         print("B key pressed! Changing to green.")
        --         sdl.set_render_draw_color(renderer, 0, 255, 0, 255) -- Green
        --     end
        -- elseif event.type == sdl.KEY_UP then
        --     print(string.format("Key released: %s (scancode: %d, keycode: %d, window_id: %d)",
        --         event.key_name, event.scancode, event.keycode, event.window_id))
        -- elseif event.type == sdl.MOUSE_BUTTON_DOWN then
        --     print(string.format("Mouse button %d down at (%f, %f), clicks: %d, window_id: %d)",
        --         event.button, event.x, event.y, event.clicks, event.window_id))
        -- elseif event.type == sdl.MOUSE_BUTTON_UP then
        --     print(string.format("Mouse button %d up at (%f, %f), window_id: %d)",
        --         event.button, event.x, event.y, event.window_id))
        -- elseif event.type == sdl.MOUSE_MOTION then
        --     print(string.format("Mouse moved to (%f, %f), relative: (%f, %f), window_id: %d)",
        --         event.x, event.y, event.xrel, event.yrel, event.window_id))
        -- end
    end

end