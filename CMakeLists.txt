cmake_minimum_required(VERSION 3.15)
project(MinePP VERSION 1.0 LANGUAGES CXX C)

# Set the C++ standard to C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# If no build type is specified, default to Release
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

# Include paths
include_directories(
	${CMAKE_SOURCE_DIR}/src
	${CMAKE_SOURCE_DIR}/asset
	${CMAKE_SOURCE_DIR}/asset/texture
	${CMAKE_SOURCE_DIR}/external/nlohmann
	${CMAKE_SOURCE_DIR}/external/stb
	${CMAKE_SOURCE_DIR}/external/glad/include
	${CMAKE_SOURCE_DIR}/external/glfw/include
	${CMAKE_SOURCE_DIR}/shaders
)

# Add GLFW subdirectory
add_subdirectory(${CMAKE_SOURCE_DIR}/external/glfw)

# Gather all source files from the project (src/ and asset/)
file(GLOB_RECURSE PROJECT_SOURCES
	${CMAKE_SOURCE_DIR}/src/*.cpp
	${CMAKE_SOURCE_DIR}/asset/*.cpp
	${CMAKE_SOURCE_DIR}/asset/texture/*.cpp
)

# Gather shader files
file(GLOB_RECURSE SHADERS
	${CMAKE_SOURCE_DIR}/shaders/*.vert
	${CMAKE_SOURCE_DIR}/shaders/*.frag
)

# Build glad as a static library
add_library(glad STATIC ${CMAKE_SOURCE_DIR}/external/glad/src/glad.c)
target_include_directories(glad PUBLIC ${CMAKE_SOURCE_DIR}/external/glad/include)

# Create executable
add_executable(MinePP ${PROJECT_SOURCES} ${SHADERS})

# Find system packages for OpenGL and GLUT
find_package(OpenGL REQUIRED)
find_package(GLUT REQUIRED)
include_directories(${GLUT_INCLUDE_DIRS})

# Link libraries
target_link_libraries(MinePP
	OpenGL::GL
	glfw
	glad
	${GLUT_LIBRARIES}
)

# Copy shader files to the executable directory after build
add_custom_command(TARGET MinePP POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_directory
	${CMAKE_SOURCE_DIR}/shaders $<TARGET_FILE_DIR:MinePP>/shaders
)

# Copy blocks.json to the executable directory
add_custom_command(TARGET MinePP POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_if_different
	${CMAKE_SOURCE_DIR}/asset/blocks.json $<TARGET_FILE_DIR:MinePP>/
)

message(STATUS "Configuration MinePP:")
message(STATUS "  Build Type      : ${CMAKE_BUILD_TYPE}")
message(STATUS "  Source Dir      : ${CMAKE_SOURCE_DIR}")
message(STATUS "  Executable Dir  : $<TARGET_FILE_DIR:MinePP>")
