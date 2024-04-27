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

time_t string2unixtime(char *timestamp){
    time_t answer = 0;
    struct tm tm;
    memset(&tm, 0, sizeof(struct tm));
    // 2024-04-20T03:34:36.367719838Z
    if (strptime(timestamp, "%Y-%m-%dT%H:%M:%S%Z", &tm) != NULL ){
        answer = mktime(&tm);
    }
    return answer;
}
#ifdef SIA_METACACHE
sia_metacache_t *dump_meta(sia_metacache_t *meta){
    if (meta != NULL)
        fprintf(stderr, "%s %llu %lu\n", meta->name, meta->size, meta->modtime);
    else
        fprintf(stderr, "META EMPTY\n");

    return meta;
}

sia_metacache_t *dump_all_meta(sia_cfg_t *opt){
    sia_metacache_t *current = opt->metacache;
    if (current != NULL){
        unsigned int i = 0;
        while (current != NULL){
            fprintf(stderr, "%u: %s %llu %lu\n", i, current->name, current->size, current->modtime);
            current = current->next;
            i++;
        }
    }
    else{
        fprintf(stderr, "META CACHE EMPTY\n");
    }
    return current;
}

sia_metacache_t *append_meta(sia_cfg_t *opt, sia_metacache_t *meta){
    sia_metacache_t *current = NULL;
    if (meta != NULL){
        if (opt->metacache != NULL){
            fprintf(stderr, "%s:%d %s APPEND TO LAST\n", __FILE_NAME__, __LINE__, __func__);
            current = opt->metacache;
            while (current->next != NULL){
                current = current->next;
            }
            current->next = meta;
            current->next->next = NULL; // Safety
        }
        else{
            fprintf(stderr, "%s:%d %s FIRST\n", __FILE_NAME__, __LINE__, __func__);
            opt->metacache = meta;
            opt->metacache->next = NULL; // Safety
            current = opt->metacache;
        }
    }
    return opt->metacache;
}

sia_metacache_t *build_meta_node_from_json(cJSON *object){
    sia_metacache_t *meta = NULL;
    if (object != NULL){
//        fprintf(stderr, "%s\n",cJSON_Print(object));
        cJSON *name = cJSON_GetObjectItemCaseSensitive(object, "name");
        cJSON *size = cJSON_GetObjectItemCaseSensitive(object, "size");
        cJSON *modTime = cJSON_GetObjectItemCaseSensitive(object, "modTime");
        if ((name != NULL) && (size != NULL) && (modTime != NULL)){
            if (cJSON_IsString(name) && cJSON_IsNumber(size) && cJSON_IsString(modTime)){
                // Process
                meta = malloc(sizeof(sia_metacache_t));
                if (meta != NULL){
                    meta->name = malloc(sizeof(name->valuestring) * strlen(name->valuestring) + 1);
                    strcpy(meta->name, name->valuestring);
                    meta->size = size->valueint;
                    meta->modtime = string2unixtime(modTime->valuestring);
                    if ((meta->size > 0) && (name->valuestring[(strlen(name->valuestring) - 1)] != '/')){
                        meta->type = SIA_FILE;
                    }
                    else if (name->valuestring[(strlen(name->valuestring) - 1)] == '/'){
                        meta->type = SIA_DIR;
                    }
                    else{
                        // A zero-byte size file maybe unknown until RenterD has a better way to detect directories or files
                        meta->type = SIA_UNKOWN;
                    }
                    meta->next = NULL;
                    meta->expire = time(NULL) + SIA_METACACHE_TTL;
                }
            }
        }
    }
    return meta;
}

sia_metacache_t *del_meta(sia_cfg_t *opt, sia_metacache_t *meta){
    sia_metacache_t *current = opt->metacache;
    sia_metacache_t *before = NULL;

    if (meta != NULL){
        free(meta->name);
        meta->name = NULL;

        if (meta == current){
            // Delete first node
            opt->metacache = meta->next;
            free(meta);
        }
        else{
            // Find one node before;
            before = current;
            while ((before->next != meta) && before->next){
                before = before->next;
            }

            if (before->next == meta){
                before->next = meta->next;
                free(meta);
            }
        }
    }
    return opt->metacache;
}

sia_metacache_t *find_meta_by_path(sia_cfg_t *opt, const char *path){
    fprintf(stderr, "%s:%d %s LOOKING FOR %s\n", __FILE_NAME__, __LINE__, __func__, path);

    sia_metacache_t *current = opt->metacache;
    if ((path != NULL) && (current != NULL)){
        if (strcmp(current->name, path)){
            while (current->next != NULL){
                if (!strcmp(current->name, path)){
                    break;
                }
                current = current->next;
            }
        }
    }
    else{
        current = NULL;
    }
    return current;
}

sia_metacache_t *update_meta(sia_cfg_t *opt, sia_metacache_t *src, sia_metacache_t *dst){
    if ((src != NULL) && (dst != NULL)){
        char *ptr = realloc(src->name, sizeof(dst->name)*strlen(dst->name) + 1);
        strcpy(src->name, dst->name);
        src->name = ptr;
        src->type = dst->type;
        src->size = dst->size;
        src->modtime = dst->modtime;
        src->expire = time(NULL) + SIA_METACACHE_TTL;
    }
    return opt->metacache;
}

sia_metacache_t *add_meta(sia_cfg_t *opt, sia_metacache_t *meta){
    sia_metacache_t *current = NULL;
    if (meta != NULL){
        sia_metacache_t *n = find_meta_by_path(opt, meta->name);
        if (n == NULL){
            fprintf(stderr, "%s:%d %s APPEND\n", __FILE_NAME__, __LINE__, __func__);
            opt->metacache = append_meta(opt, meta);
        }
        else{
            fprintf(stderr, "%s:%d %s UPDATE\n", __FILE_NAME__, __LINE__, __func__);
            opt->metacache = update_meta(opt, n, meta);
            current = meta;
        }
    }
    return opt->metacache;
}
#endif
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
        ptr = calloc(data->len + realsize + 1, sizeof(char));
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
