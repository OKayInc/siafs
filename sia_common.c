#ifdef __cplusplus
extern "C"
{
#endif

#include "sia_common.h"
extern sia_cfg_t opt;

sia_payload_t sia_cache = {
    .src = NULL,
    .payload = NULL,
    .time = 0
};

sia_upload_t *append_upload(sia_cfg_t *opt, sia_upload_t *upload){
    sia_upload_t *current = NULL;
    if (upload != NULL){
        if (opt->uploads != NULL){
            current = opt->uploads;
            while (current->next != NULL){
                current = current->next;
            }
            current->next = upload;
            current->next->next = NULL; // Safety
        }
        else{
            opt->uploads = upload;
            opt->uploads->next = NULL; // Safety
            current = opt->uploads;
        }
    }
    return current;
}

sia_upload_t *del_upload(sia_cfg_t *opt, sia_upload_t *upload){
    sia_upload_t *current = opt->uploads;
    sia_upload_t *before = NULL;

    if (upload != NULL){
        for (int i = 0; i < SIA_MAX_PARTS; i++){
            if (upload->part[i].etag != NULL){
                free(upload->part[i].etag);
                upload->part[i].etag = NULL;
            }
        }

        free(upload->name);
        free(upload->uploadID);
        upload->name = upload->uploadID = NULL;

        if (upload == current){
            // Delete first node
            opt->uploads = upload->next;
            free(upload);
        }
        else{
            // Find one node before;
            before = current;
            while ((before->next != upload) && before->next){
                before = before->next;
            }

            if (before->next == upload){
                before->next = upload->next;
                free(upload);
            }
        }
    }
    return opt->uploads;
}

sia_upload_t *find_upload_by_path(sia_cfg_t *opt, const char *path){
    sia_upload_t *current = opt->uploads;
    if ((path != NULL) && (current != NULL)){
        while (strcmp(current->name, path) && current->next){
            current = current->next;
        }

        if (strcmp(current->name, path))
            current = NULL;
    }
    else{
        current = NULL;
    }
    return current;
}

size_t send_payload(void *contents, size_t sz, size_t nmemb, void *ctx){
    size_t realsize = sz * nmemb;
//    if(opt.verbose){
        fprintf(stderr, "%s:%d %s(\"%s\", %lu, %lu, \"%s\")\n", __FILE_NAME__, __LINE__, __func__, "(char *)contents", sz, nmemb, "(char *)ctx");
//    }
    sia_http_payload_t *data = (sia_http_payload_t *)ctx;
    memcpy(contents, data->data, data->len);

    return data->len;
}

size_t capture_payload(void *contents, size_t sz, size_t nmemb, void *ctx){
    size_t realsize = sz * nmemb;
    if(opt.verbose){
        fprintf(stderr, "%s:%d %s(\"%s\", %lu, %lu, \"%s\")\n", __FILE_NAME__, __LINE__, __func__, "(char *)contents", sz, nmemb, "(char *)ctx");
    }
    sia_http_payload_t *data = (sia_http_payload_t *)ctx;
    char *ptr;
    if (data->data == NULL){
        ptr = malloc(data->len + realsize + 1);
        memset(ptr, 0, data->len + realsize + 1);
    }
    else{
        ptr = realloc(data->data, data->len + realsize + 1);
    }
    if(!ptr)
        return 0;
    data->data = ptr;
    memcpy(&(data->data[data->len]), contents, realsize);
    data->len += realsize;
    data->data[data->len] = 0;

  if(opt.verbose){
        fprintf(stderr, "%s:%d Data Len: %lu\n", __FILE_NAME__, __LINE__, data->len);
    }
  return realsize;
}

// TODO: get ready for caching, do not use them for now
char *sia_get_from_cache(const char *src){
    if(opt.verbose){
        fprintf(stderr, "%s:%d %s(\"%s\")\n", __FILE_NAME__, __LINE__, __func__, src);
    }
    if ((sia_cache.time != 0) && (sia_cache.src != NULL)){
        // there is something
        time_t now = time(NULL);
        if ((now - sia_cache.time) <= SIA_CACHE_TTL){
            // cache TTL still valid
            if (!strcmp(sia_cache.src, src)){
                // same source
                if(opt.verbose){
                    fprintf(stderr, "%s:%d\tHIT\n", __FILE_NAME__, __LINE__);
                }
                return sia_cache.payload;
            }
        }
    }
    return NULL;
}

char *sia_set_to_cache(const char *src, const char *payload){
    if(opt.verbose){
        fprintf(stderr, "%s:%d %s(\"%s\", \"%s\")\n", __FILE_NAME__, __LINE__, __func__, src, payload);
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

#ifdef __cplusplus
}
#endif
