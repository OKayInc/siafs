#ifndef SIA_BUS_OBJECTS_H
#define SIA_BUS_OBJECTS_H
#ifdef __cplusplus
extern "C"
{
#endif

#include "sia_common.h"

char *sia_bus_objects_json(sia_cfg_t *opt, const char *path);
unsigned short int sia_bus_objects_is_dir(sia_cfg_t *opt, const char *path);
unsigned short int sia_bus_objects_is_file(sia_cfg_t *opt, const char *path);
unsigned int sia_bus_objects_size(sia_cfg_t *opt, const char *path);
char *sia_bus_objects_modtime(sia_cfg_t *opt, const char *path);

#ifdef __cplusplus
}
#endif
#endif
