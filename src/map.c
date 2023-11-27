#include <stdlib.h>
#include "libs.h"
#include "map.h"
#include "rand.h"

static double	distp(int x1, int y1, int x2, int y2);
static void	check_distp(struct mapspace *map);
static void	make_path(struct mapspace *map, int room1, int room2);
static int	check_paths(struct mapspace *map);
static void	carve_room(struct mapspace *room);
static void	place_room(struct mapspace *map, struct mapspace *room, int x, int y);

char *floor_names[5] = { "Open floor", "Stone wall", "Open path", "", "Door" };

struct mapspace *
init_mapspace(int w, int h)
{
	int i;
	struct mapspace *map;

	map = malloc(sizeof(*map));
	map->w = w;
	map->h = h;
	map->floorspace = malloc(sizeof(*map->floorspace) * w * h);
	map->vis = malloc(sizeof(*map->vis) * w * h);
	for (i = 0; i < w * h; i += 1) {
		*(map->floorspace + i) = FLOOR_WALL;
		*(map->vis + i) = 0;
	}
	map->n_rooms = 0;
	map->room_x = NULL;
	map->room_y = NULL;
	return map;
}

void
kill_mapspace(struct mapspace *map)
{
	free(map->floorspace);
	free(map->vis);
	if (map->room_x != NULL) {
		free(map->room_x);
		free(map->room_y);
	}
	free(map);
}

static double
distp(int x1, int y1, int x2, int y2)
{
	int a, b;
	double c;

	/* Pythagorean theorem */
	a = x1 - x2;
	b = y1 - y2;
	c = sqrt(pow(a, 2) + pow(b, 2));	
	return c;
}

#define MIN_DIST 30
static void
check_distp(struct mapspace *map)
{
	int i, j;
	int count;
	double dist;
	
	for (i = 0, count = 0, dist = 0.0; i < map->n_rooms - 1; i += 1) {
		for (j = i + 1; j < map->n_rooms; j += 1) {
			dist += distp(*(map->room_x + i), *(map->room_y + i), *(map->room_x + j), *(map->room_y + j));
			count += 1;
		}
	}
	dist /= count;
	/* If rooms are too close, void them out and try to add again */
	if (dist < MIN_DIST) {
		for (i = 0; i < map->n_rooms; i += 1) {
			*(map->floorspace + xy2flat(*(map->room_x + i), *(map->room_y + i), map->w)) = FLOOR_WALL;
		}
		free(map->room_x);
		free(map->room_y);
		add_rooms(map, map->n_rooms);
	}
}

void
add_rooms(struct mapspace *map, int n_rooms)
{
	int i, x, y;

	/* Initialize rooms */
	map->n_rooms = n_rooms;
	map->room_x = malloc(sizeof(*map->room_x) * n_rooms);
	map->room_y = malloc(sizeof(*map->room_y) * n_rooms);
	for (i = 0; i < n_rooms; i += 1) {
		x = rand_num(5, 94);
		y = rand_num(2, 22);
		*(map->room_x + i) = x;
		*(map->room_y + i) = y;
		*(map->floorspace + xy2flat(x, y, map->w)) = FLOOR_ROOM;
	}
	/* Make sure rooms are far enough apart */
	check_distp(map);
}

void
make_paths(struct mapspace *map)
{
	int i, j, r;

	for (i = 0; i < map->n_rooms - 1; i += 1) {
		for (j = i + 1; j < map->n_rooms; j += 1) {
			if (j == i) continue;
			r = rand_num(0, 99);
			if (r > 50) {
				make_path(map, i, j);
			}
		}
	}
	check_paths(map);
}

static void
make_path(struct mapspace *map, int room1, int room2)
{
	int r, x, y;

	x = *(map->room_x + room1);
	y = *(map->room_y + room1);
	while (1) {
		r = rand_num(0, 5);
		switch (r) {
			case 0:
				if (*(map->room_y + room2) < y) {
					y -= 1;
				}
				break;
			case 1:
			case 2:
				if (*(map->room_x + room2) > x) {
					x += 1;
				}
				break;
			case 3:
				if (*(map->room_y + room2) > y) {
					y += 1;
				}
				break;
			case 4:
			case 5:
				if (*(map->room_x + room2) < x) {
					x -= 1;
				}
				break;
		}
		if (x == *(map->room_x + room2) && y == *(map->room_y + room2)) break;
		if (*(map->floorspace + xy2flat(x, y, map->w)) == FLOOR_WALL) *(map->floorspace + xy2flat(x, y, map->w)) = FLOOR_PATH;
	}
}

static int
check_paths(struct mapspace *map)
{
	return 0;
}

void
make_rooms(struct mapspace *map, int min_w, int min_h, int max_w, int max_h)
{
	int i, w, h;
	struct mapspace *room;
	
	for (i = 0; i < map->n_rooms; i += 1) {
		w = rand_num(min_w, max_w);
		h = rand_num(min_h, max_h);
		room = init_mapspace(w, h);
		carve_room(room);
		place_room(map, room, *(map->room_x + i), *(map->room_y + i));
		kill_mapspace(room);
	}	
}

static void
carve_room(struct mapspace *room)
{
	int i, j;

	for (j = 1; j < room->h - 1; j += 1) {
		for (i = 1; i < room->w - 1; i += 1) {
			*(room->floorspace + xy2flat(i, j, room->w)) = FLOOR_OPEN;
		}
	}
}

static void
place_room(struct mapspace *map, struct mapspace *room, int x, int y)
{
	int f, i, j;
	int m_f, r_f;

	/* Place room so coordinate is center */
	x = x - room->w / 2;
	y = y - room->h / 2;
	/* Check if room will be off map */
	if (x < 1) x = 1;
	if (x > map->w - room->w - 1) x = map->w - room->w - 1;
	if (y < 1) y = 1;
	if (y > map->h - room->h - 1) y = map->h - room->h - 1;
	/* Place the map */
	for (j = 0; j < room->h; j += 1) {
		for (i = 0; i < room->w; i += 1) {
			m_f = *(map->floorspace + xy2flat(x + i, y + j, map->w));
			r_f = *(room->floorspace + xy2flat(i, j, room->w));
			if (m_f == FLOOR_PATH && r_f == FLOOR_WALL) {
				if (rand_num(0, 5) == 0) {
					f = FLOOR_DOOR;
				} else {
					f = FLOOR_OPEN;
				}
			} else if (m_f == FLOOR_OPEN && r_f == FLOOR_WALL) {
				f = FLOOR_OPEN;
			} else {
				f = r_f;
			}
			*(map->floorspace + xy2flat(x + i, y + j, map->w)) = f;
		}
	}		
}

void
char_start(struct mapspace *map, int *cx, int *cy)
{
	while (1) {
		*cx = rand_num(0, 99);
		*cy = rand_num(0, 24);
		if (*(map->floorspace + xy2flat(*cx, *cy, map->w)) == FLOOR_OPEN) break;
	}

}

int
xy2flat(int x, int y, int max_w)
{
	return y * max_w + x;
}
