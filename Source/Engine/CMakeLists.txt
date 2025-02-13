# Source/Engine/CMakeLists.txt

# Récupérer tous les .cpp du dossier Engine et sous-dossiers
file(GLOB_RECURSE ENGINE_SOURCES
	 "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
)

add_library(EngineLib STATIC
	${ENGINE_SOURCES}
)

# Include directories
target_include_directories(EngineLib 
	PUBLIC 
	# Le dossier Source/Engine pour accéder aux .hpp
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}/../Game
	${CMAKE_SOURCE_DIR}/External/Include
	# Si tu as besoin d'ajouter ../.. ou d'autres includes
)

find_package(SDL2 REQUIRED)
find_package(GLEW REQUIRED)


set(OpenGL_GL_PREFERENCE "GLVND")
find_package(OpenGL REQUIRED)

target_link_libraries(EngineLib
	PUBLIC
	SDL2::SDL2
	GLEW::GLEW
	OpenGL::GL
)

# On peut aussi définir des options de compilation
target_compile_features(EngineLib PUBLIC cxx_std_17)
