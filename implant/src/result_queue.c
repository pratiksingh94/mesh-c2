#include "result_queue.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define INITIAL_CAPACITY 10



void rq_init(ResultQueue *q) {
    q->item = malloc(sizeof(Result) * INITIAL_CAPACITY);
    q->len = 0;
    q->cap = INITIAL_CAPACITY;
}


void rq_free(ResultQueue *q) {
    for(size_t i = 0; i < q->len; i++) {
        free(q->item[i].output);
    }

    free(q->item);
    q->item = NULL;
    q->len = q->cap = 0;
}


int rq_find(ResultQueue *queue, int cmd_id) {
    for (size_t i = 0; i < queue->len; i++) {
        if (queue->item[i].cmd_id == cmd_id)
            return (int)i;
    }
    return -1;
}


void rq_add(ResultQueue *q, int cmd_id, const char *output) {
    if (q->len == q->cap) {
        size_t new_cap = q->cap ? q->cap * 2 : 4;
        Result *tmp = realloc(q->item, sizeof(*tmp) * new_cap);
        if (!tmp) {
            fprintf(stderr, "❌ - out of memory error expanding ResultQueue to %zu entries open an issue if you this lol\n", new_cap);
            return;
        }
        q->item = tmp;
        q->cap  = new_cap;
    }

    
    q->item[q->len].cmd_id     = cmd_id;
    q->item[q->len].output     = strdup(output);
    q->len++;
}




