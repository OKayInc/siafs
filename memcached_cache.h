#ifndef MEMCACHED_CACHE_H
#define MEMCACHED_CACHE_H
#ifdef __cplusplus
extern "C"
{
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libmemcached/memcached.h>

#include "md5.h"

#define MEMCACHED_HOST          "127.0.0.1"

char *mc_key(const char *endpoint, const char *path, const char* extra);
unsigned int mc_init(void **memc, void **servers);
unsigned int mc_set(const void *memc, const char *key, const void *payload, const unsigned long int payload_len, time_t expiration);
unsigned int mc_get(const void *memc, const char *key, void **payload, unsigned long int *payload_len);
unsigned int mc_del(const void *memc, const char *key);
unsigned int mc_flush(const void *memc);
#ifdef __cplusplus
}
#endif
#endif

