#include <unistd.h>
#include "disp.h"
#include "libs.h"
#include "menu.h"
#include "save.h"

int STATE;

void
init_menu(void)
{
	STATE = 0;
	STATE += save_exists();
}

int
main_menu(void)
{
	int c;
	while (1) {
		c = getch();
		erase();
		draw_menu(STATE);
		refresh();
		usleep(8333);
		switch (c) {
			case 'l':
			case 'L':
				if ((STATE & STATE_SAVE) == STATE_SAVE) {
					return MENU_LOAD;
				}
				break;
			case 'n':
			case 'N':
				return MENU_NEW;
				break;
			case 'o':
			case 'O':
				// do options stuff
				break;
			case 'q':
			case 'Q':
				return MENU_QUIT;
				break;
		}
	}
}

int
get_gamestate(void)
{
	return STATE;
}

void
set_gamestate(int state)
{
	STATE = (STATE & state);
}
