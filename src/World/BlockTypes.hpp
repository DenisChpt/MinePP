#pragma once

#include "../Common.hpp"

// Block data structure
struct BlockData
{
    enum class BlockClass
    {
        air,
        solid,
        semiTransparent,
        transparent
    };

    enum class BlockType
    {
        bedrock,
        planks,
        grass,
        dirt,
        sand,
        stone,
        cobblestone,
        glass,
        oak_wood,
        oak_leaves,
        water,
        lava,
        iron,
        diamond,
        gold,
        obsidian,
        sponge,
        air
    };

    glm::vec4 getColor() const
    {
        switch (type)
        {
        case BlockType::bedrock:
            return glm::vec4(0.341, 0.341, 0.341, 1);
        case BlockType::planks:
            return glm::vec4(0.706, 0.565, 0.353, 1);
        case BlockType::grass:
            return glm::vec4(0.376, 0.627, 0.212, 1);
        case BlockType::dirt:
            return glm::vec4(0.588, 0.424, 0.29, 1);
        case BlockType::sand:
            return glm::vec4(0.82, 0.792, 0.576, 1);
        case BlockType::stone:
            return glm::vec4(0.498, 0.498, 0.498, 1);
        case BlockType::cobblestone:
            return glm::vec4(0.427, 0.427, 0.427, 1);
        case BlockType::glass:
            return glm::vec4(0.996, 0.996, 0.996, 1);
        case BlockType::oak_wood:
            return glm::vec4(0.416, 0.333, 0.204, 1);
        case BlockType::oak_leaves:
            return glm::vec4(0.114, 0.506, 0.114, 1);
        case BlockType::water:
            return glm::vec4(0.216, 0.325, 0.655, 1);
        case BlockType::lava:
            return glm::vec4(0.988, 0.631, 0., 1);
        case BlockType::iron:
            return glm::vec4(0.914, 0.914, 0.914, 1);
        case BlockType::diamond:
            return glm::vec4(0.412, 0.875, 0.855, 1);
        case BlockType::gold:
            return glm::vec4(0.996, 0.984, 0.365, 1);
        case BlockType::obsidian:
            return glm::vec4(0.035, 0.035, 0.055, 1);
        case BlockType::sponge:
            return glm::vec4(0.898, 0.898, 0.306, 1);
        default:
            return glm::vec4(1, 1, 1, 1);
        }
    }

    BlockType type;
    BlockClass blockClass;

    static BlockClass typeToClass(BlockType type)
    {
        if (type == BlockType::air)
        {
            return BlockClass::air;
        }
        else if (type == BlockType::water)
        {
            return BlockClass::semiTransparent;
        }
        else if (type == BlockType::oak_leaves || type == BlockType::glass)
        {
            return BlockClass::transparent;
        }

        return BlockClass::solid;
    }
    
    BlockData(BlockData::BlockType type = BlockType::air) : type(type), blockClass(typeToClass(type)) {}
};

// Block name utilities
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

// Inline implementations
inline const char *BlockName::blockTypeToName(BlockData::BlockType type)
{
    int32_t index = blockTypeToIndex(type);
    if (index >= 0)
    {
        return BlockNames[index].second;
    }

    return "unknown";
}

inline int32_t BlockName::blockTypeToIndex(BlockData::BlockType type)
{
    for (int32_t i = 0; i < BlockNames.size(); ++i)
    {
        if (type == BlockNames[i].first)
        {
            return i;
        }
    }

    return -1;
}