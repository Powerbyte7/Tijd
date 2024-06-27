#ifndef BLOCKS_H
#define BLOCKS_H

typedef enum
{
    AIR = 0,
    BG_DIRT = 100,
    BG_GRASS,
    BG_STONE,
    CRAFTING_STATION = 200,
    DIRT,
    GRASS,
    STONE
} block_t;

_Static_assert((sizeof(block_t) == sizeof(uint8_t)),
               "Block size should fit within single byte");

// Can be placed on and walked across
#define VACANT(block) (block < CRAFTING_STATION)
// Can be walked across, not placed on
#define WALKABLE(block) (block < DIRT)
// Cannot be walked across nor placed on
#define OCCUPIED(block) (block >= DIRT)

#endif