# Mosslight - Game Boy Color ROM
# GBDK-2020 build system

GBDK_DIR = /tmp/gbdk-dl/gbdk
LCC = $(GBDK_DIR)/bin/lcc
SDCC_BINDIR = --sdccbindir=$(GBDK_DIR)/bin/
GBDK_LIBDIR = --gbdklibdir=$(GBDK_DIR)/lib/
GBDK_INCDIR = --gbdkincludedir=$(GBDK_DIR)/include/

# ROM settings - CGB only, MBC5+RAM+Battery
# -Wm-yC : Game Boy Color only
# -Wm-yt0x1B: MBC5 + RAM + Battery (cart type 0x1B = 27)
# -Wm-yo4: 4 ROM banks = 64KB
# -Wm-ya4: 4 RAM banks = 32KB SRAM
ROM_FLAGS = -Wm-yC -Wm-yt0x1B -Wm-yo4 -Wm-ya4

# Source files (only files that exist)
SRC = src/main.c \
      src/engine.c \
      src/input.c \
      src/maps.c \
      src/player.c \
      src/garden.c \
      src/wildlife.c \
      src/ui.c \
      src/save.c \
      src/audio.c \
      src/tiles.c \
      src/sprites.c \
      src/text.c

# Banked sources
BANKED_SRC = src/banks/bank2.c

# Output
ROM = build/mosslight.gb

.PHONY: all clean run web

all: $(ROM)

$(ROM): $(SRC) $(BANKED_SRC)
	mkdir -p build
	$(LCC) $(SDCC_BINDIR) $(GBDK_LIBDIR) $(GBDK_INCDIR) $(ROM_FLAGS) \
		-Isrc/include \
		-o $(ROM) $(SRC) $(BANKED_SRC)
	@python3 -c "f=open('$(ROM)','r+b'); f.seek(0x134); f.write(b'MOSSLIGHT'+b'\x00'*6); f.seek(0x143); f.write(b'\xc0'); f.seek(0x134); d=f.read(0x19); c=sum((-b-1) for b in d)&0xFF; f.seek(0x14D); f.write(bytes([c])); f.close(); print('Patched ROM title and header')"

clean:
	rm -rf build/*.gb build/*.ihx build/*.lk build/*.map build/*.sym build/*.noi

# Web emulator wrapper
web: $(ROM)
	@echo "Building web wrapper..."
	mkdir -p build/web
	cp $(ROM) build/web/mosslight.gb
	cp src/web/index.html build/web/index.html

run: $(ROM)
	@echo "ROM built: $(ROM)"
