#ifndef PLAYER_H
#define PLAYER_H

#include "types.h"

extern void			move_player(struct mapspace *map, struct playerspace *player, struct npc_info npcs, int x, int y);
extern void			rando_name(char name[18]);
extern struct playerspace *	init_playerspace(struct mapspace *map, int x, int y);
extern void			kill_playerspace(struct playerspace *player);
extern void			move_floors(struct mapspace *map, struct playerspace *player, int z, int maxfloor);
extern void			check_vis(struct mapspace *map, struct playerspace *player);

#endif
