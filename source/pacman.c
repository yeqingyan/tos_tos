#include <kernel.h>

#define MAZE_WIDTH  19
#define MAZE_HEIGHT 16
#define GHOST_CHAR  '@'

typedef struct {
    int x;
    int y;
} GHOST;

WINDOW *pacman_wnd;

// Since our font only support ASCII(0-127) now, Extended ASCII code is 
// disabled.
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

/**
 * Draw maze char 
 * 
 * Now our font only support ASCII(0-127), so Extended ASCII code is disabled.
 * @param maze_char     char
 */
void draw_maze_char(char maze_char) {
    output_char(pacman_wnd, maze_char);
}

/**
 * draw pacman maze
 */
void draw_maze() {
    int x, y;
    WORD color_backup;

    clear_window(pacman_wnd);
    color_backup = get_fore_colour();
    set_fore_colour(COLOR_GREEN);
    y = 0;
    while (maze[y] != NULL) {
        char *row = maze[y];
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

/**
 * Generate pseudo random number
 * 
 * @return  random int
 */
int random() {
    last_random_number = (25173 * last_random_number + 13849) % 65536;
    return last_random_number;
}

/**
 * Initialise a ghost
 * 
 * @param ghost
 */
void init_ghost(GHOST *ghost) {
    while (1) {
        int x = random() % MAZE_WIDTH;
        int y = random() % MAZE_HEIGHT;
        if (maze[y][x] != ' ') continue;
        ghost->x = x;
        ghost->y = y;
        break;
    }
}

/**
 * Choose a random direction
 * 
 * @param dx
 * @param dy
 */
void choose_random_direction(int *dx, int *dy) {
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

/**
 * Move ghost to a position
 * 
 * @param ghost
 * @param dx
 * @param dy
 * @return 
 */
BOOL move_ghost(GHOST* ghost, int dx, int dy) {
    int old_x = ghost->x;
    int old_y = ghost->y;
    int new_x = old_x + dx;
    int new_y = old_y + dy;
    WORD color_backup;
    if (maze[new_y][new_x] != ' ') {
        // Don't run into a wall
        return FALSE;
    }

    volatile unsigned int cpsr_flag;
    SAVE_CPSR_DIS_IRQ(cpsr_flag);

    color_backup = get_fore_colour();
    set_fore_colour(COLOR_RED);
    move_cursor(pacman_wnd, old_x, old_y);
    remove_cursor(pacman_wnd);
    move_cursor(pacman_wnd, new_x, new_y);
    show_cursor(pacman_wnd);
    ghost->x = new_x;
    ghost->y = new_y;
    set_fore_colour(color_backup);
    RESUME_CPSR(cpsr_flag);
    return TRUE;
}

/**
 * Create a new ghost
 */
void create_new_ghost() {
    GHOST ghost;
    int dx, dy;

    init_ghost(&ghost);
    choose_random_direction(&dx, &dy);

    while (1) {
        while (move_ghost(&ghost, dx, dy) == FALSE) {
            choose_random_direction(&dx, &dy);
            sleep(1);
        }
    }
}

/**
 * Ghost process
 * 
 * @param self
 * @param param
 */
void ghost_proc(PROCESS self, PARAM param) {
    create_new_ghost();
}

/**
 * Init pacman 
 * 
 * @param wnd
 * @param num_ghosts
 */
void init_pacman(WINDOW *wnd, int num_ghosts) {
    pacman_wnd = wnd;
    pacman_wnd->width = MAZE_WIDTH;
    pacman_wnd->height = MAZE_HEIGHT + 1;
    pacman_wnd->cursor_char = GHOST_CHAR;

    draw_maze();
    int i;

    for (i = 0; i < num_ghosts; i++) {
        create_process(ghost_proc, 5, 0, "Ghost");
    }

    return;
}
