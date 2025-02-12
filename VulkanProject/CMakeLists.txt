cmake_minimum_required (VERSION 3.8)
project ("VulkanProject")

# Add source to this project's executable.
add_executable (VulkanProject "VulkanProject.cpp" "VulkanProject.h")

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET VulkanProject PROPERTY CXX_STANDARD 20)
endif()


