# Games Programming for the Amstrad CPC
My childhood computer was an Amstrad CPC464 (with 64K RAM Pack and 3 inch Disc).  The computer came with 10 games, one of which was called Sultans Maze.  I playe that game alot, but it ran so slowly. Now with the help of modern tools running on Linux I want to write a game based on Sultans Maze to see if I can do any better.  Using the z88dk compiler, I will attempt to use C and ASM to create a fast 3d maze game.

# Programs
Once you have set up the Dev Env (instructions below) then you can try:  
[Hello World](https://github.com/mathsDOTearth/CPCprogramming/tree/main/Helloworld)  
[Sultans-ish Maze II](https://github.com/mathsDOTearth/CPCprogramming/tree/main/Maze)  

# Setting up Dev Env
Here is what I did to set up the z88dk dev tools on my Linux PC:
```
sudo apt update
sudo apt install -y \
    git build-essential \
    libboost-all-dev \
    libxml2-dev \
    bison flex \
    libgmp-dev \
    libmpfr-dev \
    texinfo \
    git \
    build-essential \
    cmake

cd ~
git clone --recursive git@github.com:z88dk/z88dk.git
cd z88dk
./build.sh

export Z88DK_HOME=$HOME/z88dk
export PATH=$PATH:$Z88DK_HOME/bin
export ZCCCFG=$Z88DK_HOME/lib/config

which zcc
zcc +cpc -v
```

# Setting up iDSK to write Disc Images
Here is what I did to set up iDSK so I could create .dsk images to load in to the emulator:
```
cd ~
git clone https://github.com/cpcsdk/iDSK.git
cd iDSK
mkdir build
cd build
cmake ..
make
cp iDSK $Z88DK_HOME/bin
```

# Writing, Compiling and Running a Test
This is what I did to test the install of these tools:
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
