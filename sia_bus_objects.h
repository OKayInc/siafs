#ifndef SIA_BUS_OBJECTS_H
#define SIA_BUS_OBJECTS_H
#ifdef __cplusplus
extern "C"
{
#endif

#include "sia_common.h"

char *sia_bus_objects_json(sia_cfg_t *opt, const char *path);
char *sia_bus_objects_list_json(sia_cfg_t *opt, const char *path);
unsigned short int sia_bus_objects_exists(sia_cfg_t *opt, const char *path);
unsigned short int sia_bus_objects_is_dir(sia_cfg_t *opt, const char *path);
unsigned short int sia_bus_objects_is_file(sia_cfg_t *opt, const char *path);
unsigned long int sia_bus_object_size(sia_cfg_t *opt, const char *path);
char *sia_bus_objects_modtime(sia_cfg_t *opt, const char *path);
char *sia_bus_del_object(sia_cfg_t *opt, const char *path);
char *sia_bus_rename_object(sia_cfg_t *opt, const char *from, const char *to, const char *mode);
unsigned long int sia_bus_used_storage_per_directory(sia_cfg_t *opt, const char *path);

#ifdef __cplusplus
}
#endif
#endif
