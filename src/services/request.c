#include <curl/curl.h>
#include <stdlib.h>
#include <string.h>

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

Response req_get(Request req) {
    CURL *curl = curl_easy_init();
    Memory chunk = {0};
    Response res;

    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, req.url);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_cb);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)&chunk);

        CURLcode errcode = curl_easy_perform(curl);

        if (errcode == CURLE_OK) {
            res.preview = chunk.response;
            res.headers = "";
            res.cookies = "";
            res.size = chunk.size;
            curl_easy_getinfo(curl, CURLINFO_TOTAL_TIME, &res.time);
            curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &res.status);
        } else {
            res.preview = (char*) curl_easy_strerror(errcode);
            res.headers = res.preview;
            res.cookies = res.preview;
            res.size = 0;
            res.status = 000;
            res.time = 0;
        }

        curl_easy_cleanup(curl);
    }

    return res;
}