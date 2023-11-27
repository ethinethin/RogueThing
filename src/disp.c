#include <stdio.h>
#include <stdlib.h>
#include "libs.h"
#include "map.h"
#include "player.h"

#define ADD_X 2
#define ADD_Y 1

static void	check_window(void);
static void	format_time(int cur_time, char time_f[20]);

static char floor_sym[] = { '.', '#', '.', '*', 'O' };

void
init_curses(void)
{
	initscr();
	check_window();
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

	x = getmaxx(stdscr);
	y = getmaxy(stdscr);
	if (x < 130 || y < 32) {
		endwin();
		printf("Please ensure the terminal screen is at least 130x32\n");
		exit(1);
	}
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
print_mapspace(struct mapspace *map)
{
	int i, j;

	for (j = 0; j < map->h; j += 1) {
		for (i = 0; i < map->w; i += 1) {
			if (*(map->vis + xy2flat(i, j, map->w)) == 1) mvaddch(j + ADD_Y, i + ADD_X, floor_sym[*(map->floorspace + xy2flat(i, j, map->w))]);
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
		mvprintw(27, 1, "[L] Look mode    [Q] Quit game");
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
	mvprintw(0, 1, "Name: %s", player->name);
	format_time(player->cur_time, time_f);
	mvprintw(0, 26, "Time: %s", time_f);
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
