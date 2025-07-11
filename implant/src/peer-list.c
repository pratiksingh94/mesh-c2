#include "peer-list.h"
#include <stdlib.h>
#include <string.h>
#include <time.h>



#define INITIAL_CAP 10

void pl_init(PeerList *pl) {
    pl->peers = malloc(sizeof(Peer) * INITIAL_CAP);
    pl->len   = 0;
    pl->cap   = INITIAL_CAP;
    
    srand((unsigned)time(NULL));
}

void pl_free(PeerList *pl) {
    for (size_t i = 0; i < pl->len; i++) {
        free(pl->peers[i].ip);
    }
    free(pl->peers);
    pl->peers = NULL;
    pl->len = pl->cap = 0;
}

int pl_find(PeerList *pl, const char *ip, int port) {
    for (size_t i = 0; i < pl->len; i++) {
        if (pl->peers[i].port == port && strcmp(pl->peers[i].ip, ip) == 0)
            return (int)i;
    }
    return -1;
}

int pl_add(PeerList *pl, const char *ip, int port) {
    if (pl_find(pl, ip, port) >= 0) return 0;  // already have it

    if (pl->len == pl->cap) {
        size_t nc = pl->cap * 2;
        Peer *tmp = realloc(pl->peers, sizeof(Peer) * nc);
        if (!tmp) return -1;
        pl->peers = tmp;
        pl->cap   = nc;
    }

    pl->peers[pl->len].ip   = strdup(ip);
    pl->peers[pl->len].port = port;
    pl->len++;
    return 1;
}

int pl_remove(PeerList *pl, const char *ip, int port) {
    int idx = pl_find(pl, ip, port);
    if (idx < 0) return 0;
    free(pl->peers[idx].ip);

    // shift peers down to fill the gap   
    memmove(&pl->peers[idx], &pl->peers[idx+1],
            sizeof(Peer) * (pl->len - idx - 1));
    pl->len--;
    return 1;
}







