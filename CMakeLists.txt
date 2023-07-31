cmake_minimum_required(VERSION 3.20)

# language settings
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_EXTENSIONS ON)

set(ROOT "${CMAKE_CURRENT_SOURCE_DIR}")

project(real_time_rendering LANGUAGES C CXX VERSION 1.1)

# build in source in not recommended
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
    message(WARNING "The binary directory of CMake should not \
                     be the same as source directory")
endif()

configure_file("${ROOT}/config/config.h.in" "${ROOT}/src/config.h")

# disable max/min definition in windows
if (WIN32)
    add_definitions(-DNOMINMAX -D_USE_MATH_DEFINES)
endif()

# add libraries
if(EMSCRIPTEN)
    message(STATUS "use GLFW in Emscripten")
    message(STATUS "use OpenGL/ES in Emscripten")
else()
    set(GLFW_BUILD_EXAMPLES OFF)
    set(GLFW_BUILD_TESTS OFF)
    set(GLFW_BUILD_DOCS OFF)
    add_subdirectory(./external/glfw)
    set_target_properties(glfw PROPERTIES FOLDER "lib/glfw")
    set_target_properties(uninstall PROPERTIES FOLDER "lib/glfw")
    set_target_properties(update_mappings PROPERTIES FOLDER "lib/glfw")

    add_subdirectory(./external/glad)
    set_target_properties(glad PROPERTIES FOLDER "lib")
endif()

add_subdirectory(./external/glm)
set_target_properties(glm PROPERTIES FOLDER "lib/glm")

add_subdirectory(./external/stb)
set_target_properties(stb PROPERTIES FOLDER "lib")

add_subdirectory(./external/imgui)
set_target_properties(stb PROPERTIES FOLDER "lib")

add_subdirectory(./external/assimp)
set_target_properties(assimp PROPERTIES FOLDER "lib")

add_subdirectory(src)