#ifndef SIA_BUS_CONSENSUS_H
#define SIA_BUS_CONSENSUS_H
#ifdef __cplusplus
extern "C"
{
#endif

#include "sia_common.h"

char *sia_bus_consensus_state_json(sia_cfg_t *opt);
unsigned short int sia_bus_consensus_state_synced(sia_cfg_t *opt);
unsigned int sia_bus_consensus_state_blockheight(sia_cfg_t *opt);
char *sia_bus_consensus_state_lastblocktime(sia_cfg_t *opt);

#ifdef __cplusplus
}
#endif
#endif
