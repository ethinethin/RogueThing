#ifndef PLAYER_H
#define PLAYER_H

#include "map.h"

struct playerspace {
	int x;
	int y;
	int cur_floor;
	int cur_time;
	char name[18];
};

extern void			move_player(struct mapspace *map, struct playerspace *player, int x, int y);
extern struct playerspace *	init_playerspace(int x, int y);
extern void			kill_playerspace(struct playerspace *player);
extern void			move_floors(struct mapspace *map, struct playerspace *player, int z, int maxfloor);

#endif
