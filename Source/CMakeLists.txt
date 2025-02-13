# Source/CMakeLists.txt

# On dit qu'on veut tous nos .hpp/.cpp en UTF-8, ou d'autres configs globales

# Ajoute le sous-répertoire Engine qui va créer la lib "Engine"
add_subdirectory(Engine)

# Ajoute le sous-répertoire Game qui va créer la lib "Game" 
# ou un exécutable "Game" selon le cas
add_subdirectory(Game)

# Ici on construit l'exécutable principal "Meinkraft" à partir du Main.cpp
# et on le link avec nos 2 libs Engine et Game.

add_executable(Meinkraft Main.cpp)

# Lien avec Engine / Game
target_link_libraries(Meinkraft
	PRIVATE
	EngineLib  # le nom qu'on aura défini dans Engine/CMakeLists
	GameLib    # le nom défini dans Game/CMakeLists
)

# Optionnel : set properties, etc.
