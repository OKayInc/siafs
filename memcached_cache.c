#ifdef __cplusplus
extern "C"
{
#endif

#include "memcached_cache.h"
memcached_server_st *servers = NULL;
memcached_st *memc;

char *key(const char *endpoint, const char *path){
    char *k = NULL;
    if ((endpoint != NULL) && (path != NULL)){
        int kl = strlen(endpoint) + strlen(path) +2;
        k = (char*)calloc(kl, sizeof(char));
        strcpy(k,endpoint);
        strcat(k,":");
        strcpy(k,path);
    }
    return k;
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

unsigned int mc_set(const char *key, const void *payload, const unsigned long int payload_len){
    memcached_return_t rc;
    size_t klen = strlen(key);

    rc = memcached_set(memc, key, klen, payload, payload_len, (time_t)MEMCACHED_EXPIRATION, (uint32_t)0);
    if (rc != MEMCACHED_SUCCESS)
        fprintf(stderr, "Couldn't set key: %s\n", memcached_strerror(memc, rc));

    return (unsigned int)rc;
}

unsigned int mc_get(const char *key, void *payload, unsigned long int *payload_len){
    memcached_return_t rc;
    size_t klen = strlen(key);
    uint32_t flags;

    payload = memcached_get(memc, key, klen, payload_len, &flags, &rc);
    if (rc != MEMCACHED_SUCCESS)
        fprintf(stderr, "Couldn't get key: %s\n", memcached_strerror(memc, rc));

    return (unsigned int)rc;
}

unsigned int mc_del(const char *key){
    memcached_return_t rc;
    size_t klen = strlen(key);

    rc = memcached_delete(memc, key, klen, (time_t)0);
    if (rc != MEMCACHED_SUCCESS)
        fprintf(stderr, "Couldn't delete key: %s\n", memcached_strerror(memc, rc));

    return (unsigned int)rc;
}
#ifdef __cplusplus
}
#endif

