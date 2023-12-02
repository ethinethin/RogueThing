#ifndef SAVE_H
#define SAVE_H

#include "map.h"
#include "player.h"

extern void	quitsave(struct mapspace **mapwallet, struct playerspace *player, int n_maps);
extern int	loadsave(struct mapspace **mapwallet, struct playerspace *player);

#endif
