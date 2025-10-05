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