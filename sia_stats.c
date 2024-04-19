#ifdef __cplusplus
extern "C"
{
#endif

#include "sia_stats.h"
extern sia_cfg_t opt;

char *sia_stats_json(sia_cfg_t *opt){
    if(opt->verbose){
        fprintf(stderr, "%s:%d %s(\"%s\")\n", __FILE_NAME__, __LINE__, __func__, opt->url);
    }

    char *final_url = "https://sia.tech/api/stats";
    if(opt->verbose){
        fprintf(stderr, "%s:%d %s(\"%s\")\n", __FILE_NAME__, __LINE__, __func__, final_url);
    }

//    void *payload = (void *)sia_get_from_cache(final_url);
    void *payload = NULL;
    sia_http_payload_t http_payload;
    http_payload.len = 0;
    http_payload.data = NULL;

    if (payload == NULL){
        CURL *curl;
        CURLcode res;
        curl = curl_easy_init();
        if(curl) {
            curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "GET");
            curl_easy_setopt(curl, CURLOPT_URL, final_url);
            curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
            curl_easy_setopt(curl, CURLOPT_DEFAULT_PROTOCOL, "https");
            curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
            curl_easy_setopt(curl, CURLOPT_TLS13_CIPHERS, "TLS_CHACHA20_POLY1305_SHA256");
            if(opt->verbose){
                curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
            }
            struct curl_slist *headers = NULL;
            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, capture_payload);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &http_payload);
//            curl_easy_setopt(curl, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_0);
            res = curl_easy_perform(curl);
        }

        if(opt->verbose){
            fprintf(stderr, "%s:%d %s payload: %s\n", __FILE_NAME__, __LINE__, __func__, (char *)http_payload.data);
        }
//        sia_set_to_cache(final_url, http_payload.data);
        payload = http_payload.data;

        curl_easy_cleanup(curl);
    }
    return payload;
}

unsigned long long int sia_stats_totalStorage(sia_cfg_t *opt){
    unsigned long long int answer = 0;
    double answerd = 0.0;

    char *json_payload = sia_stats_json(opt);
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
            cJSON *totalStorage = NULL;
            totalStorage = cJSON_GetObjectItemCaseSensitive(monitor_json, "totalStorage");
            if (totalStorage != NULL){
                if (cJSON_IsString(totalStorage)){
                    if(opt->verbose){
                        fprintf(stderr, "%s:%d totalStorage: %s\n", __FILE_NAME__, __LINE__, totalStorage->valuestring);
                    }
                    // 6.74 PB
                    // TODO: Convert PB prefix to bytes
                    char *ptr;
                    answerd = strtof(totalStorage->valuestring, &ptr);
                    unsigned long long int m = 1;
                    if (strcmp(ptr+1, "PB") == 0){
                        m = 1000000000000000;
                    } else if (strcmp(ptr+1, "TB") == 0){
                        m = 1000000000000;
                    }
                    else if (strcmp(ptr+1, "GB") == 0){
                        m = 1000000000;
                    }
                    else if (strcmp(ptr+1, "MB") == 0){
                        m = 1000000;
                    }
                    else if (strcmp(ptr+1, "kB") == 0){
                        m = 1000;
                    }
                    answer = m * answerd;
                    if(opt->verbose){
                        fprintf(stderr, "%s:%d bytes: %.2f [%s]\n", __FILE_NAME__, __LINE__, answerd, ptr+1);
                        fprintf(stderr, "%s:%d bytes: %lu\n", __FILE_NAME__, __LINE__, answer);
                    }
                }
            }
            cJSON_Delete(monitor_json);
        }
    }

    return answer;
}

#ifdef __cplusplus
}
#endif
