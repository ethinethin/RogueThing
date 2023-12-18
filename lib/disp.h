#ifndef DISP_H
#define DISP_H

#include "types.h"

extern void	init_curses(void);
extern void	kill_curses(void);
extern void	draw_character(int x, int y);
extern void	print_mapspace(struct mapspace *map, struct playerspace *player);
extern void	draw_cursor(int x, int y);
extern void	draw_commands(int mode);
extern void	draw_look(int floorspace, char explored, char vis, char on_player, struct npc_info npcs, int x, int y);
extern void	draw_playerinfo(struct playerspace *player);
extern void	draw_npcs(struct mapspace *map, struct playerspace *player, struct npc_info npcs);
extern void	draw_menu(int state);
extern void	draw_progress(char *message, int x, int y, int progress);
extern void	init_log(void);
extern void	draw_log(int adj);
extern void	add_log(char *mesg);

#endif
