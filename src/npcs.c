#include <stdlib.h>
#include "libs.h"
#include "map.h"
#include "npcs.h"
#include "rand.h"

struct npcspace *NPCS = NULL;
int N_NPCS;

static void	new_npc(struct mapspace **mapwallet, int i, int n_floors);

void
init_npcspace(struct mapspace **mapwallet, int n_floors, int n_npcs)
{
	int i;

	/* Adjust the number randomly */
	n_npcs *= n_floors;
	n_npcs += rand_num(0 - (0.05 * n_npcs), 0 + (0.05 * n_npcs));
	/* Allocate memory and set up NPCs */
	N_NPCS = n_npcs;
	NPCS = malloc(sizeof(*NPCS) * N_NPCS);
	for (i = 0; i < N_NPCS; i += 1) {
		new_npc(mapwallet, i, n_floors);
	}
}

void
kill_npcspace(void)
{
	if (NPCS != NULL) {
		free(NPCS);
	}
}

static void
new_npc(struct mapspace **mapwallet, int i, int n_floors)
{
	int cur_floor, x, y;
	struct mapspace *map;

	/* Figure out a random floor and starting point */
	cur_floor = rand_num(0, n_floors - 1);
	map = *(mapwallet + cur_floor);
	do {
		x = rand_num(5, 94);
		y = rand_num(2, 22);
	} while((x == map->begin[0] && y == map->begin[1]) ||
		*(map->floorspace + xy2flat(x, y, map->w)) == FLOOR_WALL);
	/* Set starting values for NPC */
	(NPCS + i)->x = x;
	(NPCS + i)->y = y;
	(NPCS + i)->cur_floor = cur_floor;
	rando_name((NPCS + i)->name);
}

void
npcs_info(int cur_floor, struct npc_info *npcs)
{
	int i, count;

	/* Count number of NPCs on current floor */
	for (i = 0, count = 0; i < N_NPCS; i += 1) {
		if ((NPCS + i)->cur_floor == cur_floor) {
			count += 1;
		}
	}
	npcs->n_npcs = count;
	/* Free old information if necessary */
	if (npcs->x != NULL) {
		free(npcs->i);
		free(npcs->x);
		free(npcs->y);
	}
	/* Allocate memory and copy info over */
	npcs->i = malloc(sizeof(*npcs->i) * count);
	npcs->x = malloc(sizeof(*npcs->x) * count);
	npcs->y = malloc(sizeof(*npcs->y) * count);
	for (i = 0, count = 0; i < N_NPCS; i += 1) {
		if ((NPCS + i)->cur_floor == cur_floor) {
			count += 1;
			*(npcs->i + count - 1) = i;
			*(npcs->x + count - 1) = (NPCS + i)->x;
			*(npcs->y + count - 1) = (NPCS + i)->y;
		}
	}
	
}
