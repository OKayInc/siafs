#include "sia.h"
#ifdef __cplusplus
extern "C"
{
#endif

extern siafs_opt_t opt;

sia_payload_t sia_cache = {
    .src = NULL,
    .payload = NULL,
    .time = 0
};

// TODO: memcached support
char *sia_get_from_cache(const char *src){
    if(opt.verbose){
        fprintf(stderr, "%s(\"%s\")\n", __func__, src);
    }
    if ((sia_cache.time != 0) && (sia_cache.src != NULL)){
        // there is something
        time_t now = time(NULL);
        if ((now - sia_cache.time) <= SIA_CACHE_TTL){
            // cache TTL still valid
            if (!strcmp(sia_cache.src, src)){
                // same source
                if(opt.verbose){
                    fprintf(stderr, "\tHIT\n");
                }
                return sia_cache.payload;
            }
        }
    }
    return NULL;
}

char *sia_set_to_cache(const char *src, const char *payload){
    if(opt.verbose){
        fprintf(stderr, "%s(\"%s, %s\")\n", __func__, src, payload);
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

size_t capture_payload(void *contents, size_t sz, size_t nmemb, void *ctx){
    size_t realsize = sz * nmemb;
    if(opt.verbose){
        fprintf(stderr, "capture_payload: %s\n", (char *)contents);
        fprintf(stderr, "sz: %lu\n", sz);
        fprintf(stderr, "nmemb: %lu\n", nmemb);
    }
    sia_http_payload_t *data = (sia_http_payload_t *)ctx;
    data->len = realsize;
    data->data = malloc(sizeof(data->data)*realsize+1);
    memcpy(data->data, contents, realsize);
    
    ctx = &data;
    return realsize;
}

char *sia_bus_concensus_state_json(siafs_opt_t *opt){
    if(opt->verbose){
        fprintf(stderr, "%s(\"%s\")\n", __func__, opt->url);
    }

    char *final_url;
    final_url = malloc(sizeof(opt->unauthenticated_url)*strlen(opt->unauthenticated_url)+1);
    strcpy(final_url, opt->unauthenticated_url);
    strcat(final_url, "api/bus/consensus/state");

    if(opt->verbose){
        fprintf(stderr, "%s(\"%s\")\n", __func__, final_url);
    }
    
    void *payload = (void *)sia_get_from_cache(final_url);
    sia_http_payload_t http_payload;
    
    if (payload == NULL){
        CURL *curl;
        CURLcode res;
        payload = malloc(1024);    // TODO: find a way to predict a suitable size
        curl = curl_easy_init();
        if(curl) {
            curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "GET");
            curl_easy_setopt(curl, CURLOPT_URL, final_url);
            curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
            curl_easy_setopt(curl, CURLOPT_DEFAULT_PROTOCOL, opt->scheme);
            curl_easy_setopt(curl, CURLOPT_USERNAME, opt->user);
            curl_easy_setopt(curl, CURLOPT_PASSWORD, opt->password);
    //        curl_easy_setopt(curl, CURLOPT_HTTPAUTH, CURLAUTH_BASIC);
            struct curl_slist *headers = NULL;
            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, capture_payload);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &http_payload);
            res = curl_easy_perform(curl);
            
        }
        
        if(opt->verbose){
            fprintf(stderr, "%s payload: %s\n", __func__, (char *)http_payload.data);
        }
        sia_set_to_cache(final_url, http_payload.data);
        payload = http_payload.data;

        curl_easy_cleanup(curl);
        free(final_url);
    }
    return payload;
}

unsigned short int sia_bus_concensus_state_synced(siafs_opt_t *opt){
    if(opt->verbose){
        fprintf(stderr, "%s(\"%s\")\n", __func__, opt->url);
    }
    char *json_payload = sia_bus_concensus_state_json(opt);
    if (json_payload != NULL){
        if(opt->verbose){
            fprintf(stderr, "json payload: %s\n", json_payload);
        }
        cJSON *monitor_json = cJSON_Parse(json_payload);
        free(json_payload);
        if (monitor_json == NULL){
            const char *error_ptr = cJSON_GetErrorPtr();
            fprintf(stderr, "Error before: %s\n", error_ptr);
        }
        else{
            cJSON *synced = NULL;
            synced = cJSON_GetObjectItemCaseSensitive(monitor_json, "synced");
            if (cJSON_IsBool(synced)){
                 if(opt->verbose){
                    fprintf(stderr, "synced value: %u\n", cJSON_IsTrue(synced));
                 }
                 return cJSON_IsTrue(synced);
            }
        }
    }
    return 0;
}


unsigned int sia_bus_concensus_state_blockheight(siafs_opt_t *opt){
    if(opt->verbose){
        fprintf(stderr, "%s(\"%s\")\n", __func__, opt->url);
    }
    char *json_payload = sia_bus_concensus_state_json(opt);
    if (json_payload != NULL){
        if(opt->verbose){
            fprintf(stderr, "json payload: %s\n", json_payload);
        }
        cJSON *monitor_json = cJSON_Parse(json_payload);
        if (monitor_json == NULL){
            const char *error_ptr = cJSON_GetErrorPtr();
            fprintf(stderr, "Error before: %s\n", error_ptr);
        }
        else{
            cJSON *blockheight = NULL;
            blockheight = cJSON_GetObjectItemCaseSensitive(monitor_json, "blockHeight");
            if (cJSON_IsNumber(blockheight)){
                 if(opt->verbose){
                    fprintf(stderr, "blockheight value: %u\n", blockheight->valueint);
                 }
                 return blockheight->valueint;
            }
        }
    }
    return 0;    
}

char *sia_bus_concensus_state_lastblocktime(siafs_opt_t *opt){
    if(opt->verbose){
        fprintf(stderr, "%s(\"%s\")\n", __func__, opt->url);
    }
    char *json_payload = sia_bus_concensus_state_json(opt);
    if (json_payload != NULL){
        if(opt->verbose){
            fprintf(stderr, "json payload: %s\n", json_payload);
        }
        cJSON *monitor_json = cJSON_Parse(json_payload);
        free(json_payload);
        if (monitor_json == NULL){
            const char *error_ptr = cJSON_GetErrorPtr();
            fprintf(stderr, "Error before: %s\n", error_ptr);
        }
        else{
            cJSON *lastblocktime = NULL;
            lastblocktime = cJSON_GetObjectItemCaseSensitive(monitor_json, "lastBlockTime");
            if (cJSON_IsString(lastblocktime)){
                 if(opt->verbose){
                    fprintf(stderr, "lastblocktime value: %s\n", lastblocktime->valuestring);
                 }
                 return lastblocktime->valuestring;
            }
        }
    }
    return NULL;    
}

char *sia_bus_objects_json(siafs_opt_t *opt, const char *path){
        if(opt->verbose){
        fprintf(stderr, "%s(\"%s, %s\")\n", __func__, opt->url, path);
    }

    char *final_url;
    final_url = malloc( sizeof(opt->unauthenticated_url)*strlen(opt->unauthenticated_url)+
                        sizeof(path)*strlen(path)+
                        sizeof(opt->bucket)*strlen(opt->bucket)+
                        16+8+1);
    strcpy(final_url, opt->unauthenticated_url);
    strcat(final_url, "api/bus/objects/");
    strcat(final_url, path);
    strcat(final_url, "?bucket=");
    strcat(final_url, opt->bucket);

    if(opt->verbose){
        fprintf(stderr, "%s(\"%s\")\n", __func__, final_url);
    }
    
    void *payload = (void *)sia_get_from_cache(final_url);
    sia_http_payload_t http_payload;
    
    if (payload == NULL){
        CURL *curl;
        CURLcode res;
        payload = malloc(255);    // TODO: find a way to predict a suitable size
        curl = curl_easy_init();
        if(curl) {
            curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "GET");
            curl_easy_setopt(curl, CURLOPT_URL, final_url);
            curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
            curl_easy_setopt(curl, CURLOPT_DEFAULT_PROTOCOL, opt->scheme);
            curl_easy_setopt(curl, CURLOPT_USERNAME, opt->user);
            curl_easy_setopt(curl, CURLOPT_PASSWORD, opt->password);
    //        curl_easy_setopt(curl, CURLOPT_HTTPAUTH, CURLAUTH_BASIC);
            struct curl_slist *headers = NULL;
            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, capture_payload);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &http_payload);
            res = curl_easy_perform(curl);
            
        }
        
        if(opt->verbose){
            fprintf(stderr, "%s payload: %s\n", __func__, (char *)http_payload.data);
        }
        sia_set_to_cache(final_url, http_payload.data);
        payload = http_payload.data;

        curl_easy_cleanup(curl);
        free(final_url);
    }
    return payload;
}

unsigned short int sia_bus_objects_is_dir(siafs_opt_t *opt, const char *path){
    if(opt->verbose){
        fprintf(stderr, "%s(\"%s, %s\")\n", __func__, opt->url, path);
    }
    char *json_payload = sia_bus_objects_json(opt, path);
    if (json_payload != NULL){
        if(opt->verbose){
            fprintf(stderr, "json payload: %s\n", json_payload);
        }
        cJSON *monitor_json = cJSON_Parse(json_payload);
        free(json_payload);
        if (monitor_json == NULL){
            const char *error_ptr = cJSON_GetErrorPtr();
            fprintf(stderr, "Error before: %s\n", error_ptr);
        }
        else{
            cJSON *entries = NULL;
            entries = cJSON_GetObjectItemCaseSensitive(monitor_json, "entries");
            if (entries){
                return cJSON_IsArray(entries);
            }
        }
    }
    return 0;
}

unsigned short int sia_bus_objects_is_file(siafs_opt_t *opt, const char *path){
    if(opt->verbose){
        fprintf(stderr, "%s(\"%s, %s\")\n", __func__, opt->url, path);
    }
    char *json_payload = sia_bus_objects_json(opt, path);
    if (json_payload != NULL){
        if(opt->verbose){
            fprintf(stderr, "json payload: %s\n", json_payload);
        }
        cJSON *monitor_json = cJSON_Parse(json_payload);
        free(json_payload);
        if (monitor_json == NULL){
            const char *error_ptr = cJSON_GetErrorPtr();
            fprintf(stderr, "Error before: %s\n", error_ptr);
        }
        else{
            cJSON *object = NULL;
            object = cJSON_GetObjectItemCaseSensitive(monitor_json, "object");
            if (object){
                return cJSON_IsObject(object);
            }
        }
    }
    return 0;
}

unsigned int sia_bus_objects_size(siafs_opt_t *opt, const char *path){
    if(opt->verbose){
        fprintf(stderr, "%s(\"%s, %s\")\n", __func__, opt->url, path);
    }
    char *json_payload = sia_bus_objects_json(opt, path);
    if (json_payload != NULL){
        if(opt->verbose){
            fprintf(stderr, "json payload: %s\n", json_payload);
        }
        cJSON *monitor_json = cJSON_Parse(json_payload);
        free(json_payload);
        if (monitor_json == NULL){
            const char *error_ptr = cJSON_GetErrorPtr();
            fprintf(stderr, "Error before: %s\n", error_ptr);
        }
        else{
            cJSON *object = NULL;
            object = cJSON_GetObjectItemCaseSensitive(monitor_json, "object");
            if (object){
                cJSON *size = cJSON_GetObjectItemCaseSensitive(object, "size");
                if (cJSON_IsNumber(size)){
                    return size->valueint;
                }
            }
        }
    }
    return 0;
}

char *sia_bus_objects_modtime(siafs_opt_t *opt, const char *path){
    if(opt->verbose){
        fprintf(stderr, "%s(\"%s, %s\")\n", __func__, opt->url, path);
    }
    char *json_payload = sia_bus_objects_json(opt, path);
    if (json_payload != NULL){
        if(opt->verbose){
            fprintf(stderr, "json payload: %s\n", json_payload);
        }
        cJSON *monitor_json = cJSON_Parse(json_payload);
        free(json_payload);
        if (monitor_json == NULL){
            const char *error_ptr = cJSON_GetErrorPtr();
            fprintf(stderr, "Error before: %s\n", error_ptr);
        }
        else{
            cJSON *object = NULL;
            object = cJSON_GetObjectItemCaseSensitive(monitor_json, "object");
            if (object){
                cJSON *modtime = cJSON_GetObjectItemCaseSensitive(object, "modTime");
                if (cJSON_IsString(modtime)){
                    return modtime->valuestring;
                }
            }
        }
    }
    return 0;
}


char *sia_worker_objects(siafs_opt_t *opt, const char *path, size_t *size, off_t *offset){
    if(opt->verbose){
        fprintf(stderr, "%s(\"%s\")\n", __func__, opt->url);
    }

    CURL *curl = curl_easy_init();
    char *path2 = NULL;
    if(curl) {
        int i = 0;
        char *encoded_path = curl_easy_unescape(curl, path, strlen(path), &i);
        if(encoded_path) {
            if(opt->verbose){
                printf("Encoded path: %s\n", encoded_path);
            }
            path2 = malloc(sizeof(encoded_path) * i + 1);
            strcpy(path2, encoded_path);
            curl_free(encoded_path);
        }
    curl_easy_cleanup(curl);
    };
    
    // path2 has the encoded file path

    
    char *final_url;
    //http://127.0.0.1:9980/api/worker/objects/test with paces.pdf?bucket=default
    final_url = malloc( sizeof(opt->unauthenticated_url)*strlen(opt->unauthenticated_url)+
                        sizeof(path2)*strlen(path2)+7+
                        sizeof(opt->bucket)*strlen(opt->bucket)+1);
    strcpy(final_url, opt->unauthenticated_url);
    strcat(final_url, "api/worker/objects");
    strcat(final_url, path2);
    strcat(final_url, "?bucket=");
    strcat(final_url, opt->bucket);

    if(opt->verbose){
        fprintf(stderr, "%s(\"%s\")\n", __func__, final_url);
    }
    
    void *payload = (void *)sia_get_from_cache(final_url);
    sia_http_payload_t http_payload;
    
    if (payload == NULL){
        CURL *curl;
        CURLcode res;
        payload = malloc(1024);    // TODO: find a way to predict a suitable size
        curl = curl_easy_init();
        if(curl) {
            curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "GET");
            curl_easy_setopt(curl, CURLOPT_URL, final_url);
            curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
            curl_easy_setopt(curl, CURLOPT_DEFAULT_PROTOCOL, opt->scheme);
            curl_easy_setopt(curl, CURLOPT_USERNAME, opt->user);
            curl_easy_setopt(curl, CURLOPT_PASSWORD, opt->password);
    //        curl_easy_setopt(curl, CURLOPT_HTTPAUTH, CURLAUTH_BASIC);
            if (offset > 0){
                char str[256];
                size_t offset2 = *offset + *size;                   // TODO: review
                sprintf(str, "%ld%s%ld", *offset, "-", offset2);
                curl_easy_setopt(curl, CURLOPT_RANGE, str);
            }
            struct curl_slist *headers = NULL;
            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, capture_payload);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &http_payload);
            res = curl_easy_perform(curl);
            
        }
        
        if(opt->verbose){
            fprintf(stderr, "%s payload: %s\n", __func__, (char *)http_payload.data);
        }
        sia_set_to_cache(final_url, http_payload.data);
        payload = http_payload.data;

        curl_easy_cleanup(curl);
        free(final_url);
    }
    return payload;
}

#ifdef __cplusplus
}
#endif
