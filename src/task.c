#include <stdint.h>
#include "task.h"

static task_t s_tasks[255];
static uint8_t taskCount;
static uint8_t currentTaskIndex;

void clearTasks()
{

    taskCount = 0;
}

