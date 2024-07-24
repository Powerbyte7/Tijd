#include <stdint.h>
#include <blocks.h>
#include <graphx.h>
#include <ti/getcsc.h>
#include "world.h"
#include "gfx/gfx.h"
#include "save.h"

// TEMP

#include <assert.h>
#include <debug.h>
#include <ti/screen.h>

#ifdef TEST
#include "test.h"
#endif

// Tasks are peformed by a character

// uint8_t place_block(uint24_t x, uint24_t y, uint8_t block) {
//     // Check if area is placeable
//     uint8_t current_block = getBlock(x,y);
//     if (VACANT(current_block)) {
//         return 1;
//     }

//     // Place background blocks
//     while (1) {
//         y += 1;
//         uint8_t current_block = getBlock(x,y);

//         if (OCCUPIED(current_block)) {
//             break;
//         }
//     }
//     return 0;
// }

// uint8_t reachable(uint24_t x, uint24_t y) {

// }

// uint8_t interact(uint24_t x, uint24_t y) {
//     // 1. Move
//     // 2. Do
// }

gfx_UninitedSprite(scaper_head_left, scaper_head_right_width, scaper_head_right_height);

void render(uint24_t posX, uint24_t posY)
{
    block_t block;
    for (uint8_t renderX = 0; renderX < 17; renderX++)
    {
        for (uint8_t renderY = 0; renderY < 15; renderY++)
        {

            block = getBlock(posX + renderX, posY + renderY);

#ifdef DEBUG
            gfx_SetColor(block);
            gfx_FillRectangle_NoClip(renderX * 16, renderY * 16, 16, 16);

#else

            switch (block)
            {
            case DIRT:
                gfx_Sprite_NoClip(dirt, renderX * 16, renderY * 16);
                break;
            case GRASS:
                gfx_Sprite_NoClip(grass_block_side, renderX * 16, renderY * 16);
                break;
            case STONE:
                gfx_Sprite_NoClip(stone, renderX * 16, renderY * 16);
                break;
            default: // Sky
                gfx_SetColor(2);
                gfx_FillRectangle_NoClip(renderX * 16, renderY * 16, 16, 16);
                break;
            }
#endif
        }
    }
    gfx_SetColor(0);
    gfx_Rectangle_NoClip(16 * 8, 16 * 7, 16, 16);

    printSaveDebug();
    printCacheDebug();

    // if (posX == 8000000) {
    //     gfx_Sprite(scaper_head_front, 16*8 + 4,16*7+3);
    // } else if (posY > 8000000) {
    //     gfx_Sprite(scaper_head_right, 16*8 + 4,16*7+3);
    // } else {
    //     gfx_Sprite(scaper_head_right, 16*8 + 4,16*7+3);
    //     // gfx_Sprite(gfx_FlipSpriteX(scaper_head_left, scaper_head_right), 16*8 + 4,16*7+3);
    // }
}

int main()
{
    os_ClrHome();

    const char saveName[] = "TEST";

    uint8_t err = 0;
    err = writeDefaultSave(saveName);
    if (err)
    {
        os_PutStrFull("Save Write Fail");
        while (os_GetCSC() != sk_Clear)
        {
        }
        return 1;
    }

    err = !loadSave(saveName);
    if (err)
    {
        os_PutStrFull("Save Load Fail");
        while (os_GetCSC() != sk_Clear)
        {
        }
        return 1;
    }

    /* Initialize graphics drawing */
    gfx_Begin();
    gfx_SetDrawBuffer();

#ifndef DEBUG
    gfx_SetPalette(global_palette, sizeof_global_palette, 0);
#endif

    gfx_FillScreen(2); // Sky

    static uint24_t x = 8000000;
    static uint24_t y = 1024;
    static uint8_t key;

    do
    {
        render(x, y);
        gfx_BlitBuffer();

        key = os_GetCSC();

        switch (key)
        {
        case sk_Up:
            y -= 1;
            break;
        case sk_Down:
            y += 1;
            break;
        case sk_Right:
            x += 1;
            break;
        case sk_Left:
            x -= 1;
            break;
        case sk_0:
            err = breakBlock(x + 8, y + 7);
            break;
        case sk_1:
            err = placeBlock(x + 8, y + 7, DIRT);
            break;
        case sk_2:
            err = placeBlock(x + 8, y + 7, GRASS);
            break;
        case sk_3:
            err = placeBlock(x + 8, y + 7, STONE);
            break;
        case sk_Clear:
            break;
        }

        if (err)
        {
            break;
        }
    } while (key != sk_Clear);

    gfx_End();

    writeSave();

#ifdef TEST
    dbg_printf("Starting test!\n");
    for (uint8_t i = 0; i < 20; i++)
    {
        dbg_printf("Test %d\n", i);
        worldTest();
    }
#endif

    if (err)
    {
        os_PutStrFull("Program Error");
        while (os_GetCSC() != sk_Clear)
        {
        }
    }
}