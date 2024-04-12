#include "siafs.h"
#include "sia.h"

extern sia_cfg_t opt;

int siafs_getattr(const char *path, struct stat *stbuf){
    if(opt.verbose){
        fprintf(stderr, "%s:%d %s(\"%s\")\n", __FILE_NAME__, __LINE__, __func__, path);
    }
    memset(stbuf, 0, sizeof(struct stat));

    stbuf->st_uid = fuse_get_context()->uid;
    stbuf->st_gid = fuse_get_context()->gid;
    stbuf->st_atime = time(NULL);
/*
    char timestamp[32] = {0};
    strcpy(timestamp, sia_bus_objects_modtime(&opt, path));  TODO: Review this
    struct tm tm;
    memset(&tm, 0, sizeof(struct tm));
    if (strptime(timestamp, "%Y-%m-%dT%H:%M:%S", &tm) != NULL ){
        stbuf->st_mtime = mktime(&tm);
    }
    else{
        stbuf->st_mtime = time(NULL);
    }
*/
    
    if(!strcmp(path, "/")){
        stbuf->st_ino = 1;
        stbuf->st_mode = S_IFDIR | 0777;
        stbuf->st_nlink = 2;
        stbuf->st_mtime = time(NULL);   // If / sends todays mod time, SIA doesn't return information for /
    }
    else if (sia_bus_objects_is_file(&opt, path) == 1 ){
        stbuf->st_mode = S_IFREG | 666;
		stbuf->st_nlink = 1;
		stbuf->st_size = sia_bus_objects_size(&opt, path);    // FIX this
        stbuf->st_mtime = time(NULL);   // Temporal

    }
    else if (sia_bus_objects_is_dir(&opt, path) == 1 ){
        stbuf->st_mode = S_IFDIR | 0777;
        stbuf->st_nlink = 2;
        stbuf->st_mtime = time(NULL);   // Temporal
    }
    return 0;
}

int siafs_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi){
    if(opt.verbose){
        fprintf(stderr, "%s:%d %s(\"%s\")\n", __FILE_NAME__, __LINE__, __func__, path);
    }
    filler(buf, ".", NULL, 0);
    filler(buf, "..", NULL, 0);

    char *json_payload = sia_bus_objects_json(&opt, path);  // TODO: check if this works, or if the search endpoint is better
    if (json_payload != NULL){
        if(opt.verbose){
            fprintf(stderr, "%s:%d json payload: %s\n", __FILE_NAME__, __LINE__, json_payload);
        }
        cJSON *monitor_json = cJSON_Parse(json_payload);

        if (monitor_json == NULL){
            const char *error_ptr = cJSON_GetErrorPtr();
            fprintf(stderr, "Error before: %s\n", error_ptr);
        }
        else{
            cJSON *entries = NULL;
            cJSON *object = NULL;
            entries = cJSON_GetObjectItemCaseSensitive(monitor_json, "entries");
            if (entries){
                cJSON_ArrayForEach(object, entries){
                    cJSON *name = cJSON_GetObjectItemCaseSensitive(object, "name");
                    if (cJSON_IsString(name)){   // TODO: verify the propierty, name contains the full path /dir/dir/dir/name.txt
                        // The name must not contain a trailing / nor a leading /
                        if (name->valuestring[strlen(name->valuestring) - 1] == '/'){
                            name->valuestring[strlen(name->valuestring) - 1] = '\0';
                        }
                        if(opt.verbose){
                            fprintf(stderr, "%s:%d filename: %s\n", __FILE_NAME__, __LINE__, name->valuestring+sizeof(char));
                        }
                        filler(buf, name->valuestring+sizeof(char), NULL, 0);
                    }
                }
            }
        }
        free(json_payload);
        json_payload = NULL;
    }

    return 0;
}

int siafs_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi){
    if(opt.verbose){
        fprintf(stderr, "%s:%d %s(\"%s\")\n", __FILE_NAME__, __LINE__, __func__, path);
    }
    size_t payload_size = size;
    off_t payload_offset = offset;
    sia_worker_objects(&opt, path, &payload_size, &payload_offset);
    return payload_size - payload_offset;
}
