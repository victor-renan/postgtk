#pragma once

#include <curl/curl.h>
#include <gtk/gtk.h>

typedef enum {
    ERR_CURL = 1,
    ERR_JSON = 2
} ErrCode;

typedef enum {
    LOC_REQ_HEADERS = 1,
    LOC_REQ_BODY = 2,
    LOC_INTERNAL = 3
} LocCode;

typedef struct {
    ErrCode code;
    LocCode loc;
    char* text;
} HTTPClientErr;

typedef struct {
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

char *get_http_client_err(HTTPClientErr *err);
HTTPClientErr *new_http_client_err(ErrCode code, LocCode, char *text);