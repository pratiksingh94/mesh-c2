#include "task_queue.h"
#include "task-log.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define INITIAL_CAPACITY 10



void tq_init(TaskQueue *q) {
    q->tasks = malloc(sizeof(Task) * INITIAL_CAPACITY);
    q->len = 0;
    q->cap = INITIAL_CAPACITY;
}

// this looks clean asf
void tq_free(TaskQueue *q) {
    for(size_t i = 0; i < q->len; i++) {
        free(q->tasks[i].cmd);
    }

    free(q->tasks);
    q->tasks = NULL;
    q->len = q->cap = 0;
}



void tq_add(TaskQueue *q, TaskLog *log, int id, const char *cmd) {
    if(tl_find(log, id) >= 0) return;

    if(q->len == q->cap) {
        q->cap *= 2;
        Task *tmp = realloc(q->tasks, sizeof(Task) * q->cap);
        if(!tmp) {
            fprintf(stderr, "FATAL: FAILED TO REALLOCATED TASK QUEUE\n");
            exit(1);
        }
        q->tasks = tmp;
    }

    q->tasks[q->len].id = id;
    q->tasks[q->len].cmd = strdup(cmd);
    q->len++;

    tl_add(log, id, cmd);

    // printf("📥 - Received task: %d, %s\n", id, cmd);
}

int tq_find(TaskQueue *queue, int id) {
    for (size_t i = 0; i < queue->len; i++) {
        if (queue->tasks[i].id == id)
            return (int)i;
    }
    return -1;
}


int tq_pop(TaskQueue *q, Task *out) {
    if(q->len == 0) return 0;

    *out = q->tasks[0];
    memmove(&q->tasks[0], &q->tasks[1], sizeof(Task) * (q->len - 1));

    q->len--;
    return 1;
}

