#ifdef __cplusplus
extern "C"
{
#endif
#include "sia_bus_consensus.h"

extern sia_cfg_t opt;

char *sia_bus_consensus_state_json(sia_cfg_t *opt){
    if(opt->verbose){
        fprintf(stderr, "%s:%d %s(\"%s\")\n", __FILE_NAME__, __LINE__, __func__, opt->url);
    }

    char *final_url;
    final_url = calloc(strlen(opt->unauthenticated_url)+24, sizeof(char));
    strcpy(final_url, opt->unauthenticated_url);
    strcat(final_url, "api/bus/consensus/state");

    if(opt->verbose){
        fprintf(stderr, "%s(\"%s\")\n", __func__, final_url);
    }

//    void *payload = (void *)sia_get_from_cache(final_url);
    void *payload = NULL;
    sia_http_payload_t http_payload;
    http_payload.len = 0;
    http_payload.data = NULL;

#ifdef SIA_MEMCACHED
    char *key = NULL;
    if (opt->L1 != NULL){
        key = opt->L1->key("api/bus/consensus/state", "*SIA*", __func__);
        if(opt->verbose){
            fprintf(stderr, "%s:%d MC Key: %s\n", __FILE_NAME__, __LINE__, key);
        }
        unsigned long int *payload_len = (unsigned long int *)calloc(1, sizeof(unsigned long int));
        memcached_return rc = opt->L1->get(opt->memc, key, &payload, payload_len);
        if ((rc != MEMCACHED_SUCCESS) || (payload == NULL)){
            payload = NULL;
        }
        else if(opt->verbose){
            fprintf(stderr, "%s:%d MC Key: %s FOUND: %s\n", __FILE_NAME__, __LINE__, key, (char *)payload);
        }
        free(payload_len);
    }
#endif

    if (payload == NULL){
        CURL *curl = curl_easy_init();
        CURLcode res;
        if(curl) {
            curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "GET");
            curl_easy_setopt(curl, CURLOPT_URL, final_url);
            curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
            curl_easy_setopt(curl, CURLOPT_DEFAULT_PROTOCOL, opt->scheme);
            curl_easy_setopt(curl, CURLOPT_USERNAME, opt->user);
            curl_easy_setopt(curl, CURLOPT_PASSWORD, opt->password);
    //        curl_easy_setopt(curl, CURLOPT_HTTPAUTH, CURLAUTH_BASIC);
            if(opt->verbose){
                curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
            }
            struct curl_slist *headers = NULL;
            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, capture_payload);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &http_payload);
            res = curl_easy_perform(curl);
            if(res != CURLE_OK){
                fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
            }
            if(opt->verbose){
                fprintf(stderr, "%s:%d payload: %s\n", __FILE_NAME__, __LINE__, (char *)http_payload.data);
            }
            // sia_set_to_cache(final_url, http_payload.data);
            payload = http_payload.data;

#ifdef SIA_MEMCACHED
            if (opt->L1 != NULL){
                memcached_return rc = opt->L1->set(opt->memc, key, http_payload.data, http_payload.len, SIA_BLOCKCHAIN_TTL);
                if (rc != MEMCACHED_SUCCESS){
        //            payload = NULL;
                }
                else if(opt->verbose){
                    fprintf(stderr, "%s:%d MC Key: %s SET\n", __FILE_NAME__, __LINE__, key);
                }
            }
#endif
            curl_easy_cleanup(curl);
        }
    }
    free(final_url);
#ifdef SIA_MEMCACHED
    if (key != NULL){
        free(key);
        key = NULL;
    }
#endif
    return payload;
}

unsigned short int sia_bus_consensus_state_synced(sia_cfg_t *opt){
    unsigned short int answer = 0;
    if(opt->verbose){
        fprintf(stderr, "%s:%d %s(\"%s\")\n", __FILE_NAME__, __LINE__, __func__, opt->url);
    }
    char *json_payload = sia_bus_consensus_state_json(opt);
    if (json_payload != NULL){
        if(opt->verbose){
            fprintf(stderr, "%s:%d json payload: %s\n", __FILE_NAME__, __LINE__, json_payload);
        }
        cJSON *monitor_json = cJSON_Parse(json_payload);
        free(json_payload);
        if (monitor_json == NULL){
            const char *error_ptr = cJSON_GetErrorPtr();
            fprintf(stderr, "%s:%d Error before: %s\n", __FILE_NAME__, __LINE__, error_ptr);
        }
        else{
            cJSON *synced = NULL;
            synced = cJSON_GetObjectItemCaseSensitive(monitor_json, "synced");
            if (cJSON_IsBool(synced)){
                 if(opt->verbose){
                    fprintf(stderr, "%s:%d synced value: %u\n", __FILE_NAME__, __LINE__, cJSON_IsTrue(synced));
                 }
                 answer = cJSON_IsTrue(synced);
            }
            cJSON_Delete(monitor_json);
        }
    }
    return answer;
}

unsigned int sia_bus_consensus_state_blockheight(sia_cfg_t *opt){
    unsigned int answer = 0;
    if(opt->verbose){
        fprintf(stderr, "%s:%d %s(\"%s\")\n", __FILE_NAME__, __LINE__, __func__, opt->url);
    }
    char *json_payload = sia_bus_consensus_state_json(opt);
    if (json_payload != NULL){
        if(opt->verbose){
            fprintf(stderr, "%s:%d json payload: %s\n", __FILE_NAME__, __LINE__, json_payload);
        }
        cJSON *monitor_json = cJSON_Parse(json_payload);
        if (monitor_json == NULL){
            const char *error_ptr = cJSON_GetErrorPtr();
            fprintf(stderr, "%s:%d Error before: %s\n", __FILE_NAME__, __LINE__, error_ptr);
        }
        else{
            cJSON *blockheight = NULL;
            blockheight = cJSON_GetObjectItemCaseSensitive(monitor_json, "blockHeight");
            if (cJSON_IsNumber(blockheight)){
                 if(opt->verbose){
                    fprintf(stderr, "%s:%d blockheight value: %u\n", __FILE_NAME__, __LINE__, blockheight->valueint);
                 }
                 answer =  blockheight->valueint;
            }
            cJSON_Delete(monitor_json);
        }
    }
    return answer;
}

char *sia_bus_consensus_state_lastblocktime(sia_cfg_t *opt){
    char *answer = NULL;
    if(opt->verbose){
        fprintf(stderr, "%s:%d %s(\"%s\")\n", __FILE_NAME__, __LINE__, __func__, opt->url);
    }
    char *json_payload = sia_bus_consensus_state_json(opt);
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
                 answer = malloc(sizeof(lastblocktime->valuestring) * strlen(lastblocktime->valuestring) + 1);
                 strcpy(answer, lastblocktime->valuestring);
            }
        }
        cJSON_Delete(monitor_json);
    }
    return answer;
}

#ifdef __cplusplus
}
#endif
