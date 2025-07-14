#include "task_queue.h"
#include "task-log.h"
#include "result_queue.h"
#include "peer-list.h"


typedef struct {
    TaskQueue *tq;
    ResultQueue *rq;
    TaskLog *tl;
} GossipContext;


// Syncs by "Gossiping" with other implants, for only peers are synced
// Returns 0 if no one to sync to, 1 if sync complete, -1 if failed
void gossip_sync(GossipContext *ctx);

void sync_job(void *c);