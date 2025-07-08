#pragma once


/*
Perform GET Request, duh
    url - the damn url, isnt that obvious?
    out_body - pointer to a string that will hold the response body, must be freed by the caller
Returns 0 on success, and non-zero on failure (of-course)
*/
int http_get(const char *url, char **out_body);


/*
Perform POST Request with JSON body
    url - the damn url, isnt that obvious?
    json_body - the JSON body to send, must be a valid JSON string
    out_body - pointer to a string that will hold the response body, must be freed by the caller
Returns 0 on success, and non-zero on failure duh
*/
int http_post(const char *url, const char *json_body, char **out_body);