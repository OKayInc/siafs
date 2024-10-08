#ifndef SIAFS_H
#define SIAFS_H

#define __USE_GNU
#define _XOPEN_SOURCE
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>
#if FUSE_USE_VERSION < 30
#include <fuse.h>
#else
#include <fuse3/fuse.h>
#endif

#if !defined (DISABLE_XATTR)
	#if defined (LIBC_XATTR)
		#include <sys/xattr.h>
	#elif defined (LIBATTR_XATTR)
		#include <attr/xattr.h>
	#else
		#error // neither libc attr nor libattr xattr defined
	#endif

	#if defined (XATTR_CREATE) && defined (XATTR_REPLACE)
		#define HAVE_XATTR
	#endif
#endif

#ifdef HAVE_XATTR
#include <sys/xattr.h>
#endif

#if FUSE_USE_VERSION < 30
#define SIAFS_SMALL_FILE_SIZE_BYTES   	4096
#define SIAFS_SMALL_FILE_SIZE_KBYTES   4
#else
#define SIAFS_SMALL_FILE_SIZE_BYTES   	131072
#define SIAFS_SMALL_FILE_SIZE_KBYTES	128
#endif


#define SIAFS_MULTIPARTS 			10000
// MUST BE 1024 MULTIPLES
#define SAIFS_WRITES_PER_MULTIPART	1024
#define SIAFS_MAX_FILE_SIZE_BYTES	(SIAFS_MULTIPARTS * SAIFS_WRITES_PER_MULTIPART * SIAFS_SMALL_FILE_SIZE_KBYTES)
#define SIAFS_MAX_FILE_SIZE_KBYTES	(SIAFS_MAX_FILE_SIZE_BYTES / 1024)
#define SIAFS_MAX_FILE_SIZE_MBYTES	(SIAFS_MAX_FILE_SIZE_KBYTES / 1024)
#define SIAFS_MAX_FILE_SIZE_GBYTES	(SIAFS_MAX_FILE_SIZE_MBYTES / 1024)

#define KB_IN_GB				1048576

#define SIAFS_ROOT_CACHE_DIR	"/var/cache/siafs/"

#include "sia.h"
#include "homedir.h"

char *sia_concensus_state(sia_cfg_t *opt);

int siafs_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi);
int siafs_mkdir(const char *path, mode_t mode);
int siafs_mknod(const char *path, mode_t mode, dev_t rdev);
int siafs_write( const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *info );
int siafs_release(const char *path, struct fuse_file_info *info);
int siafs_open(const char *path, struct fuse_file_info *info);
int siafs_flush(const char *path, struct fuse_file_info *info);

#ifdef HAVE_XATTR
#warning HAVE_XATTR defined
int siafs_getxattr(const char *path, const char *key, char *val, size_t sz);
int siafs_setxattr(const char *path, const char *key, const char *val, size_t sz, int flags);
#endif
int siafs_unlink(const char *path);
int siafs_rmdir(const char *path);
int siafs_access(const char *path, int mask);

#if FUSE_USE_VERSION < 30
#warning FUSE_USE_VERSION < 30
int siafs_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi);
int siafs_getattr(const char *path, struct stat *stbuf);
int siafs_rename(const char *from, const char *to);
void *siafs_init(struct fuse_conn_info *conn);
#else
#warning FUSE_USE_VERSION >= 30
int siafs_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi, enum fuse_readdir_flags flags);
int siafs_getattr(const char *path, struct stat *stbuf, struct fuse_file_info *fi);
int siafs_rename(const char *from, const char *to, unsigned int flags);
void *siafs_init(struct fuse_conn_info *conn, struct fuse_config *cfg);
#endif
#endif

int sias_statfs(const char *path, struct statvfs *stbuf);

