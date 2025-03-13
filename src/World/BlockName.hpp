/**
 * @class BlockName
 * @brief Fournit des utilitaires pour associer un type de bloc à son nom lisible.
 *
 * @details La classe BlockName définit un tableau statique associant les BlockType à leurs noms respectifs
 *          et propose des méthodes pour obtenir le nom ou l'indice correspondant à un type donné.
 */


#pragma once

#include "../MinePP.hpp"
#include "BlockData.hpp"

struct BlockName
{
private:
	using Name = std::pair<BlockData::BlockType, const char *>;

public:
	static constexpr size_t BlockCount = 17;
	using NameArray = std::array<const char *, BlockCount>;
	static constexpr std::array<Name, BlockCount> BlockNames{{
		{BlockData::BlockType::grass, "Grass"},
		{BlockData::BlockType::dirt, "Dirt"},
		{BlockData::BlockType::stone, "Stone"},
		{BlockData::BlockType::cobblestone, "Cobblestone"},
		{BlockData::BlockType::sand, "Sand"},
		{BlockData::BlockType::glass, "Glass"},
		{BlockData::BlockType::oak_wood, "Oak Wood"},
		{BlockData::BlockType::oak_leaves, "Oak Leaves"},
		{BlockData::BlockType::bedrock, "Bedrock"},
		{BlockData::BlockType::planks, "Wooden Planks"},
		{BlockData::BlockType::water, "Water"},
		{BlockData::BlockType::lava, "Lava"},
		{BlockData::BlockType::iron, "Block of Iron"},
		{BlockData::BlockType::diamond, "Block of Diamond"},
		{BlockData::BlockType::gold, "Block of Gold"},
		{BlockData::BlockType::obsidian, "Obsidian"},
		{BlockData::BlockType::sponge, "Sponge"},
	}};

	static consteval NameArray getBlockNames()
	{
		NameArray names{};
		for (int32_t i = 0; i < BlockCount; ++i)
		{
			names[i] = BlockNames[i].second;
		}

		return names;
	}

	static int32_t blockTypeToIndex(BlockData::BlockType type);
	static const char *blockTypeToName(BlockData::BlockType type);
};
