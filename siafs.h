#ifndef SIAFS_H
#define SIAFS_H

#define __USE_GNU
#define _XOPEN_SOURCE
#define FUSE_USE_VERSION 30

#include <errno.h>
#include <fuse.h>
#include "sia.h"

char *sia_concensus_state(sia_cfg_t *opt);

int siafs_getattr(const char *path, struct stat *stbuf);
int siafs_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi);
int siafs_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi);
int siafs_mkdir(const char *path, mode_t mode);
int siafs_mknod(const char *path, mode_t mode, dev_t rdev);
int siafs_write( const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *info );
int siafs_release(const char *path, struct fuse_file_info *info);

#endif
