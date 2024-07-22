#ifndef WORLD_H
#define WORLD_H

#include "blocks.h"

#define CHUNK_ID(metadata, x, y) (((metadata << 16) | (x << 8)) | y)
#define HASH_SIZE 256
#define CACHE_SIZE 20
#define CHUNK_UNLOADED HASH_SIZE
#define OVERWORLD 1
#define URA 2
#define VOID 3
#define INVALID_CHUNK 0x100000
#define INVALID_POS 0x200000
#define CHUNK_SIZE 8
#define CHUNK_AREA (CHUNK_SIZE * CHUNK_SIZE)

typedef struct chunk
{
    uint24_t chunkID; // metadata, x, and y
    block_t blocks[CHUNK_AREA];
} chunk_t;

typedef struct chunk_entry
{
    uint24_t chunkID; // metadata, x, and y
    uint8_t offset;
} chunk_entry_t;

typedef struct world
{
    uint24_t seed;
    uint8_t type;
    char name[32];
} world_t;

typedef enum
{
    SUCCESS = 0,
    LOAD_ERROR,
    BLOCK_OCCUPIED
} world_error_t;

block_t getBlock(uint24_t pox_x, uint24_t pox_y);
world_error_t placeBlock(uint24_t pox_x, uint24_t pox_y, block_t block);
world_error_t breakBlock(uint24_t pox_x, uint24_t pox_y);



uint8_t saveWorld();
void printCacheDebug();
uint8_t updateAllChunks();


#endif