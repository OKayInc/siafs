#ifndef SIA_COMMON_H
#define SIA_COMMON_H
#ifdef __cplusplus
extern "C"
{
#endif

#define _XOPEN_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdarg.h>
#include <curl/curl.h>
#include <cjson/cJSON.h>

#include "base64.h"
#ifdef SIA_MEMCACHED
#include "memcached_cache.h"
#endif

#ifdef SIA_DISK_CACHE
#include "disk_cache.h"
#endif


#define SIA_METACACHE_TTL   600
#define SIA_BLOCKCHAIN_TTL  3600
#define SIA_CACHE_TTL       5
#define SIA_MAX_PARTS       10000

    // Caching structures
typedef struct{
    char *src;
    char *payload;
    time_t time;
}sia_payload_t;

typedef enum file_type_e {
    SIA_UNKOWN,
    SIA_DIR,
    SIA_FILE
} sia_object_type_t;

typedef struct sia_metacache_s{
    char *name;                     // the path
    sia_object_type_t type;         // check ENUM for possible values
    unsigned long long int size;    // Size, must be 0 for SIA_DIR
    time_t modtime;
    time_t expire;                  // Data valid until expire
    struct sia_metacache_s *next;
} sia_metacache_t;

typedef struct sia_cache_s{
    char *(*key)(const char *endpoint, const char *path, const char *extra);
    unsigned int (*init)(void **memc, void **servers);
    unsigned int (*set)(const void *memc, const char *key, const void *payload, const unsigned long int payload_len, time_t expiration);
    unsigned int (*del)(const void *memc, const char *key);
    unsigned int (*get)(const void *memc, const char *key, void **payload, unsigned long int *payload_len);
    unsigned int (*flush)(const void *memc);
} sia_cache_t;

typedef struct sia_cache2_s{
    char *(*key)(const char *path, const size_t size, const off_t offset);
    int (*init)(const char *cache_dir);
    unsigned int (*set)(const void *memc, const char *key, const void *payload, const unsigned long int payload_len, time_t expiration);
    unsigned int (*del)(const void *memc, const char *key);
    unsigned int (*get)(const void *memc, const char *key, void **payload, unsigned long int *payload_len);
    unsigned int (*flush)(const void *memc);
} sia_cache2_t;


// Multi-part upload structures
typedef struct{
    char *etag;
#ifdef SIA_HUGE_FILES
    char *tmpfn;
#endif
} sia_uploaded_part_t;

typedef struct sia_upload_s{
    sia_uploaded_part_t part[SIA_MAX_PARTS];
    char *name;
    char *uploadID;
    struct sia_upload_s *next;
} sia_upload_t;

// HTTP Payload
typedef struct{
    char *data;
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
#ifdef SIA_METACACHE
    sia_metacache_t *metacache;
#endif
    sia_cache_t *L1;    // L1 Cache should be a meta cache
    sia_cache2_t *L2;    // L2 Cache should be a data cache
#ifdef SIA_MEMCACHED
    memcached_server_st *servers;
    memcached_st *memc;
#endif
    cJSON *payload_buffer;
    char *cache_dir;
}sia_cfg_t;

char *append(const char before, char *str, const char after);
char *b64cat(unsigned int n, ...);
time_t string2unixtime(char *timestamp);
cJSON *push_file(sia_cfg_t *opt, const char *path);
cJSON *find_file_by_path(sia_cfg_t *opt, const char *path);
cJSON *push_payload_multipart(sia_cfg_t *opt, const cJSON *file, const unsigned pn, const char *base64);
cJSON *find_payload_multipart_by_number(sia_cfg_t *opt, const cJSON *file, const unsigned pn);
unsigned long long find_payload_multipart_size(sia_cfg_t *opt, const cJSON *file);
void flush_payload_multiparts(sia_cfg_t *opt, const cJSON *file);

#ifdef SIA_METACACHE
sia_metacache_t *dump_meta(sia_metacache_t *meta);
sia_metacache_t *dump_all_meta(sia_cfg_t *opt);
sia_metacache_t *append_meta(sia_cfg_t *opt, sia_metacache_t *meta);
sia_metacache_t *del_meta(sia_cfg_t *opt, sia_metacache_t *meta);
sia_metacache_t *find_meta_by_path(sia_cfg_t *opt, const char *path);
sia_metacache_t *build_meta_node_from_json(cJSON *object);
sia_metacache_t *update_meta(sia_cfg_t *opt, sia_metacache_t *src, sia_metacache_t *dst);
sia_metacache_t *add_meta(sia_cfg_t *opt, sia_metacache_t *meta);
#endif

sia_upload_t *create_upload(sia_cfg_t *opt, const char *path, const char *upload_id);
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
