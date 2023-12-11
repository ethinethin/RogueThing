#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "libs.h"
#include "map.h"
#include "menu.h"
#include "npcs.h"
#include "player.h"

#define ADD_X 1
#define ADD_Y 1

static void	check_window(void);
static void	init_colors(void);
static void	format_time(int cur_time, char time_f[20]);
static void	scroll_log(int n_lines);
static void	add_mesg(char *mesg, int n_lines);

static char floor_sym[] = { ' ', '#', '.', '*', 'O', 'v', '^' };

void
init_curses(void)
{
	/* Initialize curses and check window size/color availability */
	initscr();
	check_window();
	init_colors();
	/* Curses settings: raw mode, no input echo, no delay on input,
	 * extended keypad input, and visible cursor off */
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
	init_pair(3, COLOR_CYAN, COLOR_BLACK);
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
			} else {
				mvaddch(j + ADD_Y, i + ADD_X, ' ');
			}
		}
	}
	refresh();
}

void
draw_cursor(int x, int y)
{
	static int counter = 0;

	/* Blink the cursor every 14 ticks */
	counter += 1;
	if (counter == 1) {
		curs_set(2);
	} else if (counter == 7) {
		curs_set(0);
	} else if (counter == 13) {
		counter = 0;
	}
	/* Move the cursor */
	move(y + ADD_Y, x + ADD_X);
	refresh();
}

void
draw_commands(int mode)
{
	int i;

	/* Clear the command line */
	move(27, 0);
	for (i = 0; i < 101; i += 1) addch(' ');
	/* Output commands at bottom, dependent upon current mode */
	if (mode == 0) {
		mvprintw(27, 1, "[M] Main menu      [L] Look mode");
	} else {
		mvprintw(27, 1, "[L] Exit Look mode");
	}
}

void
draw_look(int floorspace, char vis, char on_player)
{
	int i;

	/* Clear the look line */
	move(28, 0);
	for (i = 0; i < 101; i += 1) addch(' ');
	/* Report what is under the cursor */
	if (on_player == 1) {
		mvprintw(28, 1, "You see: yourself");
	} else if (vis == 0) {
		mvprintw(28, 1, "You see: unexplored");
	} else {
		mvprintw(28, 1, "You see: %s", floor_names[floorspace]);
	}
}

void
draw_playerinfo(struct playerspace *player)
{
	char time_f[20];
	int barloc[5] = { 24, 42, 60, 72, 85 }, i;

	/* Output important player info at the top line with some formatting */
	for (i = 0; i < 5; i += 1) mvaddch(0, barloc[i], '|');
	mvprintw(0, 0, "Name: %-17s", player->name);
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

	/* Format time from seconds to D-HH:MM into the given string */
	d = cur_time / 1440;
	h = cur_time / 60 - d * 24;
	m = cur_time - d * 1440 - h * 60;
	sprintf(time_f, "%d-%02d:%02d  ", d, h, m);
}

void
draw_npcs(struct npc_info npcs, struct mapspace *map, struct playerspace *player)
{
	int i, x, y;

	for (i = 0; i < npcs.n_npcs; i += 1) {
		x = *(npcs.x + i);
		y = *(npcs.y + i);
		if (*(player->vis + xy2flat(x, y, map->w)) != 0) {
			mvaddch(*(npcs.y + i) + ADD_Y, *(npcs.x + i) + ADD_X, '$');
		}
	}
}

void
draw_menu(int state)
{
	int line;

	mvprintw(1, 2, "Game Title Goes Here");
	mvprintw(3, 2, "Main Menu");
	mvprintw(4, 5, "[N] New game");
	/* Allow loading a game if a savegame exists, allow back to game if in progress */
	if ((state & STATE_SAVE) == STATE_SAVE) {
		line = 6;
		mvprintw(5, 5, "[L] Load game");
	} else if ((state & STATE_PROGRESS) == STATE_PROGRESS) {
		line = 6;
		mvprintw(5, 5, "[B] Back to game");
	} else {
		/* Don't leave an empty line if there's no savegame */
		line = 5;
	}
	mvprintw(line, 5, "[O] Options");
	/* Allow saving and quitting if there is an active game */
	if ((state & STATE_PROGRESS) == STATE_PROGRESS) {
		mvprintw(line + 1, 5, "[Q] Save and quit");		
	} else {
		mvprintw(line + 1, 5, "[Q] Quit");
	}
}

void
draw_progress(char *mesg, int x, int y, int progress)
{
	int i;

	/* Print the message at the given coordinates */
	mvprintw(y, x, mesg);
	/* Draw an empty progress bar on the next line */
	move(y + 1, x);
	addch('|');
	for (i = 0; i < 100; i += 1) addch('-');
	addch('|');
	/* Fill the progress bar to the given amount */
	move(y + 1, x + 1);
	for (i = 0; i < progress; i += 1) addch('#');
	/* Refresh output */
	refresh();
}

/* Internal values for the message log: max length per message line,
 * max length of message log, and amount of message log visible on
 * the screen */
#define MESG_LEN 28
#define LOG_LEN 10000
#define DISP_LEN 30
/* The message log itself */
char MESG[LOG_LEN][MESG_LEN + 1];
int N_MESG;

void
init_log(void)
{
	int i;
	/* Initialize the message log by filling it with empty entries */
	for (i = 0; i < LOG_LEN; i += 1) MESG[i][0] = '\0';
	N_MESG = 0;
}

void
draw_log(int adj)
{
	int i, j;

	/* Clear the log */
	for (i = 0; i < DISP_LEN; i += 1) {
		move(i + 2, 103);
		clrtoeol();
	}
	refresh();
	/* Draw the message log header */
	for (i = 0; i < 32; i += 1) mvaddch(i, 101, '|');
	mvprintw(0, 103, "Messages --------------------");
	/* Draw the newest DISP_LEN entries in the message log */
	for (i = 0; i < DISP_LEN; i += 1) {
		for (j = 0; j < MESG_LEN; j += 1) {
			/* If at the end of the message, skip to the next one */
			if (MESG[LOG_LEN - DISP_LEN + i][j] == '\0') break;
			mvaddch(i + 2, 103 + j, MESG[LOG_LEN - DISP_LEN + i][j]);
		}
	}
	// Remove later.. silencing warning
	if (adj == -1) return;
}

void
add_log(char *mesg)
{
	int m_len, n_lines;

	/* Calculate number of lines the message will require */
	m_len = strnlen(mesg, 1000);
	n_lines = m_len / MESG_LEN;
	if (m_len % MESG_LEN != 0) n_lines += 1;
	/* Scroll the buffer the necessary number of lines */
	scroll_log(n_lines);
	/* Add the message to the buffer */
	add_mesg(mesg, n_lines);
	/* Increase the number of messages counter */
	if (N_MESG < LOG_LEN) N_MESG += 1;
}

static void
scroll_log(int n_lines)
{
	int i, j;

	/* Start at entry n_lines, and move each entry n_lines positions
	 * back in the message log */
	for (i = n_lines; i < LOG_LEN; i += 1) {
		for (j = 0; j < MESG_LEN; j += 1) {
			if (MESG[i][j] == '\0') break;
			MESG[i - n_lines][j] = MESG[i][j];
		}
		MESG[i - n_lines][j] = '\0';
	}
}

static void
add_mesg(char *mesg, int n_lines)
{
	int i, j, pos;

	/* Add a message to the message log by splitting it into
	 * n_lines entries */
	for (i = 0, pos = 0; i < n_lines; i += 1) {
		for (j = 0; j < MESG_LEN; j+= 1, pos += 1) {
			/* If end of message, don't copy any more */
			if (*(mesg + pos) == '\0') break;
			MESG[LOG_LEN - n_lines + i][j] = *(mesg + pos);
		}
		/* Add a string terminator after the last copied char */
		MESG[LOG_LEN - n_lines + i][j] = '\0';
	}
}
