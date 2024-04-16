#ifndef SIA_H
#define SIA_H
#ifdef __cplusplus
extern "C"
{
#endif

#define FUSE_USE_VERSION 30
#include <fuse.h>

#include "sia_common.h"
#include "sia_bus_accounts.h"
#include "sia_bus_consensus.h"
#include "sia_bus_multiparts.h"
#include "sia_bus_objects.h"

#include "sia_worker_objects.h"

#include "structures.h"
#include "urlcode.h"

#ifdef __cplusplus
}
#endif
#endif
