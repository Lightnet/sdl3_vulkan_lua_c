

```c
#include <SDL3/SDL.h>
```

```c
SDL_Window* window = SDL_CreateWindow("SDL 3 Lua", 800, 600, 0);
```

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
```c
SDL_Renderer* renderer = SDL_CreateRenderer(window, NULL);
```
By default to direct3d11 if on windows. As it try by list one by one to load check.

```c
SDL_PropertiesID props = SDL_GetRendererProperties(renderer);
const char* name = SDL_GetStringProperty(props, SDL_PROP_RENDERER_NAME_STRING, "unknown");
Sint64 max_texture_size = SDL_GetNumberProperty(props, SDL_PROP_RENDERER_MAX_TEXTURE_SIZE_NUMBER, 0);
SDL_Log("Renderer created: name=%s, max_texture_size=%lld", name, (long long)max_texture_size);
```
 - SDL_PROP_RENDERER_NAME_STRING: the name of the rendering driver
 - SDL_PROP_RENDERER_MAX_TEXTURE_SIZE_NUMBER: the maximum texture width and height

# Reserved Keyword:
- repeat can't be use due to lua is a reserved keyword in Lua.


# SDL Lua Module API Documentation

This module provides a Lua interface to the SDL3 library for creating windows, rendering graphics, and handling input events. It uses Lua userdata with metatables to manage SDL resources (SDL_Window, SDL_Renderer, SDL_Texture) and ensures proper cleanup via garbage collection.

## Usage

lua
```lua
local sdl = require 'sdl'
```

## Functions

sdl.init([flags])

Initializes the SDL library.

- Parameters:
    - flags (integer, optional): SDL initialization flags. Defaults to sdl.INIT_VIDEO.
- Returns: None
- Errors: Raises a Lua error if initialization fails with an SDL error message.
- Example:
    
    lua
    ```lua
    sdl.init(sdl.INIT_VIDEO)
    ```
    

sdl.create_window(title, width, height, [flags])

Creates an SDL window.

- Parameters:
    - title (string): Window title.
    - width (integer): Window width in pixels.
    - height (integer): Window height in pixels.
    - flags (integer, optional): Window flags (e.g., sdl.WINDOW_FULLSCREEN). Defaults to 0.
- Returns: A window userdata (sdl.window).
- Errors: Raises a Lua error if window creation fails.
- Example:
    
    lua
    ```lua
    local window = sdl.create_window("My Game", 800, 600, sdl.WINDOW_RESIZABLE)
    ```
    

sdl.create_renderer(window, [driver])

Creates a renderer for a given window.
- Parameters:
    - window (userdata): Window userdata (sdl.window).
    - driver (string, optional): Renderer driver name (e.g., "opengl", "vulkan"). Defaults to nil (uses default driver).
- Returns: A renderer userdata (sdl.renderer).
- Errors: Raises a Lua error if renderer creation fails.
- Example:
    
    lua
    ```lua
    local renderer = sdl.create_renderer(window)
    ```
    

sdl.create_window_and_renderer(title, width, height, [flags])

Creates a window and renderer simultaneously.
- Parameters:
    - title (string): Window title.
    - width (integer): Window width in pixels.
    - height (integer): Window height in pixels.
    - flags (integer, optional): Window flags (e.g., sdl.WINDOW_FULLSCREEN). Defaults to 0.
- Returns:
    - Window userdata (sdl.window).
    - Renderer userdata (sdl.renderer).
- Errors: Raises a Lua error if creation fails.
- Example:
    
    lua
    ```lua
    local window, renderer = sdl.create_window_and_renderer("My Game", 800, 600)
    ```
    

sdl.poll_events()

Polls and returns all pending SDL events as a table.
- Parameters: None
- Returns: A table where each entry is an event table with fields depending on the event type (see Events section below).
- Example:
    
    lua
    ```lua
    local events = sdl.poll_events()
    for i, event in ipairs(events) do
        if event.type == sdl.QUIT then
            print("Quit event received")
        end
    end
    ```
    

sdl.set_render_draw_color(renderer, r, g, b, [a])

Sets the drawing color for rendering operations.
- Parameters:
    - renderer (userdata): Renderer userdata (sdl.renderer).
    - r (integer): Red component (0-255).
    - g (integer): Green component (0-255).
    - b (integer): Blue component (0-255).
    - a (integer, optional): Alpha component (0-255). Defaults to 255 (opaque).
- Returns: None
- Errors: Raises a Lua error if the renderer is invalid or setting the color fails.
- Example:
    
    lua
    ```lua
    sdl.set_render_draw_color(renderer, 255, 0, 0, 255) -- Set to red
    ```
    

sdl.render_clear(renderer)

Clears the renderer with the current draw color.
- Parameters:
    - renderer (userdata): Renderer userdata (sdl.renderer).
- Returns: None
- Errors: Raises a Lua error if the renderer is invalid or clearing fails.
- Example:
    
    lua
    ```lua
    sdl.render_clear(renderer)
    ```
    

sdl.render_present(renderer)

Presents the renderer to the screen.
- Parameters:
    - renderer (userdata): Renderer userdata (sdl.renderer).
- Returns: None
- Errors: Raises a Lua error if the renderer is invalid.
- Example:
    
    lua
    ```lua
    sdl.render_present(renderer)
    ```
    

sdl.render_line(renderer, x1, y1, x2, y2)

Draws a line on the renderer.
- Parameters:
    - renderer (userdata): Renderer userdata (sdl.renderer).
    - x1 (number): Starting X coordinate.
    - y1 (number): Starting Y coordinate.
    - x2 (number): Ending X coordinate.
    - y2 (number): Ending Y coordinate.
- Returns: None
- Errors: Raises a Lua error if the renderer is invalid or drawing fails.
- Example:
    
    lua
    ```lua
    sdl.render_line(renderer, 100, 100, 200, 200)
    ```
    

sdl.render_debug_text(renderer, x, y, text)

Renders debug text at the specified coordinates.
- Parameters:
    - renderer (userdata): Renderer userdata (sdl.renderer).
    - x (number): X coordinate for text.
    - y (number): Y coordinate for text.
    - text (string): Text to render.
- Returns: None
- Errors: Raises a Lua error if the renderer is invalid or rendering fails.
- Example:
    
    lua
    ```lua
    sdl.render_debug_text(renderer, 10, 10, "FPS: 60")
    ```
    

sdl.render_point(renderer, x, y)

Draws a single point on the renderer.
- Parameters:
    - renderer (userdata): Renderer userdata (sdl.renderer).
    - x (number): X coordinate of the point.
    - y (number): Y coordinate of the point.
- Returns: None
- Errors: Raises a Lua error if the renderer is invalid or drawing fails.
- Example:
    
    lua
    ```lua
    sdl.render_point(renderer, 150, 150)
    ```
    

sdl.render_points(renderer, points)

Draws multiple points on the renderer.
- Parameters:
    - renderer (userdata): Renderer userdata (sdl.renderer).
    - points (table): Array of tables, each with x and y fields (e.g., {{x=100, y=100}, {x=150, y=150}}).
- Returns: None
- Errors: Raises a Lua error if the renderer is invalid, the points table is malformed, or drawing fails.
- Example:
    
    lua
    ```lua
    local points = {{x=100, y=100}, {x=150, y=150}}
    sdl.render_points(renderer, points)
    ```
    

sdl.render_rect(renderer, x, y, w, h)

Draws a rectangle outline on the renderer.
- Parameters:
    - renderer (userdata): Renderer userdata (sdl.renderer).
    - x (number): X coordinate of the top-left corner.
    - y (number): Y coordinate of the top-left corner.
    - w (number): Width of the rectangle.
    - h (number): Height of the rectangle.
- Returns: None
- Errors: Raises a Lua error if the renderer is invalid or drawing fails.
- Example:
    
    lua
    ```lua
    sdl.render_rect(renderer, 50, 50, 100, 100)
    ```
    

sdl.render_fill_rect(renderer, x, y, w, h)

Draws a filled rectangle on the renderer.
- Parameters:
    - renderer (userdata): Renderer userdata (sdl.renderer).
    - x (number): X coordinate of the top-left corner.
    - y (number): Y coordinate of the top-left corner.
    - w (number): Width of the rectangle.
    - h (number): Height of the rectangle.
- Returns: None
- Errors: Raises a Lua error if the renderer is invalid or drawing fails.
- Example:
    
    lua
    ```lua
    sdl.render_fill_rect(renderer, 50, 50, 100, 100)
    ```
    

sdl.render_lines(renderer, points)

Draws connected lines between points on the renderer.
- Parameters:
    - renderer (userdata): Renderer userdata (sdl.renderer).
    - points (table): Array of tables, each with x and y fields (e.g., {{x=100, y=100}, {x=150, y=150}}). Requires at least two points.
- Returns: None
- Errors: Raises a Lua error if the renderer is invalid, fewer than two points are provided, the points table is malformed, or drawing fails.
- Example:
    
    lua
    ```lua
    local points = {{x=100, y=100}, {x=150, y=150}, {x=200, y=100}}
    sdl.render_lines(renderer, points)
    ```
    

sdl.create_texture(renderer, format, access, width, height)

Creates a texture for the renderer.
- Parameters:
    - renderer (userdata): Renderer userdata (sdl.renderer).
    - format (integer): Pixel format (e.g., sdl.PIXELFORMAT_RGBA8888).
    - access (integer): Texture access mode (e.g., sdl.TEXTUREACCESS_STATIC).
    - width (integer): Texture width in pixels.
    - height (integer): Texture height in pixels.
- Returns: A texture userdata (sdl.texture).
- Errors: Raises a Lua error if the renderer is invalid or texture creation fails.
- Example:
    
    lua
    ```lua
    local texture = sdl.create_texture(renderer, sdl.PIXELFORMAT_RGBA8888, sdl.TEXTUREACCESS_STATIC, 256, 256)
    ```
    

sdl.render_geometry(renderer, texture, vertices, [indices])

Renders geometry with optional texture and indices.
- Parameters:
    - renderer (userdata): Renderer userdata (sdl.renderer).
    - texture (userdata or nil): Texture userdata (sdl.texture) or nil for no texture.
    - vertices (table): Array of vertex tables, each with fields:
        - x (number): X coordinate.
        - y (number): Y coordinate.
        - r (number, optional): Red color component (0.0-1.0). Defaults to 1.0.
        - g (number, optional): Green color component (0.0-1.0). Defaults to 1.0.
        - b (number, optional): Blue color component (0.0-1.0). Defaults to 1.0.
        - a (number, optional): Alpha component (0.0-1.0). Defaults to 1.0.
        - u (number, optional): Texture U coordinate (0.0-1.0). Defaults to 0.0.
        - v (number, optional): Texture V coordinate (0.0-1.0). Defaults to 0.0.
    - indices (table, optional): Array of integers specifying vertex indices (1-based in Lua, converted to 0-based for SDL).
- Returns: None
- Errors: Raises a Lua error if the renderer is invalid, vertices table is malformed, or rendering fails.
- Example:
    
    lua
    ```lua
    local vertices = {
        {x=100, y=100, r=1, g=0, b=0, a=1, u=0, v=0},
        {x=200, y=100, r=0, g=1, b=0, a=1, u=1, v=0},
        {x=150, y=200, r=0, g=0, b=1, a=1, u=0.5, v=1}
    }
    local indices = {1, 2, 3}
    sdl.render_geometry(renderer, texture, vertices, indices)
    ```
    

sdl.destroy_window(window)

Destroys an SDL window.
- Parameters:
    - window (userdata): Window userdata (sdl.window).
- Returns: None
- Errors: Raises a Lua error if the window is invalid.
- Example:
    
    lua
    ```lua
    sdl.destroy_window(window)
    ```
    

sdl.quit()

Shuts down the SDL library.
- Parameters: None
- Returns: None
- Example:
    
    lua
    ```lua
    sdl.quit()
    ```
    

Window Properties

The sdl.window userdata supports the following property via the __index metamethod:

- windowID (integer): The unique ID of the window, accessible via window.windowID.
    - Example:
        
        lua
        ```lua
        local id = window.windowID
        print("Window ID:", id)
        ```
        

## Events

The sdl.poll_events() function returns a table of event tables. Each event table contains a type field and additional fields depending on the event type:

- sdl.QUIT:
    - type (integer): sdl.QUIT.
- sdl.WINDOW_CLOSE:
    - type (integer): sdl.WINDOW_CLOSE.
    - window_id (integer): ID of the window.
- sdl.KEY_DOWN, sdl.KEY_UP:
    - type (integer): sdl.KEY_DOWN or sdl.KEY_UP.
    - scancode (integer): SDL scancode.
    - scancode_name (string): Name of the scancode (e.g., "A", "Space").
    - keycode (integer): SDL keycode (e.g., sdl.KEY_SPACE).
    - key_name (string): Name of the key.
    - is_repeat (boolean): Whether the key event is a repeat.
    - window_id (integer): ID of the window.
- sdl.MOUSE_BUTTON_DOWN, sdl.MOUSE_BUTTON_UP:
    - type (integer): sdl.MOUSE_BUTTON_DOWN or sdl.MOUSE_BUTTON_UP.
    - button (integer): Mouse button (e.g., sdl.BUTTON_LEFT).
    - clicks (integer): Number of clicks (e.g., 1 for single-click, 2 for double-click).
    - x (number): X coordinate of the mouse.
    - y (number): Y coordinate of the mouse.
    - window_id (integer): ID of the window.
- sdl.MOUSE_MOTION:
    - type (integer): sdl.MOUSE_MOTION.
    - x (number): X coordinate of the mouse.
    - y (number): Y coordinate of the mouse.
    - xrel (number): Relative X motion since the last event.
    - yrel (number): Relative Y motion since the last event.
    - window_id (integer): ID of the window.

ConstantsInitialization Flags

- sdl.INIT_VIDEO: Initialize SDL with video support.

Window Flags

- sdl.WINDOW_FULLSCREEN: Create a fullscreen window.
- sdl.WINDOW_RESIZABLE: Create a resizable window.
- sdl.WINDOW_HIDDEN: Create a hidden window.
- sdl.WINDOW_BORDERLESS: Create a borderless window.
- sdl.WINDOW_VULKAN: Create a window compatible with Vulkan.

Texture Pixel Formats

- sdl.PIXELFORMAT_RGBA8888: RGBA 32-bit pixel format.
- sdl.PIXELFORMAT_ARGB8888: ARGB 32-bit pixel format.

Texture Access Modes

- sdl.TEXTUREACCESS_STATIC: Texture is read-only.
- sdl.TEXTUREACCESS_STREAMING: Texture can be updated frequently.
- sdl.TEXTUREACCESS_TARGET: Texture can be used as a render target.

Event Types

- sdl.QUIT: Application quit event.
- sdl.WINDOW_CLOSE: Window close requested event.
- sdl.KEY_DOWN: Key pressed event.
- sdl.KEY_UP: Key released event.
- sdl.MOUSE_BUTTON_DOWN: Mouse button pressed event.
- sdl.MOUSE_BUTTON_UP: Mouse button released event.
- sdl.MOUSE_MOTION: Mouse motion event.

Keycodes

- sdl.KEY_SPACE: Space key.
- sdl.KEY_RETURN: Return/Enter key.
- sdl.KEY_ESCAPE: Escape key.
- sdl.KEY_A: A key.
- sdl.KEY_B: B key.

Mouse Buttons

- sdl.BUTTON_LEFT: Left mouse button.
- sdl.BUTTON_RIGHT: Right mouse button.
- sdl.BUTTON_MIDDLE: Middle mouse button.

Notes

- Resource Management: Window, renderer, and texture userdata are managed via Lua's garbage collector. When no longer referenced, their respective SDL resources (SDL_Window, SDL_Renderer, SDL_Texture) are automatically destroyed.
- Error Handling: Most functions raise Lua errors if SDL operations fail, including detailed SDL error messages.
- SDL Version: The module uses SDL3, and version information is logged during sdl.init().
- Renderer Drivers: Available renderer drivers are logged during sdl.init(). The driver can optionally be specified in sdl.create_renderer() (though the provided code has this feature commented out).

Example Program

lua
```lua
local sdl = require 'sdl'

-- Initialize SDL
sdl.init(sdl.INIT_VIDEO)

-- Create window and renderer
local window, renderer = sdl.create_window_and_renderer("Example", 800, 600, sdl.WINDOW_RESIZABLE)

-- Main loop
local running = true
while running do
    -- Handle events
    local events = sdl.poll_events()
    for i, event in ipairs(events) do
        if event.type == sdl.QUIT or event.type == sdl.WINDOW_CLOSE then
            running = false
        elseif event.type == sdl.KEY_DOWN and event.keycode == sdl.KEY_ESCAPE then
            running = false
        end
    end

    -- Clear screen (black)
    sdl.set_render_draw_color(renderer, 0, 0, 0, 255)
    sdl.render_clear(renderer)

    -- Draw a red rectangle
    sdl.set_render_draw_color(renderer, 255, 0, 0, 255)
    sdl.render_fill_rect(renderer, 100, 100, 200, 150)

    -- Draw debug text
    sdl.render_debug_text(renderer, 10, 10, "Hello, SDL!")

    -- Present the renderer
    sdl.render_present(renderer)
end

-- Cleanup
sdl.destroy_window(window)
sdl.quit()
```

---

This documentation covers all functions and constants defined in the provided module_sdl.c code, with examples aligned with the Lua usage shown (e.g., sdl.init(sdl.INIT_VIDEO)). It includes detailed parameter descriptions, return values, error conditions, and a complete example to demonstrate typical usage.

# links:
- https://wiki.libsdl.org/SDL3/SDL_CreateRenderer
- https://wiki.libsdl.org/SDL3/SDL_GetRendererProperties
- https://wiki.libsdl.org/SDL3/SDL_SetRenderDrawColor
- https://wiki.libsdl.org/SDL3/SDL_RenderPresent
- https://wiki.libsdl.org/SDL3/SDL_RenderClear

