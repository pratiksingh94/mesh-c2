#pragma once
#include "task_queue.h"
#include "result_queue.h"


typedef struct {
    TaskQueue *tq;
    ResultQueue *rq;
} PayloadContext;



/**
 * Pop the next pending task (FIFO), run it, and push the result into the ResultQueue
 * Returns:
 *   1  -> a task was executed
 *   0  -> no pending tasks
 *  -1  -> you fucking mushroom
 */
int execute_payload(PayloadContext *ctx);


/**
 * A wrapper runs the main job of payload stuff
 */
void payload_job(void *c);