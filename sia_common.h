#ifndef SIA_COMMON_H
#define SIA_COMMON_H
#ifdef __cplusplus
extern "C"
{
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <curl/curl.h>
#include <cjson/cJSON.h>

#define SIA_CACHE_TTL   5

typedef struct{
    char *src;
    char *payload;
    time_t time;
}sia_payload_t;

typedef struct{
    void *data;
    unsigned long int len;
}sia_http_payload_t;

typedef struct{
    char *url;
    char *scheme;
    char *host;
    char *user;
    char *password;
    char *port_s;
    unsigned int port;
    char *bucket;
    char *unauthenticated_url;
    short verbose;
    unsigned int maxhandle;
}sia_cfg_t;

size_t send_payload(void *contents, size_t sz, size_t nmemb, void *ctx);
size_t capture_payload(void *contents, size_t sz, size_t nmemb, void *ctx);

char *sia_get_from_cache(const char *src);
char *sia_set_to_cache(const char *src, const char *payload);

#ifdef __cplusplus
}
#endif
#endif
