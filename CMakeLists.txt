cmake_minimum_required(VERSION 3.23)
project(mini-fps-level-editor)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_BINARY_DIR bin/)

set(IMGUI_SOURCE
        imgui/imgui.cpp
        imgui/imgui_demo.cpp
        imgui/imgui_draw.cpp
        imgui/imgui_impl_sdl2.cpp
        imgui/imgui_impl_sdlrenderer.cpp
        imgui/imgui_tables.cpp
        imgui/imgui_widgets.cpp)

set(IMGUI_HEADERS
        imgui/imconfig.h
        imgui/imgui.h
        imgui/imgui_impl_sdl2.h
        imgui/imgui_impl_sdlrenderer.h
        imgui/imgui_internal.h
        imgui/imstb_rectpack.h
        imgui/imstb_textedit.h
        imgui/imstb_truetype.h)

set(EDITOR_SOURCE
        src/main.cpp
        src/Application.cpp)

set(EDITOR_HEADERS
        src/Application.h)

if(WIN32)
    message(FATAL_ERROR "Unsupported platform")
elseif(APPLE)
    set(SDL2_FRAMEWORK "${PROJECT_SOURCE_DIR}/sdl/SDL2.framework")
    set(SDL2_HEADERS "${SDL2_FRAMEWORK}/Headers")

    set(CMAKE_BUILD_RPATH "@executable_path/../Frameworks")

    add_executable(mini-fps-level-editor ${EDITOR_SOURCE} ${IMGUI_SOURCE})

    target_include_directories(mini-fps-level-editor
            PRIVATE
            imgui/
            ${SDL2_HEADERS})

    target_link_libraries(mini-fps-level-editor PRIVATE
            ${SDL2_FRAMEWORK})

endif()