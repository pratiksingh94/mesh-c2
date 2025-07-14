#pragma once
#include "task-log.h"
#include <stddef.h>

typedef struct {
    int id;
    char *cmd;
} Task;

typedef struct {
    Task *tasks;
    size_t len;
    size_t cap;
} TaskQueue;

// Initialize a task queue, not gonna write too much kind of self explanatory
void tq_init(TaskQueue *queue);

// Free a task queue, free all tasks 
void tq_free(TaskQueue *queue);

// Add a task to the queue, duh
void tq_add(TaskQueue *queue, TaskLog *log, int id, const char *cmd);

// you know what this is
int tq_find(TaskQueue *queue, int id);

// Pop a task from the queue, First-in-First-Out (FIFO) order
int tq_pop(TaskQueue *queue, Task *out);