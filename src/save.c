#include <stdio.h>
#include <stdlib.h>
#include "map.h"
#include "menu.h"
#include "player.h"

static void	save_maps(struct mapspace **mapwallet, int n_maps, FILE *f);
static void	save_player(struct playerspace *player, FILE *f);
static int	load_maps(struct mapspace ***mapwallet, FILE *f);
static void	load_player(struct playerspace **player, FILE *f, struct mapspace *map);

int
save_exists(void)
{
	int c;
	FILE *f;
	
	f = fopen("game.dat", "r");
	if (f == NULL) {
		return STATE_EMPTY;
	} else if ((c = fgetc(f)) == EOF) {
		fclose(f);
		return STATE_EMPTY;
	} else {
		fclose(f);
		return STATE_SAVE;
	}
}

void
quit_save(struct mapspace **mapwallet, struct playerspace *player, int n_maps)
{
	FILE *f;

	f = fopen("game.dat", "w");
	if (f == NULL) {
		printf("Cannot open 'game.dat'\n");
		exit(1);
	}
	save_maps(mapwallet, n_maps, f);
	save_player(player, f);
	fclose(f);
}

static void
save_maps(struct mapspace **mapwallet, int n_maps, FILE *f)
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
save_player(struct playerspace *player, FILE *f)
{
	fprintf(f, "%d %d %d %d %s\n", player->x, player->y, player->cur_floor, player->cur_time, player->name);
	fprintf(f, "%d %d %d %d %d %d %d %d %d %d\n", player->stats.hp, player->stats.maxhp, player->stats.sp, player->stats.maxsp, player->stats.pa, player->stats.pd, player->stats.ra, player->stats.rd, player->stats.bp, player->stats.ep);
}

int
load_save(struct mapspace ***mapwallet, struct playerspace **player)
{
	int n_maps;
	FILE *f;

	f = fopen("game.dat", "r");
	n_maps = load_maps(mapwallet, f);
	load_player(player, f, **mapwallet);
	fclose(f);
	return n_maps;
}

static int
load_maps(struct mapspace ***mapwallet, FILE *f)
{
	int c, i, j, n_maps, x, y;
	struct mapspace *map;

	fscanf(f, "%d\n", &n_maps);
	*mapwallet = malloc(sizeof(**mapwallet) * n_maps);
	for (i = 0; i < n_maps; i += 1) {
		*((*mapwallet) + i) = malloc(sizeof(***mapwallet));
		map = *(*(mapwallet) + i);
		fscanf(f, "%d %d %d %d\n", &map->cur_floor, &map->w, &map->h, &map->n_rooms);
		fscanf(f, "%d %d %d %d\n", &map->begin[0], &map->end[0], &map->begin[1], &map->end[1]);
		map->room_x = malloc(sizeof(*map->room_x) * map->n_rooms);
		map->room_y = malloc(sizeof(*map->room_y) * map->n_rooms);
		for (j = 0; j < map->n_rooms; j += 1) {
			fscanf(f, "%d %d\n", map->room_x + j, map->room_y + j);
		}
		map->floorspace = malloc(sizeof(*map->floorspace) * map->w * map->h); 
		for (y = 0; y < map->h; y += 1) {
			for (x = 0; x < map->w; x += 1) {
				fscanf(f, "%d ", map->floorspace + xy2flat(x, y, map->w));
			}
		}
		map->explored = malloc(sizeof(*map->explored) * map->w * map->h);
		for (y = 0; y < map->h; y += 1) {
			for (x = 0; x < map->w; x += 1) {
				fscanf(f, "%d ", &c);
				*(map->explored + xy2flat(x, y, map->w)) = c;
			}
		}
	}
	return n_maps;
}

static void
load_player(struct playerspace **player, FILE *f, struct mapspace *map)
{
	struct playerspace *p;

	*player = malloc(sizeof(**player));
	p = *player;
	fscanf(f, "%d %d %d %d %s\n", &p->x, &p->y, &p->cur_floor, &p->cur_time, p->name);
	p->vis = malloc(sizeof(*p->vis) * map->w * map->h);
	fscanf(f, "%d %d %d %d %d %d %d %d %d %d\n", &p->stats.hp, &p->stats.maxhp, &p->stats.sp, &p->stats.maxsp, &p->stats.pa, &p->stats.pd, &p->stats.ra, &p->stats.rd, &p->stats.bp, &p->stats.ep);
}
