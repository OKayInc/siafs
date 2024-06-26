#ifdef __cplusplus
extern "C"
{
#endif

#include "sia_bus_multiparts.h"
extern sia_cfg_t opt;

char *sia_bus_get_uploadid(sia_cfg_t *opt, const char *path){
    char *answer = NULL;
    if(opt->verbose){
        fprintf(stderr, "%s:%d %s(\"%s\",\"%s\")\n", __FILE_NAME__, __LINE__, __func__, opt->url, path);
    }
    char *uploadid = malloc(65*sizeof(char));

    char *json_payload = sia_bus_multipart_listuploads_json(opt, path);
    short int m = 1;
    short int found = 0;
    // Process the JSON
    cJSON *monitor_json = cJSON_Parse(json_payload);
    free(json_payload);
    if (monitor_json == NULL){
        const char *error_ptr = cJSON_GetErrorPtr();
        fprintf(stderr, "Error before: %s\n", error_ptr);
    }
    else{
        // Recovered ID
        cJSON *uploads = cJSON_GetObjectItemCaseSensitive(monitor_json, "uploads");
        if (cJSON_IsArray(uploads)){
            cJSON *upload = NULL;
            cJSON_ArrayForEach(upload, uploads){
                if (cJSON_IsObject(upload)){
                    cJSON *juploadID = cJSON_GetObjectItemCaseSensitive(upload, "uploadID");
                    cJSON *jpath = cJSON_GetObjectItemCaseSensitive(upload, "path");
                    if (cJSON_IsString(juploadID) && cJSON_IsString(jpath)){
                        if(opt->verbose){
                            fprintf(stderr, "%s:%d path: %s uploadID: %s\n", __FILE_NAME__, __LINE__, jpath->valuestring, juploadID->valuestring);
                        }
                        if (!strcmp(path, jpath->valuestring)){
                            answer = malloc(sizeof(juploadID->valuestring) * strlen(juploadID->valuestring) + 1);
                            strcpy(answer, juploadID->valuestring);
                            found = 1;
                        }
                    }
                }
            }
        }
        cJSON_Delete(monitor_json);
    }

    if (!found){
        m++;
        json_payload = sia_bus_multipart_create_json(opt, path, "0000000000000000000000000000000000000000000000000000000000000000");

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
                // New
                cJSON *juploadID = cJSON_GetObjectItemCaseSensitive(monitor_json, "uploadID");
                if (cJSON_IsString(juploadID)){
                    if(opt->verbose){
                        fprintf(stderr, "%s:%d uploadID: %s\n", __FILE_NAME__, __LINE__, juploadID->valuestring);
                    }
                    answer = malloc(sizeof(juploadID->valuestring) * strlen(juploadID->valuestring) + 1);
                    strcpy(answer, juploadID->valuestring);
                }
                cJSON_Delete(monitor_json);
            }
        }
    }
    return answer;
}

char *sia_bus_multipart_abort_json(sia_cfg_t *opt, const char *path, const char *uploadid){
    if(opt->verbose){
       fprintf(stderr, "%s:%d %s(\"%s\",\"%s\",\"%s\")\n", __FILE_NAME__, __LINE__, __func__, opt->url, path, uploadid);
     }

    char *final_url;
    final_url = malloc( sizeof(opt->unauthenticated_url)*(strlen(opt->unauthenticated_url)+
                        24+1));
    strcpy(final_url, opt->unauthenticated_url);
    strcat(final_url, "api/bus/multipart/abort");

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
            char *post_data_format ="{\n    \"bucket\": \"%s\",\n    \"path\": \"%s\",\n    \"uploadID\": \"%s\"\n}";
            char *post_data = malloc(sizeof(char) * (strlen(post_data_format) + strlen(opt->bucket) + strlen(path) + strlen(uploadid)) + 1);
            sprintf(post_data, post_data_format, opt->bucket, path, uploadid);
            if(opt->verbose){
                fprintf(stderr, "%s:%d %s post_data: %s\n", __FILE_NAME__, __LINE__, __func__, (char *)post_data);
            }
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post_data);
            res = curl_easy_perform(curl);
            if(res != CURLE_OK){
                fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
            }
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

char *sia_bus_multipart_complete_create_json_payload(sia_cfg_t *opt, const char *path, const char *uploadid){
    char *answer = NULL;
    if(opt->verbose){
       fprintf(stderr, "%s:%d %s(\"%s\",\"%s\",\"%s\")\n", __FILE_NAME__, __LINE__, __func__, opt->url, path, uploadid);
    }
    cJSON *monitor = cJSON_CreateObject();
    if (monitor != NULL){
        /**
         {
  "bucket": "multipart",
  "path": "/foo",
  "uploadID": "096473fd324b681e2c3a6df7b69b046e5b10b5a1610de52201705fb33868920d",
  "Parts": [
    {
      "partNumber": 1,
      "eTag": "324dcf027dd4a30a932c441f365a25e86b173defa4b8e58948253471b81b72cf"
    },
    {
      "partNumber": 2,
      "eTag": "9a3440c9d1529b122faceef33739b6e814616658d53faaf6e4f129fb20edfb13"
    },
    {
      "partNumber": 3,
      "eTag": "0268be9dbd0446eaa217e1dec8f399249305e551d7fc1437dd84521f74aa621c"
    }
  ]
}
        **/
        cJSON *jbucket = NULL;
        jbucket = cJSON_CreateString(opt->bucket);
        if (jbucket != NULL){
            cJSON_AddItemToObject(monitor, "bucket", jbucket);
        }

        cJSON *jpath = NULL;
        jpath = cJSON_CreateString(path);
        if (jpath != NULL){
            cJSON_AddItemToObject(monitor, "path", jpath);
        }

        cJSON *juploadid = NULL;
        juploadid = cJSON_CreateString(uploadid);
        if (juploadid != NULL){
            cJSON_AddItemToObject(monitor, "uploadID", juploadid);
        }

        cJSON *jparts = NULL;
        jparts = cJSON_CreateArray();
        if (jparts != NULL){
            cJSON_AddItemToObject(monitor, "Parts", jparts);
            sia_upload_t *upl = find_upload_by_path(opt, path);
            if (upl != NULL){
                cJSON *jpart = NULL;
                for (unsigned int i = 0; i < SIA_MAX_PARTS; i++){
                    if (upl->part[i].etag != NULL){
                        jpart = cJSON_CreateObject();
                        if (jpart != NULL){
                            cJSON_AddItemToArray(jparts, jpart);

                            cJSON *jpartnumber = NULL;
                            jpartnumber = cJSON_CreateNumber(i + 1);
                            if (jpartnumber != NULL){
                                cJSON_AddItemToObject(jpart, "partNumber", jpartnumber);
                            }

                            cJSON *jetag = NULL;
                            jetag = cJSON_CreateString(upl->part[i].etag);
                            if (jetag != NULL){
                                cJSON_AddItemToObject(jpart, "eTag", jetag);
                            }
                        }
                    }
                }
            }
        }

        if (cJSON_Print(monitor) == NULL){
            fprintf(stderr, "%s:%d Failed to print monitor.\n", __FILE_NAME__, __LINE__);
        }
        else{
            answer = malloc(sizeof(answer) * strlen(cJSON_Print(monitor)) + 1);
            strcpy(answer, cJSON_Print(monitor));
            if(opt->verbose){
                for (sia_upload_t *upload = opt->uploads; upload != NULL; upload = upload->next){
                    fprintf(stderr, "%s:%d name: %s %s\n", __FILE_NAME__, __LINE__, upload->name, upload->uploadID);
                    for (int i = 0; (i < SIA_MAX_PARTS) && (upload->part[i].etag != NULL); i++){
                        fprintf(stderr, "%s:%d\t%d: %s\n", __FILE_NAME__, __LINE__, i, upload->part[i].etag);
                    }
                }
                fprintf(stderr, "%s:%d multipart complete %s\n", __FILE_NAME__, __LINE__, answer);
            }
        }
        cJSON_Delete(monitor);
    }
    return answer;
}

char *sia_bus_multipart_complete_json(sia_cfg_t *opt, const char *path, const char *uploadid){
    if(opt->verbose){
       fprintf(stderr, "%s:%d %s(\"%s\",\"%s\",\"%s\")\n", __FILE_NAME__, __LINE__, __func__, opt->url, path, uploadid);
     }

    char *final_url;
    final_url = malloc( sizeof(opt->unauthenticated_url)*(strlen(opt->unauthenticated_url)+
                        26+1));
    strcpy(final_url, opt->unauthenticated_url);
    strcat(final_url, "api/bus/multipart/complete");

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
            // build JSON usint cJSON
            // "{\n  \"bucket\": \"multipart\",\n  \"path\": \"/foo\",\n  \"uploadID\": \"096473fd324b681e2c3a6df7b69b046e5b10b5a1610de52201705fb33868920d\",\n  \"Parts\": [\n    {\n      \"partNumber\": 1,\n      \"eTag\": \"324dcf027dd4a30a932c441f365a25e86b173defa4b8e58948253471b81b72cf\"\n    },\n    {\n      \"partNumber\": 2,\n      \"eTag\": \"9a3440c9d1529b122faceef33739b6e814616658d53faaf6e4f129fb20edfb13\"\n    },\n    {\n      \"partNumber\": 3,\n      \"eTag\": \"0268be9dbd0446eaa217e1dec8f399249305e551d7fc1437dd84521f74aa621c\"\n    }\n  ]\n}";
            char *post_data = sia_bus_multipart_complete_create_json_payload(opt, path, uploadid);
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post_data);
            res = curl_easy_perform(curl);
            if(res != CURLE_OK){
                fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
            }
            if (post_data){
                free(post_data);
            }
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

char *sia_bus_multipart_create_json(sia_cfg_t *opt, const char *path, const char *key){
    if(opt->verbose){
        fprintf(stderr, "%s:%d %s(\"%s\",\"%s\",\"%s\")\n", __FILE_NAME__, __LINE__, __func__, opt->url, path, key);
    }

    char *final_url;
    final_url = malloc( sizeof(opt->unauthenticated_url)*(strlen(opt->unauthenticated_url)+
                        24+1));
    strcpy(final_url, opt->unauthenticated_url);
    strcat(final_url, "api/bus/multipart/create");

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
            char *post_data_format ="{\n    \"bucket\": \"%s\",\n    \"key\": \"key:%s\",\n    \"path\": \"%s\", \"generateKey\": false\n}";;
            char *post_data = malloc(sizeof(char) * (strlen(post_data_format) + strlen(opt->bucket) + strlen(path) + strlen(key)) + 1);
            sprintf(post_data, post_data_format, opt->bucket, key, path);
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post_data);
            res = curl_easy_perform(curl);
            if(res != CURLE_OK){
                fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
            }
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

char *sia_bus_multipart_listparts_json(sia_cfg_t *opt, const char *path, const char *uploadid){
    if(opt->verbose){
        fprintf(stderr, "%s:%d %s(\"%s\",\"%s\",\"%s\")\n", __FILE_NAME__, __LINE__, __func__, opt->url, path, uploadid);
    }

    char *final_url;
    final_url = malloc( sizeof(opt->unauthenticated_url)*(strlen(opt->unauthenticated_url)+
                        26+1));
    strcpy(final_url, opt->unauthenticated_url);
    strcat(final_url, "api/bus/multipart/listpart");

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
            char *post_data_format = "{\n  \"bucket\": \"%s\",\n  \"path\": \"%s\",\n  \"uploadID\": \"%s\",\n  \"partNumberMarker\": 0,\n  \"limit\": -1\n}";
            char *post_data = malloc(sizeof(char) * (strlen(post_data_format) + strlen(opt->bucket) + strlen(path) + strlen(uploadid)) + 1);
            sprintf(post_data, post_data_format, opt->bucket, path, uploadid);
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post_data);
            res = curl_easy_perform(curl);
            if(res != CURLE_OK){
                fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
            }
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

char *sia_bus_multipart_listuploads_json(sia_cfg_t *opt, const char *path){
    if(opt->verbose){
        fprintf(stderr, "%s:%d %s(\"%s\",\"%s\")\n", __FILE_NAME__, __LINE__, __func__, opt->url, path);
    }

    char *final_url;
    final_url = malloc( sizeof(opt->unauthenticated_url)*(strlen(opt->unauthenticated_url)+
                        29+1));
    strcpy(final_url, opt->unauthenticated_url);
    strcat(final_url, "api/bus/multipart/listuploads");

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
            char *post_data_format = "{\n  \"bucket\": \"%s\",\n  \"prefix\": \"%s\",\n  \"pathMarker\": \"\",\n  \"uploadIDMarker\": \"\",\n  \"limit\": -1\n}";;
            char *post_data = malloc(sizeof(char) * (strlen(post_data_format) + strlen(opt->bucket) + strlen(path)) + 1);
            sprintf(post_data, post_data_format, opt->bucket, path);
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post_data);
            res = curl_easy_perform(curl);
            if(res != CURLE_OK){
                fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
            }
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

#ifdef __cplusplus
}
#endif
