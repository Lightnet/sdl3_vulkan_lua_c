-- main.txt (copied to main.lua)
local sdl = require 'sdl'
local vulkan = require 'vulkan'

sdl.init()

local window = sdl.create_window("SDL3 Renderer Demo", 800, 600, sdl.WINDOW_VULKAN | sdl.WINDOW_RESIZABLE)
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


print("Window and renderer created. Press ESC or close to exit.")

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