#include <stdlib.h>
#include <string.h>
#include "libs.h"
#include "map.h"
#include "npcs.h"
#include "player.h"
#include "rand.h"
#include "types.h"

struct npcspace *NPCS = NULL;
int N_NPCS;

enum behavior {
	BEH_HOST_ROOM_RANGED, BEH_HOST_ROOM_CLOSE, BEH_HOST_ROOM_FEINT, BEH_HOST_ROOM_RUN,
	BEH_HOST_TUNN_RANGED, BEH_HOST_TUNN_CLOSE, BEH_HOST_TUNN_FEINT, BEH_HOST_TUNN_RUN,
	BEH_HOST_BOTH_RANGED, BEH_HOST_BOTH_CLOSE, BEH_HOST_BOTH_FEINT, BEH_HOST_BOTH_RUN,
	BEH_NONH_ROOM_DRUNK, BEH_NONH_TUNN_DRUNK, BEH_NONH_BOTH_DRUNK
};

#define N_BEHAVIORS 15

struct advancement {
	int behavior;
	int tile1;
	int tile2;
	int prob[6];
} ADVANCEMENT_TABLE[] = {
	{ BEH_HOST_ROOM_RANGED, FLOOR_OPEN, -1, { 14, 22, 6, 14, 26, 18 } },
	{ BEH_HOST_ROOM_CLOSE, FLOOR_OPEN, -1, { 22, 14, 22, 22, 6, 14 } },
	{ BEH_HOST_ROOM_FEINT, FLOOR_OPEN, -1, { 14, 22, 6, 14, 22, 22 } },
	{ BEH_HOST_ROOM_RUN, FLOOR_OPEN, -1, { 6, 6, 6, 38, 6, 38 } },
	{ BEH_HOST_TUNN_RANGED, FLOOR_PATH, -1, { 14, 22, 6, 14, 26, 18 } },
	{ BEH_HOST_TUNN_CLOSE, FLOOR_PATH, -1, { 22, 14, 22, 22, 6, 14 } },
	{ BEH_HOST_TUNN_FEINT, FLOOR_PATH, -1, { 14, 22, 6, 14, 22, 22 } },
	{ BEH_HOST_TUNN_RUN, FLOOR_PATH, -1, { 6, 6, 6, 38, 6, 38 } },
	{ BEH_HOST_BOTH_RANGED, FLOOR_OPEN, FLOOR_PATH, { 14, 22, 6, 14, 26, 18 } },
	{ BEH_HOST_BOTH_CLOSE, FLOOR_OPEN, FLOOR_PATH, { 22, 14, 22, 22, 6, 14 } },
	{ BEH_HOST_BOTH_FEINT, FLOOR_OPEN, FLOOR_PATH, { 14, 22, 6, 14, 22, 22 } },
	{ BEH_HOST_BOTH_RUN, FLOOR_OPEN, FLOOR_PATH, { 6, 6, 6, 38, 6, 38 } },
	{ BEH_NONH_ROOM_DRUNK, FLOOR_OPEN, -1, { 17, 16, 16, 16, 16, 16 } },
	{ BEH_NONH_TUNN_DRUNK, FLOOR_PATH, -1, { 17, 16, 16, 16, 16, 16 } },
	{ BEH_NONH_BOTH_DRUNK, FLOOR_OPEN, FLOOR_PATH, { 17, 16, 16, 16, 16, 16 } }
};

static void	new_npc(struct mapspace **mapwallet, int i, int n_floors);
static void	new_npc_stats(int i);
static void	upgrade_npc(int i);
static int	determine_stat(int personality);

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
		NPCS = NULL;
	}
}

static void
new_npc(struct mapspace **mapwallet, int i, int n_floors)
{
	int floorspace, j, tile1, tile2, x, y;
	struct mapspace *map;

	/* Figure out a random floor and personality */
	(NPCS + i)->cur_floor = rand_num(0, n_floors - 1);
	(NPCS + i)->personality = ADVANCEMENT_TABLE[rand_num(0, N_BEHAVIORS - 1)].behavior;
	/* Determine start tile they need to be on */
	for (j = 0, tile1 = FLOOR_ROOM, tile2 = FLOOR_ROOM; j < N_BEHAVIORS; j += 1) {
		if (ADVANCEMENT_TABLE[j].behavior == (NPCS + i)->personality) {
			tile1 = ADVANCEMENT_TABLE[j].tile1;
			tile2 = ADVANCEMENT_TABLE[j].tile2;
			break;
		}
	}
	map = *(mapwallet + (NPCS + i)->cur_floor);
	do {
		x = rand_num(5, 94);
		y = rand_num(2, 22);
		floorspace = *(map->floorspace + xy2flat(x, y, map->w));
	} while(floorspace != tile1 && floorspace != tile2);
	/* Set starting location for NPC */
	(NPCS + i)->x = x;
	(NPCS + i)->y = y;
	/* Set starting values for NPC */
	new_npc_stats(i);
	upgrade_npc(i);
}

static void
new_npc_stats(int i)
{
	rando_name((NPCS + i)->name);
	(NPCS + i)->stats.level = (NPCS + i)->cur_floor + 1;
	(NPCS + i)->stats.hp = 10;
	(NPCS + i)->stats.maxhp = 10;
	(NPCS + i)->stats.sp = 10;
	(NPCS + i)->stats.maxsp = 10;
	(NPCS + i)->stats.pa = 10;
	(NPCS + i)->stats.pd = 10;
	(NPCS + i)->stats.ra = 10;
	(NPCS + i)->stats.rd = 10;
	(NPCS + i)->stats.bp = (NPCS + i)->stats.level * 10;
	(NPCS + i)->stats.ep = 0;
}

static void
upgrade_npc(int i)
{
	int ds;

	while ((NPCS + i)->stats.bp > 0) {
		ds = determine_stat((NPCS + i)->personality);
		switch(ds) {
			case 0:
				(NPCS + i)->stats.maxhp += 1;
				break;
			case 1:
				(NPCS + i)->stats.maxsp += 1;
				break;
			case 2:
				(NPCS + i)->stats.pa += 1;
				break;
			case 3:
				(NPCS + i)->stats.pd += 1;
				break;
			case 4:
				(NPCS + i)->stats.ra += 1;
				break;
			case 5:
				(NPCS + i)->stats.rd += 1;
				break;
		}
		(NPCS + i)->stats.bp -= 1;
	}
	(NPCS + i)->stats.hp = (NPCS + i)->stats.maxhp;
	(NPCS + i)->stats.sp = (NPCS + i)->stats.maxsp;
}

static int
determine_stat(int personality)
{
	int i, j, prob, r;

	/* Determine row of advancement table */
	for (i = 0; i < N_BEHAVIORS; i += 1) {
		if (ADVANCEMENT_TABLE[i].behavior == personality) break;
	}
	if (i == N_BEHAVIORS) return 0;
	/* Do a probability roll! */
	r = rand_num(0, 99);
	/* Step through the probability values */
	for (j = 0, prob = 0; j < 6; j += 1) {
		prob += ADVANCEMENT_TABLE[i].prob[j];
		if (r < prob) return j;
	}
	return 0;
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

char *
get_name(int n)
{
	return (NPCS + n)->name;
}

int
get_n_npcs(void)
{
	return N_NPCS;
}

void
format_npc_for_saving(int i, char *s)
{
	sprintf(s, "%d %d %d %s %d\n%d %d %d %d %d %d %d %d %d %d %d\n",
		(NPCS + i)->x, (NPCS + i)->y, (NPCS + i)->cur_floor,
		(NPCS + i)->name, (NPCS + i)->personality,
		(NPCS + i)->stats.level, (NPCS + i)->stats.hp, (NPCS + i)->stats.maxhp,
		(NPCS + i)->stats.sp, (NPCS + i)->stats.maxsp, (NPCS + i)->stats.pa,
		(NPCS + i)->stats.pd, (NPCS + i)->stats.ra, (NPCS + i)->stats.rd,
		(NPCS + i)->stats.bp, (NPCS + i)->stats.ep);
}

void
add_loaded_npc(int n_npcs, int n, char name[20], int stats[15])
{
	if (NPCS == NULL) {
		N_NPCS = n_npcs;
		NPCS = malloc(sizeof(*NPCS) * N_NPCS);
	}
	strncpy((NPCS + n)->name, name, 18); (NPCS + n)->name[17] = '\0';
	(NPCS + n)->x = stats[0];
	(NPCS + n)->y = stats[1];
	(NPCS + n)->cur_floor = stats[2];
	(NPCS + n)->personality = stats[3];
	(NPCS + n)->stats.level = stats[4];
	(NPCS + n)->stats.hp = stats[5];
	(NPCS + n)->stats.maxhp = stats[6];
	(NPCS + n)->stats.sp = stats[7];
	(NPCS + n)->stats.maxsp = stats[8];
	(NPCS + n)->stats.pa = stats[9];
	(NPCS + n)->stats.pd = stats[10];
	(NPCS + n)->stats.ra = stats[11];
	(NPCS + n)->stats.rd = stats[12];
	(NPCS + n)->stats.bp = stats[13];
	(NPCS + n)->stats.ep = stats[14];
}
