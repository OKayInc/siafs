#ifndef SIA_STATS_H
#define SIA_STATS_H
#ifdef __cplusplus
extern "C"
{
#endif

#include "sia_common.h"

char *sia_stats_json(sia_cfg_t *opt);
unsigned long long int sia_stats_totalStorage(sia_cfg_t *opt);

#ifdef __cplusplus
}
#endif
#endif
