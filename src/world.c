#include <stdint.h>
#include <string.h>
#include <fileioc.h>
#include "blocks.h"
#include "world.h"
#include "save.h"
#include <graphx.h>

#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <debug.h>

// Flat world generation
static void generateChunk(chunk_t *chunk, uint24_t chunkID, uint24_t seed)
{
    (void)seed;
    chunk->chunkID = chunkID;

    const uint8_t blockdataLength = sizeof(chunk->blocks) / sizeof(chunk->blocks[0]);
    uint8_t chunk_y = (chunkID & 0xFF);
    if (chunk_y > 129)
    {
        memset(&chunk->blocks[0], STONE, blockdataLength);
    }
    else if (chunk_y == 129)
    {
        for (uint8_t i = 0; i < CHUNK_AREA; i += CHUNK_SIZE)
        {
            chunk->blocks[i] = GRASS;
            memset(&chunk->blocks[i + 1], DIRT, CHUNK_SIZE - 1);
        }
    }
    else
    {
        memset(&chunk->blocks[0], AIR, blockdataLength);
    }
}

static chunk_t s_chunkCache[CACHE_SIZE];
static uint8_t s_cacheCounter = 0;

static chunk_t *getChunk(uint24_t chunkID)
{
    assert((chunkID != 0) && "getChunk: Expecting ChunkID to be non-zero.");

    /* Check cache buffer for matching chunk */
    for (uint8_t i = 0; i < CACHE_SIZE; i++)
    {
        chunk_t *chunk = &s_chunkCache[i];
        if (chunk->chunkID == chunkID)
        {
            return chunk;
        }
    }

    /* Make space in cache for readChunk/generateChunk by writing last saved chunk to save */
    s_cacheCounter = (s_cacheCounter + 1) % CACHE_SIZE;
    if (s_chunkCache[s_cacheCounter].chunkID != 0)
    {
        saveChunk(&s_chunkCache[s_cacheCounter]);
    }

    /* Attempt read from save file, this is only for player-modified chunks */
    uint8_t err = loadChunk(&s_chunkCache[s_cacheCounter], chunkID);
    if (!err)
    {
        // This fails occassionally :(
        assert((chunkID == s_chunkCache[s_cacheCounter].chunkID) && "getChunk: ChunkID mismatch");
    }

    /* Chunk not found, generate it */
    if (err)
    {
        generateChunk(&s_chunkCache[s_cacheCounter], chunkID, OVERWORLD);
    }

    return &s_chunkCache[s_cacheCounter];
}

block_t getBlock(uint24_t pos_x, uint24_t pos_y)
{
    /* Get chunk position from most significant bits */
    const uint8_t chunk_x = pos_x >> 3;
    const uint8_t chunk_y = pos_y >> 3;

    chunk_t *chunk = getChunk(CHUNK_ID(OVERWORLD, chunk_x, chunk_y));

    assert((chunk != 0) && "getBlock: Chunk not found.");

    /* Get block position from least significant bits */
    const uint8_t chunk_block_x = pos_x & 0x7;
    const uint8_t chunk_block_y = pos_y & 0x7;

    return chunk->blocks[chunk_block_x * 8 + chunk_block_y];
}

world_error_t placeBlock(uint24_t pos_x, uint24_t pos_y, block_t block)
{
    /* Get chunk position from most significant bits */
    uint8_t chunk_x = pos_x >> 3;
    uint8_t chunk_y = pos_y >> 3;

    uint24_t chunk_id = CHUNK_ID(OVERWORLD, chunk_x, chunk_y);
    chunk_t *chunk = getChunk(chunk_id);

    assert((chunk != 0) && "getBlock: Chunk not found.");

    /* Get block position from least significant bits */
    uint8_t chunk_block_x = pos_x & 0x7;
    uint8_t chunk_block_y = pos_y & 0x7;

    block_t old_block = chunk->blocks[chunk_block_x * 8 + chunk_block_y];

    if (OCCUPIED(old_block))
    {
        return BLOCK_OCCUPIED;
    }

    chunk->blocks[chunk_block_x * 8 + chunk_block_y] = block;

    /* Registers chunk and makes allocation for saving, it will be saved after leaving the cache */
    registerChunkSave(chunk_id);

    /* Replace all air blocks bellow with a background block */
    while (1)
    {
        chunk_block_y += 1;
        if (chunk_block_y == 8)
        {
            chunk_y += 1;
            chunk_block_y = 0;
            chunk_id = CHUNK_ID(OVERWORLD, chunk_x, chunk_y);
            chunk = getChunk(chunk_id);
            assert((chunk) && "getBlock: Chunk not found.");
        }

        block = chunk->blocks[chunk_block_x * 8 + chunk_block_y];

        if (block == AIR)
        {
            chunk->blocks[chunk_block_x * 8 + chunk_block_y] = DIRT;
            registerChunkSave(chunk_id);
        }
        else
        {
            break;
        }
    }

    return 0;
}

world_error_t breakBlock(uint24_t pos_x, uint24_t pos_y)
{
    /* Get chunk position from most significant bits */
    uint8_t chunk_x = pos_x >> 3;
    uint8_t chunk_y = pos_y >> 3;

    uint24_t chunk_id = CHUNK_ID(OVERWORLD, chunk_x, chunk_y);
    chunk_t *chunk = getChunk(chunk_id);

    if (!chunk)
    {
        return LOAD_ERROR;
    }

    /* Get block position from least significant bits */
    uint8_t chunk_block_x = pos_x & 0x7;
    uint8_t chunk_block_y = pos_y & 0x7;

    chunk->blocks[chunk_block_x * 8 + chunk_block_y] = AIR;

    /* Make space in save file for chunk, it will be saved after leaving the cache */
    registerChunkSave(chunk_id);

    return 0;
}

void printCacheDebug()
{
    gfx_SetTextXY(0, 64);
    gfx_PrintUInt(s_cacheCounter, 4);
}

uint8_t updateAllChunks()
{
    for (uint8_t i = 0; i < CACHE_SIZE; i++)
    {
        if (s_chunkCache[i].chunkID != 0)
        {
            saveChunk(&s_chunkCache[i]);
        }
    }
    return 0;
}