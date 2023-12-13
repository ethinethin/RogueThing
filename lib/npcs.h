#ifndef NPCS_H
#define NPCS_H

#include "types.h"

extern void	init_npcspace(struct mapspace **mapwallet, int n_floors, int n_npcs);
extern void	kill_npcspace(void);
extern void	npcs_info(int cur_floor, struct npc_info *npcs);
extern char *	get_name(int n);
extern int	get_n_npcs(void);
extern void	format_npc_for_saving(int n, char *s);
extern void	add_loaded_npc(int n_npcs, int n, char name[20], int stats[15]);

#endif
