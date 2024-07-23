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
            // Chunk found, return its index in save
            return s_saveHeader.chunkLookup[index].index;
        }
        index = (index + 1) % HASH_SIZE;
    } while (index != startIndex && s_saveHeader.chunkLookup[index].chunkID != 0);

    // Chunk not found
    return INVALID_CHUNK;
}

save_error_t registerChunkSave(uint24_t chunkID)
{
    assert((chunkID != 0) && "registerChunkSave: Expecting ChunkID to be non-zero.");

    if (getChunkSaveIndex(chunkID) != INVALID_CHUNK)
    {
        return LOOKUP_ERROR;
    }

    uint24_t index = chunkID % HASH_SIZE;
    uint24_t startIndex = index;

    do
    {
        if (s_saveHeader.chunkLookup[index].chunkID == 0)
        {
            s_saveHeader.chunkLookup[index].chunkID = chunkID;
            s_saveHeader.chunkLookup[index].index = s_saveHeader.chunkCount;

            // Resize to avoid ti_Seek reaching EOF and breaking saveChunk()
            ti_Resize(ti_GetSize(s_activeSaveHandle) + sizeof(chunk_t), s_activeSaveHandle);
            s_saveHeader.chunkCount += 1;

#ifdef DEBUG
            index = getChunkSaveIndex(chunkID);
            assert((index == (s_saveHeader.chunkCount - 1)) && "ChunkID Cannot be found after append");
#endif

            return OK;
        }
        index = (index + 1) % HASH_SIZE;
    } while (index != startIndex);

    return LOOKUP_ERROR;
}

save_error_t loadSave(const char *filename)
{
    /* This handle will be used whilst the world is open */
    s_activeSaveHandle = ti_Open(filename, "r+");
    if (s_activeSaveHandle == 0)
    {
        return 0;
    }

    /* Read the world's save header. This specifies the data layout. */
    uint8_t count = ti_Read(&s_saveHeader, sizeof(s_saveHeader), 1, s_activeSaveHandle);
    assert((count == 1) && "loadSave: Count should be 1");

    /* Bots */
    for (uint8_t i = 0; i < s_saveHeader.playerCount; i++)
    {
        character_t *character = getCharacter(i);
        ti_Read(&character, sizeof(character_t), 1, s_activeSaveHandle);
    }

    /* Chunks are stored as last region of save file, the offset is important */
    s_saveChunkOffset = ti_Tell(s_activeSaveHandle);

    /* In case I ever wat to load any other data */
    // Skip over reading chunks, they are loaded dynamically
    // uint24_t offset = sizeof(chunk_t) * s_saveHeader.chunkCount;
    // ti_Seek(offset, SEEK_CUR, s_activeSaveHandle);

    // for (uint8_t i = 0; i < s_saveHeader.dataCount; i++) {
    //     data_t *data = getData(i);
    //     ti_Read(&data, sizeof(data_t), 1, s_activeSaveHandle);
    // }

    return 1;
}

save_error_t loadChunk(chunk_t *destination, uint24_t chunkID)
{
    assert((chunkID != 0) && "readChunk: Expecting ChunkID to be non-zero.");

    uint24_t index = getChunkSaveIndex(chunkID);

    // Chunk not found
    if (index == INVALID_CHUNK)
    {
        return LOOKUP_ERROR;
    }

    uint24_t saveOffset = (index * sizeof(*destination)) + s_saveChunkOffset;
    ti_Seek(saveOffset, SEEK_SET, s_activeSaveHandle);
    uint8_t count = ti_Read(destination, sizeof(*destination), 1, s_activeSaveHandle);
    dbg_printf("Loading chunkID %d, index %d, offset %d\n", chunkID, index, ti_Tell(s_activeSaveHandle));
    assert((count == 1) && "Unable to read chunk from save");
    assert((chunkID == destination->chunkID) && "loadChunk: ChunkID mismatch");

    if (count != 1)
    {
        return READ_ERROR;
    }

    return OK;
}

save_error_t saveChunk(chunk_t *chunk)
{
    assert((chunk->chunkID != 0) && "readChunk: Expecting ChunkID to be non-zero.");

    uint24_t index = getChunkSaveIndex(chunk->chunkID);

    // Chunk not found
    if (index == INVALID_CHUNK)
    {
        return LOOKUP_ERROR;
    }

    uint24_t saveOffset = (index * sizeof(*chunk)) + s_saveChunkOffset;
    assert((saveOffset < ti_GetSize(s_activeSaveHandle)) && "saveOffset out of bounds");

    ti_Seek(saveOffset, SEEK_SET, s_activeSaveHandle);

    uint8_t writeCount = ti_Write(chunk, sizeof(*chunk), 1, s_activeSaveHandle);

    assert((writeCount == 1) && "Unable to write updated chunk to save");

    if (writeCount != 1)
    {
        dbg_printf("Failed chunk %d with index %d, offset %d, size %d\n", chunk->chunkID, index, ti_Tell(s_activeSaveHandle), ti_GetSize(s_activeSaveHandle));
        return WRITE_ERROR;
    }

    dbg_printf("Written chunk %d with index %d, offset %d, size %d\n", chunk->chunkID, index, ti_Tell(s_activeSaveHandle), ti_GetSize(s_activeSaveHandle));
    
    chunk_t test;
    ti_Seek(saveOffset, SEEK_SET, s_activeSaveHandle);
    uint8_t readcount = ti_Read(&test, sizeof(test), 1, s_activeSaveHandle);
    assert((readcount == 1) && "Unable to load updated chunk from save");
    assert((test.chunkID == chunk->chunkID) && "Writing/reading should match");

    return OK;
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

save_error_t writeSave()
{
    // Write cache to savefile
    uint8_t err = updateAllChunks();
    if (err != 0)
    {
        return 1;
    }

#ifdef DEBUG
    const uint8_t actualSaveCount = (ti_GetSize(s_activeSaveHandle) - s_saveChunkOffset) / sizeof(chunk_t);
    assert((s_saveHeader.chunkCount == actualSaveCount) && "Number of chunks should match savecount");
#endif

    ti_Rewind(s_activeSaveHandle);

    err = ti_Write(&s_saveHeader, sizeof(s_saveHeader), 1, s_activeSaveHandle);
    if (err != 1)
    {
        return WRITE_ERROR;
    }

    err = ti_Close(s_activeSaveHandle);
    assert((err != 0) && "Failed to close save");
    if (err == 0)
    {
        return CLOSE_ERROR;
    }

    s_activeSaveHandle = 0;

    return OK;
}

save_error_t writeDefaultSave(const char *filename)
{
    const uint8_t existingSave = ti_Open(filename, "r");
    if (existingSave)
    {
        return ti_Close(existingSave);
    }

    const uint8_t saveHandle = ti_Open(filename, "w");
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