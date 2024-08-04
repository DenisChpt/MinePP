cmake_minimum_required(VERSION 3.10)
project(Meinkraft)

# Définir la version de C++
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_BUILD_TYPE Debug)

# Inclure les dépendances
find_package(OpenGL REQUIRED)
find_package(glm REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(PNG REQUIRED libpng)

# Détection de la plateforme
if (WIN32)
	message(STATUS "Compiling on Windows")
	find_package(GLEW REQUIRED)
	find_package(glfw3 3.3 REQUIRED)
	add_definitions(-DPLATFORM_WINDOWS)
elseif (UNIX)
	message(STATUS "Compiling on Linux")
	find_package(GLEW REQUIRED)
	find_package(PkgConfig REQUIRED)
	pkg_search_module(GLFW REQUIRED glfw3)
	add_definitions(-DPLATFORM_LINUX)
endif()

# Répertoires d'inclusion
include_directories(${OPENGL_INCLUDE_DIRS} ${GLM_INCLUDE_DIRS} ${GLEW_INCLUDE_DIRS} ${GLFW_INCLUDE_DIRS})
include_directories(${CMAKE_SOURCE_DIR}/src/engine/input)
include_directories(${CMAKE_SOURCE_DIR}/src/engine/rendering)
include_directories(${CMAKE_SOURCE_DIR}/src/engine/shaders)
include_directories(${CMAKE_SOURCE_DIR}/src/engine/world)
include_directories(${CMAKE_SOURCE_DIR}/src/engine/camera)
include_directories(${CMAKE_SOURCE_DIR}/src/engine/physics)
include_directories(${CMAKE_SOURCE_DIR}/src/game/player)
include_directories(${CMAKE_SOURCE_DIR}/include)

# Fichiers sources
file(GLOB_RECURSE SOURCES src/*.cpp src/*.h)

# Exécutable
add_executable(Meinkraft ${SOURCES})

# Lien des bibliothèques
if (WIN32)
	target_link_libraries(Meinkraft ${OPENGL_LIBRARIES} GLEW::GLEW glfw)
elseif (UNIX)
	target_link_libraries(Meinkraft ${OPENGL_LIBRARIES} ${GLEW_LIBRARIES} ${GLFW_LIBRARIES} ${GLFW_LIBRARIES})
endif()
