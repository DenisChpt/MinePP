cmake_minimum_required(VERSION 3.10)
project(MinePP)

set(OpenGL_GL_PREFERENCE GLVND)

# Si aucun type de build n'est spécifié, on force Debug
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Choix du type de build" FORCE)
endif()

# Recherche récursive des fichiers sources C++ dans le répertoire src
file(GLOB_RECURSE SOURCE_FILES CONFIGURE_DEPENDS src/*.cpp)

add_executable(MinePP
    ${SOURCE_FILES}
    external/glew/src/glew.c
    external/sqlite/sqlite3.c
    external/tinycthread/tinycthread.c
)

# Utilisation du standard C++17
target_compile_features(MinePP PUBLIC cxx_std_17)

#

target_compile_options(MinePP PRIVATE -O3)

# Ajout des répertoires d'inclusion pour la cible MinePP
target_include_directories(MinePP PRIVATE
    ${CMAKE_SOURCE_DIR}/src/auth
    ${CMAKE_SOURCE_DIR}/src/config
    ${CMAKE_SOURCE_DIR}/src/database
    ${CMAKE_SOURCE_DIR}/src/network
    ${CMAKE_SOURCE_DIR}/src/noise
    ${CMAKE_SOURCE_DIR}/src/utils
    ${CMAKE_SOURCE_DIR}/src/world
    ${CMAKE_SOURCE_DIR}/src/window
    ${CMAKE_SOURCE_DIR}/src/render
    ${CMAKE_SOURCE_DIR}/src/worker
    ${CMAKE_SOURCE_DIR}/src/player
    ${CMAKE_SOURCE_DIR}/src/input
    ${CMAKE_SOURCE_DIR}/src/command
    ${CMAKE_SOURCE_DIR}/src/ressource
    ${CMAKE_SOURCE_DIR}/external/glew/include
    ${CMAKE_SOURCE_DIR}/external/glfw/include
    ${CMAKE_SOURCE_DIR}/external/stb_image
    ${CMAKE_SOURCE_DIR}/external/sqlite
    ${CMAKE_SOURCE_DIR}/external/tinycthread
)

# Ajout du sous-répertoire pour GLFW si nécessaire
add_subdirectory(external/glfw)

# Récupération des dépendances
find_package(CURL REQUIRED)
find_package(OpenGL REQUIRED)
target_include_directories(MinePP PRIVATE ${CURL_INCLUDE_DIRS})

# Configuration pour les différentes plateformes
if(APPLE)
    target_link_libraries(MinePP glfw ${CURL_LIBRARIES})
elseif(UNIX AND NOT APPLE)
    target_link_libraries(MinePP dl glfw ${CURL_LIBRARIES} ${OPENGL_LIBRARIES})
elseif(MINGW)
    target_link_libraries(MinePP ws2_32 glfw ${CURL_LIBRARIES})
endif()
