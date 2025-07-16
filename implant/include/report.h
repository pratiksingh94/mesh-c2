#include "task_queue.h"
#include "result_queue.h"

typedef struct {
    ResultQueue *rq;
} ReportContext;

void send_report(ReportContext *ctx);

void reporting_job(void *c);