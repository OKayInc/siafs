#ifdef __cplusplus
extern "C"
{
#endif

#include "disk_cache.h"

char *disk_key(const char *path){
    char *shash = NULL;
    if (path != NULL){
        char *k = NULL;
        unsigned int kl = strlen(path) + 1;
        k = (char *)calloc(kl, sizeof(char));
        if (k != NULL){
            strcpy(k, path);

            uint8_t hash[16];
            shash = calloc(33, sizeof(char));
            if (shash != NULL){
                md5String(k, hash);
                for(unsigned int i = 0; i < 16; ++i){
                    sprintf(shash + strlen(shash),"%02x", hash[i]);
                }

                if (strlen(shash) > 250){
                    fprintf(stderr, "%s:%d WARNING MC Key: %s is too long. Sending NULL. \n", __FILE_NAME__, __LINE__, shash);
                }
            }
            free(k);
            k = NULL;
        }
    }
    return shash;
}

int disk_init(const char *cache_dir){
    int status = 0;
    struct stat st = {0};
    if (stat(cache_dir, &st) == -1) {
        status = mkdir(cache_dir, 0700);
    }

    return status;
}

// cachedir / md5sum / size.offset.dat
// cachedir / md5sum / metadata.txt
unsigned int disk_set(const char *cache_dir, const char *key, const void *payload, const unsigned long int payload_len, size_t size, off_t offset, char *etag){
    fprintf(stderr, "%s:%d %s(\"%s\", \"%s\", \"%s\", %lu, %lu, %lu, \"%s\")\n", __FILE_NAME__, __LINE__, __func__, cache_dir, key, "*payload", payload_len, size, offset, etag);
    unsigned int answer = 0;
    size_t klen = strlen(key);
    // cache_dir should end with /
    char *full_path = calloc(strlen(cache_dir) + strlen(key) + 2, sizeof(char));
    if (full_path){
        strcpy(full_path, cache_dir);
        strcat(full_path, key);
        struct stat st = {0};
        int st_result = stat(full_path, &st);

        if (st_result != 0) {
            int status = mkdir(full_path, 0700);
            if (status != 0){
                // Error, directory couldn't be created or accessed
                fprintf(stderr, "Couldn't create %s.\n", full_path);
                return answer;
            }
        }

        st_result = stat(full_path, &st);   // make the test again
        if (st_result == 0 && S_ISDIR(st.st_mode)){
            //char *fn = calloc(strlen(full_path) + (int)ceil(log10(offset)) + (int)ceil(log10(size)) + 10, sizeof(char));
            char *fn = calloc(strlen(full_path) + 64 + 10, sizeof(char));

            if (fn){
                sprintf(fn, "%s/%ld%s%ld%s", full_path, size, ".", offset, ".dat");      // This is the formula for the naming
                fprintf(stderr, "%s:%d %s fn: %s\n", __FILE_NAME__, __LINE__, __func__, fn);
                FILE *f = fopen(fn, "wb");
                if (f == NULL){
                    fprintf(stderr, "Couldn't create %s.\n", fn);
                }
                else{
                    fwrite(payload, payload_len, 1, f);
                    fclose(f);
                    answer = 1;
                }
                free(fn);
            }
        }
        else{
            fprintf(stderr, "Couldn't access %s.\n", full_path);
        }
        free(full_path);
    }

    fprintf(stderr, "%s:%d %s answer: %d\n", __FILE_NAME__, __LINE__, __func__, answer);
    return answer;
}

unsigned int disk_get(const char *cache_dir, const char *key, void **payload, unsigned long int *payload_len, size_t size, off_t offset){
    fprintf(stderr, "%s:%d %s(\"%s\", \"%s\", \"%s\", %lu, %lu, %lu)\n", __FILE_NAME__, __LINE__, __func__, cache_dir, key, "*payload", *payload_len, size, offset);
    unsigned int answer = 0;
    size_t klen = strlen(key);
    // cache_dir should end with /
    char *full_path = calloc(strlen(cache_dir) + strlen(key) + 2, sizeof(char));
    strcpy(full_path, cache_dir);
    strcat(full_path, key);
    struct stat st = {0};
    if (stat(full_path, &st) == -1) {
        fprintf(stderr, "%s doesn't exist.\n", full_path);
    }
    else{
        // char *fn = calloc(strlen(full_path) + (int)ceil(log10(offset)) + (int)ceil(log10(size)) + 10, sizeof(char));
        char *fn = calloc(strlen(full_path) + 64 + 10, sizeof(char));
        sprintf(fn, "%s/%ld%s%ld%s", full_path, size, ".", offset, ".dat");      // This is the formula for the naming
        fprintf(stderr, "%s:%d %s fn: %s\n", __FILE_NAME__, __LINE__, __func__, fn);
        FILE *f = fopen(fn, "rb");
        if (f == NULL){
            fprintf(stderr, "Couldn't open %s.\n", fn);
        }
        else{
            *payload = malloc(size);
            *payload_len = fread(*payload, 1, size, f);
            fclose(f);
            answer = 1;
        }
        free(fn);
    }
    free(full_path);

    return answer;
}

unsigned int disk_del(const void *memc, const char *key){
    memcached_return_t rc;
    size_t klen = strlen(key);

    rc = memcached_delete((memcached_st *)memc, key, klen, (time_t)0);
    if (rc != MEMCACHED_SUCCESS)
        fprintf(stderr, "Couldn't delete key: %s\n", memcached_strerror(memc, rc));

    return (unsigned int)rc;
}

unsigned int disk_flush(const void *memc){
    memcached_return_t rc;
    rc = memcached_flush((memcached_st *)memc, 0);
    return (unsigned int)rc;
}
#ifdef __cplusplus
}
#endif

