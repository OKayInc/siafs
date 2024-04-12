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
    final_url = malloc(sizeof(opt->unauthenticated_url)*strlen(opt->unauthenticated_url)+1);
    strcpy(final_url, opt->unauthenticated_url);
    strcat(final_url, "api/bus/consensus/state");

    if(opt->verbose){
        fprintf(stderr, "%s(\"%s\")\n", __func__, final_url);
    }

    void *payload = (void *)sia_get_from_cache(final_url);
    sia_http_payload_t http_payload;
    http_payload.len = 0;
    http_payload.data = NULL;

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

unsigned short int sia_bus_consensus_state_synced(sia_cfg_t *opt){
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


unsigned int sia_bus_consensus_state_blockheight(sia_cfg_t *opt){
    if(opt->verbose){
        fprintf(stderr, "%s:%d %s(\"%s\")\n", __FILE_NAME__, __LINE__, __func__, opt->url);
    }
    char *json_payload = sia_bus_consensus_state_json(opt);
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

char *sia_bus_consensus_state_lastblocktime(sia_cfg_t *opt){
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
                 return lastblocktime->valuestring;
            }
        }
    }
    return NULL;
}


#ifdef __cplusplus
}
#endif
