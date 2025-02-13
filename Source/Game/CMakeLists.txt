# Source/Game/CMakeLists.txt

# Récupérer tous les .cpp du dossier Game
file(GLOB_RECURSE GAME_SOURCES
	 "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
)

add_library(GameLib STATIC
	${GAME_SOURCES}
)

target_include_directories(GameLib 
	PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}/../Engine
)

target_link_libraries(GameLib
	PUBLIC
	EngineLib
)

target_compile_features(GameLib PUBLIC cxx_std_17)
