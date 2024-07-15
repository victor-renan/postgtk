#pragma once

#include <curl/curl.h>
#include <gtk/gtk.h>

typedef struct {
    GtkBox *reqplace;
    uint *method;
    char *url;
    char *body;
    char *headers;
} Request;

typedef struct {
    char *preview;
    char *headers;
    char *cookies;
    size_t size;
    double time;
    long status;
} Response;

typedef struct {
    char *response;
    size_t size;
} Memory;

Response req_get(Request req);
Response req_post(Request req);
Response req_put(Request req);
Response req_patch(Request req);
Response req_delete(Request req);
Response req_options(Request req);
Response req_head(Request req);
Response req_connect(Request req);