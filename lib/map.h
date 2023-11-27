#ifndef MAP_H
#define MAP_H

struct mapspace {
	int w;
	int h;
	int *floorspace;
	char *vis;
	int n_rooms;
	int *room_x;
	int *room_y;
};

extern struct mapspace *	init_mapspace(int w, int h);
extern void			kill_mapspace(struct mapspace *map);
extern void			add_rooms(struct mapspace *map, int n_rooms);
extern void			make_paths(struct mapspace *map);
extern void			make_rooms(struct mapspace *map, int min_w, int min_h, int max_w, int max_h);
extern void			char_start(struct mapspace *map, int *cx, int *cy);
extern int			xy2flat(int x, int y, int max_w);

enum floor_type { FLOOR_OPEN, FLOOR_WALL, FLOOR_PATH, FLOOR_ROOM, FLOOR_DOOR };
extern char *floor_names[5];

#endif
