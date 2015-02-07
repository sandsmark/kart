CFILES=$(wildcard *.c) $(wildcard ../*.c) ../libs/cJSON/cJSON.c
OBJECTS=$(patsubst %.c, %.o, $(CFILES))
CFLAGS+=-std=c99 -Wall -Wextra -pedantic -O2 -g -I/usr/local/cross-tools/i686-w64-mingw32/include/ -DREVISION=\"$(shell git rev-parse --short HEAD)\"
LDFLAGS+=-static -Wl,-Bdynamic -lm -lws2_32 -lmingw32 -lSDL2main -lSDL2 -L/usr/local/cross-tools/i686-w64-mingw32/lib/ -L/usr/i686-w64-mingw32/lib/
EXECUTABLE=../kart.exe
CC=/usr/bin/i686-w64-mingw32-gcc

all: $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) -o $@ $^ $(LDFLAGS)

clean:
	rm -f $(EXECUTABLE) $(OBJECTS)
