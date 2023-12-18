#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "disp.h"
#include "libs.h"
#include "map.h"
#include "menu.h"
#include "npcs.h"
#include "player.h"
#include "rand.h"
#include "save.h"
#include "types.h"

static void	look_cursor(int cx, int cy);
static void	game_startup(int menu);
static void	game_new(int in_progress);
static void	game_quit(void);
static void	game_teardown(void);

struct mapspace **MAP_WALLET = NULL;
struct mapspace *MAP = NULL;
int N_MAPS = 100;
struct playerspace *PLAYER = NULL;
struct npc_info CUR_NPCS;

int
main(void)
{
	int c, menu, update;
	long int seed;

	/* seed rng */
	seed = seed_rng();
	printf("%ld\n", seed);
	/* Initialize curses, the message log, & the menu, then go to the main menu */
	init_curses();
	init_log();
	CUR_NPCS.n_npcs = 0;
	CUR_NPCS.x = NULL;
	CUR_NPCS.y = NULL;
	init_menu();
	menu = main_menu();
	/* Act on menu options returned */
	game_startup(menu);
	/* input loop */
	update = 1;
	while (1) {
		MAP = *(MAP_WALLET + PLAYER->cur_floor);
		if (update == 1) {
			/* Only redraw screen if user input was received */
			check_vis(MAP, PLAYER);
			print_mapspace(MAP, PLAYER);
			npcs_info(MAP->cur_floor, &CUR_NPCS);
			draw_npcs(MAP, PLAYER, CUR_NPCS);
			draw_character(PLAYER->x, PLAYER->y);
			draw_commands(0);
			draw_playerinfo(PLAYER);
			draw_log(0);
		}
		/* Get input and sleep for 1/60th of a second */
		c = getch();
		usleep(16667);
		update = 1;
		switch (c) {
			case 'q':
			case 'Q':
				move_player(MAP, PLAYER, CUR_NPCS, -1, -1);
				break;
			case 'w':
			case 'W':
				move_player(MAP, PLAYER, CUR_NPCS, 0, -1);
				break;
			case 'e':
			case 'E':
				move_player(MAP, PLAYER, CUR_NPCS, 1, -1);
				break;
			case 'a':
			case 'A':
				move_player(MAP, PLAYER, CUR_NPCS, -1, 0);
				break;
			case 's':
			case 'S':
				move_player(MAP, PLAYER, CUR_NPCS, 0, 1);
				break;
			case 'd':
			case 'D':
				move_player(MAP, PLAYER, CUR_NPCS, 1, 0);
				break;
			case 'z':
			case 'Z':
				move_player(MAP, PLAYER, CUR_NPCS, -1, 1);
				break;
			case 'x':
			case 'X':
				move_player(MAP, PLAYER, CUR_NPCS, 0, 0);
				break;
			case 'c':
			case 'C':
				move_player(MAP, PLAYER, CUR_NPCS, 1, 1);
				break;
			case 'l':
			case 'L':
				look_cursor(PLAYER->x, PLAYER->y);
				break;
			case '<':
				move_floors(MAP, PLAYER, 1, N_MAPS);
				break;
			case '>':
				move_floors(MAP, PLAYER, -1, N_MAPS);
				break;
			case 'p':
			case 'P':
				game_quit();
				break;
			case 'm':
			case 'M':
				menu = main_menu();
				if (menu == MENU_QUIT) {
					game_quit();
				} else if (menu == MENU_NEW) {
					game_new(1);
				}
				break;
			default:
				/* No input of consequence was received, don't update */
				update = 0;
				break;
		}
	}
	return 0;
}

static void
look_cursor(int cx, int cy)
{
	char on_player;
	int c, x, y, xyflat;

	/* Draw initial screen */
	draw_commands(1);
	draw_playerinfo(PLAYER);
	print_mapspace(MAP, PLAYER);
	draw_log(0);
	npcs_info(MAP->cur_floor, &CUR_NPCS);
	draw_npcs(MAP, PLAYER, CUR_NPCS);
	draw_character(cx, cy);
	/* Set cursor on character */
	x = cx;
	y = cy;
	while ((c = getch()) != 'l' && c != 'L') {
		if (x == cx && y == cy) {
			on_player = 1;
		} else {
			on_player = 0;
		}
		xyflat = xy2flat(x, y, MAP->w);
		draw_look(*(MAP->floorspace + xyflat), *(MAP->explored + xyflat), *(PLAYER->vis + xyflat), on_player, CUR_NPCS, x, y);
		draw_cursor(x, y);
		usleep(16667);
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
				if (x < MAP->w - 1) x += 1;
				if (y > 0) y -= 1;
				break;
			case 'a':
			case 'A':
				if (x > 0) x -= 1;
				break;
			case 's':
			case 'S':
				if (y < MAP->h - 1) y += 1;
				break;
			case 'd':
			case 'D':
				if (x < MAP->w - 1) x += 1;
				break;
			case 'z':
			case 'Z':
				if (x > 0) x -= 1;
				if (y < MAP->h - 1) y += 1;
				break;
			case 'x':
			case 'X':
				break;
			case 'c':
			case 'C':
				if (x < MAP->w - 1) x += 1;
				if (y < MAP->h - 1) y += 1;
				break;
		}
	}
	curs_set(0);
	erase();
}

static void
game_startup(int menu)
{
	if (menu == MENU_NEW) {
		set_gamestate(STATE_PROGRESS);
		game_new(0);
	} else if (menu == MENU_LOAD) {
		set_gamestate(STATE_PROGRESS);
		N_MAPS = load_save(&MAP_WALLET, &PLAYER);
	} else if (menu == MENU_QUIT) {
		game_quit();
	}
}

static void
game_new(int in_progress)
{
	char message[1024];
	int i, prev_x, prev_y;

	/* If game is in progress, tear down existing game without saving
	 * and re-initialize the message log */
	if (in_progress == 1) {
		game_teardown();
		init_log();
	}
	/* make maps */
	MAP_WALLET = malloc(sizeof(*MAP_WALLET) * N_MAPS);
	prev_x = -1; prev_y = -1;
	for (i = 0; i < N_MAPS; i += 1) {
		sprintf(message, "Generating maps (map %d of %d)", i + 1, N_MAPS);
		draw_progress(message, 2, 10, (i * 100) / (N_MAPS - 1));
		*(MAP_WALLET + i) = init_mapspace(100, 25, i, prev_x, prev_y);
		prev_x = (*(MAP_WALLET + i))->end[0];
		prev_y = (*(MAP_WALLET + i))->end[1];
	}
	MAP = *(MAP_WALLET + 0);
	/* place the character on the entrance */
	PLAYER = init_playerspace(MAP, MAP->begin[0], MAP->begin[1]);
	/* Initialize the NPCs */
	init_npcspace(MAP_WALLET, N_MAPS, 10);
	erase();
}

static void
game_quit(void)
{
	int i;
	struct mapspace *map;

	if (PLAYER != NULL) {
		quit_save(MAP_WALLET, PLAYER, N_MAPS);
		kill_playerspace(PLAYER);
	}
	if (MAP_WALLET != NULL) {
		for (i = 0; i < N_MAPS; i += 1) {
			map = *(MAP_WALLET + i);
			kill_mapspace(map);
		}
		free(MAP_WALLET);
	}
	kill_npcspace();
	kill_curses();
	exit(0);
}

static void
game_teardown(void)
{
	int i;
	struct mapspace *map;

	if (PLAYER != NULL) kill_playerspace(PLAYER);
	PLAYER = NULL;
	if (MAP_WALLET != NULL) {
		for (i = 0; i < N_MAPS; i += 1) {
			map = *(MAP_WALLET + i);
			kill_mapspace(map);
		}
		free(MAP_WALLET);
	}
	MAP_WALLET = NULL;
	kill_npcspace();
}
