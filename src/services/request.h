#pragma once

#include <curl/curl.h>
#include <gtk/gtk.h>

typedef struct {
    GtkBox *reqplace;
    uint method;
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

Response perform_request(Request req);