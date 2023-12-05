#include <stdio.h>
#include <stdlib.h>
#include "libs.h"
#include "map.h"
#include "menu.h"
#include "player.h"

#define ADD_X 2
#define ADD_Y 1

static void	check_window(void);
static void	init_colors(void);
static void	format_time(int cur_time, char time_f[20]);

static char floor_sym[] = { ' ', '#', '.', '*', 'O', 'v', '^' };

void
init_curses(void)
{
	initscr();
	check_window();
	init_colors();
	raw();
	noecho();
	nodelay(stdscr, 1);
	keypad(stdscr, 1);
	curs_set(0);
}

static void
check_window(void)
{
	int x, y;
	return;

	x = getmaxx(stdscr);
	y = getmaxy(stdscr);
	if (x < 130 || y < 32) {
		endwin();
		printf("Please ensure the terminal screen is at least 130x32\n");
		exit(1);
	}
	if (has_colors() == FALSE) {
		endwin();
		printf("Your terminal does not support colors\n");
		exit(1);
	}
}

static void
init_colors(void)
{
	start_color();
	init_pair(1, COLOR_WHITE, COLOR_BLACK);
	init_pair(2, COLOR_BLUE, COLOR_BLACK);
}

void
kill_curses(void)
{
	endwin();
}

void
draw_character(int x, int y)
{
	mvaddch(y + ADD_Y, x + ADD_X, '@');
}

void
print_mapspace(struct mapspace *map, struct playerspace *player)
{
	char c;
	int i, j;

	for (j = 0; j < map->h; j += 1) {
		for (i = 0; i < map->w; i += 1) {
			if (*(map->explored + xy2flat(i, j, map->w)) == 1) {
				c = floor_sym[*(map->floorspace + xy2flat(i, j, map->w))];
				if (*(player->vis + xy2flat(i, j, map->w)) != 0) {
					attron(COLOR_PAIR(1));
					mvaddch(j + ADD_Y, i + ADD_X, c);
				} else {
					attron(COLOR_PAIR(2));
					mvaddch(j + ADD_Y, i + ADD_X, c);
					attron(COLOR_PAIR(1));
				}
			}
		}
	}
}

void
draw_cursor(int x, int y)
{
	static int counter = 0;

	/* Blink the cursor every 30 frames */
	counter += 1;
	if (counter == 1) {
		curs_set(2);
	} else if (counter == 15) {
		curs_set(0);
	} else if (counter == 29) {
		counter = 0;
	}
	/* Move the cursor */
	move(y + ADD_Y, x + ADD_X);
}

void
draw_commands(int look)
{
	if (look == 0) {
		mvprintw(27, 1, "[L] Look mode    [P] pSave/Quit");
	} else {
		mvprintw(27, 1, "[L] Exit Look mode                  ");
	}
}

void
draw_look(int floorspace, char vis, char on_player)
{
	if (on_player == 1) {
		mvprintw(28, 1, "You see: yourself                              ");
	} else if (vis == 0) {
		mvprintw(28, 1, "You see: unexplored                             ");
	} else {
		mvprintw(28, 1, "You see: %s                             ", floor_names[floorspace]);
	}
}

void
draw_playerinfo(struct playerspace *player)
{
	char time_f[20];
	int barloc[5] = { 24, 42, 60, 72, 85 }, i;

	for (i = 0; i < 5; i += 1) mvaddch(0, barloc[i], '|');
	mvprintw(0, 0, "Name: %s", player->name);
	mvprintw(0, 26, "HP: %4d / %4d", player->stats.hp, player->stats.maxhp);
	mvprintw(0, 44, "SP: %4d / %4d", player->stats.sp, player->stats.maxsp);
	mvprintw(0, 62, "Level: %2d", player->stats.level);
	mvprintw(0, 74, "Floor: %3d", player->cur_floor + 1);
	format_time(player->cur_time, time_f);
	mvprintw(0, 87, "Time: %s", time_f);
}

static void
format_time(int cur_time, char time_f[20])
{
	int d, h, m;

	d = cur_time / 1440;
	h = cur_time / 60 - d * 24;
	m = cur_time - d * 24 * 60 - h * 60;
	sprintf(time_f, "%d-%02d:%02d  ", d, h, m);
}

void
draw_menu(int state)
{
	int line;

	mvprintw(1, 2, "Game Title Goes Here");
	mvprintw(3, 2, "Main Menu");
	mvprintw(4, 5, "[N] New game");
	if ((state & STATE_SAVE) == STATE_SAVE) {
		line = 6;
		mvprintw(5, 5, "[L] Load game");
	} else {
		line = 5;
	}
	mvprintw(line, 5, "[O] Options");
	if ((state & STATE_PROGRESS) == STATE_PROGRESS) {
		mvprintw(line + 1, 5, "[Q] Save and quit");		
	} else {
		mvprintw(line + 1, 5, "[Q] Quit");
	}
}

void
draw_progress(char *message, int x, int y, int progress, int *oarg1, int *oarg2)
{
	int i;

	if (oarg1 != NULL && oarg2 != NULL) {
		mvprintw(y, x, message, *oarg1, *oarg2);
	} else if (oarg1 != NULL) {
		mvprintw(y, x, message, *oarg1);
	} else {
		mvprintw(y, x, message);
	}
	move(y + 1, x);
	addch('|');
	for (i = 0; i < 100; i += 1) addch('-');
	addch('|');
	move(y + 1, x + 1);
	for (i = 0; i < progress; i += 1) addch('#');
	refresh();
}
