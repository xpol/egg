RM=rm -f

INCLUDES = -Isrc
LOADLIBES = -L./
CFLAGS = -g -Wall -static ${INCLUDES}

all: libegg.a tests/egg

libegg.a: src/egg.o src/pvr.o
	$(AR) rcs $@ $^

tests/egg: tests/test.o libegg.a
	$(CC) -o $@ $^ -legg $(CFLAGS) $(LOADLIBES)


src/egg.o: src/egg.c src/egg.h src/internal.h

src/pvr.o: src/pvr.c src/egg.h src/internal.h


tests/test.o: tests/test.c src/egg.h

.PHONY: test


test: all
	@echo "Running tests..."
	@echo [1/5]:./tests/1x1.pvr...
	gdb -q -n -ex bt -batch ./tests/egg ./tests/1x1.pvr RGB565:1x1 [0,0,0xff7733]
	@echo [2/5]:./tests/64x1.pvr...
	gdb -q -n -ex bt -batch ./tests/egg ./tests/64x1.pvr RGB565:64x1 [0,0,0x000000] [63,0,0xffffff]
	@echo [3/5]:./tests/1x64.pvr...
	gdb -q -n -ex bt -batch ./tests/egg ./tests/1x64.pvr RGB565:1x64 [0,0,0xff7733] [0,63,0x000000]
	@echo [4/5]:./tests/colors.pvr...
	gdb -q -n -ex bt -batch ./tests/egg ./tests/colors.pvr RGB565:32x32 [0,0,0xff0000] [8,0,0x00ff00] [16,0,0x0000ff] [24,0,0xffffff] [0,16,0xff00ff] [8,16,0xffff00] [16,16,0x00ffff] [24,16,0x000000]
	@echo [5/5]:./tests/ball.pvr...
	gdb -q -n -ex bt -batch ./tests/egg ./tests/ball.pvr RGBA4444:64x64 [0,0,0x00000000] [63,0,0x00000000] [63,63,0x00000000] [0,63,0x00000000] [32,32,0xff9911dd] [6,24,0x88221133]
	@echo "Pass"

clean:
	$(RM) src/egg.o src/pvr.o

dist-clean: clean
	$(RM) libegg.a
