#ifndef SIA_WORKER_OBJECTS_H
#define SIA_WORKER_OBJECTS_H
#ifdef __cplusplus
extern "C"
{
#endif

#include "sia_common.h"

char *sia_worker_get_object(sia_cfg_t *opt, const char *path, size_t size, off_t offset);
char *sia_worker_put_object(sia_cfg_t *opt, const char *path, size_t size, off_t offset, void* payload);

#ifdef __cplusplus
}
#endif
#endif
