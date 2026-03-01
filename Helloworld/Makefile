# Hello World - Makefile for z88dk
# Amstrad CPC 6128 target
# by @mathsDOTearth on github
# https://github.com/mathsDOTearth/CPCprogramming/
# 
# Usage:
#   make          - Build the programe (produces hello.dsk)
#   make clean    - Remove build artifacts
#   make run      - Build and attempt to run in emulator
#
# Prerequisites:
#   - z88dk installed and in PATH
#   - For 'make run': Retro Virtual Machine installed and in path

# Compiler settings
CC = zcc
TARGET = +cpc
CFLAGS =  -clib=ansi -lndos -O2 -create-app
LDFLAGS = 
NAME = hello
OUTPUT = $(NAME).bin

# Source files
SRCS = $(NAME).c

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
	@echo "  3. Insert $(NAME).dsk in Drive A"
	@echo '  4. Type: RUN"$(NAME).cpc'
	@echo ""

clean:
	rm -f $(OUTPUT) $(NAME).dsk $(NAME).bin $(NAME).cpc
	rm -f *.o *.err *.lis zcc_opt.def

run: $(OUTPUT).dsk
	@echo "Launching Retro Virtual Machine..."
	RetroVirtualMachine $(OUTPUT).dsk 2>/dev/null || \
		echo "Could not launch emulator. Please open $(OUTPUT).dsk manually."

.PHONY: all clean run
