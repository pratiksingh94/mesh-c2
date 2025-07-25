#pragma once
#include <stddef.h>

typedef struct {
    int cmd_id;
    char *output;
} Result;

typedef struct {
    Result *item;
    size_t len;
    size_t cap;
} ResultQueue;






// Initialize a result queue
void rq_init(ResultQueue *queue);

// Free the results
void rq_free(ResultQueue *queue);


// find using implant ip and cmd id, used in syncing to make sure no duplicates
int rq_find(ResultQueue *queue, int cmd_id);


// Add a result with all info like ip, output, etc
void rq_add(ResultQueue *queue, int cmd_id, const char *output);
