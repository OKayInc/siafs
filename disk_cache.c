#ifdef __cplusplus
extern "C"
{
#endif

#include "disk_cache.h"

char *disk_key(const char *path, const size_t size, const off_t offset){
    char *shash = NULL;
    if (path != NULL){
        char *k = NULL;
        char s_size[256] = {0};
        char s_offset[256] = {0};
        sprintf(s_size, "%lu", size);
        sprintf(s_offset, "%lu", offset);
        unsigned int kl = strlen(path) + strlen(s_size) + strlen(s_offset) + 5;
        k = (char *)calloc(kl, sizeof(char));
        if (k != NULL){
            strcpy(k, path);
            strcat(k,"::");
            strcat(k,s_size);
            strcat(k,"::");
            strcat(k,s_offset);

            uint8_t hash[16];
            shash = calloc(33, sizeof(char));
            if (shash != NULL){
                md5String(k, hash);
                for(unsigned int i = 0; i < 16; ++i){
                    sprintf(shash + strlen(shash),"%02x", hash[i]);
                }

                if (strlen(shash) > 250){
                    fprintf(stderr, "%s:%d WARNING MC Key: %s is too long. Sending NULL. \n", __FILE_NAME__, __LINE__, shash);
                }
            }
            free(k);
            k = NULL;
        }
    }
    return shash;
}

int disk_init(const char *cache_dir){
    int status = 0;
    struct stat st = {0};
    if (stat(cache_dir, &st) == -1) {
        status = mkdir(cache_dir, 0700);
    }

    return status;
}

unsigned int disk_set(const void *memc, const char *key, const void *payload, const unsigned long int payload_len, time_t expiration){
    memcached_return_t rc;
    size_t klen = strlen(key);

    rc = memcached_set((memcached_st *)memc, key, klen, payload, payload_len, (time_t)expiration, (uint32_t)0);
    if (rc != MEMCACHED_SUCCESS)
        fprintf(stderr, "Couldn't set key: %s\n", memcached_strerror(memc, rc));

    return (unsigned int)rc;
}

unsigned int disk_get(const void *memc, const char *key, void **payload, unsigned long int *payload_len){
    memcached_return_t rc;
    size_t klen = strlen(key);
    uint32_t flags;

    *payload = memcached_get((memcached_st *)memc, key, klen, payload_len, &flags, &rc);
    if ((rc != MEMCACHED_SUCCESS) || (*payload == NULL)){
        fprintf(stderr, "Couldn't get key %s: %s\n", key, memcached_strerror(memc, rc));
    }

    return (unsigned int)rc;
}

unsigned int disk_del(const void *memc, const char *key){
    memcached_return_t rc;
    size_t klen = strlen(key);

    rc = memcached_delete((memcached_st *)memc, key, klen, (time_t)0);
    if (rc != MEMCACHED_SUCCESS)
        fprintf(stderr, "Couldn't delete key: %s\n", memcached_strerror(memc, rc));

    return (unsigned int)rc;
}

unsigned int disk_flush(const void *memc){
    memcached_return_t rc;
    rc = memcached_flush((memcached_st *)memc, 0);
    return (unsigned int)rc;
}
#ifdef __cplusplus
}
#endif

