#ifdef __cplusplus
extern "C"
{
#endif

#include "sia_worker_objects.h"
extern sia_cfg_t opt;

char *sia_worker_head_object(sia_cfg_t *opt, const char *path){
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
    //http://127.0.0.1:9980/api/worker/objects/test with paces.pdf?bucket=default
    final_url = malloc( sizeof(opt->unauthenticated_url)*strlen(opt->unauthenticated_url)+
                        18+
                        sizeof(path2)*strlen(path2)+
                        8+
                        sizeof(opt->bucket)*strlen(opt->bucket)+
                        1);
    strcpy(final_url, opt->unauthenticated_url);
    strcat(final_url, "api/worker/objects");
    strcat(final_url, path2);
    strcat(final_url, "?bucket=");
    strcat(final_url, opt->bucket);

    if(opt->verbose){
        fprintf(stderr, "%s(\"%s\")\n", __func__, final_url);
    }

//    void *payload = (void *)sia_get_from_cache(final_url);
    void *payload = NULL;
    sia_http_payload_t http_payload;
    http_payload.len = 0;
    http_payload.data = NULL;

    // For now, we won't do any caching on the meta as it is the only way to make sure the data hasn't changed
    if (payload == NULL){
        CURL *curl;
        CURLcode res;
        curl = curl_easy_init();
        if(curl) {
            curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "HEAD");
            curl_easy_setopt(curl, CURLOPT_URL, final_url);
            curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
            curl_easy_setopt(curl, CURLOPT_DEFAULT_PROTOCOL, opt->scheme);
            curl_easy_setopt(curl, CURLOPT_USERNAME, opt->user);
            curl_easy_setopt(curl, CURLOPT_PASSWORD, opt->password);
            curl_easy_setopt(curl, CURLOPT_HEADER, 0L);
            if(opt->verbose){
                curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
            }
    //        curl_easy_setopt(curl, CURLOPT_HTTPAUTH, CURLAUTH_BASIC);
//            if (offset > 0){
//                char str[256];
//                sprintf(str, "%ld%s%ld", offset, "-", (offset+size-1));
//                curl_easy_setopt(curl, CURLOPT_RANGE, str);
//            }
//            struct curl_slist *headers = NULL;
//            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, capture_payload);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &http_payload);
            curl_easy_setopt(curl, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_0);
            res = curl_easy_perform(curl);
            if(res != CURLE_OK){
                fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
            }

            // Save the following headers
            // Content-Length
            // Etag
            // Last-Modified
            // Date
            cJSON *monitor = cJSON_CreateObject();
            if (monitor != NULL){
                struct curl_header *header;
                CURLHcode h;

                h = curl_easy_header(curl, "Content-Length", 0, CURLH_HEADER, -1, &header);
                if (header->value[strlen(header->value) - 1] == '"'){
                    header->value[strlen(header->value) - 1] = '\0';
                }
                if (header->value[0] == '"'){
                    memmove(header->value, header->value + 1, strlen(header->value));
                }
                if(opt->verbose){
                    fprintf(stderr, "%s:%d Status: %d\n", __FILE_NAME__, __LINE__, h);
                    fprintf(stderr, "%s:%d %s: %s\n", __FILE_NAME__, __LINE__, header->name, header->value);
                }
                if (h == CURLHE_OK){
                    char *ptr;
                    cJSON *content_lenght = cJSON_CreateNumber(strtol(header->value, &ptr, 10));
                    if (content_lenght != NULL){
                        cJSON_AddItemToObject(monitor, "Content-Length", content_lenght);
                    }
                }
                else{
                    fprintf(stderr, "curl_easy_perform() failed: %s\n", (char *)curl_easy_strerror(h));
                }

                ///////////////////////////////////////////////////////////////////
                h = curl_easy_header(curl, "Etag", 0, CURLH_HEADER, -1, &header);
                if (header->value[strlen(header->value) - 1] == '"'){
                    header->value[strlen(header->value) - 1] = '\0';
                }
                if (header->value[0] == '"'){
                    memmove(header->value, header->value + 1, strlen(header->value));
                }
                if(opt->verbose){
                    fprintf(stderr, "%s:%d Status: %d\n", __FILE_NAME__, __LINE__, h);
                    fprintf(stderr, "%s:%d %s: %s\n", __FILE_NAME__, __LINE__, header->name, header->value);
                }
                if (h == CURLHE_OK){
                    char *ptr;
                    cJSON *etag = cJSON_CreateString(header->value);
                    if (etag != NULL){
                        cJSON_AddItemToObject(monitor, "Etag", etag);
                    }
                }
                else{
                    fprintf(stderr, "curl_easy_perform() failed: %s\n", (char *)curl_easy_strerror(h));
                }

                //////////////////////////////////////////
                h = curl_easy_header(curl, "Last-Modified", 0, CURLH_HEADER, -1, &header);
                if (header->value[strlen(header->value) - 1] == '"'){
                    header->value[strlen(header->value) - 1] = '\0';
                }
                if (header->value[0] == '"'){
                    memmove(header->value, header->value + 1, strlen(header->value));
                }
                if(opt->verbose){
                    fprintf(stderr, "%s:%d Status: %d\n", __FILE_NAME__, __LINE__, h);
                    fprintf(stderr, "%s:%d %s: %s\n", __FILE_NAME__, __LINE__, header->name, header->value);
                }
                if (h == CURLHE_OK){
                    char *ptr;
                    cJSON *last_modified = cJSON_CreateString(header->value);
                    if (last_modified != NULL){
                        cJSON_AddItemToObject(monitor, "Last-Modified", last_modified);
                    }
                }
                else{
                    fprintf(stderr, "curl_easy_perform() failed: %s\n", (char *)curl_easy_strerror(h));
                }
                //////////////////////////////////////////
                h = curl_easy_header(curl, "Date", 0, CURLH_HEADER, -1, &header);
                if (header->value[strlen(header->value) - 1] == '"'){
                    header->value[strlen(header->value) - 1] = '\0';
                }
                if (header->value[0] == '"'){
                    memmove(header->value, header->value + 1, strlen(header->value));
                }
                if(opt->verbose){
                    fprintf(stderr, "%s:%d Status: %d\n", __FILE_NAME__, __LINE__, h);
                    fprintf(stderr, "%s:%d %s: %s\n", __FILE_NAME__, __LINE__, header->name, header->value);
                }
                if (h == CURLHE_OK){
                    char *ptr;
                    cJSON *date = cJSON_CreateString(header->value);
                    if (date != NULL){
                        cJSON_AddItemToObject(monitor, "Date", date);
                    }
                }
                else{
                    fprintf(stderr, "curl_easy_perform() failed: %s\n", (char *)curl_easy_strerror(h));
                }

                payload = cJSON_Print(monitor);
                cJSON_Delete(monitor);

            }
//            if (headers != NULL){
//                curl_slist_free_all(headers);
//            }
            curl_easy_cleanup(curl);
        }

        payload = http_payload.data;
        if(opt->verbose){
            fprintf(stderr, "%s:%d  %s payload: %s\n", __FILE_NAME__, __LINE__, __func__, "(char *)payload");
        }
        //sia_set_to_cache(final_url, http_payload.data);

        free(final_url);
        free(path2);
        final_url = NULL;
    }
    return (char *)payload;
}

char *sia_worker_get_object(sia_cfg_t *opt, const char *path, size_t size, off_t offset, size_t *payload_size){
    if(opt->verbose){
        fprintf(stderr, "%s:%d %s(\"%s\", \"%s\", %lu, %ld, %lu)\n", __FILE_NAME__, __LINE__, __func__, opt->url, path, size, offset, *payload_size);
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
    //http://127.0.0.1:9980/api/worker/objects/test with paces.pdf?bucket=default
    final_url = malloc( sizeof(opt->unauthenticated_url)*strlen(opt->unauthenticated_url)+
                        18+
                        sizeof(path2)*strlen(path2)+
                        8+
                        sizeof(opt->bucket)*strlen(opt->bucket)+
                        1);
    strcpy(final_url, opt->unauthenticated_url);
    strcat(final_url, "api/worker/objects");
    strcat(final_url, path2);
    strcat(final_url, "?bucket=");
    strcat(final_url, opt->bucket);

    if(opt->verbose){
        fprintf(stderr, "%s(\"%s\")\n", __func__, final_url);
    }
#ifdef SIA_DISK_CACHE
    char *disk_key = (opt->L2->key)(path);
    if(opt->verbose){
        fprintf(stderr, "%s:%d  %s disk key hash: %s\n", __FILE_NAME__, __LINE__, __func__, disk_key);
    }
#endif

//    void *payload = (void *)sia_get_from_cache(final_url);
    void *payload = NULL;
    sia_http_payload_t http_payload;
    http_payload.len = 0;
    http_payload.data = NULL;

#ifdef SIA_DISK_CACHE
    unsigned int status = (opt->L2->get)(opt->cache_dir, disk_key, &payload, payload_size, size, offset);
#endif


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
            curl_easy_setopt(curl, CURLOPT_HEADER, 0L);
            if(opt->verbose){
                curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
            }
    //        curl_easy_setopt(curl, CURLOPT_HTTPAUTH, CURLAUTH_BASIC);
//            if (offset > 0){
                char str[256];
                off_t offset2 = offset + size - 1;
                unsigned long int fsize = sia_bus_object_size(opt, path) - 1;
                off_t offset3 = offset2;
                if (offset2 > fsize){
                    offset3 = fsize;
                    if(opt->verbose){
                        fprintf(stderr, "%s:%d %lu vs %lu = %lu\n", __FILE_NAME__, __LINE__, offset2, fsize, offset3);
                    }
                }
                sprintf(str, "%ld%s%ld", offset, "-", offset3);
                curl_easy_setopt(curl, CURLOPT_RANGE, str);
//            }
//            struct curl_slist *headers = NULL;
//            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, capture_payload);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &http_payload);
            curl_easy_setopt(curl, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_0);
            res = curl_easy_perform(curl);
            if(res != CURLE_OK){
                fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
            }
            else{
                long http_code = 0;
                curl_easy_getinfo (curl, CURLINFO_RESPONSE_CODE, &http_code);
                if (http_code < 400){
                    struct curl_header *header;
                    CURLHcode h;
                    h = curl_easy_header(curl, "Content-Length", 0, CURLH_HEADER, -1, &header);
                    if (header->value[strlen(header->value) - 1] == '"'){
                        header->value[strlen(header->value) - 1] = '\0';
                    }
                    if (header->value[0] == '"'){
                        memmove(header->value, header->value + 1, strlen(header->value));
                    }
                    if(opt->verbose){
                        fprintf(stderr, "%s:%d Status: %d\n", __FILE_NAME__, __LINE__, h);
                        fprintf(stderr, "%s:%d %s: %s\n", __FILE_NAME__, __LINE__, header->name, header->value);
                    }

                    if (h == CURLHE_OK){
                        char *ptr;
                        *payload_size = strtol(header->value, &ptr, 10);
                    }
                    else{
                        fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(h));
                    }

                    char *etag = NULL;
                    etag = sia_bus_object_etag(opt, path);
//                    if(opt->verbose){
//                        fprintf(stderr, "%s:%d eTag: %s\n", __FILE_NAME__, __LINE__, etag);
//                    }

/*
                    struct curl_header *header2;
                    CURLHcode h2;
                    h2 = curl_easy_header(curl, "Etag", 0, CURLH_HEADER, -1, &header2);
                    if (header2->value[strlen(header2->value) - 1] == '"'){
                        header2->value[strlen(header2->value) - 1] = '\0';
                    }
                    if (header2->value[0] == '"'){
                        memmove(header2->value, header2->value + 1, strlen(header2->value));
                    }
                    if(opt->verbose){
                        fprintf(stderr, "%s:%d Status: %d\n", __FILE_NAME__, __LINE__, h2);
                        fprintf(stderr, "%s:%d %s: %s\n", __FILE_NAME__, __LINE__, header2->name, header2->value);
                    }

                    if (h2 == CURLHE_OK){
                        char *ptr;
                        strtol(header2->value, &ptr, 10);
                        strcpy(etag, header2->value);
                    }
                    else{
                        fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(h2));
                    }
//            if (headers != NULL){
//                curl_slist_free_all(headers);
//            }
*/
                    payload = http_payload.data;
                    if(opt->verbose){
                        fprintf(stderr, "%s:%d  %s payload: %s\n", __FILE_NAME__, __LINE__, __func__, "(char *)payload");
                    }

#ifdef SIA_DISK_CACHE
                //sia_set_to_cache(final_url, http_payload.data);
                    if (etag != NULL){
                        unsigned int status = (opt->L2->set)(opt->cache_dir, disk_key, payload, *payload_size, size, offset, etag);
                    }
                    else{
                        if(opt->verbose){
                            fprintf(stderr, "%s:%d  %s eTag is NULL, not skiping disk_set()\n", __FILE_NAME__, __LINE__, __func__);
                        }
                    }
#endif
                }
            }
            curl_easy_cleanup(curl);
        }
        free(final_url);
        free(path2);
        final_url = NULL;
    }
#ifdef SIA_DISK_CACHE
    else{
        if(opt->verbose){
            fprintf(stderr, "%s:%d  %s dis_get() HIT\n", __FILE_NAME__, __LINE__, __func__);
        }
    }
#endif
    return (char *)payload;
}

char *sia_worker_put_multipart_from_file(sia_cfg_t *opt, const char *path, const char *uploadid, size_t size, off_t offset, void *ctx, off_t slot){
    if(opt->verbose){
        fprintf(stderr, "%s:%d %s(\"%s\", \"%s\", %lu, %ld, \"%s\", %lu)\n", __FILE_NAME__, __LINE__, __func__, opt->url, uploadid, size, offset, (char *)ctx, slot);
    }
    char *final_url;
    //http://127.0.0.1:9980/api/worker/multipart/:key?bucket=mybucket&uploadid=0bdbea34e2be1b3de7c60766dc1a9f400e0cf6d2db8f5f3842720f8549559f29&partnumber=1"
    // Encode the path
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
    final_url = malloc( sizeof(opt->unauthenticated_url)*strlen(opt->unauthenticated_url)+
                        21+
                        sizeof(path2)*strlen(path2)+
                        8+
                        sizeof(opt->bucket)*strlen(opt->bucket)+
                        10+
                        sizeof(uploadid)*strlen(uploadid)+
                        12+
                        1);
    strcpy(final_url, opt->unauthenticated_url);
    strcat(final_url, "api/worker/multipart");
    strcat(final_url, path2); // Where is it
    strcat(final_url, "?bucket=");
    strcat(final_url, opt->bucket);
    strcat(final_url, "&uploadid=");
    strcat(final_url, uploadid);
    strcat(final_url, "&partnumber=");

    char str_slot[6];
    sprintf(str_slot, "%ld", slot);
    strcat(final_url, str_slot);

    if(opt->verbose){
        fprintf(stderr, "%s(\"%s\")\n", __func__, final_url);
    }

    void *payload = NULL;
    sia_http_payload_t http_h_payload;
    http_h_payload.len = 0;
    http_h_payload.data = NULL;

    CURLcode res;
    curl = curl_easy_init();
    char *fn = (char *)ctx;
    FILE *f = fopen(fn, "rb");
    if (f){
        if(curl) {
            curl_easy_setopt(curl, CURLOPT_BUFFERSIZE, 102400L);
            curl_easy_setopt(curl, CURLOPT_HTTP_VERSION, (long)CURL_HTTP_VERSION_2TLS);
            curl_easy_setopt(curl, CURLOPT_FTP_SKIP_PASV_IP, 1L);
            curl_easy_setopt(curl, CURLOPT_TCP_KEEPALIVE, 1L);
            curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PUT");
            curl_easy_setopt(curl, CURLOPT_URL, final_url);
            curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
            curl_easy_setopt(curl, CURLOPT_DEFAULT_PROTOCOL, opt->scheme);
            curl_easy_setopt(curl, CURLOPT_USERNAME, opt->user);
            curl_easy_setopt(curl, CURLOPT_PASSWORD, opt->password);
            if(opt->verbose){
                curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
            }
    //        curl_easy_setopt(curl, CURLOPT_HTTPAUTH, CURLAUTH_BASIC);
            struct curl_slist *headers = NULL;
            if ((ctx != NULL) && (size > 0)){
                if(opt->verbose){
                    fprintf(stderr, "%s:%d There is something to write\n", __FILE_NAME__, __LINE__);
                }

                char cl[256] = {0};
                sprintf(cl, "Content-Length: %lu", size);
                headers = curl_slist_append(headers, cl);
//                headers = curl_slist_append(headers, "Content-Type: multipart/form-data");
                headers = curl_slist_append(headers, "Content-Type: application/octet-stream");
//                headers = curl_slist_append(headers, "Expect:");
                curl_easy_setopt(curl, CURLOPT_INFILESIZE, (curl_off_t)size);
                curl_easy_setopt(curl, CURLOPT_INFILESIZE_LARGE, (curl_off_t)size);
                curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);
                curl_easy_setopt(curl, CURLOPT_READDATA, (void *)f);
                curl_easy_setopt(curl, CURLOPT_TIMEOUT, 0L);
            }
            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
            res = curl_easy_perform(curl);
            long response_code;
            curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
            if(opt->verbose){
                fprintf(stderr, "%s:%d HTTP RESPONSE CODE: %lu\n", __FILE_NAME__, __LINE__, response_code);
            }
            if(res != CURLE_OK){
                fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
            }
            if (response_code < 300){
                struct curl_header *header;
                CURLHcode h;
                h = curl_easy_header(curl, "ETag", 0, CURLH_HEADER, -1, &header);
                if (header->value[strlen(header->value) - 1] == '"'){
                    header->value[strlen(header->value) - 1] = '\0';
                }
                if (header->value[0] == '"'){
                    memmove(header->value, header->value + 1, strlen(header->value));
                }
                if(opt->verbose){
                    fprintf(stderr, "%s:%d Status: %d\n", __FILE_NAME__, __LINE__, h);
                    fprintf(stderr, "%s:%d %s: %s\n", __FILE_NAME__, __LINE__, header->name, header->value);
                }
                if (h == CURLHE_OK){
                    http_h_payload.data = malloc((sizeof(char) * strlen(header->value) + 1));
                    strcpy(http_h_payload.data, header->value);
                    http_h_payload.len = strlen(header->value);
                    payload = http_h_payload.data;
                }
                if (headers != NULL){
                    curl_slist_free_all(headers);
                }
            }
        }
        fclose(f);
        curl_easy_cleanup(curl);
    }
    //sia_set_to_cache(final_url, http_payload.data);

    free(final_url);
    final_url = NULL;

    return payload;
}

char *sia_worker_put_multipart(sia_cfg_t *opt, const char *path, const char *uploadid, size_t size, off_t offset, void *ctx, off_t slot){
    if(opt->verbose){
        fprintf(stderr, "%s:%d %s(\"%s\", \"%s\", %lu, %ld, \"%s\", %lu)\n", __FILE_NAME__, __LINE__, __func__, opt->url, uploadid, size, offset, "(char *)ctx", slot);
    }
    char *final_url;
    //http://127.0.0.1:9980/api/worker/multipart/:key?bucket=mybucket&uploadid=0bdbea34e2be1b3de7c60766dc1a9f400e0cf6d2db8f5f3842720f8549559f29&partnumber=1"
    // Encode the path
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
    final_url = malloc( sizeof(opt->unauthenticated_url)*strlen(opt->unauthenticated_url)+
                        21+
                        sizeof(path2)*strlen(path2)+
                        8+
                        sizeof(opt->bucket)*strlen(opt->bucket)+
                        10+
                        sizeof(uploadid)*strlen(uploadid)+
                        12+
                        1);
    strcpy(final_url, opt->unauthenticated_url);
    strcat(final_url, "api/worker/multipart");
    strcat(final_url, path2); // Where is it
    strcat(final_url, "?bucket=");
    strcat(final_url, opt->bucket);
    strcat(final_url, "&uploadid=");
    strcat(final_url, uploadid);
    strcat(final_url, "&partnumber=");

    char str_slot[6];
    sprintf(str_slot, "%ld", slot);
    strcat(final_url, str_slot);

    if(opt->verbose){
        fprintf(stderr, "%s(\"%s\")\n", __func__, final_url);
    }

    void *payload = NULL;
    sia_http_payload_t http_h_payload;
    http_h_payload.len = 0;
    http_h_payload.data = NULL;

    CURLcode res;
    curl = curl_easy_init();
    if(curl) {
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PUT");
        curl_easy_setopt(curl, CURLOPT_URL, final_url);
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl, CURLOPT_DEFAULT_PROTOCOL, opt->scheme);
        curl_easy_setopt(curl, CURLOPT_USERNAME, opt->user);
        curl_easy_setopt(curl, CURLOPT_PASSWORD, opt->password);
        if(opt->verbose){
            curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
        }
//        curl_easy_setopt(curl, CURLOPT_HTTPAUTH, CURLAUTH_BASIC);
        struct curl_slist *headers = NULL;
        if ((ctx != NULL) && (size > 0)){
            if(opt->verbose){
                fprintf(stderr, "%s:%d There is something to write\n", __FILE_NAME__, __LINE__);
            }

            char cl[256] = {0};
            sprintf(cl, "Content-Length: %lu", size);
            headers = curl_slist_append(headers, cl);
            headers = curl_slist_append(headers, "Content-Type: multipart/form-data");
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, ctx);
            curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, size);
        }
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        res = curl_easy_perform(curl);
        if(res != CURLE_OK){
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        }
        struct curl_header *header;
        CURLHcode h;
        h = curl_easy_header(curl, "ETag", 0, CURLH_HEADER, -1, &header);
        if (header->value[strlen(header->value) - 1] == '"'){
            header->value[strlen(header->value) - 1] = '\0';
        }
        if (header->value[0] == '"'){
            memmove(header->value, header->value + 1, strlen(header->value));
        }
        if(opt->verbose){
            fprintf(stderr, "%s:%d Status: %d\n", __FILE_NAME__, __LINE__, h);
            fprintf(stderr, "%s:%d %s: %s\n", __FILE_NAME__, __LINE__, header->name, header->value);
        }
        if (h == CURLHE_OK){
            http_h_payload.data = malloc((sizeof(char) * strlen(header->value) + 1));
            strcpy(http_h_payload.data, header->value);
            http_h_payload.len = strlen(header->value);
            payload = http_h_payload.data;
        }
        if (headers != NULL){
            curl_slist_free_all(headers);
        }
        curl_easy_cleanup(curl);
    }
    //sia_set_to_cache(final_url, http_payload.data);

    free(final_url);
    final_url = NULL;

    return payload;
}

char *sia_worker_put_object(sia_cfg_t *opt, const char *path, size_t size, off_t offset, void *ctx){
    if(opt->verbose){
        fprintf(stderr, "%s:%d %s(\"%s\", \"%s\", %lu, %ld, \"%s\")\n", __FILE_NAME__, __LINE__, __func__, opt->url, path, size, offset, (char *)ctx);
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

//    void *payload = (void *)sia_get_from_cache(final_url);
    void *payload = NULL;
    sia_http_payload_t http_payload;
    http_payload.len = 0;
    http_payload.data = NULL;

//    if (ctx != NULL){
//        if(opt->verbose){
//            fprintf(stderr, "%s:%d Payload is not NULL\n", __FILE_NAME__, __LINE__);
//        }

        //CURL *curl;
        CURLcode res;
        curl = curl_easy_init();
        if(curl) {
            curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PUT");
            curl_easy_setopt(curl, CURLOPT_URL, final_url);
            curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
            curl_easy_setopt(curl, CURLOPT_DEFAULT_PROTOCOL, opt->scheme);
            curl_easy_setopt(curl, CURLOPT_USERNAME, opt->user);
            curl_easy_setopt(curl, CURLOPT_PASSWORD, opt->password);
            if(opt->verbose){
                curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
            }
    //        curl_easy_setopt(curl, CURLOPT_HTTPAUTH, CURLAUTH_BASIC);
            struct curl_slist *headers = NULL;
            if ((ctx != NULL) && (size > 0)){
                if(opt->verbose){
                    fprintf(stderr, "%s:%d There is something to write\n", __FILE_NAME__, __LINE__);
                }
//                headers = curl_slist_append(headers, "Expect: 100-continue");

                char cl[256] = {0};
                sprintf(cl, "Content-Length: %lu", size);
                headers = curl_slist_append(headers, cl);
                headers = curl_slist_append(headers, "Content-Type: application/octet-stream");
//              headers = curl_slist_append(headers, "Transfer-Encoding: chunked");

//                curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);
                curl_easy_setopt(curl, CURLOPT_POSTFIELDS, ctx);
                curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, size);

//                curl_easy_setopt(curl, CURLOPT_READFUNCTION, send_payload);
//                http_payload.data = ctx;
//                http_payload.len = size;
//                curl_easy_setopt(curl, CURLOPT_READDATA, &http_payload);
//                curl_easy_setopt(curl, CURLOPT_INFILESIZE_LARGE, (curl_off_t)size);
                if (offset > 0){
//                    Content-Range: bytes 4096-6573/2477
                    char cr[256] = {0};
                    size_t offset2 = offset + size;
                    sprintf(cr, "Content-Range: bytes %" CURL_FORMAT_CURL_OFF_T "-" "%" CURL_FORMAT_CURL_OFF_T "/*", offset,  offset2);
                    headers = curl_slist_append(headers, cr);
//                    curl_easy_setopt(curl, CURLOPT_RESUME_FROM, offset);
//                    char str[256];
//                    size_t offset2 = offset + size;                   // TODO: review
//                    sprintf(str, "%ld%s%ld", offset, "-", offset2);
//                    curl_easy_setopt(curl, CURLOPT_RANGE, str);
                }
            }
            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
            payload = ctx;
            res = curl_easy_perform(curl);
            if(res != CURLE_OK){
                fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
            }
            if (headers != NULL){
                curl_slist_free_all(headers);
            }
            curl_easy_cleanup(curl);
        }
        //sia_set_to_cache(final_url, http_payload.data);

        free(final_url);
        final_url = NULL;
//    }

    return payload;
}

#ifdef __cplusplus
}
#endif

