#ifdef __cplusplus
extern "C"
{
#endif

#include "sia_worker_objects.h"
extern sia_cfg_t opt;

char *sia_worker_objects(sia_cfg_t *opt, const char *path, size_t *size, off_t *offset){
    if(opt->verbose){
        fprintf(stderr, "%s:%d %s(\"%s\")\n", __FILE_NAME__, __LINE__, __func__, opt->url);
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

