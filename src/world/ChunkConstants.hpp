#ifndef CHUNKCONSTANTS_HPP
#define CHUNKCONSTANTS_HPP

namespace world {

// Dimensions d'un chunk et de ses sections.
constexpr int CHUNK_SIZE_X = 16;
constexpr int CHUNK_SIZE_Z = 16;
constexpr int CHUNK_HEIGHT = 256;
constexpr int CHUNK_SECTION_HEIGHT = 16;
constexpr int NUM_CHUNK_SECTIONS = CHUNK_HEIGHT / CHUNK_SECTION_HEIGHT;

} // namespace world

#endif // CHUNKCONSTANTS_HPP
