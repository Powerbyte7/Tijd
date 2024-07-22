#include <stdint.h>
#include <assert.h>
#include "world.h"
#include "blocks.h"
#include "save.h"
#include "tice.h"
#include "fileioc.h"

// Get blocks from unloaded chunks to wipe cache
static void wipeCache() {
    for (uint24_t x = 20; x < 800; x++) {
        x += 20;
        uint8_t block = getBlock(x, 1000);
        assert((block == AIR) && "wipeCache: Expecting air");
    }
}

int worldTest() {
    // Create a save file, overwrite if necessary
    uint8_t error_code = defaultSave("ORBIS");
    assert((error_code != 0) && "worldTest: World save error");

    error_code = loadSave("ORBIS");
    assert((error_code != 0) && "worldTest: World load error");

    // Starting position
    const uint24_t posX = 8000000;
    const uint24_t posY = 1000;

    // Inital block should be 0 (AIR)
    block_t block = getBlock(posX, posY);
    assert((block == AIR) && "worldTest: Expecting air");

    error_code = placeBlock(posX, posY, STONE);
    assert((error_code == SUCCESS) && "worldTest: Cannot place block");

    block = getBlock(posX, posY);
    assert((block == STONE) && "worldTest: Expecting stone");

    wipeCache();
    block = getBlock(posX, posY);
    assert((block == STONE) && "worldTest: Expecting stone after uncaching");

    // Assert again after reloading world
    error_code = writeSave();
    assert((error_code == 0) && "worldTest: World save append error");

    error_code = loadSave("ORBIS");
    assert((error_code != 0) && "worldTest: World save reload error");

    block = getBlock(posX, posY);
    assert((block == STONE) && "worldTest: Expecting stone");

    // Assert again after reloading world with cache wipe
    wipeCache();
    error_code = writeSave();
    assert((error_code == 0) && "worldTest: World save append error");

    error_code = loadSave("ORBIS");
    assert((error_code != 0) && "worldTest: World save reload error");

    block = getBlock(posX, posY);
    assert((block == STONE) && "worldTest: Expecting stone");

    error_code = writeSave();

    // Wipe cache at and to avoid cached chunks to be loaded in second test
    // (╯°□°)╯︵ ┻━┻
    wipeCache();
}