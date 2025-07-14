#include "task-log.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>

#define INITIAL_CAPACITY 10



void tl_init(TaskLog *log) {
    log->tasks = malloc(sizeof(*log->tasks) * INITIAL_CAPACITY);
    if (!log->tasks) {
        fprintf(stderr, "FATAL: Out of memory in tl_init\n");
        exit(1);
    }

    log->len = 0;
    log->cap = INITIAL_CAPACITY;
}


void tl_free(TaskLog *log) {
    for (size_t i = 0; i < log->len; i++) {
        free(log->tasks[i].cmd);
    }
    free(log->tasks);
    log->tasks = NULL;
    log->len   = log->cap = 0;
}



void tl_add(TaskLog *log, int id, const char *cmd) {
    if(tl_find(log, id) >= 0) return;

    if (log->len == log->cap) {
        size_t new_cap = log->cap * 2;
        TaskLogItem *tmp = realloc(log->tasks, sizeof(*tmp) * new_cap);
        if (!tmp) {
            fprintf(stderr, "❌ - Failed to realloc TaskLog to %zu entries\n", new_cap);
            exit(1);
        }
        log->tasks = tmp;
        log->cap = new_cap;
    }

    // tf shuld i name this
    TaskLogItem *it = &log->tasks[log->len++];
    it->id = id;
    it->added_at = time(NULL);
    it->cmd = strdup(cmd);

    // printf("📥 - Received task: %d, %s\n", id, cmd);
}

int tl_find(const TaskLog *log, int id) {
    for (size_t i = 0; i < log->len; i++) {
        if (log->tasks[i].id == id)
            return (int)i;
    }
    return -1;
}

