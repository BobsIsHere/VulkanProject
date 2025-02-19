cmake_minimum_required (VERSION 3.11)
project ("VulkanProject")

# Use C++20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Vulkan REQUIRED)

# Add source to this project's executable.
add_executable (VulkanProject "VulkanProject.cpp")

# Include glfw
include(FetchContent)

FetchContent_Declare(
	  glfw
	  GIT_REPOSITORY https://github.com/glfw/glfw.git
	  GIT_TAG        3.4
	  GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(glfw)

target_link_libraries(${PROJECT_NAME} PRIVATE Vulkan::Vulkan glfw)

# Include glm
include(FetchContent)

FetchContent_Declare(
	  glm
	  GIT_REPOSITORY https://github.com/g-truc/glm.git
	  GIT_TAG        1.0.1
	  GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(glm)

target_link_libraries(${PROJECT_NAME} PRIVATE Vulkan::Vulkan glm::glm)

# Define the shader directory
set(SHADER_DIR "${CMAKE_SOURCE_DIR}/shaders")
set(OUTPUT_DIR "${CMAKE_BINARY_DIR}/shaders")

# Ensure shaders are copied to the build directory
add_custom_target(CopyShaders ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${SHADER_DIR} ${OUTPUT_DIR}
    COMMENT "Copying shaders to output directory..."
)

# Ensure shaders are copied whenever the project is built
add_dependencies(${PROJECT_NAME} CopyShaders)