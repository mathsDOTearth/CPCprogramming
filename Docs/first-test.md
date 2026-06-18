# Writing, Compiling and Running a Test
[Back To Doc Home](README.md)

This is what I did to test the install of z88dk:
```
cd ~
mkdir cpc-src
cd cpc-src
mkdir test
cd test
vim main.c
```
This is the test program main.c:
```
#include <stdio.h>

int main(void)
{
    puts("Hello, world!");
    return 0;   /* returns to BASIC */
}
```
And now to build and put on a .dsk file:
```
zcc +cpc -clib=ansi -lndos -O2 -create-app main.c -o test
iDSK test.dsk -n
iDSK test.dsk -i ./test.cpc
```
Now open your favorite emulator, I use [Retro Virtual Machine](https://www.retrovirtualmachine.org/)   
Insert the disc (load the test.dsk file)  
Form the Amstrad Basic prompt type: run"test.cpc  

[Back To Doc Home](README.md)
