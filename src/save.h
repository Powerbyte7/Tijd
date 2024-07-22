#ifndef SAVE_H
#define SAVE_H


#define LOOKUP_SIZE 256
typedef struct saveHeader
{
    world_t world;
    uint8_t playerCount;
    uint8_t chunkCount;
    chunk_entry_t chunkLookup[LOOKUP_SIZE];
} saveheader_t;

/**
 * Loads a save file by name.
 *
 * @param[in] handle AppVar name.
 * @returns 0 on error
 */
uint8_t loadSave(const char *filename);

/**
 * Adds chunk to lookup table to track the chunk in the save file
 * Used to allocate a spot in save before
 *
 * @param[in] chunkID The chunk ID.
 * @returns 0 on error
 */
uint8_t appendChunkLookup(uint24_t chunkID);

uint8_t readChunk(chunk_t *destination, uint24_t chunkID);

uint8_t updateChunk(chunk_t *chunk);

void printSave();

void printSaveDebug();

uint8_t writeSave();

uint8_t defaultSave();

#endif