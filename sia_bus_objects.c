#ifdef __cplusplus
extern "C"
{
#endif

#include "sia_bus_objects.h"
extern sia_cfg_t opt;

char *sia_bus_objects_json(sia_cfg_t *opt, const char *path){
    if(opt->verbose){
        fprintf(stderr, "%s:%d %s(\"%s\", \"%s\")\n", __FILE_NAME__, __LINE__, __func__, opt->url, path);
    }

    char *final_url;
    final_url = malloc( sizeof(opt->unauthenticated_url)*strlen(opt->unauthenticated_url)+
                        sizeof(path)*strlen(path)+
                        sizeof(opt->bucket)*strlen(opt->bucket)+
                        16+8+1);
    strcpy(final_url, opt->unauthenticated_url);
    strcat(final_url, "api/bus/objects");
    strcat(final_url, path);
    strcat(final_url, "?bucket=");
    strcat(final_url, opt->bucket);

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
            curl_easy_setopt(curl, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_0);
            res = curl_easy_perform(curl);

        }

        if(opt->verbose){
            fprintf(stderr, "%s:%d %s payload: %s\n", __FILE_NAME__, __LINE__, __func__, (char *)http_payload.data);
        }
//        sia_set_to_cache(final_url, http_payload.data);
        payload = http_payload.data;

        curl_easy_cleanup(curl);
        free(final_url);
    }
    return payload;
}

unsigned short int sia_bus_objects_is_dir(sia_cfg_t *opt, const char *path){
    if(opt->verbose){
        fprintf(stderr, "%s:%d %s(\"%s\", \"%s\")\n", __FILE_NAME__, __LINE__, __func__, opt->url, path);
    }
    // Add a / to the end
    char *path2 = NULL;
    char ch = '/';
    path2 = malloc(sizeof(char) * strlen(path) + 1);
    strcpy(path2, path);
    strncat(path2, &ch, 1);
    char *json_payload = sia_bus_objects_json(opt, path2);
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
            cJSON *entries = NULL;
            entries = cJSON_GetObjectItemCaseSensitive(monitor_json, "entries");
            if (entries){
                return cJSON_IsArray(entries);
            }
        }
    }
    free(path2);
    return 0;
}

unsigned short int sia_bus_objects_is_file(sia_cfg_t *opt, const char *path){
    if(opt->verbose){
        fprintf(stderr, "%s:%d %s(\"%s\", \"%s\")\n", __FILE_NAME__, __LINE__, __func__, opt->url, path);
    }

    char *json_payload = sia_bus_objects_json(opt, path);
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
            cJSON *object = NULL;
            object = cJSON_GetObjectItemCaseSensitive(monitor_json, "object");
            if (object){
                return cJSON_IsObject(object);
            }
        }
        free(json_payload);
    }
    return 0;
}

unsigned int sia_bus_objects_size(sia_cfg_t *opt, const char *path){
    if(opt->verbose){
        fprintf(stderr, "%s:%d %s(\"%s\", \"%s\")\n", __FILE_NAME__, __LINE__, __func__, opt->url, path);
    }
    char *json_payload = sia_bus_objects_json(opt, path);
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

char *sia_bus_objects_modtime(sia_cfg_t *opt, const char *path){
    if(opt->verbose){
        fprintf(stderr, "%s:%d %s(\"%s\", \"%s\")\n", __FILE_NAME__, __LINE__, __func__, opt->url, path);
    }

    char *json_payload = sia_bus_objects_json(opt, path);
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
#ifdef __cplusplus
}
#endif
