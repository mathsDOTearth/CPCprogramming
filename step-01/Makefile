CC=zcc
CFLAGS=+cpc -clib=ansi -lndos -O0 -create-app -zorg=0x1000 -o
LDFLAGS=
TARGET=test
SRC=main.c 

build: $(SRC)
  $(CC) $(CFLAGS) $(TARGET) $(SRC)

run: build
  ./$(TARGET)

mkdsk:
  iDSK $(TARGET).dsk -n
  iDSK $(TARGET).dsk -i $(TARGET).cpc

clean:
	rm -f $(TARGET) $(TARGET).cpc $(TARGET).dsk 
