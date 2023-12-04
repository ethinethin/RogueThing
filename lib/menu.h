#ifndef MENU_H
#define MENU_H

enum state { STATE_EMPTY = 0, STATE_PROGRESS = 1, STATE_SAVE = 2, STATE_4 = 4, STATE_5 = 8 };
enum menu { MENU_NEW, MENU_LOAD, MENU_QUIT };

extern void	init_menu(void);
extern int	main_menu(void);
extern int	get_gamestate(void);

#endif
