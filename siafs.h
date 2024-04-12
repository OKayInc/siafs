#ifndef SIAFS_H
#define SIAFS_H

#define __USE_GNU
#define _XOPEN_SOURCE
#define FUSE_USE_VERSION 30

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fuse.h>
#include <curl/curl.h>

#include "structures.h"

char *sia_concensus_state(siafs_opt_t *opt);

int siafs_getattr(const char *path, struct stat *stbuf);
int siafs_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi);
int siafs_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi);
#endif
