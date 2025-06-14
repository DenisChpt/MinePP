cmake_minimum_required(VERSION 3.22.1)
project(MinePP)

set(CMAKE_CXX_STANDARD 23)

if (CMAKE_BUILD_TYPE MATCHES Release)
	set(CMAKE_CXX_FLAGS "-O3")
elseif (CMAKE_BUILD_TYPE MATCHES Trace)
	set(CMAKE_CXX_FLAGS "-O3")
	add_compile_definitions(ENABLE_TRACING)
elseif (CMAKE_BUILD_TYPE MATCHES Debug)
	set(CMAKE_CXX_FLAGS "-g")
endif ()

# Liste explicite des fichiers sources
set(MinePPSources
    src/main.cpp
    src/Application/Application.cpp
    src/Application/Window.cpp
    src/Core/Assets.cpp
    src/Game/Behaviors.cpp
    src/Game/Effects.cpp
    src/Persistence/Persistence.cpp
    src/Physics/MovementSimulation.cpp
    src/Rendering/BlockVertex.cpp
    src/Rendering/ColorRenderPass.cpp
    src/Rendering/Framebuffers.cpp
    src/Rendering/Mesh.cpp
    src/Rendering/ParticleSystem.cpp
    src/Rendering/Shaders.cpp
    src/Rendering/Textures.cpp
    src/Scene/Player.cpp
    src/Scene/Scene.cpp
    src/World/Chunk.cpp
    src/World/ChunkMeshBuilder.cpp
    src/World/ChunkRegion.cpp
    src/World/World.cpp
    src/World/WorldGenerator.cpp
)

# Headers principaux (pour l'IDE)
set(MinePPHeaders
    src/Common.hpp
    src/Application/Application.hpp
    src/Application/Window.hpp
    src/Core/Assets.hpp
    src/Core/PerformanceMonitor.hpp
    src/Game/Behaviors.hpp
    src/Game/Effects.hpp
    src/Math/Math.hpp
    src/Math/Math.inl
    src/Persistence/Persistence.hpp
    src/Physics/MovementSimulation.hpp
    src/Rendering/BlockVertex.hpp
    src/Rendering/Buffers.hpp
    src/Rendering/ColorRenderPass.hpp
    src/Rendering/Framebuffers.hpp
    src/Rendering/Mesh.hpp
    src/Rendering/ParticleSystem.hpp
    src/Rendering/Shaders.hpp
    src/Rendering/Textures.hpp
    src/Scene/Camera.hpp
    src/Scene/Player.hpp
    src/Scene/Scene.hpp
    src/Utils/Utils.hpp
    src/World/BlockTypes.hpp
    src/World/Chunk.hpp
    src/World/ChunkMeshBuilder.hpp
    src/World/ChunkMeshTask.hpp
    src/World/ChunkRegion.hpp
    src/World/World.hpp
    src/World/WorldConstants.hpp
    src/World/WorldGenerator.hpp
)

add_executable(MinePP ${MinePPSources} ${MinePPHeaders})
target_precompile_headers(MinePP PRIVATE src/Common.hpp)

# glfw
add_subdirectory(external/glfw EXCLUDE_FROM_ALL)

# GLAD
add_subdirectory(external/GLAD EXCLUDE_FROM_ALL)

# glm
add_subdirectory(external/glm EXCLUDE_FROM_ALL)

# imgui
add_library(
		imgui EXCLUDE_FROM_ALL
		external/imgui/imgui.cpp
		external/imgui/imgui.h
		external/imgui/imgui_demo.cpp
		external/imgui/imgui_draw.cpp
		external/imgui/imgui_internal.h
		external/imgui/imgui_widgets.cpp
		external/imgui/imstb_rectpack.h
		external/imgui/imstb_textedit.h
		external/imgui/imstb_truetype.h
		external/imgui/imconfig.h
		external/imgui/imgui_tables.cpp
		external/imgui/backends/imgui_impl_glfw.h
		external/imgui/backends/imgui_impl_glfw.cpp
		external/imgui/backends/imgui_impl_opengl3.h
		external/imgui/backends/imgui_impl_opengl3.cpp
)
include_directories(imgui external/imgui external/imgui/backends)

# include FastNoiseLite
include_directories(MinePP external/FastNoiseLite)

# include FrustumCulling
include_directories(MinePP external/FrustumCulling)

# include nlohmann
include_directories(MinePP external/nlohmann)

# include stb
include_directories(MinePP external/stb)

# lodepng
add_library(lodepng EXCLUDE_FROM_ALL external/lodepng/lodepng.h external/lodepng/lodepng.cpp)
include_directories(lodepng external/lodepng)

# link glfw to imgui and link everything to the MinePP app
target_link_libraries(imgui PRIVATE glfw)
target_link_libraries(MinePP PRIVATE glfw glm glad imgui lodepng)

# set a symlink to the assets dir
add_custom_command(
		TARGET MinePP PRE_BUILD COMMAND
		${CMAKE_COMMAND} -E create_symlink
		${CMAKE_SOURCE_DIR}/assets ${CMAKE_CURRENT_BINARY_DIR}/assets
)