#ifndef PLAYER_H
#define PLAYER_H

#include "map.h"

struct playerspace {
	int x;
	int y;
	int cur_floor;
	int cur_time;
	char name[18];
	char *vis;
	struct stats {
		int level;
		int hp;
		int maxhp;
		int sp;
		int maxsp;
		int pa;
		int pd;
		int ra;
		int rd;
		int bp;
		int ep;
	} stats;
};

extern void			move_player(struct mapspace *map, struct playerspace *player, int x, int y);
extern struct playerspace *	init_playerspace(struct mapspace *map, int x, int y);
extern void			kill_playerspace(struct playerspace *player);
extern void			move_floors(struct mapspace *map, struct playerspace *player, int z, int maxfloor);
extern void			check_vis(struct mapspace *map, struct playerspace *player);

#endif
