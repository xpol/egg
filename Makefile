RM=rm -f

INCLUDES = -Isrc
LOADLIBES = -L./
CFLAGS = -g -Wall -static ${INCLUDES}

all: libegg.a

libegg.a: src/egg.o src/pvr.o
	$(AR) rcs $@ $^

#tests/egg: tests/test.o libegg.a
#	$(CC) -o $@ $^ -legg $(CFLAGS) $(LOADLIBES)


src/egg.o: src/egg.c src/egg.h src/internal.h

src/pvr.o: src/pvr.c src/egg.h src/internal.h


#tests/test.o: tests/test.c src/egg.h

#.PHONY: test

#test: all
#	@echo "Running tests..."
#	bash ./tests/setup.sh
#	./tests/egg target
#	@! [ -d target ]
#	@echo "Pass"

clean:
	$(RM) src/egg.o src/pvr.o

dist-clean: clean
	$(RM) libegg.a
