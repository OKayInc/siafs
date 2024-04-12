#ifndef SIA_H
#define SIA_H
#ifdef __cplusplus
extern "C"
{
#endif

#define FUSE_USE_VERSION 30
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <fuse.h>
#include <curl/curl.h>
#include <cjson/cJSON.h>

#include "structures.h"
#include "urlcode.h"

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

char *sia_get_from_cache(const char *src);
char *sia_set_to_cache(const char *src, const char *payload);

char *sia_bus_concensus_state_json(siafs_opt_t *opt);
unsigned short int sia_bus_concensus_state_synced(siafs_opt_t *opt);
unsigned int sia_bus_concensus_state_blockheight(siafs_opt_t *opt);
char *sia_bus_concensus_state_lastblocktime(siafs_opt_t *opt);


char *sia_bus_objects_json(siafs_opt_t *opt, const char *path);
unsigned short int sia_bus_objects_is_dir(siafs_opt_t *opt, const char *path);
unsigned short int sia_bus_objects_is_file(siafs_opt_t *opt, const char *path);
unsigned int sia_bus_objects_size(siafs_opt_t *opt, const char *path);
char *sia_bus_objects_modtime(siafs_opt_t *opt, const char *path);

char *sia_worker_objects(siafs_opt_t *opt, const char *path, size_t *size, off_t *offset);

#ifdef __cplusplus
}
#endif
#endif
