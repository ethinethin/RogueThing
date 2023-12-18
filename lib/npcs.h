#ifndef NPCS_H
#define NPCS_H

#include "types.h"

extern void	init_npcspace(struct mapspace **mapwallet, int n_floors, int n_npcs);
extern void	kill_npcspace(void);
extern void	upgrade_all_npcs(void);
extern void	npcs_info(int cur_floor, struct npc_info *npcs);
extern char *	get_name(int n);
extern int	get_n_npcs(void);
extern void	format_npc_for_saving(int n, char *s);
extern void	add_loaded_npc(int n_npcs, int n, char name[20], int stats[16]);
extern int	is_hostile(int n);
extern int	is_alive(int n);
extern void	swap_spaces(struct playerspace *player, int n);
extern int	kill_enemy(int n);
extern void	move_all_npcs(struct mapspace *map, struct playerspace *player, struct npc_info npcs);

#endif
