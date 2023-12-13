#ifndef MAP_H
#define MAP_H

#include "types.h"

extern int			xy2flat(int x, int y, int max_w);
extern struct mapspace *	init_mapspace(int w, int h, int floor_n, int r1_x, int r1_y);
extern void			kill_mapspace(struct mapspace *map);

enum floor_type { FLOOR_OPEN = 0, FLOOR_WALL, FLOOR_PATH, FLOOR_ROOM, FLOOR_DOOR, FLOOR_BEGIN, FLOOR_END };
extern char *floor_names[7];

#endif
