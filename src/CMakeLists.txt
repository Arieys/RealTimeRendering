project(renderer LANGUAGES C CXX VERSION 1.1)

# add the project src
file(GLOB_RECURSE PROJECT_SRC CONFIGURE_DEPENDS *.cpp)
file(GLOB_RECURSE PROJECT_HDR CONFIGURE_DEPENDS *.h *.hpp)

add_executable(${PROJECT_NAME} ${PROJECT_HDR} ${PROJECT_SRC})

target_include_directories(${PROJECT_NAME} PRIVATE "${ROOT}/external/assimp/include")

target_link_libraries(${PROJECT_NAME} PRIVATE glad)
target_link_libraries(${PROJECT_NAME} PRIVATE glfw)
target_link_libraries(${PROJECT_NAME} PRIVATE glm)
target_link_libraries(${PROJECT_NAME} PRIVATE stb)
target_link_libraries(${PROJECT_NAME} PRIVATE imgui)
target_link_libraries(${PROJECT_NAME} PRIVATE assimp)
target_link_libraries(${PROJECT_NAME} PRIVATE spdlog)
