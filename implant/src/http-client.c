#include "http-client.h"
#include <curl/curl.h>
#include <stdlib.h>
#include <string.h>


// this dude is here to hold dynamically allocated data
typedef struct {
    char *data;
    size_t len;
} buffer_t;


// libcurl callback function to write the response
// I am an atheist, but I swear only God and maybe Linus know how this shit works
static size_t _write_cb(void *ptr, size_t size, size_t nmemb, void *userdata) {
    size_t realChunk = size * nmemb;
    buffer_t *buf = (buffer_t *)userdata;

    char *tmp = realloc(buf->data, buf->len + realChunk + 1);
    if(!tmp) return 0; // shit failed lmao just like your last relationship
    buf->data = tmp;

    memcpy(buf->data + buf->len, ptr, realChunk);
    buf->len += realChunk;
    buf->data[buf->len] = '\0';

    return realChunk;
}


// Performs the GET method, sets the neccessary stuff like url and callback function and shit
int http_get(const char *url, char **out_body) {
    CURL *curl = curl_easy_init();
    if(!curl) return -1; // doomed

    buffer_t buf = {
        .data = NULL,
        .len  = 0
    };

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, _write_cb);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buf);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 15L);


    CURLcode res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);

    if(res != CURLE_OK) {
        free(buf.data);
        return (int)res;
    };

    *out_body = buf.data;
    return 0;
}


// Performs the POST method
int http_post(const char *url, const char *json_body, char **out_body) {
    CURL *curl = curl_easy_init();
    if(!curl) return -1; // doomed in this case too lmaooo

    buffer_t buf = {
        .data = NULL,
        .len  = 0
    };

    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, "Content-Type: application/json");

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_body);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, _write_cb);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buf);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 15L);


    CURLcode res = curl_easy_perform(curl);
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    if(res != CURLE_OK) {
        free(buf.data);
        return (int)res;
    };

    *out_body = buf.data;
    return 0;
}