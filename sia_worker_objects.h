#ifndef SIA_WORKER_OBJECTS_H
#define SIA_WORKER_OBJECTS_H
#ifdef __cplusplus
extern "C"
{
#endif

#include "sia_common.h"

char *sia_worker_get_object(sia_cfg_t *opt, const char *path, size_t size, off_t offset, size_t *payload_size);
char *sia_worker_put_multipart_from_file(sia_cfg_t *opt, const char *path, const char *uploadid, size_t size, off_t offset, void *ctx, off_t slot);
char *sia_worker_put_multipart(sia_cfg_t *opt, const char *path, const char *uploadid, size_t size, off_t offset, void *ctx, off_t slot);
char *sia_worker_put_object(sia_cfg_t *opt, const char *path, size_t size, off_t offset, void* payload);

#ifdef __cplusplus
}
#endif
#endif
