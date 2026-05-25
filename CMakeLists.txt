cmake_minimum_required(VERSION 3.20)
project(hahaha2)

set(CMAKE_CXX_STANDARD 17)

# Collect all source files
file(GLOB_RECURSE SOURCES
    "src/*.cpp"
    "src/*.c"
    "include/imgui/*.cpp"
    "include/imgui/backends/imgui_impl_glfw.cpp"
    "include/imgui/backends/imgui_impl_opengl3.cpp"
)

add_executable(hahaha2 ${SOURCES})

# Include directories
target_include_directories(hahaha2 PRIVATE
    src/
    include/
    include/imgui/
    include/imgui/backends/
)

# Find packages
find_package(glfw3 REQUIRED)
find_package(OpenAL REQUIRED)
find_package(assimp REQUIRED)
find_package(OpenGL REQUIRED)
find_package(glm REQUIRED)

# Link libraries
target_link_libraries(hahaha2 PRIVATE
    glfw
    OpenAL::OpenAL
    assimp::assimp
    OpenGL::GL
    glm::glm
)

# Copy assets to build output directory
add_custom_command(TARGET hahaha2 POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/shaders $<TARGET_FILE_DIR:hahaha2>/shaders
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/textures $<TARGET_FILE_DIR:hahaha2>/textures
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/audio $<TARGET_FILE_DIR:hahaha2>/audio
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/models $<TARGET_FILE_DIR:hahaha2>/models
)

# Linux-specific
if(UNIX)
    target_link_libraries(hahaha2 PRIVATE dl m pthread)
endif()