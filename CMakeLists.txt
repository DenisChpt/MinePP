cmake_minimum_required(VERSION 3.10)
project(MinePP)

# Utiliser GLVND pour OpenGL et éviter le warning CMP0072
cmake_policy(SET CMP0072 NEW)
set(OpenGL_GL_PREFERENCE GLVND)

# Si aucun type de build n'est spécifié, on force Debug
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Choix du type de build" FORCE)
endif()

# Recherche récursive des fichiers sources C++ dans le répertoire src
file(GLOB_RECURSE SOURCE_FILES CONFIGURE_DEPENDS src/*.cpp)

add_executable(MinePP
    ${SOURCE_FILES}
    deps/glew/src/glew.c
    deps/sqlite/sqlite3.c
    deps/tinycthread/tinycthread.c
)

# Utilisation du standard C++17
target_compile_features(MinePP PUBLIC cxx_std_17)

#
# Options de compilation selon le type de build
#
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    # Optimisation forte en Release
    target_compile_options(MinePP PRIVATE -O3)
elseif(CMAKE_BUILD_TYPE STREQUAL "Debug")
    # Debug : symboles de débogage + pas d'optimisation
    target_compile_options(MinePP PRIVATE -g -O0 -DDEBUG)
    # Optionnel : activer l'AddressSanitizer et l'UndefinedBehaviorSanitizer
    # si vous voulez traquer les erreurs mémoire
    # target_compile_options(MinePP PRIVATE -fsanitize=address,undefined)
    # target_link_options(MinePP PRIVATE -fsanitize=address,undefined)
endif()

# Ajout d'avertissements utiles (GCC/Clang)
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(MinePP PRIVATE -Wall -Wextra -Wpedantic)
endif()

# Ajout des répertoires d'inclusion pour la cible MinePP
target_include_directories(MinePP PRIVATE
    ${CMAKE_SOURCE_DIR}/src/auth
    ${CMAKE_SOURCE_DIR}/src/config
    ${CMAKE_SOURCE_DIR}/src/database
    ${CMAKE_SOURCE_DIR}/src/network
    ${CMAKE_SOURCE_DIR}/src/noise
    ${CMAKE_SOURCE_DIR}/src/utils
    ${CMAKE_SOURCE_DIR}/src/world
    ${CMAKE_SOURCE_DIR}/deps/glew/include
    # ${CMAKE_SOURCE_DIR}/deps/glfw/include
    ${CMAKE_SOURCE_DIR}/deps/stb_image
    ${CMAKE_SOURCE_DIR}/deps/sqlite
    ${CMAKE_SOURCE_DIR}/deps/tinycthread
)

# Ajout du sous-répertoire pour GLFW si nécessaire
# add_subdirectory(deps/glfw)

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
