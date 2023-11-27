#include <stdlib.h>
#include <string.h>
#include "libs.h"
#include "map.h"
#include "player.h"
#include "rand.h"

static void	rando_name(char name[18]);

struct playerspace *
init_playerspace(int cx, int cy)
{
	struct playerspace *player;

	player = malloc(sizeof(*player));
	player->x = cx;
	player->y = cy;
	player->cur_floor = 0;
	player->cur_time = 0;
	rando_name(player->name);
	return player;
}

void
kill_playerspace(struct playerspace *player)
{
	free(player);
} 

void
move_player(struct mapspace *map, struct playerspace *player, int cx, int cy)
{
	int x, y;

	/* Change coordinates */
	x = player->x + cx;
	y = player->y + cy;
	/* Make sure it is on the map */
	if (x < 0 || y < 0 || x > map->w - 1 || y > map->h - 1) return;
	/* If floor type is WALL, don't go changin' */
	if (*(map->floorspace + xy2flat(x, y, 100)) == FLOOR_WALL) return;
	/* Otherwise, make the change and increase the time */
	player->x = x;
	player->y = y;
	player->cur_time += 10;
}

static void
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
