# sdl3_vulkan_lua_c

# License: MIT

# Status:
- Prototype
- Current working on vulkan set up test.

# Program languages:
- c language
- cmake

# Libraries:
- SDL 3.2.22
- Lua 5.4
- vulkan sdk 1.4 from github

  Note using the cmake to fetchcontent from github build and compile.

# Information:
  Keep it simple.

  Sample build for SDL3, Lua 5.4, Vulkan SDK for c language build. 
  
  By using lua script to expose SDL api events and vulkan api set up to build triangle sample.

# SDL 3.2

```
Available renderer driver 0: direct3d11
Available renderer driver 1: direct3d12
Available renderer driver 2: direct3d
Available renderer driver 3: opengl
Available renderer driver 4: opengles2
Available renderer driver 5: vulkan
Available renderer driver 6: gpu
Available renderer driver 7: software
```

# Lua:
  Lua can be easy and hard.

  To expose some SDL and Vulkan api is no easy task. As need to access the table from c to lua as lua to c. To config the vulkan render set up. As well clean garbage on lua api side.

## Vulkan test:
  Work in progress.

```lua
-- main.lua
local sdl = require 'sdl'
local vulkan = require 'vulkan'
-- sdl 3.2
local window = sdl.create_window("Vulkan Test", 800, 600, 0)
-- vulkan

-- Create VkApplicationInfo
local appinfo = vulkan.create_vk_application_info({pApplicationName = "Vulkan SDL3"})

-- get SDL extensions, detect depend on the OS
local extensions = vulkan.get_instance_extensions()

-- Create VkInstanceCreateInfo
local createinfo = vulkan.create_vk_create_info({
    pApplicationInfo = appinfo,
    enabledExtensionCount = #extensions,
    ppEnabledExtensionNames = extensions
})
-- Create VkInstance
local instance = vulkan.vk_create_instance(createinfo, nil)
-- Create VkSurfaceKHR
local surface = vulkan.create_surface(window, instance, nil)
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

```


## Render test:
```lua
-- main.txt (copied to main.lua)
local sdl = require 'sdl'

sdl.init()

local window = sdl.create_window("SDL3 Renderer Demo", 800, 600, sdl.WINDOW_RESIZABLE)
local window_id = window.windowID

-- local renderer, err = sdl.create_renderer(window)
-- if not renderer then
--     print("Error creating renderer: " .. (err or "Unknown error"))
--     return
-- end

print("Window and renderer created. Press ESC or close to exit.")

-- Initial color (red)
-- local status, err = pcall(sdl.set_render_draw_color, renderer, 255, 0, 0, 255)
-- if not status then
--     print("Error setting render draw color: " .. (err or "Unknown error"))
--     return
-- end
-- sdl.set_render_draw_color(renderer, 255, 0, 0, 255)
-- sdl.render_clear(renderer)
-- sdl.render_present(renderer)

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

    -- Update the screen
    -- sdl.render_clear(renderer)
    -- sdl.render_present(renderer)
end
```

# Notes:
- console log will lag if there too much in logging.

# refs:
- https://wiki.libsdl.org/SDL3/CategoryAPI
- https://wiki.libsdl.org/SDL3/SDL_GetRenderDriver
- https://wiki.libsdl.org/SDL3/QuickReference
- https://wiki.libsdl.org/SDL3/SDL_HINT_RENDER_DRIVER
- https://wiki.libsdl.org/SDL3/SDL_GetCurrentVideoDriver
- https://wiki.libsdl.org/SDL3/SDL_GetRendererProperties
- https://wiki.libsdl.org/SDL3/SDL_SetHint
- 

- https://wiki.libsdl.org/SDL3/SDL_FillSurfaceRect
- https://wiki.libsdl.org/SDL3/SDL_RenderLine
- https://wiki.libsdl.org/SDL3/SDL_RenderDebugText

# SDL_GetRendererProperties:
- SDL_PROP_RENDERER_NAME_STRING: the name of the rendering driver
  - direct3d11, opengl, vulkan, etc...


# Credits:
- Grok A.I on x.
- Github repo examples from users.
- 