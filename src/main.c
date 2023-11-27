#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "disp.h"
#include "libs.h"
#include "map.h"
#include "player.h"
#include "rand.h"

static void	check_vis(struct mapspace *map, int cx, int cy);
static void	look_cursor(struct mapspace *map, struct playerspace *player, int cx, int cy);

int
main(void)
{
	int c;
	int cx, cy;
	long int seed;
	struct mapspace *map;
	struct playerspace *player;

	/* seed rng */
	seed = seed_rng();
	printf("%ld\n", seed);	
	/* make map */
	map = init_mapspace(100, 25);
	/* add room locations and paths */
	add_rooms(map, 9);
	make_paths(map);
	/* make rooms and place */
	make_rooms(map, 8, 5, 15, 8);
	/* find character starting space */
	char_start(map, &cx, &cy);
	player = init_playerspace(cx, cy);
	init_curses();
	/* input loop */
	while ((c = getch()) != 'q' || c == 'Q') {
		check_vis(map, player->x, player->y);
		print_mapspace(map);
		draw_character(player->x, player->y);
		draw_commands(0);
		draw_playerinfo(player);
		refresh();
		usleep(8333);
		switch (c) {
			case 'w':
			case 'W':
				move_player(map, player, 0, -1);
				break;
			case 'a':
			case 'A':
				move_player(map, player, -1, 0);
				break;
			case 's':
			case 'S':
				move_player(map, player, 0, 1);
				break;
			case 'd':
			case 'D':
				move_player(map, player, 1, 0);
				break;
			case 'l':
			case 'L':
				look_cursor(map, player, player->x, player->y);
				break;
		}
	}
	kill_curses();
	kill_mapspace(map);
	kill_playerspace(player);
	return 0;
}

static void
check_vis(struct mapspace *map, int cx, int cy)
{
	int i, j;

	for (j = cy - 2; j <= cy + 2; j += 1) {
		for (i = cx - 4; i <= cx + 4; i += 1) {
			if (i < 0 || j < 0 || i >= map->w - 1 || j >= map->h - 1) continue;
			*(map->vis + xy2flat(i, j, map->w)) = 1;
		}
	}
}

static void
look_cursor(struct mapspace *map, struct playerspace *player, int cx, int cy)
{
	char on_player;
	int c, x, y;
	
	x = cx;
	y = cy;
	while ((c = getch()) != 'l' || c == 'L') {
		print_mapspace(map);
		draw_character(cx, cy);
		draw_commands(1);
		draw_playerinfo(player);
		if (x == cx && y == cy) {
			on_player = 1;
		} else {
			on_player = 0;
		}
		draw_look(*(map->floorspace + xy2flat(x, y, 100)), *(map->vis + xy2flat(x, y, 100)), on_player);
		draw_cursor(x, y);
		refresh();
		usleep(8333);
		switch (c) {
			case 'w':
			case 'W':
				if (y > 0) y -= 1;
				break;
			case 'a':
			case 'A':
				if (x > 0) x -= 1;
				break;
			case 's':
			case 'S':
				if (y < map->h - 1) y += 1;
				break;
			case 'd':
			case 'D':
				if (x < map->w - 1) x += 1;
				break;
		}
	}
	curs_set(0);
}
