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
/**
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
**/
    
    if(!strcmp(path, "/")){
        if(opt.verbose){
            fprintf(stderr, "%s:%d / directory\n", __FILE_NAME__, __LINE__);
        }
        stbuf->st_ino = 1;
        stbuf->st_mode = S_IFDIR | 0777;
        stbuf->st_nlink = 2;
        stbuf->st_mtime = time(NULL);   // If / sends todays mod time, SIA doesn't return information for /
    }
    else if (sia_bus_objects_is_file(&opt, path) == 1 ){
        if(opt.verbose){
            fprintf(stderr, "%s:%d %s is a file\n", __FILE_NAME__, __LINE__, path);
        }
        stbuf->st_mode = S_IFREG | 0666;
		stbuf->st_nlink = 1;
		stbuf->st_size = sia_bus_object_size(&opt, path);    // FIX this
        stbuf->st_mtime = time(NULL);   // Temporal
    }
    else if (sia_bus_objects_is_dir(&opt, path) == 1 ){
        if(opt.verbose){
            fprintf(stderr, "%s:%d %s is a directory\n", __FILE_NAME__, __LINE__, path);
        }
        stbuf->st_mode = S_IFDIR | 0777;
        stbuf->st_nlink = 2;
        stbuf->st_mtime = time(NULL);   // Temporal
    }
    else{
        return -ENOENT;
    }
    return 0;
}

int siafs_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi){
    if(opt.verbose){
        fprintf(stderr, "%s:%d %s(\"%s\")\n", __FILE_NAME__, __LINE__, __func__, path);
    }
    filler(buf, ".", NULL, 0);
    filler(buf, "..", NULL, 0);

    char *path2 = (char *)path;
    if (path[(strlen(path) - 1)] != '/'){
        // Add a / to the end
        path2 = NULL;
        char ch = '/';
        path2 = malloc(sizeof(char) * strlen(path) + 1);
        strcpy(path2, path);
        strncat(path2, &ch, 1);
    }

    char *json_payload = sia_bus_objects_json(&opt, path2);  // TODO: check if this works, or if the search endpoint is better
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
                        char *fn = strrchr(name->valuestring, '/');
                        if (fn == NULL){
                            fn = name->valuestring;
                        }
                        if(opt.verbose){
                            fprintf(stderr, "%s:%d filename: %s\n", __FILE_NAME__, __LINE__, fn+sizeof(char));
                        }
                        filler(buf, fn+sizeof(char), NULL, 0);
                    }
                }
            }
            cJSON_Delete(monitor_json);
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
    char *payload = sia_worker_get_object(&opt, path, payload_size, payload_offset);
    memcpy(buf, payload + offset, size - offset);
    free(payload);
    return payload_size - payload_offset;
}

int siafs_mkdir(const char *path, mode_t mode){
    if(opt.verbose){
        fprintf(stderr, "%s:%d %s(\"%s\", %d)\n", __FILE_NAME__, __LINE__, __func__, path, mode);
    }

    char *path2 = NULL;
    if (path[(strlen(path) - 1)] != '/'){
        char *path3 = (char *)path;
        char ch = '/';
        path2 = malloc(sizeof(char) * strlen(path3) + 1);
        strcpy(path2, path3);
        strncat(path2, &ch, 1);
    }

    sia_worker_put_object(&opt, path2, 0, 0, NULL);
    if (path2 != NULL){
        free(path2);
    }
    return 0;
}

int siafs_mknod(const char *path, mode_t mode, dev_t rdev){
    if(opt.verbose){
        fprintf(stderr, "%s:%d %s(\"%s\", %d, %lu)\n", __FILE_NAME__, __LINE__, __func__, path, mode, rdev);
    }
    sia_worker_put_object(&opt, path, 0, 0, NULL);
    return 0;
}

int siafs_write(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *info){
    if(opt.verbose){
        fprintf(stderr, "%s:%d %s(\"%s\", \"%s\", %zu, %ld)\n", __FILE_NAME__, __LINE__, __func__, path, "(buf)", size, offset);
    }
    if ((offset == 0) && (size < 4096)){
        // Smaller files are sent through the simple API
        sia_worker_put_object(&opt, path, size, offset, (void *)buf);
    }
    else{
        char *upload_id = sia_bus_get_uploadid(&opt, path);
        char *etag = sia_worker_put_multipart(&opt, path, upload_id, size, offset, (void *)buf);
        if (etag != NULL){
            sia_upload_t *upload;

            if (offset == 0){
                upload = malloc(sizeof(sia_upload_t));
                upload->name = malloc(sizeof(const char) * strlen(path) + 1);
                strcpy(upload->name, path);
                upload->uploadID = malloc(sizeof(upload->uploadID ) * strlen(upload_id) + 1);
                strcpy(upload->uploadID, upload_id);
                for (int i = 0; i < SIA_MAX_PARTS; i++){
                    upload->part[i].etag = NULL;
                }
                opt.uploads = append_upload(&opt, upload);
            }
            else{
                upload = find_upload_by_path(&opt, path);
            }
            off_t slot = offset / 4096;
            upload->part[slot].etag = etag;
            if(opt.verbose){
                fprintf(stderr, "%s:%d Upload Name: %s uploadID: %s\n", __FILE_NAME__, __LINE__, upload->name, upload->uploadID);
                fprintf(stderr, "%s:%d Slot: %ld eTag: %s\n", __FILE_NAME__, __LINE__, slot, upload->part[slot].etag);
            }

            for (int i = 0; (i <= slot); i++){
                fprintf(stderr, "%s:%d\t%d: %s\n", __FILE_NAME__, __LINE__, i, upload->part[i].etag);
            }
        }
        else{
            return 0;
        }
    }
	return size;
}

int siafs_release(const char *path, struct fuse_file_info *info){
    if(opt.verbose){
        fprintf(stderr, "%s:%d %s(\"%s\", \"%s\")\n", __FILE_NAME__, __LINE__, __func__, path, "(info)");
    }

    if (sia_bus_objects_is_file(&opt, path) || (sia_bus_object_size(&opt, path) == 0)){
        // the file is zero bytesthen it might a multipart write op
        sia_upload_t *upload = find_upload_by_path(&opt, path);
        if (upload != NULL){
            char *etag = sia_bus_multipart_complete_json(&opt, path, upload->uploadID);
        }
    }
    return 0;
}

int siafs_open(const char *path, struct fuse_file_info *info){
    if(opt.verbose){
        fprintf(stderr, "%s:%d %s(\"%s\", \"%s\")\n", __FILE_NAME__, __LINE__, __func__, path, "(info)");
    }
    if (!sia_bus_objects_exists(&opt, path))
		return -ENOENT;

	if (sia_bus_objects_is_dir(&opt, path))
		return -EISDIR;

//	if ((info->flags & 3) != O_RDONLY)
//		return -EACCES;
    return 0;
}

int siafs_getxattr(const char *path, const char *key, char *val, size_t sz){
    if(opt.verbose){
        fprintf(stderr, "%s:%d %s(\"%s\", \"%s\", \"%s\", %ld)\n", __FILE_NAME__, __LINE__, __func__, path, key, val, sz);
    }
    return -ENOTSUP;
}

int siafs_setxattr(const char *path, const char *key, const char *val, size_t sz, int flags){
    if(opt.verbose){
        fprintf(stderr, "%s:%d %s(\"%s\", \"%s\", \"%s\", %ld, %d)\n", __FILE_NAME__, __LINE__, __func__, path, key, val, sz, flags);
    }
    return -ENOTSUP;
}

int siafs_unlink(const char *path){
    if(opt.verbose){
        fprintf(stderr, "%s:%d %s(\"%s\")\n", __FILE_NAME__, __LINE__, __func__, path);
    }
    sia_bus_del_object(&opt, path);
    return 0;
}
