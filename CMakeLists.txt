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

file(GLOB_RECURSE MinePPSources src/*.cpp src/*.hpp)
add_executable(MinePP ${MinePPSources})
target_precompile_headers(MinePP PRIVATE src/MinePP.hpp)

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
