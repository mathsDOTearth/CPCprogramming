/*
 * SULTAN'S MAZE II - ASCII Topdown
 * Title + Input + Maze Generation + Top-down map
 *
 * by @mathsDOTearth on github
 * https://github.com/mathsDOTearth/CPCprogramming/
 * 
 * Compile:
 *   zcc +cpc -clib=ansi -lndos -O2 -create-app maze.c -o maze.bin
 *   iDSK maze.dsk -n
 *   iDSK maze.dsk -i ./maze.cpc
 *
 * Run: run"maze.cpc
 */

/* Request a 512 byte stack instead of the default 256 */
#pragma output CRT_STACK_SIZE = 512

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* z88dk console key read */
extern int fgetc_cons(void);

/* ============================================================
 * CONSTANTS
 * ============================================================ */

#define MAZE_W    16
#define MAZE_H    16
#define NUM_GEMS   6

/* Directions: 0=North, 1=East, 2=South, 3=West */
#define DIR_N 0
#define DIR_E 1
#define DIR_S 2
#define DIR_W 3

/* Cell flags */
#define CELL_WALL  1
#define CELL_GEM   2
#define CELL_EXIT  4

/* ============================================================
 * GLOBAL STATE
 * ============================================================ */

unsigned char maze[MAZE_H][MAZE_W];
unsigned char px, py, pdir;  /* player position and direction */
int energy;
unsigned char gems_collected;
unsigned char gems_total;
unsigned char ghost_x, ghost_y;
unsigned char ghost_timer;
unsigned char game_running;
unsigned char maze_seed;

/* Gem positions */
unsigned char gem_x[NUM_GEMS], gem_y[NUM_GEMS];
unsigned char gem_taken[NUM_GEMS];

/* Exit position */
unsigned char exit_x, exit_y;

/* Direction deltas: N, E, S, W */
const signed char dx[] = { 0, 1, 0, -1 };
const signed char dy[] = { -1, 0, 1, 0 };

/* ============================================================
 * SIMPLE RNG (LFSR)
 * ============================================================ */

unsigned int rng_state = 12345;

unsigned char rng(void)
{
    unsigned char bit;
    bit = ((rng_state >> 0) ^ (rng_state >> 2) ^
           (rng_state >> 3) ^ (rng_state >> 5)) & 1;
    rng_state = (rng_state >> 1) | (bit << 15);
    return (unsigned char)(rng_state & 0xFF);
}

/* ============================================================
 * SCREEN HELPERS
 * Using ANSI escape codes via -clib=ansi
 * ============================================================ */

void cls(void)
{
    /* ANSI clear screen + home cursor */
    putchar(27); putchar('['); putchar('2'); putchar('J');
    putchar(27); putchar('['); putchar('H');
}

void goto_xy(unsigned char x, unsigned char y)
{
    /* ANSI cursor positioning: ESC[row;colH */
    /* row and col are 1-based */
    putchar(27); putchar('[');
    /* output row as digits */
    if (y >= 10) putchar('0' + y / 10);
    putchar('0' + y % 10);
    putchar(';');
    if (x >= 10) putchar('0' + x / 10);
    putchar('0' + x % 10);
    putchar('H');
}

void print_at(unsigned char x, unsigned char y, char *s)
{
    goto_xy(x, y);
    while (*s) {
        putchar(*s);
        s++;
    }
}

void print_num_at(unsigned char x, unsigned char y, int n)
{
    char buf[8];
    int i = 0;

    goto_xy(x, y);

    if (n == 0) {
        putchar('0');
        return;
    }
    if (n < 0) {
        putchar('-');
        n = -n;
    }
    while (n > 0 && i < 7) {
        buf[i++] = '0' + (n % 10);
        n /= 10;
    }
    while (i > 0) {
        putchar(buf[--i]);
    }
}

/* ============================================================
 * MAZE GENERATION (Recursive backtracker, explicit stack)
 * ============================================================ */

#define STACK_SIZE 128
unsigned char stk_x[STACK_SIZE];
unsigned char stk_y[STACK_SIZE];
unsigned int stk_ptr;

void generate_maze(void)
{
    unsigned char x, y, nx, ny;
    unsigned char dirs[4];
    unsigned char i, j, count;

    /* Fill with walls */
    for (y = 0; y < MAZE_H; y++)
        for (x = 0; x < MAZE_W; x++)
            maze[y][x] = CELL_WALL;

    /* Start carving from (1,1) */
    x = 1; y = 1;
    maze[y][x] = 0;
    stk_ptr = 0;
    stk_x[stk_ptr] = x;
    stk_y[stk_ptr] = y;
    stk_ptr++;

    while (stk_ptr > 0) {
        x = stk_x[stk_ptr - 1];
        y = stk_y[stk_ptr - 1];

        /* Find unvisited neighbours 2 cells away */
        count = 0;
        for (i = 0; i < 4; i++) {
            nx = x + dx[i] * 2;
            ny = y + dy[i] * 2;
            if (nx >= 1 && nx < MAZE_W - 1 &&
                ny >= 1 && ny < MAZE_H - 1 &&
                maze[ny][nx] == CELL_WALL) {
                dirs[count++] = i;
            }
        }

        if (count == 0) {
            stk_ptr--;
        } else {
            j = rng() % count;
            i = dirs[j];
            nx = x + dx[i] * 2;
            ny = y + dy[i] * 2;

            /* Carve passage */
            maze[y + dy[i]][x + dx[i]] = 0;
            maze[ny][nx] = 0;

            stk_x[stk_ptr] = nx;
            stk_y[stk_ptr] = ny;
            stk_ptr++;
        }
    }

    /* Add a few extra openings for variety */
    for (i = 0; i < 10; i++) {
        x = (rng() % (MAZE_W - 4)) + 2;
        y = (rng() % (MAZE_H - 4)) + 2;
        if (maze[y][x] == CELL_WALL) {
            count = 0;
            if (y > 0 && maze[y-1][x] == 0) count++;
            if (y < MAZE_H-1 && maze[y+1][x] == 0) count++;
            if (x > 0 && maze[y][x-1] == 0) count++;
            if (x < MAZE_W-1 && maze[y][x+1] == 0) count++;
            if (count == 2)
                maze[y][x] = 0;
        }
    }
}

/* ============================================================
 * PLACE ITEMS
 * ============================================================ */

void place_items(void)
{
    unsigned char i, x, y;

    /* Player at start */
    px = 1; py = 1; pdir = DIR_E;
    maze[1][1] = 0;

    /* Place gems */
    gems_total = NUM_GEMS;
    for (i = 0; i < NUM_GEMS; i++) {
        do {
            x = (rng() % (MAZE_W - 2)) + 1;
            y = (rng() % (MAZE_H - 2)) + 1;
        } while (maze[y][x] != 0 || (x <= 2 && y <= 2));
        gem_x[i] = x;
        gem_y[i] = y;
        gem_taken[i] = 0;
        maze[y][x] |= CELL_GEM;
    }

    /* Exit in far corner */
    exit_x = MAZE_W - 2;
    exit_y = MAZE_H - 2;
    /* Ensure exit is reachable */
    maze[exit_y][exit_x] = CELL_EXIT;
    if (maze[exit_y - 1][exit_x] == CELL_WALL &&
        maze[exit_y][exit_x - 1] == CELL_WALL) {
        maze[exit_y - 1][exit_x] = 0;
    }

    /* Ghost starts away from player */
    do {
        ghost_x = (rng() % (MAZE_W - 4)) + 2;
        ghost_y = (rng() % (MAZE_H - 4)) + 2;
    } while ((maze[ghost_y][ghost_x] & CELL_WALL) ||
             (ghost_x + ghost_y < 8));

    ghost_timer = 0;
    energy = 500;
    gems_collected = 0;
    game_running = 1;
}

/* ============================================================
 * GHOST AI
 * ============================================================ */

unsigned char is_wall(unsigned char cx, unsigned char cy)
{
    if (cx >= MAZE_W || cy >= MAZE_H) return 1;
    return (maze[cy][cx] & CELL_WALL) ? 1 : 0;
}

void move_ghost(void)
{
    unsigned char best_dir, best_dist, i, nx, ny, dist;
    unsigned char moved;

    /* 25% random movement */
    if ((rng() & 3) == 0) {
        i = rng() & 3;
        nx = ghost_x + dx[i];
        ny = ghost_y + dy[i];
        if (!is_wall(nx, ny)) {
            ghost_x = nx;
            ghost_y = ny;
            return;
        }
    }

    /* Chase player */
    best_dir = 0;
    best_dist = 255;
    moved = 0;
    for (i = 0; i < 4; i++) {
        nx = ghost_x + dx[i];
        ny = ghost_y + dy[i];
        if (!is_wall(nx, ny)) {
            dist = abs((int)nx - (int)px) + abs((int)ny - (int)py);
            if (dist < best_dist) {
                best_dist = dist;
                best_dir = i;
                moved = 1;
            }
        }
    }

    if (moved) {
        ghost_x += dx[best_dir];
        ghost_y += dy[best_dir];
    }
}

/* ============================================================
 * DRAW TOP-DOWN MAP (for testing, will be replaced by 3D)
 * ============================================================ */

void draw_map(void)
{
    unsigned char x, y;
    char dir_char;

    /* Draw maze starting at screen row 2 */
    for (y = 0; y < MAZE_H; y++) {
        goto_xy(1, y + 2);
        for (x = 0; x < MAZE_W; x++) {
            if (x == px && y == py) {
                switch (pdir) {
                    case DIR_N: dir_char = '^'; break;
                    case DIR_E: dir_char = '>'; break;
                    case DIR_S: dir_char = 'v'; break;
                    case DIR_W: dir_char = '<'; break;
                    default:    dir_char = '@'; break;
                }
                putchar(dir_char);
            } else if (x == ghost_x && y == ghost_y) {
                putchar('G');
            } else if (maze[y][x] & CELL_WALL) {
                putchar('#');
            } else if (maze[y][x] & CELL_GEM) {
                putchar('*');
            } else if (maze[y][x] & CELL_EXIT) {
                putchar('E');
            } else {
                putchar('.');
            }
        }
    }
}

void draw_status(void)
{
    print_at(1, 19, "Energy:");
    print_num_at(9, 19, energy);
    /* clear trailing digits without newline */
    putchar(' '); putchar(' '); putchar(' ');

    print_at(1, 20, "Gems:");
    print_num_at(7, 20, (int)gems_collected);
    putchar('/');
    putchar('0' + gems_total);

    print_at(15, 20, "Dir:");
    goto_xy(20, 20);
    switch (pdir) {
        case DIR_N: putchar('N'); break;
        case DIR_E: putchar('E'); break;
        case DIR_S: putchar('S'); break;
        case DIR_W: putchar('W'); break;
    }

    print_at(1, 22, "WASD=move Q=quit       ");
}

/* ============================================================
 * TITLE SCREEN
 * ============================================================ */

void title_screen(void)
{
    int key;
    unsigned char seed;

    cls();
    puts("");
    puts("    ========================");
    puts("     SULTAN'S  MAZE  II    ");
    puts("    ========================");
    puts("");
    puts("  The Sultan's jewels are lost");
    puts("  deep within the ancient");
    puts("  maze. The ghost of his");
    puts("  bodyguard still haunts");
    puts("  these corridors...");
    puts("");
    puts("  Find all 6 gems and escape!");
    puts("  But beware - your energy is");
    puts("  limited, and the ghost");
    puts("  hunts you!");
    puts("");
    puts("  Controls:");
    puts("   W-Forward  S-Backward");
    puts("   A-Left     D-Right");
    puts("   M-Minimap  Q-Quit");
    puts("");
    puts("  Enter maze number (1-255)");
    puts("  or ENTER for random:");

    /* Read up to 3 digits */
    seed = 0;
    while (1) {
        key = fgetc_cons();
        if (key == 13 || key == 10) {
            /* Enter pressed */
            break;
        }
        if (key >= '0' && key <= '9') {
            putchar(key);
            seed = seed * 10 + (key - '0');
        }
    }

    if (seed == 0) {
        /* No number entered - use a changing default */
        rng_state = 7777;
        maze_seed = 0;
        puts("");
        puts("  Random maze selected!");
    } else {
        rng_state = (unsigned int)seed * 257 + 1;
        maze_seed = seed;
        puts("");
        printf("  Maze #%d selected!", seed);
    }
    puts("");
    puts("  Press any key to begin...");
    fgetc_cons();
}

/* ============================================================
 * GAME OVER / VICTORY
 * ============================================================ */

void victory_screen(void)
{
    cls();
    puts("");
    puts("  **************************");
    puts("  *     YOU ESCAPED!        *");
    puts("  **************************");
    puts("");
    puts("  You collected all the gems");
    puts("  and escaped the maze!");
    puts("");
    printf("  Remaining energy: %d\n", energy);
    if (maze_seed > 0)
        printf("  Maze #%d\n", maze_seed);
    puts("");
    puts("  Press any key...");
    fgetc_cons();
}

void gameover_screen(void)
{
    cls();
    puts("");
    puts("  **************************");
    puts("  *      GAME  OVER        *");
    puts("  **************************");
    puts("");
    puts("  Your energy ran out!");
    puts("  The maze claims another");
    puts("  victim...");
    puts("");
    printf("  Gems collected: %d / %d\n",
           gems_collected, gems_total);
    if (maze_seed > 0)
        printf("  Maze #%d\n", maze_seed);
    puts("");
    puts("  Press any key...");
    fgetc_cons();
}

/* ============================================================
 * MAIN GAME LOOP
 * ============================================================ */

void game_loop(void)
{
    int key;
    unsigned char nx, ny;
    unsigned char i;

    cls();

    while (game_running) {
        draw_map();
        draw_status();

        /* Get key */
        key = fgetc_cons();

        switch (key) {
            case 'w': case 'W':
                nx = px + dx[pdir];
                ny = py + dy[pdir];
                if (!is_wall(nx, ny)) {
                    px = nx; py = ny;
                    energy -= 2;
                    ghost_timer++;
                }
                break;

            case 's': case 'S':
                nx = px - dx[pdir];
                ny = py - dy[pdir];
                if (!is_wall(nx, ny)) {
                    px = nx; py = ny;
                    energy -= 2;
                    ghost_timer++;
                }
                break;

            case 'a': case 'A':
                pdir = (pdir + 3) & 3;
                energy -= 1;
                break;

            case 'd': case 'D':
                pdir = (pdir + 1) & 3;
                energy -= 1;
                break;

            case 'q': case 'Q':
                game_running = 0;
                break;
        }

        /* Check gem pickup */
        for (i = 0; i < NUM_GEMS; i++) {
            if (!gem_taken[i] && px == gem_x[i] && py == gem_y[i]) {
                gem_taken[i] = 1;
                gems_collected++;
                maze[gem_y[i]][gem_x[i]] &= ~CELL_GEM;
                print_at(1, 23, "** GEM FOUND! **");
            }
        }

        /* Check exit */
        if (px == exit_x && py == exit_y) {
            if (gems_collected >= gems_total) {
                game_running = 0;
                victory_screen();
                return;
            } else {
                print_at(1, 23, "Find all gems first!");
            }
        }

        /* Move ghost every 3 player moves */
        if (ghost_timer >= 3) {
            move_ghost();
            ghost_timer = 0;
        }

        /* Ghost collision */
        if (px == ghost_x && py == ghost_y) {
            energy -= 50;
            print_at(1, 23, "!! GHOST !! -50 energy");
            /* Push player back */
            nx = px - dx[pdir];
            ny = py - dy[pdir];
            if (!is_wall(nx, ny)) {
                px = nx; py = ny;
            }
        }

        /* Check energy */
        if (energy <= 0) {
            game_running = 0;
            gameover_screen();
            return;
        }
    }
}

/* ============================================================
 * MAIN
 * ============================================================ */

int main(void)
{
    title_screen();

    generate_maze();
    place_items();

    game_loop();

    cls();
    puts("");
    puts("  Thanks for playing!");
    puts("");

    return 0;
}
