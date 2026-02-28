# Sultan's Maze II - Makefile for z88dk
# Amstrad CPC 6128 target
# by @mathsDOTearth on github
# https://github.com/mathsDOTearth/CPCprogramming/
# 
# Usage:
#   make          - Build the game (produces maze.dsk)
#   make clean    - Remove build artifacts
#   make run      - Build and attempt to run in emulator
#
# Prerequisites:
#   - z88dk installed and in PATH
#   - For 'make run': Retro Virtual Machine installed

# Compiler settings
CC = zcc
TARGET = +cpc
CFLAGS =  -clib=ansi -lndos -O2 -create-app
LDFLAGS = 
NAME = maze
OUTPUT = maze.bin

# Source files
SRCS = maze.c

# Default target
all: $(OUTPUT).dsk

$(OUTPUT).dsk: $(SRCS)
	$(CC) $(TARGET) $(CFLAGS) $(LDFLAGS) -o $(OUTPUT) $(SRCS)
	iDSK $(NAME).dsk -n
	iDSK $(NAME).dsk -i ./$(NAME).cpc
	@echo ""
	@echo "Build complete! Output: $(OUTPUT).dsk"
	@echo ""
	@echo "To run in Retro Virtual Machine:"
	@echo "  1. Open Retro Virtual Machine"
	@echo "  2. Select Amstrad CPC 6128"
	@echo "  3. Insert $(OUTPUT).dsk in Drive A"
	@echo '  4. Type: RUN"MAZE'
	@echo ""

clean:
	rm -f $(OUTPUT) $(NAME).dsk $(NAME).bin $(NAME).cpc
	rm -f *.o *.err *.lis zcc_opt.def

run: $(OUTPUT).dsk
	@echo "Launching Retro Virtual Machine..."
	RetroVirtualMachine $(OUTPUT).dsk 2>/dev/null || \
		echo "Could not launch emulator. Please open $(OUTPUT).dsk manually."

.PHONY: all clean run
