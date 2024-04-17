#ifndef SIA_BUS_MULTIPARTS_H
#define SIA_BUS_MULTIPARTS_H
#ifdef __cplusplus
extern "C"
{
#endif

#include "sia_common.h"

char *sia_bus_get_uploadid(sia_cfg_t *opt, const char *path);
char *sia_bus_multipart_abort_json(sia_cfg_t *opt, const char *path, const char *uploadid);
char *sia_bus_multipart_complete_json(sia_cfg_t *opt, const char *path, const char *uploadid);
char *sia_bus_multipart_create_json(sia_cfg_t *opt, const char *path, const char *key);
char *sia_bus_multipart_listparts_json(sia_cfg_t *opt, const char *path, const char *uploadid);
char *sia_bus_multipart_listuploads_json(sia_cfg_t *opt, const char *path);


#ifdef __cplusplus
}
#endif
#endif
