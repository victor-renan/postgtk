#include <curl/curl.h>
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
        char *header = malloc(len);

        sprintf(header, org, i, nc->data);

        if (result != NULL) {
            result = realloc(result, strlen(result) + len + 1);
            strcat(result, header);
        } else {
            result = header;
        }

        nc = nc->next;
        i++;
    }

    if(i == 1) {
        result = "No cookies\n";
    }

    curl_slist_free_all(cookies);

    return result;
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

Response perform_request(Request req)
{
    CURL *curl = curl_easy_init();  
    Memory chunk = {0};
    Response res;

    if (curl) {
        match_method(req, curl);
        curl_easy_setopt(curl, CURLOPT_URL, req.url);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_cb);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)&chunk);

        CURLcode errcode = curl_easy_perform(curl);

        if (errcode == CURLE_OK) {
            res.preview = chunk.response;
            res.headers = write_headers(curl);
            res.cookies = write_cookies(curl);
            res.size = chunk.size;
            curl_easy_getinfo(curl, CURLINFO_TOTAL_TIME, &res.time);
            curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &res.status);
        } else {
            res.preview = (char*) curl_easy_strerror(errcode);
            res.headers = "No headers";
            res.cookies = "No cookies";
            res.size = 0;
            res.status = 0;
            res.time = 0;
        }

        curl_easy_cleanup(curl);
    }

    return res;
}