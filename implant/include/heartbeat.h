#pragma once

/**
 * Send a heartbeat to the server thats it
 */
int send_heartbeat();
void heartbeat_job(void *ctx);