#include <stdint.h>
#include "bot.h"

static character_t s_characters[5];

character_t *getCharacter(uint8_t index)
{
    return index < sizeof(s_characters) ? &s_characters[index] : (character_t *)0;
}

// Breadth First Traversal
int moveTo(uint24_t x_from, uint24_t y_from, uint24_t x_to, uint24_t y_to)
{
}