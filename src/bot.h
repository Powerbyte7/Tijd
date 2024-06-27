#ifndef CHARACTER_H
#define CHARACTER_H

#include "task.h"

typedef struct character
{
    uint8_t health;
    uint8_t hunger;
    uint24_t posX;
    uint24_t posY;
    char *name;
    uint8_t taskCount;
    uint8_t currentTaskIndex;
    struct task tasks[16];
} character_t;

character_t *getCharacter(uint8_t index);

#endif