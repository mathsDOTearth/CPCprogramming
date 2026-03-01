/*
 * Hello World
 * Simple Toolchain test
 * Amstrad CPC 6128 / z88dk
 *
 * by @mathsDOTearth on github
 * https://github.com/mathsDOTearth/CPCprogramming/
 *
 * Compile:
 *   zcc +cpc -clib=ansi -lndos -O2 -create-app hello.c -o hello.bin
 *   iDSK hello.dsk -n
 *   iDSK hello.dsk -i ./hello.cpc
 * Run: run"hello.cpc
 */

#include <stdio.h>

/* ============================================================
 * SCREEN HELPERS
 * ============================================================ */

void cls(void)
{
    putchar(27); putchar('['); putchar('2'); putchar('J');
    putchar(27); putchar('['); putchar('H');
}


/* ============================================================
 * MAIN 
 * ============================================================ */

int main(void)
{
    cls();
    puts("");
    puts("Hello World");
    puts("");
    puts("  Press any key...");
    fgetc_cons();
    return 0;
}
