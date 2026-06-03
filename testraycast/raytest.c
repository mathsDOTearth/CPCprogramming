/* raytest.c
 * DDA raycaster with movement, Amstrad CPC Mode 1 (320x200, 4 colours).
 * Half resolution: 40 rays each 2 bytes wide, 2 scanlines tall per pixel.
 * All arithmetic is fixed-point integer (256 = 1 cell).
 *
 * Controls: Q=forward  A=backward  O=turn left  P=turn right
 *
 * Build:
 *   make
 */

/* Place the stack at 0x7FFF in user RAM.  The CPC firmware leaves SP in its
 * own OS workspace (0xBExx).  render() uses ~90 bytes of stack which pushes
 * SP into the BIOS jump-vector table (0xBBxx-0xBDxx), overwriting the JP
 * instructions there.  Any BIOS call after render() then jumps to garbage.
 * Values 0x0001-0x7FFF are interpreted as a direct ld sp,nn by z88dk;
 * values >= 0x8000 are sign-negative and trigger an indirect load instead. */
#pragma output REGISTER_SP = 0x7FFF

#include <arch/cpc/cpc.h>

/* -------------------------------------------------------------------------
 * Screen constants - Mode 1: 80 bytes/line, non-linear layout.
 * Line y address = 0xC000 + (y%8)*0x800 + (y/8)*80
 * ------------------------------------------------------------------------- */
#define SCREEN_BASE    0xC000u
#define BYTES_PER_ROW  80u
#define SCREEN_ROWS    200
#define HALF_ROWS      100
#define NUM_COLS       80   /* ray columns = byte columns */

/* Mode 1 solid-colour bytes (all 4 pixels same pen):
 * pen 0 = 0x00, pen 1 = 0x0F, pen 2 = 0xF0, pen 3 = 0xFF  */
#define CLR_SKY   0x0F
#define CLR_WALL  0xFF
#define CLR_FLOOR 0xF0

/* -------------------------------------------------------------------------
 * Map
 * ------------------------------------------------------------------------- */
#define MAP_W  8
#define MAP_H  8

static const unsigned char worldmap[MAP_H][MAP_W] = {
    {1,1,1,1,1,1,1,1},
    {1,0,0,0,0,0,0,1},
    {1,0,1,1,0,0,0,1},
    {1,0,1,0,0,0,0,1},
    {1,0,0,0,0,1,0,1},
    {1,0,0,0,0,1,0,1},
    {1,0,0,0,0,0,0,1},
    {1,1,1,1,1,1,1,1}
};

/* -------------------------------------------------------------------------
 * Pre-computed line start addresses to avoid repeated layout arithmetic.
 * 200 pointers x 2 bytes = 400 bytes.
 * ------------------------------------------------------------------------- */
static unsigned char *line_addr[SCREEN_ROWS];

static void build_line_table(void)
{
    int y;
    for (y = 0; y < SCREEN_ROWS; y++) {
        unsigned int off = (unsigned int)(y & 7) * 0x800u
                         + (unsigned int)(y >> 3) * BYTES_PER_ROW;
        line_addr[y] = (unsigned char *)(SCREEN_BASE + off);
    }
}

/* -------------------------------------------------------------------------
 * Draw one byte column x (0..79): sky above wall, wall strip, floor below.
 * wall_top = first wall row, wall_bot = one past last wall row.
 * ------------------------------------------------------------------------- */
/* Each call covers a 2-byte-wide, 2-scanline-tall block.
 * This halves both the ray count and the per-column write iterations. */
static void draw_column(int x, int wall_top, int wall_bot)
{
    int y;
    for (y = 0; y < wall_top; y += 2) {
        line_addr[y  ][x] = CLR_SKY;  line_addr[y  ][x+1] = CLR_SKY;
        line_addr[y+1][x] = CLR_SKY;  line_addr[y+1][x+1] = CLR_SKY;
    }
    for (; y < wall_bot; y += 2) {
        line_addr[y  ][x] = CLR_WALL; line_addr[y  ][x+1] = CLR_WALL;
        line_addr[y+1][x] = CLR_WALL; line_addr[y+1][x+1] = CLR_WALL;
    }
    for (; y < SCREEN_ROWS; y += 2) {
        line_addr[y  ][x] = CLR_FLOOR; line_addr[y  ][x+1] = CLR_FLOOR;
        line_addr[y+1][x] = CLR_FLOOR; line_addr[y+1][x+1] = CLR_FLOOR;
    }
}

/* Block until a keypress; returns ASCII code. */
extern int fgetc_cons(void);


/* Direction deltas: N=0, E=1, S=2, W=3.
 * Use int, not signed char — sccz80 may not sign-extend narrow types correctly. */
static const int ddx[4] = { 0,  1,  0, -1};
static const int ddy[4] = {-1,  0,  1,  0};

/* -------------------------------------------------------------------------
 * DDA raycaster — fixed-point integer, all 4 facing directions.
 *
 * Positions: 8.8 fixed-point (256 = 1 cell). Player always at cell centre.
 * Camera plane magnitude: 169 (= 0.66 * 256), ~66 deg horizontal FOV.
 *
 * Ray direction per column for each facing:
 *   East  (1): rdx= 256,  rdy= rv       rv = planeY * camX
 *   West  (3): rdx=-256,  rdy=-rv
 *   North (0): rdx=-rv,   rdy=-256
 *   South (2): rdx= rv,   rdy= 256
 *
 * Bresenham cross-multiply comparison avoids all division in the DDA loop.
 * ------------------------------------------------------------------------- */
void render(int gx, int gy, int dir)
{
    int px     = gx * 256 + 128;
    int py     = gy * 256 + 128;
    int planey = 169;

    int col;
    for (col = 0; col < NUM_COLS; col += 2) {
        int cam = (2 * col - NUM_COLS) * 256 / NUM_COLS;
        int rv  = (int)((long)planey * cam / 256);

        int rdx, rdy;
        switch (dir) {
            case 0: rdx = -rv;  rdy = -256; break;
            case 2: rdx =  rv;  rdy =  256; break;
            case 3: rdx = -256; rdy = -rv;  break;
            default: /* East */
                    rdx =  256; rdy =  rv;  break;
        }

        int mx = px >> 8;
        int my = py >> 8;
        int fx = px & 0xFF;
        int fy = py & 0xFF;

        int stepx   = (rdx >= 0) ? 1 : -1;
        int stepy   = (rdy >= 0) ? 1 : -1;
        int abs_rdx = (rdx < 0) ? -rdx : rdx;
        int abs_rdy = (rdy < 0) ? -rdy : rdy;

        /* Bresenham accumulators: sx and sy share the same scale so they
         * compare directly — smaller means that boundary is nearer. */
        long sx, dx_step, sy, dy_step;

        if (abs_rdx == 0) {
            sx = 0x7FFFFFFFL; dx_step = 0;
        } else {
            int d0 = (rdx > 0) ? 256 - fx : fx;
            sx = (long)d0 * abs_rdy;
            dx_step = (long)256 * abs_rdy;
        }
        if (abs_rdy == 0) {
            sy = 0x7FFFFFFFL; dy_step = 0;
        } else {
            int d0 = (rdy > 0) ? 256 - fy : fy;
            sy = (long)d0 * abs_rdx;
            dy_step = (long)256 * abs_rdx;
        }

        int side = 0;
        while (!worldmap[my][mx]) {
            if (sx < sy) {
                sx += dx_step;
                mx += stepx;
                side = 0;
            } else {
                sy += dy_step;
                my += stepy;
                side = 1;
            }
        }

        /* Perpendicular distance and column height.
         * h = SCREEN_ROWS * abs_r(perp_axis) / d_in_256ths */
        long h_long;
        if (side == 0) {
            int d = (stepx > 0) ? mx * 256 - px : px - (mx + 1) * 256;
            h_long = (d > 0 && abs_rdx > 0)
                   ? (long)SCREEN_ROWS * abs_rdx / d : SCREEN_ROWS;
        } else {
            int d = (stepy > 0) ? my * 256 - py : py - (my + 1) * 256;
            h_long = (d > 0 && abs_rdy > 0)
                   ? (long)SCREEN_ROWS * abs_rdy / d : SCREEN_ROWS;
        }
        int h = (h_long > SCREEN_ROWS) ? SCREEN_ROWS : (int)h_long;

        int top = HALF_ROWS - h / 2;
        int bot = HALF_ROWS + h / 2;
        if (top < 0)           top = 0;
        if (bot > SCREEN_ROWS) bot = SCREEN_ROWS;

        draw_column(col, top, bot);
    }
}

/* -------------------------------------------------------------------------
 * Entry point
 * ------------------------------------------------------------------------- */
int main(void)
{
    int gx = 1, gy = 4;  /* starting grid cell */
    int dir = 1;          /* 0=North 1=East 2=South 3=West */
    int nx, ny, c, moved;

    cpc_SetModo(1);
    cpc_SetInk(0,  0);
    cpc_SetInk(1,  1);
    cpc_SetInk(2, 24);
    cpc_SetInk(3, 26);
    cpc_SetBorder(0);

    build_line_table();
    render(gx, gy, dir);

    for (;;) {
        moved = 0;
        c = fgetc_cons();

        switch (c) {
            case 'q': case 'Q':        /* move forward */
                nx = gx + ddx[dir];
                ny = gy + ddy[dir];
                if (!worldmap[ny][nx]) { gx = nx; gy = ny; }
                moved = 1; break;
            case 'a': case 'A':        /* move backward */
                nx = gx - ddx[dir];
                ny = gy - ddy[dir];
                if (!worldmap[ny][nx]) { gx = nx; gy = ny; }
                moved = 1; break;
            case 'o': case 'O':        /* turn left */
                dir = (dir + 3) & 3;
                moved = 1; break;
            case 'p': case 'P':        /* turn right */
                dir = (dir + 1) & 3;
                moved = 1; break;
        }

        if (moved) render(gx, gy, dir);
    }
    return 0;
}
