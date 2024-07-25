#ifndef DISK_CACHE_H
#define DISK_CACHE_H
#ifdef __cplusplus
extern "C"
{
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <libmemcached/memcached.h>
#include <unistd.h>
#include <sys/stat.h>
#include <math.h>

#include "md5.h"

#define MEMCACHED_HOST          "127.0.0.1"

char *disk_key(const char *path);
int disk_init(const char *cache_dir);
unsigned int disk_set(const char *cache_dir, const char *key, const void *payload, const unsigned long int payload_len, size_t size, off_t offset, char *etag);
unsigned int disk_get(const char *cache_dir, const char *key, void **payload, unsigned long int *payload_len, size_t size, off_t offset);
unsigned int disk_del(const void *memc, const char *key);
unsigned int disk_flush(const void *memc);
#ifdef __cplusplus
}
#endif
#endif

