#ifdef __cplusplus
extern "C"
{
#endif

#include "sia_common.h"
extern sia_cfg_t opt;

sia_payload_t sia_cache = {
    .src = NULL,
    .payload = NULL,
    .time = 0
};

size_t send_payload(void *contents, size_t sz, size_t nmemb, void *ctx){
    size_t realsize = sz * nmemb;
//    if(opt.verbose){
        fprintf(stderr, "%s:%d %s(\"%s\", %lu, %lu, \"%s\")\n", __FILE_NAME__, __LINE__, __func__, "(char *)contents", sz, nmemb, "(char *)ctx");
//    }
    sia_http_payload_t *data = (sia_http_payload_t *)ctx;
    memcpy(contents, data->data, data->len);

    return data->len;
}

size_t capture_payload(void *contents, size_t sz, size_t nmemb, void *ctx){
    size_t realsize = sz * nmemb;
    if(opt.verbose){
        fprintf(stderr, "%s:%d %s(\"%s\", %lu, %lu, \"%s\")\n", __FILE_NAME__, __LINE__, __func__, "(char *)contents", sz, nmemb, "(char *)ctx");
    }
    sia_http_payload_t *data = (sia_http_payload_t *)ctx;
    if (data->len == 0){
        data->len = realsize;
        data->data = malloc(sizeof(data->data)*realsize+1);
        memmove(data->data, contents, realsize);
    }
    else{
        unsigned long int oldlen = data->len;
        data->len += realsize;
        data->data = realloc(data->data, data->len + realsize + 1);
        memcpy(data->data + oldlen, contents, realsize);
    }

    ctx = &data;
    return realsize;
}

// TODO: memcached support
char *sia_get_from_cache(const char *src){
    if(opt.verbose){
        fprintf(stderr, "%s:%d %s(\"%s\")\n", __FILE_NAME__, __LINE__, __func__, src);
    }
    if ((sia_cache.time != 0) && (sia_cache.src != NULL)){
        // there is something
        time_t now = time(NULL);
        if ((now - sia_cache.time) <= SIA_CACHE_TTL){
            // cache TTL still valid
            if (!strcmp(sia_cache.src, src)){
                // same source
                if(opt.verbose){
                    fprintf(stderr, "%s:%d\tHIT\n", __FILE_NAME__, __LINE__);
                }
                return sia_cache.payload;
            }
        }
    }
    return NULL;
}

char *sia_set_to_cache(const char *src, const char *payload){
    if(opt.verbose){
        fprintf(stderr, "%s:%d %s(\"%s\", \"%s\")\n", __FILE_NAME__, __LINE__, __func__, src, payload);
    }
    if ((src != NULL) && (payload != NULL)){
        if (sia_cache.src != NULL){
            free(sia_cache.src);
            sia_cache.src = NULL;
        }
        if (sia_cache.payload != NULL){
            free(sia_cache.payload);
            sia_cache.payload = NULL;
        }
        sia_cache.src = malloc(strlen(src));
        strcpy(sia_cache.src, src);
        sia_cache.payload = malloc(strlen(payload));
        strcpy(sia_cache.payload, payload);
        sia_cache.time = time(NULL);
        return sia_cache.payload;
    }
    return NULL;
}

#ifdef __cplusplus
}
#endif
