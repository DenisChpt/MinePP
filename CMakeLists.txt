cmake_minimum_required(VERSION 3.15)
project(MinePP VERSION 1.0 LANGUAGES CXX C)

# Utiliser le standard C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Optionnel : définir le build par défaut sur Release si non précisé
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

# Ajout des répertoires d'includes
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

# Ajouter les sous-dossiers pour les dépendances internes
add_subdirectory(${CMAKE_SOURCE_DIR}/external/glfw)

# Récupérer tous les fichiers sources dans src et asset
file(GLOB_RECURSE PROJECT_SOURCES
	${CMAKE_SOURCE_DIR}/src/*.cpp
	${CMAKE_SOURCE_DIR}/asset/*.cpp
	${CMAKE_SOURCE_DIR}/asset/texture/*.cpp
)

# Récupérer les fichiers shaders (pour copie post-compilation)
file(GLOB_RECURSE SHADERS
	${CMAKE_SOURCE_DIR}/shaders/*.vert
	${CMAKE_SOURCE_DIR}/shaders/*.frag
)

add_library(glad STATIC ${CMAKE_SOURCE_DIR}/external/glad/src/glad.c)
target_include_directories(glad PUBLIC ${CMAKE_SOURCE_DIR}/external/glad/include)

# Créer l'exécutable
add_executable(MinePP ${PROJECT_SOURCES} ${SHADERS})

# Lier les bibliothèques externes (OpenGL fourni par le système)
find_package(OpenGL REQUIRED)
find_package(GLUT REQUIRED)
include_directories(${GLUT_INCLUDE_DIRS})

target_link_libraries(MinePP
	OpenGL::GL
	glfw          # Utilise la cible compilée de GLFW
	glad          # Utilise la cible compilée de glad
	${GLUT_LIBRARIES}
)

# Copier les shaders dans le dossier de l'exécutable après compilation
add_custom_command(TARGET MinePP POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_directory
	${CMAKE_SOURCE_DIR}/shaders $<TARGET_FILE_DIR:MinePP>/shaders
)

# Copier le fichier blocks.json dans le dossier de l'exécutable
add_custom_command(TARGET MinePP POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_if_different
	${CMAKE_SOURCE_DIR}/asset/blocks.json $<TARGET_FILE_DIR:MinePP>/
)

message(STATUS "Configuration MinePP:")
message(STATUS "  Build Type      : ${CMAKE_BUILD_TYPE}")
message(STATUS "  Source Dir      : ${CMAKE_SOURCE_DIR}")
message(STATUS "  Executable Dir  : $<TARGET_FILE_DIR:MinePP>")
