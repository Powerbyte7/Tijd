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

typedef enum
{
    OK,
    WRITE_ERROR,
    LOOKUP_ERROR,
    READ_ERROR,
    OPEN_ERROR,
    CLOSE_ERROR,
    INVALID_CHUNK = 0x100000
} save_error_t;

/**
 * Loads a save file by name.
 *
 * @param[in] handle AppVar name.
 * @returns 0 on error
 */
save_error_t loadSave(const char *filename);

/**
 * Makes an allocation for the chunk in save file.
 *
 * @param[in] chunkID The chunk ID.
 * @returns 0 on error
 */
save_error_t registerChunkSave(uint24_t chunkID);

/**
 * Reads chunk with matching chunk ID from save file
 *
 * @param[in] chunkID The chunk ID.
 * @returns READ_ERROR on error
 */
save_error_t loadChunk(chunk_t *destination, uint24_t chunkID);

save_error_t saveChunk(chunk_t *chunk);

void printSave();

void printSaveDebug();

save_error_t writeSave();

save_error_t writeDefaultSave(const char *filename);

#endif