cmake_minimum_required (VERSION 3.11)
project ("VulkanProject")

# Use C++20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Vulkan REQUIRED)

# Add source to this project's executable.
add_executable (VulkanProject "VulkanProject.cpp" "utils/utils.h" "core/Window.h" "core/Window.cpp" "core/VulkanInstance.cpp" "core/VulkanInstance.h" "core/VulkanDevice.h" "core/VulkanDevice.cpp" "core/VulkanSwapChain.h" "core/VulkanSwapChain.cpp" "core/VulkanRenderPass.h" "core/VulkanRenderPass.cpp" "pipelines/GraphicsPipeline.h" "pipelines/GraphicsPipeline.cpp" "core/VulkanCommandPool.h" "core/VulkanCommandPool.cpp" "core/VulkanCommandBuffer.h" "core/VulkanCommandBuffer.cpp" "buffers/VulkanBuffer.h" "buffers/VulkanBuffer.cpp" "buffers/VertexBuffer.h" "buffers/VertexBuffer.cpp" "buffers/IndexBuffer.h" "buffers/IndexBuffer.cpp" "utils/CommandUtils.h" "utils/CommandUtils.cpp" "buffers/UniformBuffer.h" "buffers/UniformBuffer.cpp"   "core/VulkanDescriptorPool.h" "core/VulkanDescriptorPool.cpp")

set(3rdParty "${CMAKE_CURRENT_SOURCE_DIR}/../3rdParty")

# Copy folders to output directory
file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/textures" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")
file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/models" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")

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

# Define the shaders
set(SHADERS
	"${SHADER_DIR}/shader.vert"
	"${SHADER_DIR}/shader.frag"
)

# Ensure shaders are copied to the build directory
add_custom_target(CopyShaders ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${SHADER_DIR} ${OUTPUT_DIR}
    COMMENT "Copying shaders to output directory..."
)

# Ensure shaders are copied whenever the project is built
add_dependencies(${PROJECT_NAME} CopyShaders)

set(GLSLC "C:/VulkanSDK/1.3.296.0/Bin/glslc.exe")

function(compile_shader shader)
	#get_filename_component(FILE_NAME ${shader} NAME)
	get_filename_component(FILE_EXT ${shader} EXT)

	# Remove the leading dot from the extension
    string(REPLACE "." "" FILE_EXT_NO_DOT ${FILE_EXT})

    set(SPIRV_OUTPUT "${SHADER_DIR}/${FILE_EXT_NO_DOT}.spv")

	add_custom_command(
            OUTPUT ${SPIRV_OUTPUT}
            COMMAND "${GLSLC}" ${shader} -o ${SPIRV_OUTPUT}
            COMMENT "Compiling ${shader} to SPIR-V"
            DEPENDS ${shader} ${GLSL_SHADERS}
            VERBATIM
    )

	list(APPEND SPIRV_SHADERS ${SPIRV_OUTPUT})
    set(SPIRV_SHADERS ${SPIRV_SHADERS} PARENT_SCOPE)
endfunction()

foreach (SHADER ${SHADERS})
    compile_shader(${SHADER})
endforeach()

add_custom_target(Shaders ALL DEPENDS ${SPIRV_SHADERS})

add_dependencies(${PROJECT_NAME} Shaders)

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${3rdParty})