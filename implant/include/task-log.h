#pragma once
// #include "task_queue.h"
#include <stddef.h>
#include <time.h>

typedef struct {
    int id;
    char *cmd;
    time_t added_at; // we dont use it right now, maybe in future idk i forgot to use it lol
} TaskLogItem;

typedef struct {
    TaskLogItem *tasks;
    size_t len;
    size_t cap;
} TaskLog;



// init the task log
void tl_init(TaskLog *log);

// free the log
void tl_free(TaskLog *log);

// add a task to log
void tl_add(TaskLog *log, int id, const char *cmd);

// find a task, returns the index
int tl_find(const TaskLog *log, int id);