#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "disp.h"
#include "libs.h"
#include "map.h"
#include "player.h"
#include "rand.h"
#include "save.h"

static void	look_cursor(struct mapspace *map, struct playerspace *player, int cx, int cy);

#define NUM_MAPS 1
struct mapspace **MAP_WALLET;

int
main(void)
{
	int c, i, prev_x, prev_y;
	long int seed;
	struct mapspace *map;
	struct playerspace *player;

	/* seed rng */
	seed = seed_rng();
	printf("%ld\n", seed);
	/* make maps */
	MAP_WALLET = malloc(sizeof(*MAP_WALLET) * NUM_MAPS);
	prev_x = -1; prev_y = -1;
	for (i = 0; i < NUM_MAPS; i += 1) {
		*(MAP_WALLET + i) = init_mapspace(100, 25, i, prev_x, prev_y);
		prev_x = (*(MAP_WALLET + i))->end[0];
		prev_y = (*(MAP_WALLET + i))->end[1];
	}
	map = *(MAP_WALLET + 0);
	/* place the character on the entrance */
	player = init_playerspace(map, map->begin[0], map->begin[1]);
	init_curses();
	/* input loop */
	while ((c = getch()) != 'p' && c != 'P') {
		erase();
		check_vis(map, player);
		print_mapspace(map, player);
		draw_character(player->x, player->y);
		draw_commands(0);
		draw_playerinfo(player);
		refresh();
		usleep(8333);
		switch (c) {
			case 'q':
			case 'Q':
				move_player(map, player, -1, -1);
				break;
			case 'w':
			case 'W':
				move_player(map, player, 0, -1);
				break;
			case 'e':
			case 'E':
				move_player(map, player, 1, -1);
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
			case 'z':
			case 'Z':
				move_player(map, player, -1, 1);
				break;
			case 'x':
			case 'X':
				move_player(map, player, 0, 0);
				break;
			case 'c':
			case 'C':
				move_player(map, player, 1, 1);
				break;
			case 'l':
			case 'L':
				look_cursor(map, player, player->x, player->y);
				break;
			case '<':
				move_floors(map, player, 1, NUM_MAPS);
				map = *(MAP_WALLET + player->cur_floor);
				break;
			case '>':
				move_floors(map, player, -1, NUM_MAPS);
				map = *(MAP_WALLET + player->cur_floor);
				break;
		}
	}
	quitsave(MAP_WALLET, player, NUM_MAPS);
	kill_curses();
	kill_mapspace(map);
	kill_playerspace(player);
	free(MAP_WALLET);
	return 0;
}

static void
look_cursor(struct mapspace *map, struct playerspace *player, int cx, int cy)
{
	char on_player;
	int c, x, y;
	
	x = cx;
	y = cy;
	while ((c = getch()) != 'l' && c != 'L') {
		print_mapspace(map, player);
		draw_character(cx, cy);
		draw_commands(1);
		draw_playerinfo(player);
		if (x == cx && y == cy) {
			on_player = 1;
		} else {
			on_player = 0;
		}
		draw_look(*(map->floorspace + xy2flat(x, y, 100)), *(map->explored + xy2flat(x, y, 100)), on_player);
		draw_cursor(x, y);
		refresh();
		usleep(8333);
		switch (c) {
			case 'q':
			case 'Q':
				if (x > 0) x -= 1;
				if (y > 0) y -= 1;
				break;
			case 'w':
			case 'W':
				if (y > 0) y -= 1;
				break;
			case 'e':
			case 'E':
				if (x < map->w - 1) x += 1;
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
			case 'z':
			case 'Z':
				if (x > 0) x -= 1;
				if (y < map->h - 1) y += 1;
				break;
			case 'x':
			case 'X':
				break;
			case 'c':
			case 'C':
				if (x < map->w - 1) x += 1;
				if (y < map->h - 1) y += 1;
				break;
		}
	}
	curs_set(0);
}
