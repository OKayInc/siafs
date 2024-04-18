#ifdef __cplusplus
extern "C"
{
#endif

#include "sia_worker_objects.h"
extern sia_cfg_t opt;

char *sia_worker_get_object(sia_cfg_t *opt, const char *path, size_t size, off_t offset, size_t *payload_size){
    if(opt->verbose){
        fprintf(stderr, "%s:%d %s(\"%s\", \"%s\", %lu %ld)\n", __FILE_NAME__, __LINE__, __func__, opt->url, path, size, offset);
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
            if (offset > 0){
                char str[256];
                size_t offset2 = offset + size;                   // TODO: review
                sprintf(str, "%ld%s%ld", offset, "-", offset2);
                curl_easy_setopt(curl, CURLOPT_RANGE, str);
            }
            struct curl_slist *headers = NULL;
            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, capture_payload);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &http_payload);
            curl_easy_setopt(curl, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_0);
            res = curl_easy_perform(curl);
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
            if (headers != NULL){
                curl_slist_free_all(headers);
            }
        }

        payload = http_payload.data;
        if(opt->verbose){
            fprintf(stderr, "%s:%d  %s payload: %s\n", __FILE_NAME__, __LINE__, __func__, (char *)payload);
        }
        //sia_set_to_cache(final_url, http_payload.data);

        curl_easy_cleanup(curl);
        free(final_url);
        free(path2);
        final_url = NULL;
    }
    return (char *)payload;
}

char *sia_worker_put_multipart(sia_cfg_t *opt, const char *path, const char *uploadid, size_t size, off_t offset, void *ctx){
    if(opt->verbose){
        fprintf(stderr, "%s:%d %s(\"%s\", \"%s\", %lu, %ld, \"%s\")\n", __FILE_NAME__, __LINE__, __func__, opt->url, uploadid, size, offset, (char *)ctx);
    }
    char *final_url;
    //http://127.0.0.1:9980/api/worker/multipart/:key?bucket=mybucket&uploadid=0bdbea34e2be1b3de7c60766dc1a9f400e0cf6d2db8f5f3842720f8549559f29&partnumber=1"
    final_url = malloc( sizeof(opt->unauthenticated_url)*strlen(opt->unauthenticated_url)+
                        21+
                        sizeof(path)*strlen(path)+
                        8+
                        sizeof(opt->bucket)*strlen(opt->bucket)+
                        10+
                        sizeof(uploadid)*strlen(uploadid)+
                        12+
                        1);
    strcpy(final_url, opt->unauthenticated_url);
    strcat(final_url, "api/worker/multipart");
    strcat(final_url, path); // Where is it
    strcat(final_url, "?bucket=");
    strcat(final_url, opt->bucket);
    strcat(final_url, "&uploadid=");
    strcat(final_url, uploadid);
    strcat(final_url, "&partnumber=");
    // TODO: offset is not the ultipart index, it needs to be calculated? MOD? DIV?   offset/size + 1
    off_t slot = offset / 4096 + 1;
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

    CURL *curl;
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
        }
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        res = curl_easy_perform(curl);
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
    //sia_set_to_cache(final_url, http_payload.data);

    curl_easy_cleanup(curl);
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
            if (headers != NULL){
                curl_slist_free_all(headers);
            }
        }
        //sia_set_to_cache(final_url, http_payload.data);

        curl_easy_cleanup(curl);
        free(final_url);
        final_url = NULL;
//    }

    return payload;
}

#ifdef __cplusplus
}
#endif

