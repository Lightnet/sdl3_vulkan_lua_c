# sdl3_vulkan_lua_c

# License: MIT

# Status:
- Prototype
- working on improve the code.

# Program languages:
- c language 
- cmake 3.x
- lua 5.4

# Libraries:
- SDL 3.2.22
- Lua 5.4
- Vulkan-Headers vulkan-sdk-1.4.313.0
- Vulkan-Loader vulkan-sdk-1.4.313.0
- stb ( n/a )
- cimgui ( n/a )
- spirv-headers  vulkan-sdk-1.4.313.0 ( shader )
- spirv-tools vulkan-sdk-1.4.313.0 ( shader )
- sglslang  16.0.0 ( shader )
- shaderc  v2025.4 ( shader )
    - require
        - SPIRV-Tools
        - glslang
    - build size is 650 MB.

  Note using the cmake to fetchcontent from github build and compile.

# Information:

  Sample build for SDL3, Lua 5.4, Vulkan SDK for c programing language. By using the lua script to expose vulkan api to set up and build triangle 2D with the help of Grok AI agent chat message. Grok AI on https://x.com/i/grok to help translate c vulkan code to lua script.

# Goals:
  To refine vulkan struct, functions and improve lua api code lua layer and meta table.

  Area need to expose is shader set up and render features.
  
  Well it required some warpper to handle some idea ways.

## Notes:
- If too much code added and it will be hard for AI to build triangle if there too many struct create ref table.
- There will be lag if there a lot print logs.
- Due to limited AI agent messages memory. Rebuilding or relearn take time.
- Due to render out date it need to recrate swapchain since translate the layers.
- Due limited use lua vulkan functions to compress to make AI to render triangle due to AI limit memory.

# Todolist:
- [x] simple triangle
- [ ] break down to able to expose api for lua
- [ ] font
- [ ] image
- [ ] 3d cube
- [ ] debug layer
- [ ] ...

# Project Files:
  This is work in progress for testing files and scripts.

## Module Vulkan
```
- include
    - module_sdl.h ( line 29 )
    - module_vulkan.h ( line 140 )
- src/
    - main.c ( lines 74 )
    - module_sdl.c ( lines 830 )
    - module_vulkan.c ( lines 2861 )
```

## Test files:
```
- window.c // test
- base_triangle.c // shader file spv
- base_triangle_string.c // triangle string
```
## Examples:
```
draw_shapes.lua (SDL renderer test)
renderer.lua (SDL renderer test)
geometry.lua (SDL renderer test)
input_test.lua (SDL input test)
```

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

# Tool:

  Using the windows 64 bit build with msys2. By using the cmake for easy build and compile. It config for debug and release as I set. This is work in progress.

## msys
```
libgcc_s_seh-1.dll
libstdc++-6.dll
libwinpthread-1.dll
```
  Note this is just ref build. It depend on the libs and config to make it stand alone or deps.

## Notes:
- not fully config might get errors.

## build.bat
```bat
@echo off
setlocal
set MSYS2_PATH=C:\msys64\mingw64\bin
if not exist build mkdir build
set PATH=%MSYS2_PATH%;%PATH%
cd build
%MSYS2_PATH%\cmake.exe .. -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Debug -DCMAKE_C_COMPILER=%MSYS2_PATH%\gcc.exe -DCMAKE_CXX_COMPILER=%MSYS2_PATH%\g++.exe
%MSYS2_PATH%\cmake.exe --build . --config Debug
endlocal
```

# Lua:
  Lua use tables and metatables for vulkan context for store and garbage collection. Required some knowlege how to proper set up c api lua to work on lua script code. There will be pros and cons to vulkan format data.

## Vulkan test:
  Work in progress. It is base sample triangle c to reference the build for lua script.

- simple_vulkan.lua (520 lines)
    - note this just a test build.

# Notes:
- console log will lag if there too much in logging.

# References:
- https://wiki.libsdl.org/SDL3/CategoryAPI
- https://wiki.libsdl.org/SDL3/SDL_GetRenderDriver
- https://wiki.libsdl.org/SDL3/QuickReference
- https://wiki.libsdl.org/SDL3/SDL_HINT_RENDER_DRIVER
- https://wiki.libsdl.org/SDL3/SDL_GetCurrentVideoDriver
- https://wiki.libsdl.org/SDL3/SDL_GetRendererProperties
- https://wiki.libsdl.org/SDL3/SDL_SetHint
- https://wiki.libsdl.org/SDL3/SDL_FillSurfaceRect
- https://wiki.libsdl.org/SDL3/SDL_RenderLine
- https://wiki.libsdl.org/SDL3/SDL_RenderDebugText

# Libraries:
- https://github.com/KhronosGroup/glslang
- https://github.com/KhronosGroup/Vulkan-Headers
- https://github.com/KhronosGroup/Vulkan-Loader
- https://github.com/KhronosGroup/SPIRV-Headers
- https://github.com/KhronosGroup/SPIRV-Tools
- https://github.com/KhronosGroup/glslang
- https://github.com/google/shaderc
- https://github.com/libsdl-org/SDL
- https://github.com/lua/lua
- https://github.com/nothings/stb

# SDL_GetRendererProperties:
- SDL_PROP_RENDERER_NAME_STRING: the name of the rendering driver
  - direct3d11, opengl, vulkan, etc...

# Credits:
- Grok A.I on x.
    - translate c vulkan to lua vulkan.
- Github repo examples from users.