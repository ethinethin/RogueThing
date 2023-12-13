#include <stdlib.h>
#include <unistd.h>
#include "libs.h"
#include "map.h"
#include "rand.h"
#include "types.h"

static void	wallify_map(struct mapspace *map, int w, int h);
static void	add_rooms(struct mapspace *map, int r1_x, int r1_y);
static void	make_paths(struct mapspace *map);
static void	make_path(struct mapspace *map, int room1, int room2);
static int	fill_point(struct mapspace *map, char *connected, int x, int y);
static void	place_rooms(struct mapspace *map, int min_w, int min_h, int max_w, int max_h);
static void	place_room(struct mapspace *map, int x, int y, int w, int h);
static void	place_exits(struct mapspace *map, int r1_x, int r1_y);
static int	check_distp(struct mapspace *map);
static double	distp(int x1, int y1, int x2, int y2);
static int	check_paths(struct mapspace *map);
static int	check_area(struct mapspace *map);

char *floor_names[7] = { "Open floor", "Stone wall", "Open path", "", "Door", "A staircase down", "A staircase up" };

#define MIN_DIST 4.3

int
xy2flat(int x, int y, int max_w)
{
	return y * max_w + x;
}

struct mapspace *
init_mapspace(int w, int h, int floor_n, int r1_x, int r1_y)
{
	struct mapspace *map;

	map = malloc(sizeof(*map));
	map->cur_floor = floor_n;
	map->w = w;
	map->h = h;
	map->floorspace = malloc(sizeof(*map->floorspace) * w * h);
	map->explored = malloc(sizeof(*map->explored) * w * h);
	map->n_rooms = rand_num(7, 9);
	map->room_x = malloc(sizeof(*map->room_x) * map->n_rooms);
	map->room_y = malloc(sizeof(*map->room_y) * map->n_rooms);
	/* add paths and rooms and make sure they're all connected */
	do {
		wallify_map(map, w, h);
		add_rooms(map, r1_x, r1_y);
		make_paths(map);
		place_rooms(map, 8, 4, 12, 5);
		place_exits(map, r1_x, r1_y);
	} while (check_distp(map) < MIN_DIST * map->n_rooms || check_paths(map) == 1 || check_area(map) < (map->n_rooms * 70));
	return map;
}

void
kill_mapspace(struct mapspace *map)
{
	free(map->floorspace);
	free(map->explored);
	if (map->room_x != NULL) {
		free(map->room_x);
		free(map->room_y);
	}
	free(map);
}

static void
wallify_map(struct mapspace *map, int w, int h)
{
	int i;

	for (i = 0; i < w * h; i += 1) {
		*(map->floorspace + i) = FLOOR_WALL;
		*(map->explored + i) = 0;
	}
}

static void
add_rooms(struct mapspace *map, int r1_x, int r1_y)
{
	int i, start, x, y;

	if (r1_x == -1) {
		start = 0;
	} else {
		start = 1;
		*(map->room_x + 0) = r1_x;
		*(map->room_y + 0) = r1_y;
	}
	for (i = start; i < map->n_rooms; i += 1) {
		x = rand_num(5, 94);
		y = rand_num(2, 22);
		*(map->room_x + i) = x;
		*(map->room_y + i) = y;
	}
}

static void
make_paths(struct mapspace *map)
{
	int i, j, r;

	for (i = 0; i < map->n_rooms - 1; i += 1) {
		for (j = i + 1; j < map->n_rooms; j += 1) {
			r = rand_num(0, 99);
			if (r >= 55) {
				make_path(map, i, j);
			}
		}
	}
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
		*(map->floorspace + xy2flat(x, y, map->w)) = FLOOR_PATH;
		if (x == *(map->room_x + room2) || y == *(map->room_y + room2)) break;
	}
}

static int
fill_point(struct mapspace *map, char *connected, int x, int y)
{
	int changes, dx, dy;
	
	changes = 0;
	for (dx = -1; dx <= 1; dx += 1) {
		for (dy = -1; dy <= 1; dy += 1) {
			if (x + dx < 0 || x + dx > map->w - 1 || y + dy < 0 || y + dy > map->h -1) continue;
			if (*(connected + xy2flat(x + dx, y + dy, map->w)) == 0 &&
			    *(map->floorspace + xy2flat(x + dx, y + dy, map->w)) != FLOOR_WALL) {
			    	changes += 1;
			    	*(connected + xy2flat(x + dx, y + dy, map->w)) = 1;
			}
		}
	}
	return changes;	
}

static void
place_rooms(struct mapspace *map, int min_w, int min_h, int max_w, int max_h)
{
	int i, w, h;
	
	for (i = 0; i < map->n_rooms; i += 1) {
		w = rand_num(min_w, max_w);
		h = rand_num(min_h, max_h);
		place_room(map, *(map->room_x + i), *(map->room_y + i), w, h);
	}	
}

static void
place_room(struct mapspace *map, int x, int y, int w, int h)
{
	int i, j;

	/* Place room so coordinate is center */
	x = x - w / 2;
	y = y - h / 2;
	/* Check if room will be off map */
	if (x < 1) x = 1;
	if (x > map->w - w - 1) x = map->w - w - 1;
	if (y < 1) y = 1;
	if (y > map->h - h - 1) y = map->h - h - 1;
	/* Place the map */
	for (i = 0; i < h; i += 1) {
		for (j = 0; j < w; j+= 1) {
			*(map->floorspace + xy2flat(x + j, y + i, map->w)) = FLOOR_OPEN;
		}
	}
}

static void
place_exits(struct mapspace *map, int r1_x, int r1_y)
{
	int x1, y1, x2, y2;
	int dist_apart, counter;

	counter = 0; dist_apart = 50;
	do {
		counter += 1;
		if (r1_x == -1) {
			x1 = rand_num(5, 94);
			y1 = rand_num(2, 22);
		} else {
			x1 = r1_x;
			y1 = r1_y;
		}
		x2 = rand_num(5, 94);
		y2 = rand_num(2, 22);
		if (counter % 100 == 0) {
			dist_apart -= 5;
			if (dist_apart < 5) dist_apart = 5;
		}
	} while (distp(x1, y1, x2, y2) < dist_apart || *(map->floorspace + xy2flat(x1, y1, map->w)) != FLOOR_OPEN || *(map->floorspace + xy2flat(x2, y2, map->w)) != FLOOR_OPEN);
	map->begin[0] = x1;
	map->begin[1] = y1;
	map->end[0] = x2;
	map->end[1] = y2;
	*(map->floorspace + xy2flat(x1, y1, map->w)) = FLOOR_BEGIN;
	*(map->floorspace + xy2flat(x2, y2, map->w)) = FLOOR_END;
}

static int
check_distp(struct mapspace *map)
{
	int i, j;
	int count;
	double mapdist;
	
	for (i = 0, count = 0, mapdist = 0.0; i < map->n_rooms - 1; i += 1) {
		for (j = i + 1; j < map->n_rooms; j += 1) {
			mapdist += distp(*(map->room_x + i), *(map->room_y + i), *(map->room_x + j), *(map->room_y + j));
			count += 1;
		}
	}
	mapdist /= count;
	return mapdist;
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

static int
check_paths(struct mapspace *map)
{
	char *connected;
	int changes, x, y;

	/* Paint-bucket fill of connected to make sure all rooms are connected */
	connected = malloc(sizeof(*connected) * map->w * map->h);
	for (x = 0; x < map->w * map->h; x += 1) *(connected + x) = 0;
	/* Set room 1 location equal to 1 */
	*(connected + xy2flat(*(map->room_x), *(map->room_y), map->w)) = 1;
	/* Enter a loop to fill all "1" connected tiles */
	while (1) {
		changes = 0;
		for (x = 0; x < map->w; x += 1) {
			for (y = 0; y < map->h; y += 1) {
				if (*(connected + xy2flat(x, y, map->w)) == 1) {
					changes += fill_point(map, connected, x, y);
				}
			}
		}
		if (changes == 0) break;
	}
	for (y = 0, x = 0; y < map->n_rooms; y += 1) {
		x += *(connected + xy2flat(*(map->room_x + y), *(map->room_y + y), map->w));
	}
	free(connected);
	return (x == map->n_rooms) ? 0 : 1;
}

static int
check_area(struct mapspace *map)
{
	int a, i;

	for (i = 0, a = 0; i < map->w * map->h; i += 1) {
		if (*(map->floorspace + i) != FLOOR_WALL) {
			a += 1;
		}
	}
	return a;
}
