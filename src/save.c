#include <stdio.h>
#include <stdlib.h>
#include "map.h"
#include "player.h"

static void	savemap(struct mapspace **mapwallet, int n_maps, FILE *f);
static void	saveplayer(struct playerspace *player, FILE *f);

void
quitsave(struct mapspace **mapwallet, struct playerspace *player, int n_maps)
{
	FILE *f;

	f = fopen("game.dat", "w");
	if (f == NULL) {
		printf("Cannot open 'game.dat'\n");
		exit(1);
	}
	savemap(mapwallet, n_maps, f);
	saveplayer(player, f);
	fclose(f);
}

static void
savemap(struct mapspace **mapwallet, int n_maps, FILE *f)
{
	int i, j, x, y;
	struct mapspace *map;

	fprintf(f, "%d\n", n_maps);
	for (i = 0; i < n_maps; i += 1) {
		map = *(mapwallet + i);
		fprintf(f, "%d %d %d %d\n", map->cur_floor, map->w, map->h, map->n_rooms);
		fprintf(f, "%d %d %d %d\n", map->begin[0], map->end[0], map->begin[1], map->end[1]);
		for (j = 0; j < map->n_rooms; j += 1) {
			fprintf(f, "%d %d\n", *(map->room_x + j), *(map->room_y + j));
		}
		for (y = 0; y < map->h; y += 1) {
			for (x = 0; x < map->w; x += 1) {
				fprintf(f, "%d ", *(map->floorspace + xy2flat(x, y, map->w)));
			}
			fputc('\n', f);
		}
		for (y = 0; y < map->h; y += 1) {
			for (x = 0; x < map->w; x += 1) {
				fprintf(f, "%d ", *(map->explored + xy2flat(x, y, map->w)));
			}
			fputc('\n', f);
		}
	}
}

static void
saveplayer(struct playerspace *player, FILE *f)
{
	fprintf(f, "%d %d %d %d %s\n", player->x, player->y, player->cur_floor, player->cur_time, player->name);
}

int
loadsave(struct mapspace **mapwallet, struct playerspace *player)
{
	return 0;
}
