#include "config.h"
#include "sync-implants.h"
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
    cJSON *peer_array = cJSON_CreateArray();

    for(size_t i = 0; i < global_pl.len; i++) {
        if ((strcmp(global_pl.peers[i].ip, random_peer->ip) == 0) && global_pl.peers[i].port == random_peer->port) continue;

        cJSON *peer_obj = cJSON_CreateObject();
        cJSON_AddStringToObject(peer_obj, "ip", global_pl.peers[i].ip);
        cJSON_AddNumberToObject(peer_obj, "port", global_pl.peers[i].port);

        cJSON_AddItemToArray(peer_array, peer_obj);
        // cJSON_Delete(peer_obj);
    }

    cJSON_AddItemToObject(root, "peers_array", peer_array);
    cJSON_AddNumberToObject(root, "my_port", WEBSERVER_PORT);
    char *body = cJSON_PrintUnformatted(root);

    char url[128];
    snprintf(url, sizeof url, "http://%s:%d/sync", random_peer->ip, random_peer->port);

    char *resp = NULL;
    int err = http_post(url, body, &resp);

    if (err == CURLE_COULDNT_CONNECT) {
        pl_remove(&global_pl, random_peer->ip, random_peer->port);
        cJSON_Delete(root);
        free(body);
        free(resp);
        return;
    }
    if (err != 0 || resp == NULL) {
        fprintf(stderr, "HTTP POST failed with error code %d\n", err);
        cJSON_Delete(root);
        free(body);
        free(resp);
        return;
    }
   
    
    



    cJSON *resp_json = cJSON_Parse(resp);
    if (!resp_json) {
        const char *errptr = cJSON_GetErrorPtr();
        fprintf(stderr, "❌ failed to parse response JSON at: %s\n", errptr ? errptr : "(unknown or idk)");
        cJSON_Delete(root);
        free(body);
        free(resp);
        return;
    }

    // cJSON *port_item = cJSON_GetObjectItemCaseSensitive(resp_json, "my_port");
    cJSON *resp_peer_array = cJSON_GetObjectItemCaseSensitive(resp_json, "peers_array");

    // int port = port_item->valueint;
    

    // sender remove receiver's ip, and receiver will add sender's ip make sure that both party do not have their peer own ip
    cJSON *sender = cJSON_CreateObject();
    cJSON_AddStringToObject(sender, "ip", random_peer->ip);
    cJSON_AddNumberToObject(sender, "port", random_peer->port);

    cJSON_AddItemToArray(resp_peer_array, sender);
    // cJSON_Delete(sender);

    cJSON *item;
    // int new_peer = 0;
    cJSON_ArrayForEach(item, resp_peer_array) {
        cJSON *ip = cJSON_GetObjectItemCaseSensitive(item, "ip");
        cJSON *port = cJSON_GetObjectItemCaseSensitive(item, "port");

        if (!cJSON_IsString(ip) || !cJSON_IsNumber(port)) continue;

        if (pl_find(&global_pl, ip->valuestring, port->valueint) >= 0) continue;

        pl_add(&global_pl, ip->valuestring, port->valueint);
        // new_peer++;
    }

    printf("🗣️ - Sync complete, now i have %zu peers\n", global_pl.len);

    cJSON_Delete(resp_json);
    free(body);
    free(resp);
    cJSON_Delete(root);
}


void sync_job(void *c) {
    GossipContext *ctx = (GossipContext *)c;

    gossip_sync(ctx);
}