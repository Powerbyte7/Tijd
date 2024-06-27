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

static void generateChunk(chunk_t *chunk, uint24_t chunkID, uint24_t seed)
{
    (void)seed;
    chunk->chunkID = chunkID;

    // Flat world generation
    const uint8_t chunk_memsize = sizeof(chunk->blocks) / sizeof(chunk->blocks[0]);
    uint8_t chunk_y = (chunkID & 0xFF);
    if (chunk_y > 129)
    {
        memset(&chunk->blocks[0], STONE, chunk_memsize);
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
        memset(&chunk->blocks[0], AIR, chunk_memsize);
    }
}

static chunk_t s_chunkCache[CACHE_SIZE];
static uint8_t s_cacheCounter = 0;

// uint8_t chunk_x, uint8_t chunk_y
static chunk_t *getChunk(uint24_t chunkID)
{
    assert((chunkID != 0) && "getChunk: Expecting ChunkID to be non-zero.");

    // Get from cache buffer
    for (uint8_t i = 0; i < CACHE_SIZE; i++)
    {
        chunk_t *chunk = &s_chunkCache[i];
        if (chunk->chunkID == chunkID)
        {
            return chunk;
        }
    }

    // Chunk not found in cache
    // Move next/oldest cache entry to save file for room

    // Load new chunk
    s_cacheCounter = (s_cacheCounter + 1) % CACHE_SIZE;
    
    if (s_chunkCache[s_cacheCounter].chunkID != 0)
    {
        uint8_t err = updateChunk(&s_chunkCache[s_cacheCounter]);
        if (!err) {
            return 0;
        }

    }
    
    uint8_t found = !readChunk(&s_chunkCache[s_cacheCounter], chunkID);

    if (!found)
    {
        generateChunk(&s_chunkCache[s_cacheCounter], chunkID, OVERWORLD);
    }

    return &s_chunkCache[s_cacheCounter];
}

block_t getBlock(uint24_t pos_x, uint24_t pos_y)
{
    // Chunk number
    uint8_t chunk_x = pos_x >> 3;
    uint8_t chunk_y = pos_y >> 3;

    chunk_t *chunk = getChunk(CHUNK_ID(OVERWORLD, chunk_x, chunk_y));

    // Clamp numbers to 0-7 range
    uint8_t chunk_block_x = pos_x & 0x7;
    uint8_t chunk_block_y = pos_y & 0x7;

    return chunk->blocks[chunk_block_x * 8 + chunk_block_y];
}

world_error_t placeBlock(uint24_t pos_x, uint24_t pos_y, block_t block)
{
    // Chunk number
    uint8_t chunk_x = pos_x >> 3;
    uint8_t chunk_y = pos_y >> 3;

    uint24_t chunk_id = CHUNK_ID(OVERWORLD, chunk_x, chunk_y);
    chunk_t *chunk = getChunk(chunk_id);

    if (!chunk) {
        return LOAD_ERROR;
    }

    // Clamp numbers to 0-7 range
    uint8_t chunk_block_x = pos_x & 0x7;
    uint8_t chunk_block_y = pos_y & 0x7;

    block_t old_block = chunk->blocks[chunk_block_x * 8 + chunk_block_y];

    if (OCCUPIED(old_block))
    {
        return BLOCK_OCCUPIED;
    }

    chunk->blocks[chunk_block_x * 8 + chunk_block_y] = block;

    // Start tracking chunk in save file
    appendChunkLookup(chunk_id);

    // Cascade background blocks downwards
    while (1)
    {
        chunk_block_y += 1;
        if (chunk_block_y == 8)
        {
            chunk_y += 1;
            chunk_block_y = 0;
            chunk_id = CHUNK_ID(OVERWORLD, chunk_x, chunk_y);
            chunk = getChunk(chunk_id);
        }

        
        block = chunk->blocks[chunk_block_x * 8 + chunk_block_y];

        if (block == AIR)
        {
            chunk->blocks[chunk_block_x * 8 + chunk_block_y] = DIRT;
            appendChunkLookup(chunk_id);
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
    // Chunk number
    uint8_t chunk_x = pos_x >> 3;
    uint8_t chunk_y = pos_y >> 3;

    uint24_t chunk_id = CHUNK_ID(OVERWORLD, chunk_x, chunk_y);
    chunk_t *chunk = getChunk(chunk_id);

    if (!chunk) {
        return LOAD_ERROR;
    }


    // Clamp numbers to 0-7 range
    uint8_t chunk_block_x = pos_x & 0x7;
    uint8_t chunk_block_y = pos_y & 0x7;

    // uint8_t old_block = chunk->blocks[chunk_block_x*8+chunk_block_y];

    chunk->blocks[chunk_block_x * 8 + chunk_block_y] = AIR;

    // Start tracking chunk in save file
    appendChunkLookup(chunk_id);

    return 0;
}

void printCacheDebug() {
    gfx_SetTextXY(0, 64);
    gfx_PrintUInt(s_cacheCounter,4);
}

void updateAllChunks() {
    for (uint8_t i = 0; i < CACHE_SIZE; i++) {
        if (s_chunkCache[i].chunkID != 0) {
            updateChunk(&s_chunkCache[i]);
        }
    }
}