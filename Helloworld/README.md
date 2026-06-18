# Hello World — Amstrad CPC 6128

This hello world code acts as a minimal toolchain smoke-test.  The program clears the screen and prints "Hello World" on an Amstrad CPC 6128 using z88dk.

## Source: hello.c

### cls()

```c
void cls(void)
```

the cls() function clears the screen and moves the cursor using ANSI escape sequences via `putchar`. z88dk uses firmware text calls, so no direct BIOS calls are needed.

- `ESC [ 2 J` — erase entire display
- `ESC [ H`  — move cursor to row 1, column 1

### main()

```c
int main(void)
```

The main() function calls cls() to clear the screen, then prints a greeting to the screen, then blocks on `fgetc_cons()` until the user presses a key. `fgetc_cons()` calls the firmware routine KM WAIT CHAR (0xBB06) via the z88dk wrapper which does not echo the keypress.

## Build

Requires z88dk and iDSK in the `PATH`.

```sh
zcc +cpc -clib=ansi -lndos -O2 -create-app hello.c -o hello.bin
iDSK hello.dsk -n
iDSK hello.dsk -i ./hello.cpc
```
There is a `Makefile` in the `Helloworld` directory which can be used to compile the code and creat the `hello.dsk` disk image file.

This produces `hello.dsk`. Load this image in to an emulator or transfer to real hardware, then at the BASIC prompt:

```
run"hello.cpc
```

## Notes

- `-lndos` excludes the AMSDOS file-system library, shrinking the binary.
- `-clib=ansi` selects the smaller ANSI C library (no floating point, no file I/O beyond console).
- The build does not need a Makefile; the single `zcc` invocation handles compilation, linking, and `.cpc` packaging in one step.
