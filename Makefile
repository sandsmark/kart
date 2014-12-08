CFILES=$(wildcard *.c)
OBJECTS=$(patsubst %.c, %.o, $(CFILES))
CFLAGS+=-std=c99 -Wall -Wextra -pedantic -O2 -g -DREVISION=\"$(shell git rev-parse --short HEAD)\"
LDFLAGS+=-lSDL2 -lm
EXECUTABLE=kart

all: $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) -o $@ $^ $(LDFLAGS)

wintendo:
	make -C windowssucks
	rm -f kart.zip
	zip kart.zip *.bmp *.map SDL2.dll kart.exe


clean:
	rm -f $(EXECUTABLE) $(OBJECTS)
