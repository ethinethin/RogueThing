#ifndef TYPES_H
#define TYPES_H

struct mapspace {
	int cur_floor;
	int w;
	int h;
	int *floorspace;
	char *explored;
	int n_rooms;
	int *room_x;
	int *room_y;
	int begin[2];
	int end[2];
};

struct stats {
	int level;
	int hp;
	int maxhp;
	int sp;
	int maxsp;
	int pa;
	int pd;
	int ra;
	int rd;
	int bp;
	int ep;
};

struct playerspace {
	int x;
	int y;
	int cur_floor;
	int cur_time;
	char name[18];
	char *vis;
	struct stats stats;
};

struct npcspace {
	int x;
	int y;
	int cur_floor;
	char name[18];
	int personality;
	struct stats stats;
};

struct npc_info {
	int n_npcs;
	int *i;
	int *x;
	int *y;
};

#endif
