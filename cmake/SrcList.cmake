set(SOURCE_DIRECTORY "${PROJECT_SOURCE_DIR}/src")

set(SRC_FILES
  # Assets
  "${SOURCE_DIRECTORY}/assets/asset.h"
  "${SOURCE_DIRECTORY}/assets/font_cache.h"
  "${SOURCE_DIRECTORY}/assets/font_cache.cpp"
  "${SOURCE_DIRECTORY}/assets/font_loader.h"
  "${SOURCE_DIRECTORY}/assets/font_loader.cpp"
  "${SOURCE_DIRECTORY}/assets/image_loader.h"
  "${SOURCE_DIRECTORY}/assets/image_loader.cpp"
  "${SOURCE_DIRECTORY}/assets/material_cache.h"
  "${SOURCE_DIRECTORY}/assets/material_cache.cpp"

  # Core
  "${SOURCE_DIRECTORY}/core/config.h"
  "${SOURCE_DIRECTORY}/core/core.h"
  "${SOURCE_DIRECTORY}/core/primal_string.h"
  "${SOURCE_DIRECTORY}/core/primal_string.cpp"
  "${SOURCE_DIRECTORY}/core/thread_context.h"
  "${SOURCE_DIRECTORY}/core/thread_context.cpp"

  # Core/DS
  "${SOURCE_DIRECTORY}/core/data_structures/array.h"
  "${SOURCE_DIRECTORY}/core/data_structures/stack.h"
  "${SOURCE_DIRECTORY}/core/data_structures/stack.cpp"

  # Core/Math
  "${SOURCE_DIRECTORY}/core/math/geometry.h"
  "${SOURCE_DIRECTORY}/core/math/math.h"
  "${SOURCE_DIRECTORY}/core/math/math.cpp"
  "${SOURCE_DIRECTORY}/core/math/noise.h"

  # Core/Memory
  "${SOURCE_DIRECTORY}/core/memory/arena.h"
  "${SOURCE_DIRECTORY}/core/memory/arena.cpp"

  # Platform
  "${SOURCE_DIRECTORY}/platform/window.h"
  "${SOURCE_DIRECTORY}/platform/window.cpp"

  # Platform/OS
  "${SOURCE_DIRECTORY}/platform/os/events.h"
  "${SOURCE_DIRECTORY}/platform/os/keys.h"
  "${SOURCE_DIRECTORY}/platform/os/os.h"


  # Platform/Vulkan
  "${SOURCE_DIRECTORY}/platform/vk_types.h"
  "${SOURCE_DIRECTORY}/platform/vulkan/vmaUsage.cpp"
  "${SOURCE_DIRECTORY}/platform/vulkan/vulkan_structures_helpers.h"

  "${SOURCE_DIRECTORY}/platform/vulkan/buffers.h"
  "${SOURCE_DIRECTORY}/platform/vulkan/buffers.cpp"
  "${SOURCE_DIRECTORY}/platform/vulkan/swapchain.h"
  "${SOURCE_DIRECTORY}/platform/vulkan/swapchain.cpp"
  "${SOURCE_DIRECTORY}/platform/vulkan/vulkan_descriptor.h"
  "${SOURCE_DIRECTORY}/platform/vulkan/vulkan_descriptor.cpp"
  "${SOURCE_DIRECTORY}/platform/vulkan/vulkan_images.h"
  "${SOURCE_DIRECTORY}/platform/vulkan/vulkan_images.cpp"
  "${SOURCE_DIRECTORY}/platform/vulkan/vulkan_loader.h"
  "${SOURCE_DIRECTORY}/platform/vulkan/vulkan_loader.cpp"
  "${SOURCE_DIRECTORY}/platform/vulkan/vulkan_pipeline.h"
  "${SOURCE_DIRECTORY}/platform/vulkan/vulkan_pipeline.cpp"
  "${SOURCE_DIRECTORY}/platform/vulkan/vulkan_renderer.h"
  "${SOURCE_DIRECTORY}/platform/vulkan/vulkan_renderer.cpp"
  "${SOURCE_DIRECTORY}/platform/vulkan/vulkan_shader.h"
  "${SOURCE_DIRECTORY}/platform/vulkan/vulkan_shader.cpp"
  "${SOURCE_DIRECTORY}/platform/vulkan/vulkan_texture.h"
  "${SOURCE_DIRECTORY}/platform/vulkan/vulkan_texture.cpp"

  # Renderer
  "${SOURCE_DIRECTORY}/renderer/material.h"
  "${SOURCE_DIRECTORY}/renderer/material.cpp"
  "${SOURCE_DIRECTORY}/renderer/material_config.h"
  "${SOURCE_DIRECTORY}/renderer/material_config.cpp"

  # Terrain
  "${SOURCE_DIRECTORY}/terrain/voxel.h"
  "${SOURCE_DIRECTORY}/terrain/voxel.cpp"

  # UI
  "${SOURCE_DIRECTORY}/ui/generated.h"
  "${SOURCE_DIRECTORY}/ui/ui_types.h"
  "${SOURCE_DIRECTORY}/ui/ui_utils.h"
  "${SOURCE_DIRECTORY}/ui/ui_manager.h"
  "${SOURCE_DIRECTORY}/ui/ui_manager.cpp"
  "${SOURCE_DIRECTORY}/ui/ui_widgets.h"
  "${SOURCE_DIRECTORY}/ui/ui_widgets.cpp"

  # Utils
  "${SOURCE_DIRECTORY}/utils/colors.h"
  "${SOURCE_DIRECTORY}/utils/geometry.h"
  "${SOURCE_DIRECTORY}/utils/fonts.h"
  "${SOURCE_DIRECTORY}/utils/fonts.cpp"

  "${SOURCE_DIRECTORY}/camera.h"
  "${SOURCE_DIRECTORY}/camera.cpp"

  "${SOURCE_DIRECTORY}/entity.h"
  "${SOURCE_DIRECTORY}/entity.cpp"

  "${SOURCE_DIRECTORY}/primal_engine.h"
  "${SOURCE_DIRECTORY}/primal_engine.cpp"
)

# Sandbox entry point
set(SANDBOX_FILES
  "${PROJECT_SOURCE_DIR}/sandbox/main.cpp"
)

# Shader files
set(GLSL_SOURCES
  "${PROJECT_SOURCE_DIR}/res/shaders/mesh.vert"
  "${PROJECT_SOURCE_DIR}/res/shaders/mesh.frag"

  "${PROJECT_SOURCE_DIR}/res/shaders/sdf_text.vert"
  "${PROJECT_SOURCE_DIR}/res/shaders/sdf_text.frag"

  "${PROJECT_SOURCE_DIR}/res/shaders/ui.frag"
  "${PROJECT_SOURCE_DIR}/res/shaders/ui.vert"

  "${PROJECT_SOURCE_DIR}/res/shaders/ui_texture.vert"
  "${PROJECT_SOURCE_DIR}/res/shaders/ui_texture.frag"

  "${PROJECT_SOURCE_DIR}/res/shaders/voxels.vert"
  "${PROJECT_SOURCE_DIR}/res/shaders/voxels.frag"

  "${PROJECT_SOURCE_DIR}/res/shaders/gbuffer_resolve.comp"
  "${PROJECT_SOURCE_DIR}/res/shaders/voxel_dda.comp"
)
set(GLSL_HEADER_FILES 
  "${PROJECT_SOURCE_DIR}/res/shaders/input_structures.h" 
  "${PROJECT_SOURCE_DIR}/res/shaders/ui_structures.h" 
)

# Add Platform-specific files
if(UNIX)
  message(STATUS "Collecting UNIX-specific source files.")
  list(APPEND SRC_FILES "${SOURCE_DIRECTORY}/platform/os/os_linux.cpp")
elseif(WIN32)
  message(STATUS "Collecting Windows-specific source files.")
  list(APPEND SRC_FILES "${SOURCE_DIRECTORY}/platform/os/os_win32.cpp")
else()
  message(FATAL_ERROR "Unknown platform.")
endif()

set(ENGINE_SOURCES ${SRC_FILES})

set(SANDBOX_SOURCES ${SANDBOX_FILES})
