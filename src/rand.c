#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

long int
seed_rng(void)
{
	struct timeval time;
	gettimeofday(&time, NULL);
	srand(time.tv_sec);
	return (time.tv_sec);
}

int
rand_num(int min, int max)
{
	return rand() % (1 + max - min) + min;
}
