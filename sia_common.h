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
#define SIA_MAX_PARTS   10000
// Caching structures
typedef struct{
    char *src;
    char *payload;
    time_t time;
}sia_payload_t;

typedef struct sia_metacache_s{
    char *name;
    unsigned long long int size;
    time_t  modtime;
    struct sia_metacache_s *next;
} sia_metacache_t;

// Multi-part upload structures
typedef struct{
    char *etag;
} sia_uploaded_part_t;

typedef struct sia_upload_s{
    sia_uploaded_part_t part[SIA_MAX_PARTS];
    char *name;
    char *uploadID;
    struct sia_upload_s *next;
} sia_upload_t;

// HTTP Payload
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
    sia_upload_t *uploads;
    sia_metacache_t *metacache;
}sia_cfg_t;

sia_upload_t *append_upload(sia_cfg_t *opt, sia_upload_t *upload);
sia_upload_t *del_upload(sia_cfg_t *opt, sia_upload_t *upload);
sia_upload_t *find_upload_by_path(sia_cfg_t *opt, const char *path);

size_t capture_payload(void *contents, size_t sz, size_t nmemb, void *ctx);
size_t send_payload(void *contents, size_t sz, size_t nmemb, void *ctx);

char *sia_get_from_cache(const char *src);
char *sia_set_to_cache(const char *src, const char *payload);

#ifdef __cplusplus
}
#endif
#endif
