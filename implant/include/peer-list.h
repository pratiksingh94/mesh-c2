#pragma once
#include <stddef.h>

typedef struct {
    char *ip;
    int port;
} Peer;


// For the peers
typedef struct {
    Peer *peers;
    size_t len;
    size_t cap;
} PeerList;





/* Initialize an empty peer list */
void pl_init(PeerList *pl);

/* Free all memory of the thingamajig */
void pl_free(PeerList *pl);

// Returns the index of Peer you are trying to find
int pl_find(PeerList *pl, const char *ip, int port);

/**
 * Add a peer if it isn’t already present
 * Returns 1 if added, 0 if already existed, -1 on if doomed
 */
int pl_add(PeerList *pl, const char *ip, int port);

/**
 * Remove a peer by ip+port
 * Returns 1 if removed, 0 if not found
 */
int pl_remove(PeerList *pl, const char *ip, int port);



