#ifndef COMBAT_H
#define COMBAT_H

#include "libs.h"
#include "types.h"

extern int	player_attack(struct playerspace *player, int n);
extern void	npc_attack(struct playerspace *player, int n);

#endif
