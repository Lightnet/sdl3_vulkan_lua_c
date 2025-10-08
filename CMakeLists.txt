cmake_minimum_required(VERSION 3.22)
project(sdl3_lua)

# Set output directories
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

# find_package(Threads REQUIRED)

# Set C standard (since you're using .c files)
# set(CMAKE_C_STANDARD 11)
# set(CMAKE_C_STANDARD 99)
# set(CMAKE_C_STANDARD_REQUIRED ON)

# Find Vulkan
# find_package(Vulkan REQUIRED)

# Include FetchContent for downloading dependencies
include(FetchContent)

# Find Vulkan
# find_package(Vulkan REQUIRED)

# Fetch raylib
FetchContent_Declare(
    SDL3
    GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
    GIT_TAG release-3.2.22
    GIT_SHALLOW TRUE
    USES_TERMINAL_DOWNLOAD TRUE
)
FetchContent_MakeAvailable(sdl3)

# Provides the Vulkan::Headers target, which sets up include directories for <vulkan/vulkan.h> and other headers.
FetchContent_Declare(
    vulkanheaders
    GIT_REPOSITORY https://github.com/KhronosGroup/Vulkan-Headers.git
    GIT_TAG vulkan-sdk-1.4.313.0
    GIT_SHALLOW TRUE
    USES_TERMINAL_DOWNLOAD TRUE
)
FetchContent_MakeAvailable(vulkanheaders)

# Fetch Vulkan-Loader
# Builds the Vulkan loader library, providing the vulkan target (equivalent to vulkan-1.lib on Windows).
# The vulkan target includes the runtime implementations of Vulkan API functions, resolving the linker errors (e.g., vkCreateInstance, vkCmdBeginRenderPass).
FetchContent_Declare(
    VulkanLoader
    GIT_REPOSITORY https://github.com/KhronosGroup/Vulkan-Loader.git
    GIT_TAG vulkan-sdk-1.4.313.0 # Match the Vulkan-Headers version
)
FetchContent_MakeAvailable(VulkanLoader)

#================================================
# Lua
#================================================
FetchContent_Declare(
    lua
    GIT_REPOSITORY https://github.com/lua/lua.git
    GIT_TAG v5.4.8
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(lua)

# Lua doesn't have a CMake build, so we manually compile it
# List Lua source files (core and libraries, excluding lua.c and luac.c)
set(LUA_SRC
    ${lua_SOURCE_DIR}/lapi.c
    ${lua_SOURCE_DIR}/lauxlib.c
    ${lua_SOURCE_DIR}/lbaselib.c
    ${lua_SOURCE_DIR}/lcode.c
    ${lua_SOURCE_DIR}/lcorolib.c
    ${lua_SOURCE_DIR}/lctype.c
    ${lua_SOURCE_DIR}/ldblib.c
    ${lua_SOURCE_DIR}/ldebug.c
    ${lua_SOURCE_DIR}/ldo.c
    ${lua_SOURCE_DIR}/ldump.c
    ${lua_SOURCE_DIR}/lfunc.c
    ${lua_SOURCE_DIR}/lgc.c
    ${lua_SOURCE_DIR}/linit.c
    ${lua_SOURCE_DIR}/liolib.c
    ${lua_SOURCE_DIR}/llex.c
    ${lua_SOURCE_DIR}/lmathlib.c
    ${lua_SOURCE_DIR}/lmem.c
    ${lua_SOURCE_DIR}/loadlib.c
    ${lua_SOURCE_DIR}/lobject.c
    ${lua_SOURCE_DIR}/lopcodes.c
    ${lua_SOURCE_DIR}/loslib.c
    ${lua_SOURCE_DIR}/lparser.c
    ${lua_SOURCE_DIR}/lstate.c
    ${lua_SOURCE_DIR}/lstring.c
    ${lua_SOURCE_DIR}/lstrlib.c
    ${lua_SOURCE_DIR}/ltable.c
    ${lua_SOURCE_DIR}/ltablib.c
    ${lua_SOURCE_DIR}/ltm.c
    ${lua_SOURCE_DIR}/lundump.c
    ${lua_SOURCE_DIR}/lvm.c
    ${lua_SOURCE_DIR}/lzio.c
    ${lua_SOURCE_DIR}/lutf8lib.c
)

# Create a static Lua library
add_library(lua STATIC ${LUA_SRC})
target_include_directories(lua PUBLIC ${lua_SOURCE_DIR})

FetchContent_Declare(
    stb
    GIT_REPOSITORY https://github.com/nothings/stb.git
    GIT_TAG        master
    EXCLUDE_FROM_ALL #this disable build?
    GIT_SHALLOW TRUE
    USES_TERMINAL_DOWNLOAD TRUE
)
FetchContent_MakeAvailable(stb)


# Fetch cimgui
# FetchContent_Declare(
#     cimgui
#     GIT_REPOSITORY https://github.com/cimgui/cimgui.git
#     # GIT_TAG master 
#     GIT_TAG docking_inter 
#     # GIT_TAG 1.92.1
#     EXCLUDE_FROM_ALL #this disable build? since customize for sdl3 and opengl.
#     GIT_SHALLOW TRUE
#     USES_TERMINAL_DOWNLOAD TRUE
# )
# FetchContent_MakeAvailable(cimgui)

#================================================
# cimgui custom
#================================================

## Define cimgui sources
# set(IMGUI_SOURCES
#     ${cimgui_SOURCE_DIR}/cimgui.cpp
#     ${cimgui_SOURCE_DIR}/imgui/imgui.cpp
#     ${cimgui_SOURCE_DIR}/imgui/imgui_draw.cpp
#     ${cimgui_SOURCE_DIR}/imgui/imgui_widgets.cpp
#     ${cimgui_SOURCE_DIR}/imgui/imgui_demo.cpp
#     ${cimgui_SOURCE_DIR}/imgui/imgui_tables.cpp
#     ${cimgui_SOURCE_DIR}/imgui/backends/imgui_impl_sdl3.cpp
#     ${cimgui_SOURCE_DIR}/imgui/backends/imgui_impl_vulkan.cpp
# )

## Build cimgui as a static library with a custom name
# add_library(custom_cimgui STATIC ${IMGUI_SOURCES})

## Set include directories for custom_cimgui
# target_include_directories(custom_cimgui PUBLIC
#     # ${cimgui_SOURCE_DIR}                      # cimgui
#     # ${cimgui_SOURCE_DIR}/imgui                # cimgui/imgui
#     # ${cimgui_SOURCE_DIR}/imgui/backends       # imgui/backends
#     ${SDL3_SOURCE_DIR}/include                  # SDL 3.2
# )

## Set compile definitions for custom_cimgui (no CIMGUI_DEFINE_ENUMS_AND_STRUCTS)
# target_compile_definitions(custom_cimgui PUBLIC
#     IMGUI_DISABLE_OBSOLETE_FUNCTIONS=1
#     IMGUI_IMPL_API=extern\ \"C\"
# )
# target_compile_definitions(custom_cimgui PUBLIC
    # IMGUI_DISABLE_OBSOLETE_FUNCTIONS=1
    # CIMGUI_USE_SDL3=1
    # CIMGUI_USE_VULKAN=1
    # IMGUI_IMPL_API=extern\ \"C\"
    # CIMGUI_DEFINE_ENUMS_AND_STRUCTS=1
# )

## Link custom_cimgui with GLFW and OpenGL
# target_link_libraries(custom_cimgui PUBLIC 
#     Vulkan::Headers # Vulkan headers
#     vulkan # Vulkan loader library (from Vulkan-Loader)
#     SDL3::SDL3
#     lua
# )

# Skip unnecessary
# set(SHADERC_ENABLE_WGSL_OUTPUT OFF CACHE BOOL "" FORCE)
set(SHADERC_SKIP_INSTALL ON CACHE BOOL "" FORCE)
set(SHADERC_SKIP_TESTS ON CACHE BOOL "" FORCE)
set(SHADERC_SKIP_EXAMPLES ON CACHE BOOL "" FORCE)  # Skip 
set(SHADERC_SKIP_EXECUTABLES ON CACHE BOOL "" FORCE)  # Skip 
set(SHADERC_SKIP_COPYRIGHT_CHECK ON CACHE BOOL "" FORCE)  # Skip 
# set(SHADERC_ENABLE_WERROR_COMPILE ON CACHE BOOL "" FORCE)  # Skip 
set(SHADERC_ENABLE_SHARED_CRT OFF CACHE BOOL "" FORCE)  # this work as does not need other libs stand alone
# set(SHADERC_ENABLE_SHARED_CRT ON CACHE BOOL "" FORCE)  # Skip # lib
set(SHADERC_ENABLE_EXAMPLES OFF CACHE BOOL "" FORCE)  # Skip 
set(SHADERC_ENABLE_INSTALL OFF CACHE BOOL "" FORCE)  # Skip 

set(SPIRV_SKIP_EXECUTABLES ON CACHE BOOL "" FORCE)  # Skip unnecessary tools
# set(SPIRV_HEADERS_ENABLE_TESTS OFF CACHE BOOL "" FORCE)
# set(SPIRV_HEADERS_ENABLE_INSTALL OFF CACHE BOOL "" FORCE)
# set(SPIRV_HEADERS_ENABLE_INSTALL ON CACHE BOOL "" FORCE)

set(SKIP_SPIRV_TOOLS_INSTALL ON CACHE BOOL "" FORCE)
set(SPIRV_SKIP_TESTS ON CACHE BOOL "" FORCE)  # Skip unnecessary tools

set(BUILD_EXTERNAL OFF CACHE BOOL "" FORCE)
# set(BUILD_WERROR ON CACHE BOOL "" FORCE)
set(GLSLANG_TESTS OFF CACHE BOOL "" FORCE)
set(GLSLANG_ENABLE_INSTALL OFF CACHE BOOL "" FORCE)
set(ENABLE_SPIRV ON CACHE BOOL "" FORCE)
set(ENABLE_GLSLANG_BINARIES OFF CACHE BOOL "" FORCE)
set(ENABLE_GLSLANG_JS OFF CACHE BOOL "" FORCE)

# set(ENABLE_HLSL OFF CACHE BOOL "" FORCE)
# set(ENABLE_RTTI OFF CACHE BOOL "" FORCE)
# set(ENABLE_EXCEPTIONS OFF CACHE BOOL "" FORCE)
# set(ENABLE_PCH OFF CACHE BOOL "" FORCE)

set(ALLOW_EXTERNAL_SPIRV_TOOLS OFF CACHE BOOL "" FORCE)

# set(SPIRV_TOOLS_BUILD_STATIC ON CACHE BOOL "" FORCE)  # Build SPIRV-Tools as static
set(SPIRV_TOOLS_BUILD_STATIC OFF CACHE BOOL "" FORCE)  # Build SPIRV-Tools as static

# set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)  # Build shaderc as shared library (DLL)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)  # Build shaderc as shared library (DLL)

# Fetch SPIRV-Headers (base for all)
FetchContent_Declare(
    spirv-headers
    GIT_REPOSITORY https://github.com/KhronosGroup/SPIRV-Headers.git
    GIT_TAG        vulkan-sdk-1.4.313.0  # Use a stable tag; check repo for latest
    # EXCLUDE_FROM_ALL TRUE
    # EXCLUDE_FROM_ALL
)
# Option to skip shaderc tests (recommended for faster builds)
# FetchContent_MakeAvailable(spirv-headers)

# Fetch SPIRV-Tools (depends on spirv-headers)
FetchContent_Declare(
    spirv-tools
    GIT_REPOSITORY https://github.com/KhronosGroup/SPIRV-Tools.git
    # GIT_TAG        v2025.4  # Stable release; update as needed
    GIT_TAG        vulkan-sdk-1.4.313.0  # Stable release; update as needed
    # EXCLUDE_FROM_ALL
)
# FetchContent_MakeAvailable(spirv-tools)

# Fetch glslang (depends on spirv-tools)
FetchContent_Declare(
    glslang
    GIT_REPOSITORY https://github.com/KhronosGroup/glslang.git
    GIT_TAG        16.0.0  # Stable release; update as needed
    # EXCLUDE_FROM_ALL
)
# FetchContent_MakeAvailable(glslang)

# set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)  # Build shaderc as shared library (DLL)
# Fetch shaderc (depends on glslang)
FetchContent_Declare(
    shaderc
    GIT_REPOSITORY https://github.com/google/shaderc.git
    GIT_TAG        v2025.4  # Or a specific tag like v2023.7 for stability
    # EXCLUDE_FROM_ALL FALSE
    # EXCLUDE_FROM_ALL
)

# FetchContent_MakeAvailable(shaderc)

#================================================
# APP
#================================================

# # Application NAME
set(APP_NAME sdl3_lua)

# Source files (add more if needed)
set(SRC_FILES
    # Add other .c files here if necessary
    src/module_sdl.c
    src/module_vulkan.c
)

message(STATUS "cimgui_SOURCE_DIR: >> ${cimgui_SOURCE_DIR}")

# Create executable
add_executable(${APP_NAME}
    ${SRC_FILES}
    # base/base_triangle_resize_context.c
    # src/base_triangle_string.c
    src/main.c
    # src/window.c
)

# LIBS
# target_link_libraries(${APP_NAME} PUBLIC SDL3::SDL3) # SDL 3.2.22
# target_link_libraries(${APP_NAME} PRIVATE custom_cimgui) # custom cimgui
target_link_libraries(${APP_NAME} PRIVATE 
    SDL3::SDL3
    Vulkan::Headers # Vulkan headers
    vulkan # Vulkan loader library (from Vulkan-Loader)
    # shaderc::shaderc_combined
    # Vulkan::shaderc_combined
    # shaderc_combined # 675mb
    # shaderc
    # SPIRV-Tools-shared  # Shared library target for SPIRV-Tools
    # glslang             # Shared library target for glslang
    # shaderc_shared      # Shared library target for shaderc
    lua
)

# Include directories
target_include_directories(${APP_NAME} PUBLIC
    ${CMAKE_SOURCE_DIR}/include                     # root project
    # ${cimgui_SOURCE_DIR}                          # cimgui
    # ${cimgui_SOURCE_DIR}/imgui                    # imgui
    # ${cimgui_SOURCE_DIR}/imgui/backends           # imgui/backends
    ${SDL3_SOURCE_DIR}/include                      # SDL 3.2.22
    ${shaderc_SOURCE_DIR}/libshaderc/include        # shaderc
    ${vulkanheaders_SOURCE_DIR}/include             # SDL 3.2.22
    ${stb_SOURCE_DIR}                               # font
    ${lua_SOURCE_DIR}                               # lua
)

# # for c #define
# target_compile_definitions( ${APP_NAME} PUBLIC
#     # IMGUI_DISABLE_OBSOLETE_FUNCTIONS=1
# 	# CIMGUI_DEFINE_ENUMS_AND_STRUCTS=1
# 	# CIMGUI_USE_SDL3=1
#     # CIMGUI_USE_VULKAN=1
# 	# CIMGUI_USE_OPENGL3=1
# )

# Platform-specific settings for Windows (MinGW/MSYS2)
if (WIN32)
    # Link necessary Windows libraries for raylib
    target_link_libraries(${APP_NAME} PRIVATE
        # opengl32
        gdi32
        winmm
    )

    # Static linking for libgcc and libstdc++
    target_link_options(${APP_NAME} PRIVATE
        -static-libgcc
        -static-libstdc++  # Uncomment if C++ code is used
        -static           # Avoid full static linking to prevent issues with system libraries
    )
endif()

# Shader compilation
# find_program(GLSLC glslc REQUIRED HINTS ENV VULKAN_SDK PATH_SUFFIXES bin)
# set(SHADER_SRC_DIR ${CMAKE_SOURCE_DIR}/assets)
# set(SHADER_OUT_DIR ${CMAKE_BINARY_DIR}/shaders)
# file(MAKE_DIRECTORY ${SHADER_OUT_DIR})

# add_custom_command(
#     OUTPUT ${SHADER_OUT_DIR}/frag.spv
#     COMMAND ${GLSLC} -fshader-stage=frag ${SHADER_SRC_DIR}/frag.glsl -o ${SHADER_OUT_DIR}/frag.spv
#     DEPENDS ${SHADER_SRC_DIR}/frag.glsl
#     COMMENT "Compiling frag.glsl to SPIR-V (fragment shader)"
# )
# add_custom_command(
#     OUTPUT ${SHADER_OUT_DIR}/vert.spv
#     COMMAND ${GLSLC} -fshader-stage=vert ${SHADER_SRC_DIR}/vert.glsl -o ${SHADER_OUT_DIR}/vert.spv
#     DEPENDS ${SHADER_SRC_DIR}/vert.glsl
#     COMMENT "Compiling vert.glsl to SPIR-V (vertex shader)"
# )

# add_custom_target(
#     Shaders ALL
#     DEPENDS ${SHADER_OUT_DIR}/frag.spv ${SHADER_OUT_DIR}/vert.spv
# )
# add_dependencies(${PROJECT_NAME} Shaders)

# # Copy shaders to output directory
# add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
#     COMMAND ${CMAKE_COMMAND} -E copy_if_different
#     ${SHADER_OUT_DIR}/vert.spv
#     $<TARGET_FILE_DIR:${PROJECT_NAME}>/vert.spv
#     COMMENT "Copying vert.spv to output directory"
# )
# add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
#     COMMAND ${CMAKE_COMMAND} -E copy_if_different
#     ${SHADER_OUT_DIR}/frag.spv
#     $<TARGET_FILE_DIR:${PROJECT_NAME}>/frag.spv
#     COMMENT "Copying frag.spv to output directory"
# )


# set(APP_TRI sdl_tri)

# add_executable(${APP_TRI}
#     src/base_triangle_string.c
# )
# target_link_libraries(${APP_TRI} PRIVATE 
#     SDL3::SDL3
#     Vulkan::Headers # Vulkan headers
#     vulkan # Vulkan loader library (from Vulkan-Loader)     
#     # shaderc_combined  # 600 mb
#     # SPIRV-Tools-shared                                          # Shared library target for SPIRV-Tools
#     # glslang                                                     # Shared library target for glslang
#     shaderc_shared                                              # Shared library target for shaderc
#     # shaderc                                                       # 648 mb added to application execute                                        
#     lua
# )

# target_include_directories(${APP_TRI} PRIVATE
#     ${CMAKE_SOURCE_DIR}/include                                 # root project
#     # ${cimgui_SOURCE_DIR}                                      # cimgui
#     # ${cimgui_SOURCE_DIR}/imgui                                # imgui
#     # ${cimgui_SOURCE_DIR}/imgui/backends                       # imgui/backends
#     ${SDL3_SOURCE_DIR}/include                                  # SDL 3.2.22
#     ${shaderc_SOURCE_DIR}/libshaderc/include                    # shaderc
#     ${vulkanheaders_SOURCE_DIR}/include                         # SDL 3.2.22
#     ${stb_SOURCE_DIR}                                           # font
#     ${lua_SOURCE_DIR}                                           # lua
# )

# if (WIN32)
#     # Link necessary Windows libraries for raylib
#     target_link_libraries(${APP_TRI} PRIVATE
#         # opengl32
#         gdi32
#         winmm
#         # Threads::Threads
#     )

#     # Static linking for libgcc and libstdc++
#     target_link_options(${APP_TRI} PRIVATE
#         # -Wl,-Bstatic
#         # -lgcc
#         # -lstdc++
#         # -lwinpthread
#         -static-libgcc
#         -static-libstdc++  # Uncomment if C++ code is used
#         -static           # Avoid full static linking to prevent issues with system libraries
#     )
# endif()

# set(APP_LUA2 sdl_lua2)
# add_executable(${APP_LUA2}
#     lua_vulkan/main.c
#     lua_vulkan/sdl_luajit.c
#     lua_vulkan/vulkan_luajit.c
# )
# target_link_libraries(${APP_LUA2} PRIVATE 
#     SDL3::SDL3
#     Vulkan::Headers # Vulkan headers
#     vulkan # Vulkan loader library (from Vulkan-Loader)     
#     # shaderc_combined  # 600 mb
#     # SPIRV-Tools-shared                                          # Shared library target for SPIRV-Tools
#     # glslang                                                     # Shared library target for glslang
#     shaderc_shared                                              # Shared library target for shaderc
#     # shaderc                                                       # 648 mb added to application execute                                        
#     lua
# )
# target_include_directories(${APP_LUA2} PRIVATE
#     # ${CMAKE_SOURCE_DIR}/include                                 # root project
#     ${CMAKE_SOURCE_DIR}/lua_include                                 # root project
#     # ${cimgui_SOURCE_DIR}                                      # cimgui
#     # ${cimgui_SOURCE_DIR}/imgui                                # imgui
#     # ${cimgui_SOURCE_DIR}/imgui/backends                       # imgui/backends
#     ${SDL3_SOURCE_DIR}/include                                  # SDL 3.2.22
#     ${shaderc_SOURCE_DIR}/libshaderc/include                    # shaderc
#     ${vulkanheaders_SOURCE_DIR}/include                         # SDL 3.2.22
#     ${stb_SOURCE_DIR}                                           # font
#     ${lua_SOURCE_DIR}                                           # lua
# )
# if (WIN32)
#     # Link necessary Windows libraries for raylib
#     target_link_libraries(${APP_LUA2} PRIVATE
#         # opengl32
#         gdi32
#         winmm
#         # Threads::Threads
#     )
#     # Static linking for libgcc and libstdc++
#     target_link_options(${APP_LUA2} PRIVATE
#         # -Wl,-Bstatic
#         # -lgcc
#         # -lstdc++
#         # -lwinpthread
#         -static-libgcc
#         -static-libstdc++  # Uncomment if C++ code is used
#         -static           # Avoid full static linking to prevent issues with system libraries
#     )
# endif()


# set(APP_LUA_VULKAN simple_vulkan)
# add_executable(${APP_LUA_VULKAN}
#     lua54/src/main.c
#     lua54/src/module_sdl.c
#     lua54/src/module_vulkan.c
# )
# target_link_libraries(${APP_LUA_VULKAN} PRIVATE 
#     SDL3::SDL3
#     Vulkan::Headers # Vulkan headers
#     vulkan # Vulkan loader library (from Vulkan-Loader)     
#     # shaderc_combined  # 600 mb
#     # SPIRV-Tools-shared                                          # Shared library target for SPIRV-Tools
#     # glslang                                                     # Shared library target for glslang
#     # shaderc_shared                                              # Shared library target for shaderc # ok
#     # shaderc                                                       # 648 mb added to application execute                                        
#     lua
# )
# target_include_directories(${APP_LUA_VULKAN} PRIVATE
#     # ${CMAKE_SOURCE_DIR}/include                               # root project
#     ${CMAKE_SOURCE_DIR}/lua54/include                           # root project
#     # ${cimgui_SOURCE_DIR}                                      # cimgui
#     # ${cimgui_SOURCE_DIR}/imgui                                # imgui
#     # ${cimgui_SOURCE_DIR}/imgui/backends                       # imgui/backends
#     ${SDL3_SOURCE_DIR}/include                                  # SDL 3.2.22
#     ${shaderc_SOURCE_DIR}/libshaderc/include                    # shaderc
#     ${vulkanheaders_SOURCE_DIR}/include                         # SDL 3.2.22
#     ${stb_SOURCE_DIR}                                           # font
#     ${lua_SOURCE_DIR}                                           # lua
# )
# if (WIN32)
#     # Link necessary Windows libraries for raylib
#     target_link_libraries(${APP_LUA_VULKAN} PRIVATE
#         # opengl32
#         gdi32
#         winmm
#         # Threads::Threads
#     )
#     # Static linking for libgcc and libstdc++
#     target_link_options(${APP_LUA_VULKAN} PRIVATE
#         # -Wl,-Bstatic
#         # -lgcc
#         # -lstdc++
#         # -lwinpthread
#         -static-libgcc
#         -static-libstdc++  # Uncomment if C++ code is used
#         -static           # Avoid full static linking to prevent issues with system libraries
#     )
# endif()