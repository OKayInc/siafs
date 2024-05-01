#include "siafs.h"
#include "sia.h"

extern sia_cfg_t opt;
#ifdef SIA_MEMCACHED
extern memcached_server_st *servers;
extern memcached_st *memc;
#endif

#if FUSE_USE_VERSION < 30
int siafs_getattr(const char *path, struct stat *stbuf){
#else
int siafs_getattr(const char *path, struct stat *stbuf, struct fuse_file_info *fi){
#endif
    if(opt.verbose){
        fprintf(stderr, "%s:%d %s(\"%s\")\n", __FILE_NAME__, __LINE__, __func__, path);
    }
    memset(stbuf, 0, sizeof(struct stat));

    stbuf->st_uid = fuse_get_context()->uid;
    stbuf->st_gid = fuse_get_context()->gid;
    stbuf->st_atime = time(NULL);

    if(!strcmp(path, "/")){
        if(opt.verbose){
            fprintf(stderr, "%s:%d / directory\n", __FILE_NAME__, __LINE__);
        }
        stbuf->st_ino = 1;
        stbuf->st_mode = S_IFDIR | 0755;
        stbuf->st_nlink = 2;
        stbuf->st_mtime = time(NULL);   // If / sends todays mod time, SIA doesn't return information for /
    }
    else{

        if (sia_bus_objects_is_file(&opt, path) == 1 ){
            if(opt.verbose){
                fprintf(stderr, "%s:%d %s is a file\n", __FILE_NAME__, __LINE__, path);
            }
            stbuf->st_mode = S_IFREG | 0644;
            stbuf->st_nlink = 1;
            stbuf->st_size = sia_bus_object_size(&opt, path);
            stbuf->st_mtime = sia_bus_objects_unixtime(&opt, path);
            if(opt.verbose){
                fprintf(stderr, "%s:%d file size: %ld\n", __FILE_NAME__, __LINE__, stbuf->st_size);
            }
        }
        else if (sia_bus_objects_is_dir(&opt, path) == 1 ){
            if(opt.verbose){
                fprintf(stderr, "%s:%d %s is a directory\n", __FILE_NAME__, __LINE__, path);
            }
            stbuf->st_mode = S_IFDIR | 0755;
            stbuf->st_nlink = 2;
            stbuf->st_mtime = sia_bus_objects_unixtime(&opt, path);
        }
        else{
            return -ENOENT;
        }
    }
    return 0;
}
#if FUSE_USE_VERSION < 30
int siafs_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi){

#else
int siafs_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi, enum fuse_readdir_flags flags){
#endif
    if(opt.verbose){
        fprintf(stderr, "%s:%d %s(\"%s\")\n", __FILE_NAME__, __LINE__, __func__, path);
    }
#if FUSE_USE_VERSION < 30
    filler(buf, ".", NULL, 0);
    filler(buf, "..", NULL, 0);
#else
    filler(buf, ".", NULL, 0, 0);
    filler(buf, "..", NULL, 0, 0);
#endif
    char *path2 = (char *)path;
    if (path[(strlen(path) - 1)] != '/'){
        // Add a / to the end
        path2 = NULL;
        char ch = '/';
        path2 = malloc(sizeof(char) * strlen(path) + 1);
        if (path2 == NULL){
            return -ENOMEM;
        }
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
#if FUSE_USE_VERSION < 30
                        filler(buf, fn+sizeof(char), NULL, 0);
#else
                        filler(buf, fn+sizeof(char), NULL, 0, 0);
#endif

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
        fprintf(stderr, "%s:%d %s(\"%s\", \"%s\", %lu, %lu)\n", __FILE_NAME__, __LINE__, __func__, path, "(buf)", size, offset);
    }
    size_t payload_size = 0;
    char *payload = sia_worker_get_object(&opt, path, size, offset, &payload_size);

    size = payload_size;

    memcpy(buf, payload, size);
    if(opt.verbose){
//        fprintf(stderr, "%s:%d Payload [%s])\n", __FILE_NAME__, __LINE__, payload);
        fprintf(stderr, "%s:%d Returning %lu)\n", __FILE_NAME__, __LINE__, size);
    }
    free(payload);
    return size;
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
        if (path2 == NULL){
            return -ENOMEM;
        }        strcpy(path2, path3);
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
    struct fuse_file_info *info;
    siafs_flush(path, info);
    return 0;
}

int siafs_write(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *info){
    if(opt.verbose){
        fprintf(stderr, "%s:%d %s(\"%s\", \"%s\", %zu, %ld)\n", __FILE_NAME__, __LINE__, __func__, path, "(buf)", size, offset);
    }
    if ((offset == 0) && (size < SIAFS_SMALL_FILE_SIZE_BYTES)){
        // Smaller files are sent through the simple API
        sia_worker_put_object(&opt, path, size, offset, (void *)buf);
    }
    else{
#ifdef SIA_HUGE_FILES
        char *upload_id = sia_bus_get_uploadid(&opt, path);
        if(opt.verbose){
            fprintf(stderr, "%s:%d Upload ID: %s\n", __FILE_NAME__, __LINE__, upload_id);
        }
        sia_upload_t *upload = NULL;
        if (offset == 0){
            // Create the upload node
            upload = create_upload(&opt, path, upload_id);
            if (upload == NULL){
                return -ENOMEM;
            }
            opt.uploads = append_upload(&opt, upload);
        }
        else{
            // Finde the upload node
            upload = find_upload_by_path(&opt, path);
            if (upload == NULL){
                return -ENOENT;
            }
        }
        char tmpfn[L_tmpnam] = {0};
        unsigned int slot = offset / (unsigned int)(SIAFS_SMALL_FILE_SIZE_BYTES * SAIFS_WRITES_PER_MULTIPART);
        if(opt.verbose){
            fprintf(stderr, "%s:%d Slot: %u\n", __FILE_NAME__, __LINE__, slot);
        }
        if (offset % (unsigned int)(SIAFS_SMALL_FILE_SIZE_BYTES * SAIFS_WRITES_PER_MULTIPART) == 0){
            // Each SIAFS_BLOCKS_PER_MULTIPART, including 0 we create a new tmp file
            tmpnam(tmpfn);
            if(opt.verbose){
                fprintf(stderr, "%s:%d New TmpFN: %s\n", __FILE_NAME__, __LINE__, tmpfn);
            }
            upload->part[slot].tmpfn = malloc(sizeof(char) * strlen(tmpfn) + 1);
            strcpy(upload->part[slot].tmpfn, tmpfn);
        }
        else{
            // Temporal filename
            strcpy(tmpfn, upload->part[slot].tmpfn);
            if(opt.verbose){
                fprintf(stderr, "%s:%d Recovered TmpFN: %s\n", __FILE_NAME__, __LINE__, tmpfn);
            }
        }
        FILE *f = fopen(tmpfn,"ab");
        if (f == NULL){
            return -EIO;
        }
        fseek(f, 0, SEEK_END);
        fwrite(buf,sizeof(unsigned char), size, f);
        fclose(f);

        if ((offset % (SIAFS_SMALL_FILE_SIZE_BYTES*SAIFS_WRITES_PER_MULTIPART) <= SIAFS_SMALL_FILE_SIZE_BYTES) ||
            (offset > 0 && size < SIAFS_SMALL_FILE_SIZE_BYTES)){
            // This is the last iteration before switching
            off_t slot = offset / (SIAFS_SMALL_FILE_SIZE_BYTES * SAIFS_WRITES_PER_MULTIPART);
            if(opt.verbose){
                fprintf(stderr, "%s:%d Last iteration.\n", __FILE_NAME__, __LINE__);
                fprintf(stderr, "%s:%d Slot: %d\n", __FILE_NAME__, __LINE__, slot);
            }

            char *etag = sia_worker_put_multipart_from_file(&opt, path, upload_id, size, offset, (void *)tmpfn, slot + 1);
            if (etag != NULL){
                sia_upload_t *upload;

                if (offset == 0){
                    upload = malloc(sizeof(sia_upload_t));
                    if (upload == NULL){
                        return -ENOMEM;
                    }
                    upload->name = malloc(sizeof(const char) * strlen(path) + 1);
                    if (upload->name == NULL){
                        return -ENOMEM;
                    }
                    strcpy(upload->name, path);
                    upload->uploadID = malloc(sizeof(upload->uploadID ) * strlen(upload_id) + 1);
                    if (upload->uploadID == NULL){
                        return -ENOMEM;
                    }
                    strcpy(upload->uploadID, upload_id);
                    for (int i = 0; i < SIA_MAX_PARTS; i++){
                        upload->part[i].etag = NULL;
                        upload->part[i].tmpfn = NULL;
                    }
                    opt.uploads = append_upload(&opt, upload);
                }
                else{
                    upload = find_upload_by_path(&opt, path);
                }
//                off_t slot = offset / SIAFS_SMALL_FILE_SIZE_BYTES;
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

            // Push the multipart
        }
/**
        cJSON *file = push_file(&opt, path);
        if (file != NULL){
            // We have a leaf
            size_t b64sz = 0;
            unsigned char *base64 = base64_encode((const unsigned char *)buf, size, &b64sz);
            if (base64 != NULL){
                unsigned pn = offset % SIAFS_MULTIPART_DIVS;
                cJSON *mpart = push_payload_multipart(&opt, file, pn, (const char *)base64);
                if ((offset % SIAFS_MULTIPART_DIVS) == (SIAFS_MULTIPART_DIVS - 1)){
                    unsigned sia_mpart = offset / 8 + 1;
                    // merge the mparts
                    unsigned char *bin_payload = NULL;
                    size_t bin_payload_sz = 0;
                    for (unsigned short int i = 0; i < SIAFS_MULTIPART_DIVS; i++){
                        cJSON *mp64 = find_payload_multipart_by_number(&opt, file, i);
                        if (mp64 != NULL){
                            cJSON *part = cJSON_GetObjectItemCaseSensitive(mp64, "part");
                            cJSON *data = cJSON_GetObjectItemCaseSensitive(mp64, "data");
                            size_t bz = 0;
                            unsigned char *b = base64_decode((const unsigned char*)data->valuestring, strlen(data->valuestring), &bz);
                            unsigned char *ptr;
                            if (bin_payload == NULL){
                                ptr = calloc(bz, sizeof(unsigned char));
                            }
                            else{
                                ptr = realloc(bin_payload, bin_payload_sz + bz);
                            }
                            if(!ptr)
                                return -ENOMEM;
                            bin_payload = ptr;
                            memcpy(&(bin_payload[bin_payload_sz]), bin_payload, bin_payload_sz);
                            bin_payload_sz += bz;
                        }
                    }
                    // bin_payload has the payload
                    // bin_payload_sz has the lenght
                    // push them as a single multi_part
                    char *upload_id = sia_bus_get_uploadid(&opt, path);
                    off_t slot = offset / SIAFS_SMALL_FILE_SIZE + 1;    // TODO: review this
                    char *etag = sia_worker_put_multipart(&opt, path, upload_id, size, offset, (void *)buf, slot);
                    if (etag != NULL){
                        sia_upload_t *upload;

                        if (offset == 0){
                            upload = malloc(sizeof(sia_upload_t));
                            if (upload == NULL){
                                return -ENOMEM;
                            }
                            upload->name = malloc(sizeof(const char) * strlen(path) + 1);
                            if (upload->name == NULL){
                                return -ENOMEM;
                            }
                            strcpy(upload->name, path);
                            upload->uploadID = malloc(sizeof(upload->uploadID ) * strlen(upload_id) + 1);
                            if (upload->uploadID == NULL){
                                return -ENOMEM;
                            }
                            strcpy(upload->uploadID, upload_id);
                            for (int i = 0; i < SIA_MAX_PARTS; i++){
                                upload->part[i].etag = NULL;
#ifdef SIA_HUGE_FILES
                                upload->part[i].tmpfn = NULL;
#endif
                            }
                            opt.uploads = append_upload(&opt, upload);
                        }
                        else{
                            upload = find_upload_by_path(&opt, path);
                        }
                        off_t slot = offset / SIAFS_SMALL_FILE_SIZE;
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
                    // flush everything
                    flush_payload_multiparts(&opt, file);

                }
            }
            else{
                return -ENOMEM;
            }
        }
**/
#else
        char *upload_id = sia_bus_get_uploadid(&opt, path);
        off_t slot = offset / SIAFS_SMALL_FILE_SIZE_BYTES + 1;

        char *etag = sia_worker_put_multipart(&opt, path, upload_id, size, offset, (void *)buf, slot);
        if (etag != NULL){
            sia_upload_t *upload;

            if (offset == 0){
                upload = create_upload(&opt, path, upload_id);
                if (upload == NULL){
                    return -ENOMEM;
                }
                opt.uploads = append_upload(&opt, upload);
            }
            else{
                upload = find_upload_by_path(&opt, path);
                if (upload == NULL){
                    return -ENOENT;
                }            }
            off_t slot = offset / SIAFS_SMALL_FILE_SIZE_BYTES;
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
#endif
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

int siafs_flush(const char *path, struct fuse_file_info *info){
#ifdef SIA_MEMCACHED
    char *key1 = opt.L1->key("api/bus/objects", path, "sia_bus_objects_json");
    if(opt.verbose){
        fprintf(stderr, "%s:%d key1 %s\n", __FILE_NAME__, __LINE__, key1);
    }

    char *key2 = opt.L1->key("api/bus/objects/list", path, "sia_bus_objects_list_json");
    if(opt.verbose){
        fprintf(stderr, "%s:%d key2 %s\n", __FILE_NAME__, __LINE__, key2);
    }

    memcached_return rc;
    if (key1 != NULL){
        rc = (memcached_return)(opt.L1->del)(opt.memc, key1);
        free(key1);
    }
    if (key2 != NULL){
        rc = (memcached_return)(opt.L1->del)(opt.memc, key2);
        free(key2);
    }
#endif
    return 0;
}

#ifdef HAVE_XATTR
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
#endif

int siafs_unlink(const char *path){
    if(opt.verbose){
        fprintf(stderr, "%s:%d %s(\"%s\")\n", __FILE_NAME__, __LINE__, __func__, path);
    }
    sia_bus_del_object(&opt, path);
    return 0;
}

int siafs_rmdir(const char *path){
    if(opt.verbose){
        fprintf(stderr, "%s:%d %s(\"%s\")\n", __FILE_NAME__, __LINE__, __func__, path);
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
    sia_bus_del_object(&opt, path2);
    if (path2 != NULL)
        free(path2);
    return 0;
}

#if FUSE_USE_VERSION < 30
int siafs_rename(const char *from, const char *to){
    unsigned int flags = 0;
#else
int siafs_rename(const char *from, const char *to, unsigned int flags){
#endif
    char *from2 = (char *)from;
    char *to2 = (char *)to;
    char *mode = NULL;
    unsigned short int free_from2 = 0;
    unsigned short int free_to2 = 0;
    if(opt.verbose){
        fprintf(stderr, "%s:%d %s(\"%s, %s, %u\")\n", __FILE_NAME__, __LINE__, __func__, from, to, flags);
    }
    if (sia_bus_objects_is_dir(&opt, from) == 1 ){
        if (from[(strlen(from) - 1)] != '/'){
            // Add a / to the end
            char ch = '/';
            from2 = malloc(sizeof(char) * strlen(from) + 2);
            if (from2 == NULL){
                return -ENOMEM;
            }
            strcpy(from2, from);
            strncat(from2, &ch, 1);
            free_from2 = 1;
        }

        if (to[(strlen(to) - 1)] != '/'){
            // Add a / to the end
            char ch = '/';
            to2 = malloc(sizeof(char) * strlen(to) + 2);
            if (to2 == NULL){
                return -ENOMEM;
            }
            strcpy(to2, to);
            strncat(to2, &ch, 1);
            free_to2 = 1;
        }

        mode = malloc(sizeof(char) * 6);
        if (mode == NULL){
            return -ENOMEM;
        }
        strcpy(mode, "multi");
    }
    else{
        mode = malloc(sizeof(char) * 7);
        if (mode == NULL){
            return -ENOMEM;
        }
        strcpy(mode, "single");
    }
    sia_bus_rename_object(&opt, from2, to2, mode);
    free(mode);
    if (free_from2 == 1){
        free(from2);
    }
    if (free_to2 == 1){
        free(to2);
    }
    return 0;
}

int sias_statfs(const char *path, struct statvfs *stbuf){
    if(opt.verbose){
        fprintf(stderr, "%s(\"%s\")\n", __func__, path);
    }

    stbuf->f_bsize = 1;     /* Optimal transfer block size */
    stbuf->f_blocks = 1;    /* Total data blocks in filesystem */
    stbuf->f_bfree = 1;     /* Free blocks in filesystem */
    stbuf->f_bavail = 1;    /* Free blocks available to unprivileged user */
    stbuf->f_files = 1;     /* Total inodes in filesystem */
    stbuf->f_ffree = 1;     /* Free inodes in filesystem */
    stbuf->f_namemax = 250;  /* maximum lenght of filenames */

    unsigned long int used = sia_bus_used_storage_per_directory(&opt, "/");
    stbuf->f_blocks = sia_stats_totalStorage(&opt);// used * 1000000000000;
    stbuf->f_bfree = stbuf->f_bavail = stbuf->f_blocks - used;
    return 0;
}

#if FUSE_USE_VERSION < 30
void *siafs_init(struct fuse_conn_info *conn){
#ifdef FUSE_CAP_BIG_WRITES
    if(conn->capable & FUSE_CAP_BIG_WRITES){
        conn->want |= FUSE_CAP_BIG_WRITES;
    }
#endif
#else
void *siafs_init(struct fuse_conn_info *conn, struct fuse_config *cfg){
    cfg->auto_cache = 1;
#endif
    conn->max_write = 1 << 20;
#ifdef SIA_MEMCACHED
    opt.L1 = calloc(1, sizeof(sia_cache_t));
    opt.L1->key = mc_key;
    opt.L1->init = mc_init;
    opt.L1->set = mc_set;
    opt.L1->get = mc_get;
    opt.L1->del = mc_del;
    opt.L1->flush = mc_flush;

    memcached_return rc;
    rc = (memcached_return)(opt.L1->init)((void**)&opt.memc, (void**)&opt.servers);
    if (rc == MEMCACHED_SUCCESS){
        if(opt.verbose){
            fprintf(stderr, "Added server successfully\n");
        }
    }
    else{
        if(opt.verbose){
            fprintf(stderr, "Couldn't add server: %s\n", memcached_strerror(opt.memc, rc));
        }
        exit(EXIT_FAILURE);
    }
#endif
    // create the payload array
    opt.payload_buffer = cJSON_CreateObject();
    cJSON *files = cJSON_CreateArray();
    cJSON_AddItemToObject(opt.payload_buffer, "files", files);

   return NULL;
}

int siafs_access(const char *path, int mask){
    if(opt.verbose){
        fprintf(stderr, "%s(\"%s\", %d)\n", __func__, path, mask);
    }
	struct stat s;

#if FUSE_USE_VERSION < 30
	if (siafs_getattr(path, &s) != 0)
#else
	if (siafs_getattr(path, &s, NULL) != 0)
#endif
        return(-ENOENT);

    return 0;
}

