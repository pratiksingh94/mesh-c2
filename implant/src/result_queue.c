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



void rq_add(ResultQueue *q, int cmd_id, const char *implant_ip, const char *output) {
    if(q->len == q->cap) {
        q->cap *= 2;
        realloc(q->item, sizeof(Result) * q->cap);
    }

    q->item[q->len].cmd_id = cmd_id;
    q->item[q->len].implant_ip = strdup(implant_ip);
    q->item[q->len].output = strdup(output);
    q->len++;
}




