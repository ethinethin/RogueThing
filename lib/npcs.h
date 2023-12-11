#ifndef NPCS_H
#define NPCS_H

#include "player.h"

struct npcspace {
	int x;
	int y;
	int cur_floor;
	char name[18];
	struct stats stats;
	int personality;
};

struct npc_info {
	int n_npcs;
	int *i;
	int *x;
	int *y;
};

extern void	init_npcspace(struct mapspace **mapwallet, int n_floors, int n_npcs);
extern void	kill_npcspace(void);
extern void	npcs_info(int cur_floor, struct npc_info *npcs);

#endif
