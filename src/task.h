#ifndef TASK_H
#define TASK_H

typedef struct task {
    uint8_t type; // Craft, break, move
    uint24_t time;
    uint24_t posX;
    uint24_t posY;
} task_t;

#endif