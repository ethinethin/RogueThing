#ifndef DISP_H
#define DISP_H

#include "map.h"
#include "player.h"

extern void	init_curses(void);
extern void	kill_curses(void);
extern void	draw_character(int x, int y);
extern void	print_mapspace(struct mapspace *map, struct playerspace *player);
extern void	draw_cursor(int x, int y);
extern void	draw_commands(int look);
extern void	draw_look(int floorspace, char vis, char on_player);
extern void	draw_playerinfo(struct playerspace *player);
extern void	draw_menu(int state);

#endif
