

```c
#include <SDL3/SDL.h>
```

```c
SDL_Window* window = SDL_CreateWindow("SDL 3 Lua", 800, 600, 0);
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

# can't use:
repeat can't be use due to lua is a reserved keyword in Lua.

# events:
Changed event constants to match your Lua script:SDL_EVENT_QUIT → sdl.QUIT
SDL_EVENT_WINDOW_CLOSE_REQUESTED → sdl.WINDOW_CLOSE
SDL_EVENT_KEY_DOWN → sdl.KEY_DOWN
SDL_EVENT_KEY_UP → sdl.KEY_UP
SDL_EVENT_MOUSE_BUTTON_DOWN → sdl.MOUSE_BUTTON_DOWN
SDL_EVENT_MOUSE_BUTTON_UP → sdl.MOUSE_BUTTON_UP
SDL_EVENT_MOUSE_MOTION → sdl.MOUSE_MOTION

# links:
- https://wiki.libsdl.org/SDL3/SDL_CreateRenderer
- https://wiki.libsdl.org/SDL3/SDL_GetRendererProperties
- https://wiki.libsdl.org/SDL3/SDL_SetRenderDrawColor
- https://wiki.libsdl.org/SDL3/SDL_RenderPresent
- https://wiki.libsdl.org/SDL3/SDL_RenderClear

