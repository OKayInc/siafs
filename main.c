#define __USE_GNU
#define _XOPEN_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>
#if FUSE_USE_VERSION < 30
#include <fuse.h>
#else
#include <fuse3/fuse.h>
#endif
#include <curl/curl.h>

#if !CURL_AT_LEAST_VERSION(7, 62, 0)
#error "I require curl 7.62.0 or later"
#endif

#include "siafs.h"
#include "sia.h"

// TODO: find a better regexp
#define URL_REGEX "[a-zA-Z0-9.-:@]+(/[^[:space:]]*)?$"
/* default options */
sia_cfg_t opt = {
    .url = NULL,
    .scheme = SIA_DEFAULT_SCHEME,
    .host = SIA_DEFAULT_HOST,
    .user = SIA_DEFAUT_USER,
    .password = SIA_DEFAULT_PASSWORD,
    .port_s = SIA_DEFAULT_PORT_S,
    .port = SIA_DEFAULT_PORT,
    .bucket = SIA_DEFAULT_HOST,
    .unauthenticated_url = NULL,
    .verbose = 0,
    .maxhandle = 10,
    .uploads = NULL,
#ifdef SIA_METACACHE
    .metacache = NULL,
#endif
    .L1 = NULL,
    .L2 = NULL,
#ifdef SIA_MEMCACHED
    .servers = NULL,
    .memc = NULL,
    .payload_buffer = NULL,
    .cache_dir = NULL,
#endif
};

struct fuse_operations operations = {
    .init       = siafs_init,
    .getattr	= siafs_getattr,
    .readdir	= siafs_readdir,
    .read       = siafs_read,
    .mkdir      = siafs_mkdir,
    .mknod      = siafs_mknod,
    .write      = siafs_write,
    .release    = siafs_release,
    .open       = siafs_open,
    .flush      = siafs_flush,
#ifdef HAVE_XATTR
    .getxattr   = siafs_getxattr,
    .setxattr   = siafs_setxattr,
#endif
    .unlink     = siafs_unlink,
    .rmdir      = siafs_rmdir,
    .rename     = siafs_rename,
    .statfs     = sias_statfs,
    .access     = siafs_access,
};

int parse_url(const char *url){
    if(opt.verbose){
        fprintf(stderr, "%s(\"%s\")\n", __func__, url);
    }
    CURLU *h;
    CURLUcode uc;
    
    h = curl_url();
    if(!h)
        return 1;

    uc = curl_url_set(h, CURLUPART_URL, url, 0);
    if(uc)
        goto fail;

    uc = curl_url_get(h, CURLUPART_SCHEME, &opt.scheme, 0);    
    uc = curl_url_get(h, CURLUPART_HOST, &opt.host, 0);
    uc = curl_url_get(h, CURLUPART_USER, &opt.user, 0);
    uc = curl_url_get(h, CURLUPART_PASSWORD, &opt.password, 0);
    uc = curl_url_get(h, CURLUPART_PORT, &opt.port_s, 0);
    uc = curl_url_get(h, CURLUPART_PATH, &opt.bucket, 0);
    
    if (!strcmp(opt.bucket, "/")){
        strcpy(opt.bucket, SIA_DEFAULT_BUCKET);
    }
    
    opt.port = atol(opt.port_s);
    uc = curl_url_set(h, CURLUPART_USER, NULL, 0);
    uc = curl_url_set(h, CURLUPART_PASSWORD, NULL, 0);
    uc = curl_url_set(h, CURLUPART_PATH, NULL, 0);
    
    uc = curl_url_get(h, CURLUPART_URL, &opt.unauthenticated_url, 0);   // This adds a / at the end

    if(opt.verbose){
        fprintf(stderr, "Scheme: %s\n", opt.scheme);
        fprintf(stderr, "Host: %s\n", opt.host);
        fprintf(stderr, "Port_S: %s\n", opt.port_s);
        fprintf(stderr, "Port: %u\n", opt.port);
        fprintf(stderr, "User: %s\n", opt.user);
        fprintf(stderr, "Password: %s\n", opt.password);
        fprintf(stderr, "bucket: %s\n", opt.bucket);
        fprintf(stderr, "Unauth URL: %s\n", opt.unauthenticated_url);
    }
    
fail:
    curl_url_cleanup(h);
    return CURLUE_OK;   // 0 = OK
}

int validate_url(const char *url){
    if(opt.verbose){
        fprintf(stderr, "%s(\"%s\")\n", __func__, url);
    }
    regex_t regex;
    int reti;
    reti = regcomp(&regex, URL_REGEX, REG_EXTENDED);
    if (reti) {
        fprintf(stderr, "Could not compile regex\n");
        exit(3);
    }
    reti = regexec(&regex, url, 0, NULL, 0);
    return reti;    // 0 == OK
}

void usage(){
    fprintf(stderr, "Usage: siafs url mountpoint\n");
    fprintf(stderr, "\turl\t http|https://user:password@server:port/bucket\n");
}

static int siafs_opt_proc(void *data, const char *arg, int key, struct fuse_args *outargs){
    fprintf(stderr, "%s:%d %s(\"%s\", \"%s\", %d, outargs\")\n", __FILE_NAME__, __LINE__, __func__, (char *)data, arg, key);
    char *str;

    if(key == FUSE_OPT_KEY_OPT){
        if(!strcmp(arg, "-v")){
            opt.verbose = 1;
        }
        else if(!strncmp(arg, "maxhandle=", strlen("maxhandle="))){
            str = strchr(arg, '=') + 1;
            opt.maxhandle = atoi(str);
        }
        else{
            fuse_opt_add_arg(outargs, arg);
       }
    }else if(key == FUSE_OPT_KEY_NONOPT){
        if(!opt.url){
            opt.url = (char*)arg;
            if (validate_url(opt.url)){
                fprintf(stderr, "%s is not a valid URL\n", opt.url);
                opt.url = NULL;
            }
        }
        else{
            fuse_opt_add_arg(outargs, arg);
        }
    }
    return 0;
}

int main(int argc, char *argv[]){
    struct fuse_args args = FUSE_ARGS_INIT(argc, argv);
    fuse_opt_parse(&args, &opt, NULL, siafs_opt_proc);
    if (opt.url == NULL){
        usage();
        fprintf(stderr, "Aborting\n");
        return EXIT_FAILURE;
    }

    parse_url(opt.url);
    unsigned short int p;
    if (sia_bus_consensus_state_synced(&opt) == 1){
        fprintf(stderr, "sia_concensus_state_synced is true!\n");
        unsigned int bh = sia_bus_consensus_state_blockheight(&opt);
        fprintf(stderr, "sia_concensus_state_blockheight is %u\n", bh);
        char *lbt = sia_bus_consensus_state_lastblocktime(&opt);
        fprintf(stderr, "sia_concensus_state_lastblocktime is %s\n", lbt);

    }
    else{
        fprintf(stderr, "sia_consensus_state_synced is false!\n");
        exit(EXIT_FAILURE);
    }

    fuse_main(args.argc, args.argv, &operations, NULL);
    fuse_opt_free_args(&args);
    exit(EXIT_SUCCESS);
}
