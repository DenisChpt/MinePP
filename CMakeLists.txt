cmake_minimum_required(VERSION 3.10)

project(Meinkraft VERSION 1.0 LANGUAGES CXX)

# Si tu utilises C++17, par exemple
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Optionnel : on peut chercher les dépendances ici (SDL2, GLEW, etc.)
# find_package(SDL2 REQUIRED)
# find_package(GLEW REQUIRED)

# Ajout du dossier Source : c'est là qu'on crée nos libraries/executables
add_subdirectory(Source)

# Ajout du dossier Tests
add_subdirectory(Tests)

# Optionnel : doc, external, etc.
# add_subdirectory(Docs)
# add_subdirectory(External)
