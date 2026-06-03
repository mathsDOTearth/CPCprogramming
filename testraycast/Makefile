Z88DK   = /home/rich/z88dk
ZCC     = $(Z88DK)/bin/zcc
IDSK    = $(Z88DK)/bin/iDSK
CFLAGS  = +cpc -clib=ansi -lndos -lm -O2 -create-app

TARGET  = raytest

.PHONY: all clean run

all: $(TARGET).dsk

$(TARGET).dsk: $(TARGET).c
	PATH=$(Z88DK)/bin:$$PATH ZCCCFG=$(Z88DK)/lib/config \
	    $(ZCC) $(CFLAGS) -o $(TARGET).bin $(TARGET).c
	$(IDSK) $(TARGET).dsk -n
	$(IDSK) $(TARGET).dsk -i ./$(TARGET).cpc
	@echo "Build complete: $(TARGET).dsk"

run: $(TARGET).dsk
	RetroVirtualMachine $(TARGET).dsk 2>/dev/null || \
	    echo "Open $(TARGET).dsk manually in Retro Virtual Machine."

clean:
	rm -f $(TARGET).dsk $(TARGET).bin $(TARGET).cpc $(TARGET).wav *.o zcc_opt.def
