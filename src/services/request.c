#include <curl/curl.h>
#include <jansson.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "request.h"

static size_t write_cb(char *data, size_t size, size_t nmemb, void *clientp)
{
    size_t totalsize = (size * nmemb);
    Memory *mem = (Memory*) clientp;
    char *ptr = realloc(mem->response, mem->size + totalsize + 1);

    if (!ptr) return 0;

    mem->response = ptr;
    memcpy(&(mem->response[mem->size]), data, totalsize);
    mem->size += totalsize;
    mem->response[mem->size] = 0; 

    return totalsize;
}

char *write_headers(CURL *curl)
{
    struct curl_header *h;
    struct curl_header *prev = NULL;
    char *result = NULL;

    while ((h = curl_easy_nextheader(curl, CURLH_HEADER, 0, prev))) {
        char *org = "%s: %s\r\n";
        size_t len = strlen(h->name) + strlen(h->value) + strlen(org) + 1;
        char *header = malloc(len);

        sprintf(header, org, h->name, h->value);

        if (result != NULL) {
            result = realloc(result, strlen(result) + len + 1);
            strcat(result, header);
        } else {
            result = header;
        }

        prev = h;
    }

    return result;
}

char *write_cookies(CURL *curl)
{
    struct curl_slist *cookies;
    struct curl_slist *nc;

    CURLcode res = curl_easy_getinfo(curl, CURLINFO_COOKIELIST, &cookies);
    if(res != CURLE_OK) {
        return (char*) curl_easy_strerror(res);
    }

    nc = cookies;

    char *result = NULL;
    int i = 1;

    while(nc) {
        char *org = "[%d]: %s\n";
        size_t len = strlen(nc->data) + strlen(org) + sizeof(i) + 1;
        char *ck = malloc(len);

        sprintf(ck, org, i, nc->data);

        if (result != NULL) {
            result = realloc(result, strlen(result) + len + 1);
            strcat(result, ck);
        } else {
            result = ck;
        }

        nc = nc->next;
        i++;
    }

    if(i == 1) {
        result = "No cookies";
    }

    curl_slist_free_all(cookies);

    return result;
}

HTTPClientErr *new_http_client_err(ErrCode code, LocCode loc, char *text)
{
    HTTPClientErr *err = malloc(sizeof(err));
    err->code = code;
    err->loc = loc;
    err->text = malloc(strlen(text));
    strcpy(err->text, text);

    return err;
}

char *get_http_client_err(HTTPClientErr *err)
{
    if (!err) {
        return "No error";
    }

    char *code, *loc, *org = "%s: %s in %s";

    switch (err->code) {
        case ERR_CURL:
            code = "curl";
            break;
        case ERR_JSON:
            code = "json";
            break;
        default:
            code = "client";
            break;
    }

    switch (err->loc) {
        case LOC_REQ_HEADERS:
            loc = "Headers";
            break;
        case LOC_REQ_BODY:
            loc = "Body";
            break;
        case LOC_INTERNAL:
            loc = "Client";
            break;
        default:
            loc = "App";
            break;
    }

    char *result = malloc(
        strlen(code) + strlen(org) + strlen(err->text) + 1);

    sprintf(result, org, code, err->text, loc);

    return result;
}

HTTPClientErr *set_user_headers(Request req, CURL *curl)
{
    const char *key;

    json_t *obj, *val;
    json_error_t err;

    obj = json_loads(req.headers, 1, &err);

    if (!obj) { 
        return new_http_client_err(ERR_JSON, LOC_REQ_HEADERS, err.text);
    };

    struct curl_slist *slist = NULL;

    json_object_foreach(obj, key, val) {
        char *parsed = (char*) json_string_value(val),
            *org = "%s: %s";
        if (strcmp(parsed, ";") == 0)
            org = "%s%s";
        
        slist = curl_slist_append(slist, json_string_value(
            json_sprintf(org, key, parsed)
        ));
    }

    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, slist);

    return NULL;
}

void match_method(Request req, CURL *curl)
{
    switch (req.method) {
        case 1:
            curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "POST");
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, req.body);
            break;
        case 2:
            curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PUT");
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, req.body);
            break;
        case 3:
            curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PATCH");
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, req.body);
            break;
        case 4:
            curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "DELETE");
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, req.body);
            break;
        case 5:
            curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "OPTIONS");
            break;
        case 6:
            curl_easy_setopt(curl, CURLOPT_NOBODY, 1L);
            break;
        default:
            break;
    }
}

Response errata(Response res, CURL *curl, HTTPClientErr *err)
{
    res.preview = get_http_client_err(err);
    res.headers = "No headers";
    res.cookies = "No cookies";
    res.size = 0;
    res.status = 0;
    res.time = 0;
    curl_easy_cleanup(curl);

    return res;
}

Response perform_request(Request req)
{
    CURL *curl = curl_easy_init();  
    Memory chunk = {0};
    Response res;

    HTTPClientErr *err = NULL;

    if (curl) {
        match_method(req, curl);

        curl_easy_setopt(curl, CURLOPT_URL, req.url);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_cb);
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)&chunk);

        err = set_user_headers(req, curl);

        if (err) return errata(res, curl, err);

        CURLcode errcode = curl_easy_perform(curl);
        if (errcode != CURLE_OK) {
            err = new_http_client_err(
                ERR_CURL, LOC_INTERNAL, 
                (char *) curl_easy_strerror(errcode));
        }
        
        if (err) return errata(res, curl, err);

        res.preview = chunk.response;
        if (!chunk.response) res.preview = "No body";
        res.headers = write_headers(curl);
        res.cookies = write_cookies(curl);
        res.size = chunk.size;
        curl_easy_getinfo(curl, CURLINFO_TOTAL_TIME, &res.time);
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &res.status);
        curl_easy_cleanup(curl);

        return res;
    }

    return errata(res, curl, new_http_client_err(
        ERR_CURL, LOC_INTERNAL,
        "Client is null"
    ));
}