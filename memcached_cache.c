#ifdef __cplusplus
extern "C"
{
#endif

#include "memcached_cache.h"

char *mc_key(const char *endpoint, const char *path, const char* extra){
    char *shash = NULL;
    if ((endpoint != NULL) && (path != NULL)){
        char *k = NULL;
        // key: [extra::]endpoint::path
        int kl = strlen(endpoint) + strlen(path) + 3;
        if (extra != NULL){
            kl += strlen(extra) + 3;
        }
        k = (char *)calloc(kl, sizeof(char));
        if (k != NULL){
            if (extra != NULL){
                strcpy(k,extra);
                strcat(k,"::");
                strcat(k, endpoint);
            }
            else{
                strcpy(k,endpoint);
            }
            strcat(k,"::");
            strcat(k,path);

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

unsigned int mc_init(void **memc, void **servers){
    memcached_st **memc2 = (memcached_st**)memc;
    memcached_server_st **servers2 = (memcached_server_st**)servers;
    memcached_return_t rc;

    *memc2 = memcached_create(NULL);
    *servers2 = memcached_server_list_append(*servers2, MEMCACHED_HOST, MEMCACHED_DEFAULT_PORT, &rc);
    rc = memcached_server_push(*memc2, *servers2);
    if (rc != MEMCACHED_SUCCESS)
        fprintf(stderr, "Couldn't init: %s\n", memcached_strerror(*memc2, rc));

    return (unsigned int)rc;
}

unsigned int mc_set(const void *memc, const char *key, const void *payload, const unsigned long int payload_len){
    memcached_return_t rc;
    size_t klen = strlen(key);

    rc = memcached_set((memcached_st *)memc, key, klen, payload, payload_len, (time_t)MEMCACHED_EXPIRATION, (uint32_t)0);
    if (rc != MEMCACHED_SUCCESS)
        fprintf(stderr, "Couldn't set key: %s\n", memcached_strerror(memc, rc));

    return (unsigned int)rc;
}

unsigned int mc_get(const void *memc, const char *key, void **payload, unsigned long int *payload_len){
    memcached_return_t rc;
    size_t klen = strlen(key);
    uint32_t flags;

    *payload = memcached_get((memcached_st *)memc, key, klen, payload_len, &flags, &rc);
    if ((rc != MEMCACHED_SUCCESS) || (*payload == NULL)){
        fprintf(stderr, "Couldn't get key %s: %s\n", key, memcached_strerror(memc, rc));
    }

    return (unsigned int)rc;
}

unsigned int mc_del(const void *memc, const char *key){
    memcached_return_t rc;
    size_t klen = strlen(key);

    rc = memcached_delete((memcached_st *)memc, key, klen, (time_t)0);
    if (rc != MEMCACHED_SUCCESS)
        fprintf(stderr, "Couldn't delete key: %s\n", memcached_strerror(memc, rc));

    return (unsigned int)rc;
}

unsigned int mc_flush(const void *memc){
    memcached_return_t rc;
    rc = memcached_flush((memcached_st *)memc, 0);
    return (unsigned int)rc;
}
#ifdef __cplusplus
}
#endif

