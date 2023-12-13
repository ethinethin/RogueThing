#ifndef SAVE_H
#define SAVE_H

#include "types.h"

extern int	save_exists(void);
extern void	quit_save(struct mapspace **mapwallet, struct playerspace *player, int n_maps);
extern int	load_save(struct mapspace ***mapwallet, struct playerspace **player);

#endif
