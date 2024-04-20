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

    CURL *curl = curl_easy_init();
    char *path2 = NULL;
    if(curl) {
        char *encoded_path = curl_easy_escape(curl, path, strlen(path));
        if(encoded_path) {
            if(opt->verbose){
                fprintf(stderr, "%s:%d Encoded path: %s\n", __FILE_NAME__, __LINE__, encoded_path);
            }
            path2 = malloc(sizeof(encoded_path) * strlen(encoded_path) + 1);
            strcpy(path2, encoded_path);
            curl_free(encoded_path);
        }
        curl_easy_cleanup(curl);
    };

    // path2 has the encoded file path
    char *final_url;
    final_url = malloc( sizeof(opt->unauthenticated_url)*strlen(opt->unauthenticated_url)+
                        sizeof(path2)*strlen(path2)+
                        sizeof(opt->bucket)*strlen(opt->bucket)+
                        16+8+1);
    strcpy(final_url, opt->unauthenticated_url);
    strcat(final_url, "api/bus/objects");
    strcat(final_url, path2);
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
        curl = curl_easy_init();
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
        final_url = NULL;
    }
    return payload;
}

char *sia_bus_objects_list_json(sia_cfg_t *opt, const char *path){
    if(opt->verbose){
        fprintf(stderr, "%s:%d %s(\"%s\", \"%s\")\n", __FILE_NAME__, __LINE__, __func__, opt->url, path);
    }

    char *final_url;
    final_url = malloc( sizeof(opt->unauthenticated_url)*(strlen(opt->unauthenticated_url)+
                        21+1));
    strcpy(final_url, opt->unauthenticated_url);
    strcat(final_url, "api/bus/objects/list");

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
            curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "POST");
            curl_easy_setopt(curl, CURLOPT_URL, final_url);
            curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
            curl_easy_setopt(curl, CURLOPT_DEFAULT_PROTOCOL, opt->scheme);
            curl_easy_setopt(curl, CURLOPT_USERNAME, opt->user);
            curl_easy_setopt(curl, CURLOPT_PASSWORD, opt->password);
//          curl_easy_setopt(curl, CURLOPT_HTTPAUTH, CURLAUTH_BASIC);
            if(opt->verbose){
                curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
            }
            struct curl_slist *headers = NULL;
            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, capture_payload);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &http_payload);
            curl_easy_setopt(curl, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_0);
            char *post_data_format = "{\n \"bucket\": \"%s\",\n \"limit\": -1,\n \"prefix\": \"%s\",\n \"marker\": \"\"\n}";
            char *post_data = malloc(sizeof(char) * (strlen(post_data_format) + strlen(opt->bucket) + strlen(path)) + 1);
            sprintf(post_data, post_data_format, opt->bucket, path);
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post_data);
            res = curl_easy_perform(curl);
            free(post_data);
        }

        if(opt->verbose){
            fprintf(stderr, "%s:%d %s payload: %s\n", __FILE_NAME__, __LINE__, __func__, (char *)http_payload.data);
        }
//        sia_set_to_cache(final_url, http_payload.data);
        payload = http_payload.data;

        curl_easy_cleanup(curl);
        free(final_url);
        final_url = NULL;
    }

    return payload;
}

unsigned short int sia_bus_objects_exists(sia_cfg_t *opt, const char *path){
    unsigned short int answer = 0;
    if(opt->verbose){
        fprintf(stderr, "%s:%d %s(\"%s\", \"%s\")\n", __FILE_NAME__, __LINE__, __func__, opt->url, path);
    }

    char *json_payload = sia_bus_objects_list_json(opt, path);
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
            cJSON *objects = NULL;
            cJSON *object = NULL;
            objects = cJSON_GetObjectItemCaseSensitive(monitor_json, "objects");
            if (objects != NULL){
                cJSON_ArrayForEach(object, objects){
                    cJSON *name = cJSON_GetObjectItemCaseSensitive(object, "name");
                    cJSON *size = cJSON_GetObjectItemCaseSensitive(object, "size");
                    if (!strcmp(name->valuestring, path)){
                        if(opt->verbose){
                            fprintf(stderr, "%s:%d Found: %s\n", __FILE_NAME__, __LINE__, name->valuestring);
                        }
                        answer = 1;
                        break;
                    }
                }
                cJSON_Delete(monitor_json);
            }
            else{
                const char *error_ptr = cJSON_GetErrorPtr();
                fprintf(stderr, "%s:%d Error before: %s\n", __FILE_NAME__, __LINE__, error_ptr);
            }
        }
    }
    return answer;
}

unsigned short int sia_bus_objects_is_dir(sia_cfg_t *opt, const char *path){
    unsigned short int answer = 0;
    if(opt->verbose){
        fprintf(stderr, "%s:%d %s(\"%s\", \"%s\")\n", __FILE_NAME__, __LINE__, __func__, opt->url, path);
    }

    char *path2 = NULL;
    if (path[(strlen(path) - 1)] != '/'){
        // Add a / to the end
        char ch = '/';
        path2 = malloc(sizeof(char) * strlen(path) + 2);
        strcpy(path2, path);
        strncat(path2, &ch, 1);
    }
    else{
        path2 = malloc(sizeof(char) * strlen(path) + 1);
        strcpy(path2, path);
    }

    char *json_payload = sia_bus_objects_list_json(opt, path2);
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
            cJSON *objects = NULL;
            cJSON *object = NULL;
            objects = cJSON_GetObjectItemCaseSensitive(monitor_json, "objects");
            if (objects){
                cJSON_ArrayForEach(object, objects){
                    cJSON *name = cJSON_GetObjectItemCaseSensitive(object, "name");
                    cJSON *size = cJSON_GetObjectItemCaseSensitive(object, "size");
                    if (!strcmp(name->valuestring, path2) && (size->valueint == 0)){
                        answer =  1;
                    }
                }
            }
            cJSON_Delete(monitor_json);
        }
    }
    free(path2);
    return answer;
}

unsigned short int sia_bus_objects_is_file(sia_cfg_t *opt, const char *path){
    unsigned short int answer = 0;
    sia_metacache_t *meta = NULL;
    if(opt->verbose){
        fprintf(stderr, "%s:%d %s(\"%s\", \"%s\")\n", __FILE_NAME__, __LINE__, __func__, opt->url, path);
    }

    meta = find_meta_by_path(opt, path);
    if (meta != NULL){
        if (meta->expire < (time(NULL) + SIA_METACACHE_TTL)){
            // Evaluate
            if (meta->type == SIA_FILE){
                answer = 1;
            }
            return answer;
        }
        else{
            // Expired
            del_meta(opt, meta);
        }
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
                answer = cJSON_IsObject(object);
            }
            cJSON_Delete(monitor_json);
        }
    }
    return answer;
}

unsigned long int sia_bus_object_size(sia_cfg_t *opt, const char *path){
    unsigned long int answer = 0;
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
                    answer = size->valueint;
                }
            }
            cJSON_Delete(monitor_json);
        }
    }
    return answer;
}

char *sia_bus_objects_modtime(sia_cfg_t *opt, const char *path){
    char *answer = NULL;
    sia_object_type_t type = SIA_UNKOWN;
    if(opt->verbose){
        fprintf(stderr, "%s:%d %s(\"%s\", \"%s\")\n", __FILE_NAME__, __LINE__, __func__, opt->url, path);
    }

    char *json_payload = NULL;
    char *path2 = NULL;
    if (sia_bus_objects_is_file(opt, path) == 1 ){
        type = SIA_FILE;
        json_payload = sia_bus_objects_json(opt, path);
    }
    else if (sia_bus_objects_is_dir(opt, path) == 1 ){
        type = SIA_DIR;
        if (path[(strlen(path) - 1)] != '/'){
            // Add a / to the end
            char ch = '/';
            path2 = malloc(sizeof(char) * strlen(path) + 2);
            strcpy(path2, path);
            strncat(path2, &ch, 1);
        }
        else{
            path2 = malloc(sizeof(char) * strlen(path) + 1);
            strcpy(path2, path);
        }
        json_payload = sia_bus_objects_list_json(opt, path2);
    }
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
            if (type == SIA_FILE){
                cJSON *object = NULL;
                object = cJSON_GetObjectItemCaseSensitive(monitor_json, "object");
                if (object){
                    cJSON *modtime = cJSON_GetObjectItemCaseSensitive(object, "modTime");
                    if (cJSON_IsString(modtime)){
                        answer = malloc(sizeof(modtime->valuestring) * strlen(modtime->valuestring) + 1);
                        strcpy(answer, modtime->valuestring);
                    }
                }
            }
            else if (type == SIA_DIR){
                cJSON *objects = NULL;
                cJSON *object = NULL;
                objects = cJSON_GetObjectItemCaseSensitive(monitor_json, "objects");
                if (objects){
                    cJSON_ArrayForEach(object, objects){
                        cJSON *name = cJSON_GetObjectItemCaseSensitive(object, "name");
                        cJSON *size = cJSON_GetObjectItemCaseSensitive(object, "size");
                        if (!strcmp(name->valuestring, path2) && (size->valueint == 0)){
                            // Directory found
                            cJSON *modtime = cJSON_GetObjectItemCaseSensitive(object, "modTime");
                            if (cJSON_IsString(modtime)){
                                answer = malloc(sizeof(modtime->valuestring) * strlen(modtime->valuestring) + 1);
                                strcpy(answer, modtime->valuestring);
                            }
                        }
                    }
                }

            }
            cJSON_Delete(monitor_json);
        }
    }

    if (path2 != NULL)
        free(path2);

    return answer;
}

time_t sia_bus_objects_unixtime(sia_cfg_t *opt, const char *path){
    time_t answer = 0;
    char timestamp[32] = {0};
    char *time_payload = sia_bus_objects_modtime(opt, path);
    strcpy(timestamp, sia_bus_objects_modtime(opt, path));
    free(time_payload);
    if(opt->verbose){
        fprintf(stderr, "%s:%d Mod time: %s\n", __FILE_NAME__, __LINE__, timestamp);
    }
    struct tm tm;
    memset(&tm, 0, sizeof(struct tm));
    // 2024-04-20T03:34:36.367719838Z
    if (strptime(timestamp, "%Y-%m-%dT%H:%M:%S%Z", &tm) != NULL ){
        answer = mktime(&tm);
        if(opt->verbose){
            fprintf(stderr, "%s:%d mtime: %lu\n", __FILE_NAME__, __LINE__, answer);
        }
    }
    return answer;
}
char *sia_bus_del_object(sia_cfg_t *opt, const char *path){
    if(opt->verbose){
        fprintf(stderr, "%s:%d %s(\"%s\", \"%s\")\n", __FILE_NAME__, __LINE__, __func__, opt->url, path);
    }

    char *final_url;
    final_url = malloc( sizeof(opt->unauthenticated_url)*(strlen(opt->unauthenticated_url)+
                        15+
                        sizeof(path)*(strlen(path))+
                        8+
                        sizeof(opt->bucket)*(strlen(opt->bucket))+
                        1));
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
        curl = curl_easy_init();
        if(curl) {
            curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "DELETE");
            curl_easy_setopt(curl, CURLOPT_URL, final_url);
            curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
            curl_easy_setopt(curl, CURLOPT_DEFAULT_PROTOCOL, opt->scheme);
            curl_easy_setopt(curl, CURLOPT_USERNAME, opt->user);
            curl_easy_setopt(curl, CURLOPT_PASSWORD, opt->password);
//          curl_easy_setopt(curl, CURLOPT_HTTPAUTH, CURLAUTH_BASIC);
            if(opt->verbose){
                curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
            }
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
        final_url = NULL;
    }

    return payload;
}

char *sia_bus_rename_object(sia_cfg_t *opt, const char *from, const char *to, const char *mode){
    if(opt->verbose){
        fprintf(stderr, "%s:%d %s(\"%s\", \"%s\", \"%s\")\n", __FILE_NAME__, __LINE__, __func__, opt->url, from, to);
    }

    char *final_url;
    final_url = malloc( sizeof(opt->unauthenticated_url)*(strlen(opt->unauthenticated_url)+
                        22+
                        1));
    strcpy(final_url, opt->unauthenticated_url);
    strcat(final_url, "api/bus/objects/rename");

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
            curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "POST");
            curl_easy_setopt(curl, CURLOPT_URL, final_url);
            curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
            curl_easy_setopt(curl, CURLOPT_DEFAULT_PROTOCOL, opt->scheme);
            curl_easy_setopt(curl, CURLOPT_USERNAME, opt->user);
            curl_easy_setopt(curl, CURLOPT_PASSWORD, opt->password);
//          curl_easy_setopt(curl, CURLOPT_HTTPAUTH, CURLAUTH_BASIC);
            if(opt->verbose){
                curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
            }
            struct curl_slist *headers = NULL;
            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, capture_payload);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &http_payload);
            curl_easy_setopt(curl, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_0);
            char *post_data_format = "{\n    \"bucket\": \"%s\",\n    \"from\": \"%s\",\n    \"to\": \"%s\",\n    \"mode\": \"%s\",\n    \"force\": false}";
            char *post_data = malloc(sizeof(char) * (strlen(post_data_format) + strlen(opt->bucket) + strlen(from) + strlen(to) + strlen(mode)) + 1);
            sprintf(post_data, post_data_format, opt->bucket, from, to, mode);
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post_data);
            res = curl_easy_perform(curl);
        }

        if(opt->verbose){
            fprintf(stderr, "%s:%d %s payload: %s\n", __FILE_NAME__, __LINE__, __func__, (char *)http_payload.data);
        }
//        sia_set_to_cache(final_url, http_payload.data);
        payload = http_payload.data;

        curl_easy_cleanup(curl);
        free(final_url);
        final_url = NULL;
    }

    return payload;
}

unsigned long int sia_bus_used_storage_per_directory(sia_cfg_t *opt, const char *path){
    unsigned long int answer = 0;
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
            cJSON *entry = NULL;
            cJSON *entries = NULL;
            entries = cJSON_GetObjectItemCaseSensitive(monitor_json, "entries");
            cJSON_ArrayForEach(entry, entries){
                if (cJSON_IsObject(entry)){
                    cJSON *size = cJSON_GetObjectItemCaseSensitive(entry, "size");
                    if (cJSON_IsNumber(size)){
                        answer += size->valueint;
                        if(opt->verbose){
                            cJSON *name = cJSON_GetObjectItemCaseSensitive(entry, "name");
                            fprintf(stderr, "%s:%d Found: %s [%ul]\n", __FILE_NAME__, __LINE__, name->valuestring, size->valueint);
                        }

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
