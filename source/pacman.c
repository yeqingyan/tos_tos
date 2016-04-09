
#include <kernel.h>

#define MAZE_WIDTH  19
#define MAZE_HEIGHT 16
#define GHOST_CHAR  0x02

typedef struct {
    int x;
    int y;
} GHOST;


WINDOW* pacman_wnd;

/* Since our font only support ASCII(0-127) now, so Extended ASCII code is disabled.*/
char *maze[] = {
    "+--------+--------+",
    "|        |        |",
    "| ++ +-+ | +-+ ++ |",
    "| ++ +-+ | +-+ ++ |",
    "|                 |",
    "| -- | --+-- | -- |",
    "|    |   |   |    |",
    "+--- +--   --+ ---+",
    "|        |        |",
    "| -+ --- | --- +- |",
    "|  |           |  |",
    "+- | | --+-- | | -+",
    "|    |   |   |    |",
    "| ---+-- | --+--- |",
    "|                 |",
    "+-----------------+",
    NULL
};

/* Now our font only support ASCII(0-127), so Extended ASCII code is disabled.*/
void draw_maze_char(char maze_char)
{
    output_char(pacman_wnd, maze_char);
}



void draw_maze()
{
    int x, y;
    WORD color_backup;
    
    clear_window(pacman_wnd);
    color_backup = get_fore_colour();
    y = 0;
    while (maze[y] != NULL) {
	char* row = maze[y];
	x = 0;
	while (row[x] != '\0') {
	    char ch = row[x];
	    draw_maze_char(ch);
	    x++;
	}
	y++;
    }
    set_fore_colour(color_backup);
    wprintf(pacman_wnd, "PacMan ");
}



// Pseudo random number generator
// http://cnx.org/content/m13572/latest/
int seed = 17489;
int last_random_number = 0;

int random()
{
    last_random_number = (25173 * last_random_number + 13849) % 65536;
    return last_random_number;
}

void init_ghost(GHOST* ghost)
{
    while (1) {
	int x = random() % MAZE_WIDTH;
	int y = random() % MAZE_HEIGHT;
	if (maze[y][x] != ' ') continue;    
	ghost->x = x;
	ghost->y = y;
	break;
    }
}


void choose_random_direction(int* dx, int* dy)
{
    *dx = 0;
    *dy = 0;
    int dir = random() % 4;
    switch (dir) {
    case 0:
	*dx = -1;
	break;
    case 1:
	*dx = 1;
	break;
    case 2:
	*dy = -1;
	break;
    case 3:
	*dy = 1;
	break;
    }
}

/*
BOOL move_ghost(GHOST* ghost, int dx, int dy)
{
    int old_x = ghost->x;
    int old_y = ghost->y;
    int new_x = old_x + dx;
    int new_y = old_y + dy;
    if (maze[new_y][new_x] != ' ')
	// Don't run into a wall
	return FALSE;
    move_cursor(pacman_wnd, old_x, old_y);
    remove_cursor(pacman_wnd);
    move_cursor(pacman_wnd, new_x, new_y);
    show_cursor(pacman_wnd);
    ghost->x = new_x;
    ghost->y = new_y;
    return TRUE;
}


void create_new_ghost()
{
    GHOST ghost;
    int dx, dy;

    init_ghost(&ghost);
    choose_random_direction(&dx, &dy);
    
    while (1) {
	sleep(10);
	while (move_ghost(&ghost, dx, dy) == FALSE)
	    choose_random_direction(&dx, &dy);
    }
}

    
void ghost_proc(PROCESS self, PARAM param)
{
    create_new_ghost();
}

*/
void init_pacman(WINDOW* wnd, int num_ghosts)
{
    pacman_wnd = wnd;
    pacman_wnd->width = MAZE_WIDTH;
    pacman_wnd->height = MAZE_HEIGHT + 1;
    pacman_wnd->cursor_char = GHOST_CHAR;

    draw_maze();
    /*
    int i;
    
    for (i = 0; i < num_ghosts; i++)
	create_process(ghost_proc, 3, 0, "Ghost");
    */
    return;
}
