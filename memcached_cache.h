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

#define MEMCACHED_HOST          "127.0.0.1"
#define MEMCACHED_EXPIRATION    10

char *key(const char *endpoint, const char *path);
unsigned int mc_init(void **memc, void **servers);
unsigned int mc_set(const char *key, const void *payload, const unsigned long int payload_len);
unsigned int mc_get(const char *key, void *payload, unsigned long int *payload_len);
unsigned int mc_del(const char *key);

#ifdef __cplusplus
}
#endif
#endif

