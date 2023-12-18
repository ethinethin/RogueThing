#include <stdlib.h>
#include <string.h>
#include "libs.h"
#include "map.h"
#include "npcs.h"
#include "player.h"
#include "rand.h"
#include "types.h"

enum behavior {
	BEH_HOST_ROOM_RANGED, BEH_HOST_ROOM_CLOSE, BEH_HOST_ROOM_FEINT, BEH_HOST_ROOM_RUN,
	BEH_HOST_TUNN_RANGED, BEH_HOST_TUNN_CLOSE, BEH_HOST_TUNN_FEINT, BEH_HOST_TUNN_RUN,
	BEH_HOST_BOTH_RANGED, BEH_HOST_BOTH_CLOSE, BEH_HOST_BOTH_FEINT, BEH_HOST_BOTH_RUN,
	BEH_NONH_ROOM_DRUNK, BEH_NONH_TUNN_DRUNK, BEH_NONH_BOTH_DRUNK
};

#define N_BEHAVIORS 15

struct behaviors {
	int behavior;
	int is_hostile;
	int tile1;
	int tile2;
	int prob[6];
} BEHAVIOR_TABLE[] = {
	{ BEH_HOST_ROOM_RANGED, 1, FLOOR_OPEN, -1, { 14, 22, 6, 14, 26, 18 } },
	{ BEH_HOST_ROOM_CLOSE, 1, FLOOR_OPEN, -1, { 22, 14, 22, 22, 14, 6 } },
	{ BEH_HOST_ROOM_FEINT, 1, FLOOR_OPEN, -1, { 14, 22, 14, 6, 22, 22 } },
	{ BEH_HOST_ROOM_RUN, 1, FLOOR_OPEN, -1, { 6, 6, 6, 38, 6, 38 } },
	{ BEH_HOST_TUNN_RANGED, 1, FLOOR_PATH, -1, { 14, 22, 6, 14, 26, 18 } },
	{ BEH_HOST_TUNN_CLOSE, 1, FLOOR_PATH, -1, { 22, 14, 22, 22, 14, 16 } },
	{ BEH_HOST_TUNN_FEINT, 1, FLOOR_PATH, -1, { 14, 22, 14, 6, 22, 22 } },
	{ BEH_HOST_TUNN_RUN, 1, FLOOR_PATH, -1, { 6, 6, 6, 38, 6, 38 } },
	{ BEH_HOST_BOTH_RANGED, 1, FLOOR_OPEN, FLOOR_PATH, { 14, 22, 6, 14, 26, 18 } },
	{ BEH_HOST_BOTH_CLOSE, 1, FLOOR_OPEN, FLOOR_PATH, { 22, 14, 22, 22, 14, 16 } },
	{ BEH_HOST_BOTH_FEINT, 1, FLOOR_OPEN, FLOOR_PATH, { 14, 22, 14, 6, 22, 22 } },
	{ BEH_HOST_BOTH_RUN, 1, FLOOR_OPEN, FLOOR_PATH, { 6, 6, 6, 38, 6, 38 } },
	{ BEH_NONH_ROOM_DRUNK, 0, FLOOR_OPEN, -1, { 17, 16, 16, 16, 16, 16 } },
	{ BEH_NONH_TUNN_DRUNK, 0, FLOOR_PATH, -1, { 17, 16, 16, 16, 16, 16 } },
	{ BEH_NONH_BOTH_DRUNK, 0, FLOOR_OPEN, FLOOR_PATH, { 17, 16, 16, 16, 16, 16 } }
};

static void	new_npc(struct mapspace **mapwallet, int i, int n_floors);
static int	is_on_other_npc(int cur_floor, int x, int y, int i);
static void	new_npc_stats(int i);
static void	upgrade_npc(int i);
static int	determine_stat(int personality);

struct npcspace *NPCS = NULL;
int N_NPCS;

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
	(NPCS + i)->personality = BEHAVIOR_TABLE[rand_num(0, N_BEHAVIORS - 1)].behavior;
	/* Determine is_hostile, and start tiles from behavior table */
	for (j = 0, tile1 = FLOOR_OPEN, tile2 = FLOOR_OPEN; j < N_BEHAVIORS; j += 1) {
		if (BEHAVIOR_TABLE[j].behavior == (NPCS + i)->personality) {
			(NPCS + i)->is_hostile = BEHAVIOR_TABLE[j].is_hostile;
			tile1 = BEHAVIOR_TABLE[j].tile1;
			tile2 = BEHAVIOR_TABLE[j].tile2;
			break;
		}
	}
	/* Pull the correct floor map and try to place them */
	while (1) {
		map = *(mapwallet + (NPCS + i)->cur_floor);
		do {
			x = rand_num(5, 94);
			y = rand_num(2, 22);
			floorspace = *(map->floorspace + xy2flat(x, y, map->w));
		} while(floorspace != tile1 && floorspace != tile2);
		if (is_on_other_npc((NPCS + i)->cur_floor, x, y, i) == 0) break;
	}
	/* Set starting location for NPC */
	(NPCS + i)->x = x;
	(NPCS + i)->y = y;
	/* Set starting values for NPC */
	new_npc_stats(i);
	upgrade_npc(i);
}

static int
is_on_other_npc(int cur_floor, int x, int y, int i)
{
	int j;
	for (j = 0; j < i; j += 1) {
		if ((NPCS + j)->cur_floor == cur_floor &&
		    (NPCS + j)->x == x &&
		    (NPCS + j)->y == y) {
			return 1;
		}
	}
	return 0;
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
	(NPCS + i)->stats.attack = 10;
	(NPCS + i)->stats.defense = 10;
	(NPCS + i)->stats.hit = 10;
	(NPCS + i)->stats.dodge = 10;
	(NPCS + i)->stats.bp = (NPCS + i)->stats.level * 1 + 10;
	(NPCS + i)->stats.xp = 0;
}

static void
upgrade_npc(int i)
{
	int ds;

	while ((NPCS + i)->stats.bp > 0) {
		ds = determine_stat((NPCS + i)->personality);
		switch(ds) {
			case 0:
				(NPCS + i)->stats.hp += 5;
				(NPCS + i)->stats.maxhp += 5;
				break;
			case 1:
				(NPCS + i)->stats.sp += 5;
				(NPCS + i)->stats.maxsp += 5;
				break;
			case 2:
				(NPCS + i)->stats.attack += 1;
				break;
			case 3:
				(NPCS + i)->stats.defense += 1;
				break;
			case 4:
				(NPCS + i)->stats.hit += 1;
				break;
			case 5:
				(NPCS + i)->stats.dodge += 1;
				break;
		}
		(NPCS + i)->stats.bp -= 1;
	}
}

void
upgrade_all_npcs(void)
{
	int i;


	for (i = 0; i < N_NPCS; i += 1) {
		if ((NPCS + i)->stats.hp == 0) continue;
		(NPCS + i)->stats.bp = 1;
		upgrade_npc(i);
	}
}

static int
determine_stat(int personality)
{
	int i, j, prob, r;

	/* Determine row of advancement table */
	for (i = 0; i < N_BEHAVIORS; i += 1) {
		if (BEHAVIOR_TABLE[i].behavior == personality) break;
	}
	if (i == N_BEHAVIORS) return 0;
	/* Do a probability roll! */
	r = rand_num(0, 99);
	/* Step through the probability values */
	for (j = 0, prob = 0; j < 6; j += 1) {
		prob += BEHAVIOR_TABLE[i].prob[j];
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
	sprintf(s, "%d %d %d %s %d %d %d %d %d %d %d %d %d %d %d %d %d\n",
		(NPCS + i)->x, (NPCS + i)->y, (NPCS + i)->cur_floor,
		(NPCS + i)->name, (NPCS + i)->personality, (NPCS + i)->is_hostile,
		(NPCS + i)->stats.level, (NPCS + i)->stats.hp, (NPCS + i)->stats.maxhp,
		(NPCS + i)->stats.sp, (NPCS + i)->stats.maxsp, (NPCS + i)->stats.attack,
		(NPCS + i)->stats.defense, (NPCS + i)->stats.hit, (NPCS + i)->stats.dodge,
		(NPCS + i)->stats.bp, (NPCS + i)->stats.xp);
}

void
add_loaded_npc(int n_npcs, int n, char name[20], int stats[16])
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
	(NPCS + n)->is_hostile = stats[4];
	(NPCS + n)->stats.level = stats[5];
	(NPCS + n)->stats.hp = stats[6];
	(NPCS + n)->stats.maxhp = stats[7];
	(NPCS + n)->stats.sp = stats[8];
	(NPCS + n)->stats.maxsp = stats[9];
	(NPCS + n)->stats.attack = stats[10];
	(NPCS + n)->stats.defense = stats[11];
	(NPCS + n)->stats.hit = stats[12];
	(NPCS + n)->stats.dodge = stats[13];
	(NPCS + n)->stats.bp = stats[14];
	(NPCS + n)->stats.xp = stats[15];
}

int
is_hostile(int n)
{
	return((NPCS + n)->is_hostile);
}

int
is_alive(int n)
{
	return ((NPCS + n)->stats.hp > 0) ? 1 : 0;
}

void
swap_spaces(struct playerspace *player, int n)
{
	int tmp_x, tmp_y;

	tmp_x = player->x;
	tmp_y = player->y;
	player->x = (NPCS + n)->x;
	player->y = (NPCS + n)->y;
	(NPCS + n)->x = tmp_x;
	(NPCS + n)->y = tmp_y;
}

int
kill_enemy(int n)
{
	(NPCS + n)->stats.hp = 0;
	return (NPCS + n)->stats.level;
}

void
move_all_npcs(struct mapspace *map, struct playerspace *player, struct npc_info npcs)
{
	int x, y, i, j, k;

	for (i = 0; i < npcs.n_npcs; i += 1) {
		j = *(npcs.i + i);
		if (rand_num(0, 99) > 25 || (NPCS + j)->stats.hp == 0) continue;
		x = (NPCS + j)->x + rand_num(-1, 1);
		y = (NPCS + j)->y + rand_num(-1, 1);
		/* Make sure it's not a wall */
		if (*(map->floorspace + xy2flat(x, y, map->w)) == FLOOR_WALL) continue;
		/* Make sure player is not there */
		if (x == player->x && y == player->y) continue;
		/* Make sure no NPC is there */
		for (k = 0; k < npcs.n_npcs; k += 1) {
			if (k == i) continue;
			if (x == *(npcs.x + k) && y == *(npcs.y + k)) {
				x = (NPCS + j)->x;
				y = (NPCS + j)->y;
				break;
			}
		}
		/* All good, move it */
		(NPCS + j)->x = x;
		(NPCS + j)->y = y;
	}
}
