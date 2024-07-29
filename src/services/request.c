#include <curl/curl.h>
#include <jansson.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "request.h"

static size_t
write_cb(char *data, size_t size, size_t nmemb, void *clientp)
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

HTTPClientErr
*new_http_client_err(ErrCode code, LocCode loc, char *text)
{
    HTTPClientErr *err = malloc(sizeof(err));
    err->code = code;
    err->loc = loc;
    err->text = malloc(strlen(text));
    strcpy(err->text, text);

    return err;
}

char
*get_http_client_err(HTTPClientErr *err)
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

    char *result = malloc(strlen(code) + strlen(org) + strlen(err->text) + 1);

    sprintf(result, org, code, err->text, loc);

    return result;
}

char
*write_res_preview(CURL *curl, char *str)
{
    if (!str) {
        return "No Preview";
    }

    return str;
}

char
*write_res_headers(CURL *curl)
{
    struct curl_header *h;
    struct curl_header *prev = NULL;
    char *result = NULL;

    while ((h = curl_easy_nextheader(curl, CURLH_HEADER, -1, prev))) {
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

    if (!result) {
        result = "No Headers";
    }

    return result;
}

char
*write_res_cookies(CURL *curl)
{
    struct curl_slist *cookies;
    struct curl_slist *nc;

    CURLcode res = curl_easy_getinfo(curl, CURLINFO_COOKIELIST, &cookies);
    if (res != CURLE_OK) {
        return (char*) curl_easy_strerror(res);
    }

    nc = cookies;

    char *result = NULL;
    int i = 1;

    while (nc) {
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
        result = "No Cookies";
    }

    curl_slist_free_all(cookies);

    return result;
}

void
write_prev_headers(CURL *curl, Request req, struct curl_slist *slist)
{
    if (req.body_type == 0) {
        curl_slist_append(slist, "Content-Type: application/json");
    } else {
        curl_slist_append(slist, "Content-Type: multipart/form-data");
    }
}

HTTPClientErr
*write_req_headers(Request req, CURL *curl)
{
    const char *key;
    json_t *obj, *val;
    json_error_t err;

    obj = json_loads(req.headers, 1, &err);

    if (!obj) { 
        return new_http_client_err(ERR_JSON, LOC_REQ_HEADERS, err.text);
    };

    struct curl_slist *slist = NULL;

    write_prev_headers(curl, req, slist);

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

HTTPClientErr
*write_req_body(Request req, CURL *curl)
{
    if (req.body_type == 0) {
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, req.body);
        return NULL;
    }

    const char *key;
    json_t *obj, *val;
    json_error_t err;

    obj = json_loads(req.body, 1, &err);

    if (!obj) {
        return new_http_client_err(ERR_JSON, LOC_REQ_BODY, err.text);
    }

    curl_mime *multipart = curl_mime_init(curl);
    if (multipart) {
        curl_mimepart *part;
        json_object_foreach(obj, key, val) if (!json_is_object(val)) {
            part = curl_mime_addpart(multipart);
            curl_mime_name(part, key);
            if (json_is_array(val)) {
                size_t index;
                json_t *arr_val;
                json_array_foreach(val, index, arr_val) {
                    curl_mime_filedata(part, (char*) json_string_value(arr_val));
                }
            } else {
                curl_mime_data(part, (char*) json_string_value(val), CURL_ZERO_TERMINATED);
            } 
        }
    }

    curl_easy_setopt(curl, CURLOPT_MIMEPOST, multipart);

    return NULL;
}

HTTPClientErr
*match_req_type(Request req, CURL *curl)
{
    HTTPClientErr *err = NULL;
    switch (req.method) {
        case 1:
            curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "POST");
            err = write_req_body(req, curl);
            break;
        case 2:
            curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PUT");
            err = write_req_body(req, curl);
            break;
        case 3:
            curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PATCH");
            err = write_req_body(req, curl);
            break;
        case 4:
            curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "DELETE");
            err = write_req_body(req, curl);
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

    return err;
}

Response
errata(Response res, CURL *curl, HTTPClientErr *err)
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

Response
perform_request(Request req)
{
    CURL *curl = curl_easy_init();  
    Memory chunk = {0};
    Response res;

    HTTPClientErr *err = NULL;

    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, req.url);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_cb);
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)&chunk);

        err = match_req_type(req, curl);
        if (err) {
            return errata(res, curl, err);
        }

        err = write_req_headers(req, curl);
        if (err) {
            return errata(res, curl, err);
        }

        CURLcode errcode = curl_easy_perform(curl);
        if (errcode != CURLE_OK) {
            err = new_http_client_err(
                ERR_CURL, LOC_INTERNAL, 
                (char *) curl_easy_strerror(errcode));
        }
        
        if (err) {
            return errata(res, curl, err);
        }

        res.preview = write_res_preview(curl, chunk.response);
        res.headers = write_res_headers(curl);
        res.cookies = write_res_cookies(curl);
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