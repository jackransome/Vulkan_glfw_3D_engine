cmake_minimum_required(VERSION 3.10)
project(Phase2)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

if(MSVC)
    set(CMAKE_GENERATOR_PLATFORM x64 CACHE STRING "" FORCE)
endif()

# Find Vulkan
find_package(Vulkan REQUIRED)

# Include directories
include_directories(
    Libraries/glfw-3.2.1.bin.WIN64/include
    libraries/glm
    libraries/stb_image
    libraries/tiny_obj_loader
    ${Vulkan_INCLUDE_DIRS}
)

# Add GLFW library directory
link_directories(
    Libraries/glfw-3.2.1.bin.WIN64/lib-vc2015
)

# Source files
file(GLOB_RECURSE SOURCES "source/*.cpp" "source/*.h")

# Add executable
add_executable(Phase2 ${SOURCES})

# Link libraries
target_link_libraries(Phase2
    glfw3.lib
    opengl32.lib
    ${Vulkan_LIBRARIES}
)

# Set output directories
set_target_properties(Phase2 PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/bin/Debug"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/bin/Release"
)

# Copy shaders and resources after build
add_custom_command(TARGET Phase2 POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/resources"
        "$<TARGET_FILE_DIR:Phase2>/resources"
)
