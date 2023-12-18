#include <stdlib.h>
#include <string.h>
#include "disp.h"
#include "libs.h"
#include "map.h"
#include "npcs.h"
#include "player.h"
#include "rand.h"
#include "types.h"

#define TIME_TICK 10

static void	clear_vis(struct mapspace *map, struct playerspace *player);
static void	vis_room(struct mapspace *map, struct playerspace *player);
static int	fill_point(struct mapspace *map, char *vis, int x, int y);
static void	update_explored(struct mapspace *map, struct playerspace *player);
static int	level_up(struct playerspace *player);
static int	determine_level(int current_xp);

int DAY;

struct playerspace *
init_playerspace(struct mapspace *map, int x, int y)
{
	int i;
	struct playerspace *player;

	player = malloc(sizeof(*player));
	player->x = x;
	player->y = y;
	player->cur_floor = 0;
	player->cur_time = 0;
	rando_name(player->name);
	/* Set up visibility space and initialize to 0 */
	player->vis = malloc(sizeof(*player->vis) * map->w * map->h);
	for (i = 0; i < map->w * map->h; i += 1) *(player->vis + i) = 0;
	/* Set stats to default numbers */
	player->stats.level = 1;
	player->stats.hp = 10;
	player->stats.maxhp = 10;
	player->stats.sp = 10;
	player->stats.maxsp = 10;
	player->stats.attack = 10;
	player->stats.defense = 10;
	player->stats.hit = 10;
	player->stats.dodge = 10;
	player->stats.bp = 10;
	player->stats.xp = 0;
	/* We're on the first day */
	DAY = 0;
	return player;
}

void
kill_playerspace(struct playerspace *player)
{
	free(player->vis);
	free(player);
}

void
load_day(int cur_time)
{
	DAY = cur_time / 1440;
}

void
move_player(struct mapspace *map, struct playerspace *player, struct npc_info npcs, int cx, int cy)
{
	char mesg[200];
	int i, level, x, y;

	/* Change coordinates */
	x = player->x + cx;
	y = player->y + cy;
	/* Make sure it is on the map */
	if (x < 0 || x > map->w - 1 || y < 0 || y > map->h - 1) return;
	/* If floor type is WALL, or NPC is standin' there, don't go changin' */
	if (*(map->floorspace + xy2flat(x, y, map->w)) == FLOOR_WALL) {
		add_log("THUD!");
		return;
	}
	/* If you're stepping on top of an NPC, check their is_hostile status
	 * and is_alive status. Three outcomes:
	 * 	Non-hostile, alive - swap places
	 *	Hostile, alive - combat
	 *	Either, dead - step on top 					*/
	for (i = 0; i < npcs.n_npcs; i += 1) {
		if (*(npcs.x + i) == x && *(npcs.y + i) == y) {
			if (is_hostile(*(npcs.i + i)) == 0 && is_alive(*(npcs.i + i)) == 1) {
				sprintf(mesg, "You swapped spaces with %s.", get_name(*(npcs.i + i)));
				add_log(mesg);
				swap_spaces(player, *(npcs.i + i));
			} else if (is_hostile(*(npcs.i + i)) == 1 && is_alive(*(npcs.i + i)) == 1) {
				/* Combat .. eventually .. will go here */
				player->stats.xp += kill_enemy(*(npcs.i + i));
				sprintf(mesg, "Oh my God! You killed %s!", get_name(*(npcs.i + i)));
				add_log(mesg);
				x = player->x;
				y = player->y;
			}
		}
	}
	/* Otherwise, make the change and increase the time */
	player->x = x;
	player->y = y;
	player->cur_time += TIME_TICK;
	if (player->cur_time / 1440 != DAY) {
		DAY = player->cur_time / 1440;
		upgrade_all_npcs();
		add_log("One more day has passed in the tower. All enemies have gotten stronger...");
	}
	level = level_up(player);
	if (level != -1) {
		sprintf(mesg, "Congratulations! You leveled up to level %d!", level);
		add_log(mesg);
	}
	/* Did they "wait"? */
	if (cx == 0 && cy == 0) {
		add_log("You waited for one turn.");
	}
	move_all_npcs(map, player, npcs);
}

void
rando_name(char name[18])
{
	char *consonants[] = { "b", "bl", "c", "cl", "cr", "d", "fl", "fr", "g", "gl",
			       "gr", "h", "kl", "kr", "l", "ll", "m", "p", "pl",
			       "pr", "s", "sl", "st", "t", "tr", "v", "w", "wr", "z", "zh" };
	char *vowels[] = { "ae", "ai", "ar", "ay", "e", "ee", "en", "i", "o", "on", "oo", "ou", "u", "y" };
	int i;
	
	/* Empty out the string */
	for (i = 0; i < 18; i++) name[i] = '\0';
	/* Add random syllables */
	for (i = 0; i < rand_num(2, 4); i++) {
		strncat(name, consonants[rand_num(0, 29)], 2);
		strncat(name, vowels[rand_num(0, 13)], 2);
	}
	/* Make first letter uppercase */
	name[0] -= 32;
}	

void
move_floors(struct mapspace *map, struct playerspace *player, int z, int maxfloor)
{
	char mesg[200];
	int floorspace;

	floorspace = *(map->floorspace + xy2flat(player->x, player->y, map->w));
	if ((z == 1 && floorspace == FLOOR_END && player->cur_floor < maxfloor - 1) ||
	    (z == -1 && floorspace == FLOOR_BEGIN && player->cur_floor > 0)) {
		player->cur_floor += z;
		player->cur_time += TIME_TICK;
		if (z == 1) {
			sprintf(mesg, "You moved up to floor %d.", player->cur_floor + 1);
		} else if (z == -1) {
			sprintf(mesg, "You moved down to floor %d.", player->cur_floor + 1);
		}
	} else {
		if (z == 1) {
			sprintf(mesg, "You could not move upstairs from here.");
		} else if (z == -1) {
			sprintf(mesg, "You could not move downstairs from here.");
		}
	}
	add_log(mesg);
}

void
check_vis(struct mapspace *map, struct playerspace *player)
{
	int f, x, y;
	char in_room;

	clear_vis(map, player);
	/* If we are in a room or adjacent to a room, we should see inside the room */
	for (x = player->x - 1, in_room = 0; x <= player->x + 1; x += 1) {
		for (y = player->y - 1; y <= player->y + 1; y += 1) {
			f = *(map->floorspace + xy2flat(x, y, map->w));
			/* Update explored and visible for this area */
			*(map->explored + xy2flat(x, y, map->w)) = 1;
			if (f == FLOOR_WALL || f == FLOOR_PATH) {
				*(player->vis + xy2flat(x, y, map->w)) = 2;
			} else {
				*(player->vis + xy2flat(x, y, map->w)) = 1;
			}
			/* Check is it a room? */
			if (*(map->floorspace + xy2flat(x, y, map->w)) == FLOOR_OPEN) {
				in_room = 1;
			}
		}
	}
	if (in_room == 0) return;
	/* We are in a room ... we need to make the whole thing visible */
	vis_room(map, player);
	/* Now update explored so we can see it */
	update_explored(map, player);
}

static void
clear_vis(struct mapspace *map, struct playerspace *player)
{
	int x, y;

	for (x = 0; x < map->w; x += 1) {
		for (y = 0; y < map->h; y += 1) {
			*(player->vis + xy2flat(x, y, map->w)) = 0;
		}
	}
}

static void
vis_room(struct mapspace *map, struct playerspace *player)
{
	int changes, x, y;

	while (1) {
		changes = 0;
		for (x = 0; x < map->w; x += 1) {
			for (y = 0; y < map->h; y += 1) {
				if (*(player->vis + xy2flat(x, y, map->w)) == 1) {
					changes += fill_point(map, player->vis, x, y);
				}
			}
		}
		if (changes == 0) break;
	}
}

static int
fill_point(struct mapspace *map, char *vis, int x, int y)
{
	int changes, dx, dy, f;
	
	changes = 0;
	for (dx = -1; dx <= 1; dx += 1) {
		for (dy = -1; dy <= 1; dy += 1) {
			if (x + dx < 0 || x + dx > map->w - 1 || y + dy < 0 || y + dy > map->h -1) continue;
			if (*(vis + xy2flat(x + dx, y + dy, map->w)) == 0) {
				f = *(map->floorspace + xy2flat(x + dx, y + dy, map->w));
				if (f == FLOOR_OPEN || f == FLOOR_BEGIN || f == FLOOR_END) {
					*(vis + xy2flat(x + dx, y + dy, map->w)) = 1;
					changes += 1;
				} else if (f == FLOOR_WALL || f == FLOOR_PATH) {
					*(vis + xy2flat(x + dx, y + dy, map->w)) = 2;
					changes += 1;
				}
			}
		}
	}
	return changes;	
}

static void
update_explored(struct mapspace *map, struct playerspace *player)
{
	int x, y;

	for (x = 0; x < map->w; x += 1) {
		for (y = 0; y < map->h; y += 1) {
			if (*(player->vis + xy2flat(x, y, map->w)) != 0) {
				*(map->explored + xy2flat(x, y, map->w)) = 1;
			}
		}
	}
}	

static int
level_up(struct playerspace *player)
{
	int level;

	level = determine_level(player->stats.xp);
	if (level != player->stats.level) {
		player->stats.level = level;
		return level;
	} else {
		return -1;
	}
}

static int
determine_level(int current_xp)
{
	int level, xp;

	for (level = 1, xp = 0; level < 1000; level += 1) {
		xp += level * 5;
		if (xp > current_xp) {
			break;
		}
	}
	return level;
}
