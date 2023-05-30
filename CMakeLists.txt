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
        src/Application.cpp
        )

set(EDITOR_HEADERS
        src/Application.h
        src/Texture.h)

if(WIN32)
    message(FATAL_ERROR "Unsupported platform")
elseif(APPLE)
    set(SDL2_FRAMEWORK "${PROJECT_SOURCE_DIR}/sdl/SDL2.framework")
    set(SDL2_HEADERS "${SDL2_FRAMEWORK}/Headers")

    set(CMAKE_BUILD_RPATH "@executable_path/../Frameworks")

    add_executable(mini-fps-level-editor MACOSX_BUNDLE ${EDITOR_SOURCE} ${IMGUI_SOURCE} ${TINYFD_SOURCE})
    set_target_properties(mini-fps-level-editor PROPERTIES
            MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/assets/Info.plist
            )

    target_include_directories(mini-fps-level-editor
            PRIVATE
            imgui/
            pfd/
            ${SDL2_HEADERS}
            stb/
            )

    target_link_libraries(mini-fps-level-editor PRIVATE
            ${SDL2_FRAMEWORK}
            )

    add_custom_command(TARGET mini-fps-level-editor POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/mini-fps-level-editor.app/Contents/Frameworks
            COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/sdl/SDL2.framework ${CMAKE_CURRENT_BINARY_DIR}/mini-fps-level-editor.app/Contents/Frameworks/SDL2.framework
            COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/mini-fps-level-editor.app/Contents/Resources
            COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/assets/mini-fps-level-editor.sh ${CMAKE_CURRENT_BINARY_DIR}/mini-fps-level-editor.app/Contents/MacOS
            COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/assets/testIcon.icns ${CMAKE_CURRENT_BINARY_DIR}/mini-fps-level-editor.app/Contents/Resources
            COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/assets/sprites ${CMAKE_CURRENT_BINARY_DIR}/mini-fps-level-editor.app/Contents/Resources/sprites
            COMMAND install_name_tool -change ${CMAKE_CURRENT_SOURCE_DIR}/lib/SDL2.framework/Versions/A/SDL2 @executable_path/../Frameworks/SDL2.framework/Versions/A/SDL2 "${CMAKE_CURRENT_BINARY_DIR}/mini-fps-level-editor.app/Contents/MacOS/mini-fps-level-editor"
            )

endif()