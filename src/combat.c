#include <stdio.h>
#include "disp.h"
#include "libs.h"
#include "npcs.h"
#include "rand.h"
#include "types.h"

int
player_attack(struct playerspace *player, int n)
{
	char mesg[200];
	int damage_amount, hit_prob, hit_prob_roll, max_damage, npc_stats[6];

	/* Get npc stats */
	get_npc_stats(n, npc_stats);
	/* Calculate chance to hit */
	hit_prob = ((10 + player->stats.hit) * 100) / (player->stats.hit + npc_stats[5]);
	/* Roll to hit */
	hit_prob_roll = rand_num(0, 99);
	if (hit_prob_roll < hit_prob) {
		/* Successfully hit, calculate damage */
		max_damage = 2 * (player->stats.attack - npc_stats[3]);
		if (max_damage < 2) max_damage = 2;
		/* Roll for damage */
		damage_amount = rand_num(0, max_damage);
		if (damage_amount == 0) {
			/* No damage - a miss! */
			sprintf(mesg, "You attacked %s, but missed!", get_name(n));
			add_log(mesg);
			return 0;
		} else {
			/* You did damage - apply it */
			sprintf(mesg, "You attacked %s and hit for %d damage!", get_name(n), damage_amount);
			if (damage_npc(n, damage_amount) == 0) {
				sprintf(mesg, "%s They died!", mesg);
				add_log(mesg);
				return get_level(n);
			}
			add_log(mesg);
			return 0;
		}
	} else {
		/* They dodged */
		sprintf(mesg, "You attacked %s, but they dodged!", get_name(n));
		add_log(mesg);
		return 0;
	}
}

void
npc_attack(struct playerspace *player, int n)
{
}
