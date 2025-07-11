#include "mongoose.h"
#include "task_queue.h"
#include "peer-list.h"
#include "peers.h"
#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cjson/cJSON.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>


static TaskQueue *global_tq;

extern PeerList global_pl;

// simple compare for URI
static int uri_is(const struct mg_str *uri, const char *s) {
    size_t n = strlen(s);
    return uri->len == n && memcmp(uri->buf, s, n) == 0;
}

// Handle POST /receive-command
static void handle_recv_cmd(struct mg_connection *c, void *ev_data) {
    struct mg_http_message *hm = ev_data;

    char *body = strndup(hm->body.buf, hm->body.len);
    cJSON *json = cJSON_Parse(body);
    free(body);

    if (!json) {
        mg_http_reply(c, 400, "Content-Type: text/plain\r\n", "Invalid JSON\n");
        return;
    }

    // get all the body items
    cJSON *id_item  = cJSON_GetObjectItem(json, "id");
    cJSON *cmd_item = cJSON_GetObjectItem(json, "cmd");


    if (!cJSON_IsNumber(id_item) || !cJSON_IsString(cmd_item)) {
        cJSON_Delete(json);
        mg_http_reply(c, 400, "Content-Type: text/plain\r\n", "Missing id or cmd\n");
        return;
    }



    int id = id_item->valueint;
    const char *cmd = cmd_item->valuestring;

    // already there in tasks
    if(tq_find(global_tq, id) >= 0) {
        mg_http_reply(c, 200, "Content-Type: text/plain\r\n", "OK\n");
        return;
    }

    // add to the task queue
    printf("📩 Queued command %d: %s\n", id, cmd);
    tq_add(global_tq, id, cmd);



    
    // printf("Entered flood stage\n");
    // printf("⚙️  global_pl has %zu peers\n", global_pl.len);
    flood_command(&global_pl, id, cmd);
    // printf("Exit flood stage\n");

    cJSON_Delete(json);
    // hl_free(&hl);

    mg_http_reply(c, 200, "Content-Type: text/plain\r\n", "OK\n");
}

// mongoose event handler (3‑arg signature)
static void fn(struct mg_connection *c, int ev, void *ev_data) {
    if (ev == MG_EV_HTTP_MSG) {
        struct mg_http_message *hm = ev_data;
        if (uri_is(&hm->uri, "/receive-command")) {
            handle_recv_cmd(c, ev_data);
        } else {
            mg_http_reply(c, 404, "Content-Type: text/plain\r\n", "Not found\n");
        }
    }
}

void start_http_server(TaskQueue *tq) {
    global_tq = tq;
    struct mg_mgr mgr;
    mg_mgr_init(&mgr);

    // Listen on port 8080
    if (mg_http_listen(&mgr, "0.0.0.0:8080", fn, NULL) == NULL) {
        fprintf(stderr, "❌ Failed to start HTTP server\n");
        exit(1);
    }
    printf("🌐 HTTP server listening on 0.0.0.0:8080\n");

    // Poll loop
    while (1) {
        mg_mgr_poll(&mgr, 1000);
    }
}
