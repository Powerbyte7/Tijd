#include <fileioc.h>
#include <ti/screen.h>
#include "world.h"
#include "bot.h"
#include "task.h"
#include "string.h"
#include "save.h"
#include <graphx.h>

#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <debug.h>

// Internal state
static saveheader_t s_saveHeader;
static uint8_t s_activeSaveHandle;
static uint24_t s_saveChunkOffset;

static uint24_t getChunkSaveIndex(uint24_t chunkID)
{
    assert((chunkID != 0) && "getChunkSaveIndex: Expecting ChunkID to be non-zero.");

    if (chunkID == 0)
    {
        return INVALID_CHUNK;
    }

    uint24_t index = chunkID % HASH_SIZE;
    uint24_t startIndex = index;

    do
    {
        if (s_saveHeader.chunkLookup[index].chunkID == chunkID)
        {
            // Chunk found, return its file location
            return s_saveHeader.chunkLookup[index].offset;
        }
        index = (index + 1) % HASH_SIZE;
    } while (index != startIndex && s_saveHeader.chunkLookup[index].chunkID != 0);

    // Chunk not found
    return INVALID_CHUNK;
}

uint8_t appendChunkLookup(uint24_t chunkID)
{
    assert((chunkID != 0) && "appendChunkLookup: Expecting ChunkID to be non-zero.");

    if (getChunkSaveIndex(chunkID) != INVALID_CHUNK)
    {
        return 0;
    }

    uint24_t index = chunkID % HASH_SIZE;
    uint24_t startIndex = index;

    do
    {
        if (s_saveHeader.chunkLookup[index].chunkID == 0)
        {
            s_saveHeader.chunkLookup[index].chunkID = chunkID;
            s_saveHeader.chunkLookup[index].offset = s_saveHeader.chunkCount;
            s_saveHeader.chunkCount += 1;

            return 1;
        }
        index = (index + 1) % HASH_SIZE;
    } while (index != startIndex);

    return 0;
}

uint8_t loadSave(const char *filename)
{
    s_activeSaveHandle = ti_Open(filename, "r+");

    // Save failed to load
    if (s_activeSaveHandle == 0)
    {
        return 0;
    }

    // Read world save header to determine save data layout
    uint8_t count = ti_Read(&s_saveHeader, sizeof(s_saveHeader), 1, s_activeSaveHandle);
    assert((count == 1) && "loadSave: Count should be 1");

    // Mutably load characters
    for (uint8_t i = 0; i < s_saveHeader.playerCount; i++)
    {
        character_t *character = getCharacter(i);
        ti_Read(&character, sizeof(character_t), 1, s_activeSaveHandle);
    }

    // Set offset to chunk portion of save file for later reading
    s_saveChunkOffset = ti_Tell(s_activeSaveHandle);

    // TODO: In case I ever wat to load any other data:
    // Skip over reading chunks, they are loaded dynamically
    // uint24_t offset = sizeof(chunk_t) * s_saveHeader.chunkCount;
    // ti_Seek(offset, SEEK_CUR, s_activeSaveHandle);

    // for (uint8_t i = 0; i < s_saveHeader.dataCount; i++) {
    //     data_t *data = getData(i);
    //     ti_Read(&data, sizeof(data_t), 1, s_activeSaveHandle);
    // }

    return 1;
}

uint8_t readChunk(chunk_t *destination, uint24_t chunkID)
{
    assert((chunkID != 0) && "readChunk: Expecting ChunkID to be non-zero.");

    uint24_t index = getChunkSaveIndex(chunkID);

    // Chunk not found
    if (index == INVALID_CHUNK)
    {
        return 1;
    }

    uint24_t saveOffset = (index * sizeof(*destination)) + s_saveChunkOffset;
    ti_Seek(saveOffset, SEEK_SET, s_activeSaveHandle);
    ti_Read(destination, sizeof(*destination), 1, s_activeSaveHandle);

    return 0;
}

uint8_t updateChunk(chunk_t *chunk)
{
    assert((chunk->chunkID != 0) && "readChunk: Expecting ChunkID to be non-zero.");

    uint24_t index = getChunkSaveIndex(chunk->chunkID);

    // Chunk not found
    if (index == INVALID_CHUNK)
    {
        return 0;
    }

    uint24_t saveOffset = (index * sizeof(*chunk)) + s_saveChunkOffset;
    ti_Seek(saveOffset, SEEK_SET, s_activeSaveHandle);
    uint8_t writeCount = ti_Write(chunk, sizeof(*chunk), 1, s_activeSaveHandle);

    return writeCount == 1;
}

uint8_t appendChunk(chunk_t *chunk)
{
    uint24_t appendOffset = (s_saveHeader.chunkCount * sizeof(*chunk)) + s_saveChunkOffset;

    ti_Seek(appendOffset, SEEK_SET, s_activeSaveHandle);
    uint8_t writeCount = ti_Write(chunk, sizeof(*chunk), 1, s_activeSaveHandle);

    // Uses chunkCount as index
    appendChunkLookup(chunk->chunkID);

    return writeCount != 1;
}

void printSave()
{
    os_PutStrLine(s_saveHeader.world.name);
}

void printSaveDebug()
{
    gfx_PrintStringXY(s_saveHeader.world.name, 0, 0);

    // Debug
    gfx_SetTextXY(0, 16);
    gfx_PrintUInt(s_saveHeader.chunkCount, 3);
    gfx_PrintString("/255");

    gfx_SetTextXY(0, 32);
    gfx_PrintUInt(s_saveChunkOffset, 4);

    gfx_SetTextXY(0, 48);
    gfx_PrintUInt(sizeof(s_saveHeader), 4);
}

uint8_t writeSave()
{
    uint8_t err = updateAllChunks();
    if (err != 0)
    {
        return 1;
    }

    err = ti_Rewind(s_activeSaveHandle);

    err = ti_Write(&s_saveHeader, sizeof(s_saveHeader), 1, s_activeSaveHandle);
    if (err != 1)
    {
        return 1;
    }

    err = ti_Close(s_activeSaveHandle);
    assert((err != 0) && "Failed to close save");
    if (err == 0)
    {
        return 1;
    }

    s_activeSaveHandle = 0;

    return 0;
}

uint8_t defaultSave(const char *filename)
{   
    const uint8_t saveHandle = ti_Open(filename, "w");

    // Save failed to load
    if (!saveHandle)
    {
        return 0;
    }

    struct saveHeader orbisSave = {
        .world = {
            .seed = 1,
            .type = 1,
            .name = "Orbis",
        },
        .playerCount = 0,
        .chunkCount = 0,
    };

    uint8_t error = !ti_Write(&orbisSave, sizeof(orbisSave), 1, saveHandle);

    if (error)
    {
        return 0;
    }

    return ti_Close(saveHandle);
}