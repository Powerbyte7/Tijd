#ifndef SAVE_H
#define SAVE_H

/**
 * Stores a save file for debugging.
 *
 * @param[in] filename AppVar name.
 * @returns 0 on error
 */
uint8_t debugSave(const char *filename);

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
uint8_t closeSave();

void printSaveDebug();

uint8_t writeSave();

#endif