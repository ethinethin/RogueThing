#ifndef MAP_H
#define MAP_H

struct mapspace {
	int cur_floor;
	int w;
	int h;
	int *floorspace;
	char *vis;
	int n_rooms;
	int *room_x;
	int *room_y;
	int begin[2];
	int end[2];
};

extern struct mapspace *	init_mapspace(int w, int h, int is_room);
extern void			kill_mapspace(struct mapspace *map);
extern int			xy2flat(int x, int y, int max_w);

enum floor_type { FLOOR_OPEN, FLOOR_WALL, FLOOR_PATH, FLOOR_ROOM, FLOOR_DOOR, FLOOR_BEGIN, FLOOR_END };
extern char *floor_names[7];

#endif
