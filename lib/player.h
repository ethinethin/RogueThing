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
extern struct playerspace *	init_playerspace(int cx, int cy);
extern void			kill_playerspace(struct playerspace *player);

#endif
