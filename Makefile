CFILES=$(wildcard *.c) libs/cJSON/cJSON.c
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
	zip -r kart.zip assets/*.bmp *.map SDL2.dll kart.exe README.md examples/

%.o: %.c
	$(CC) -MMD -MP $(CFLAGS) -o $@ -c $<

DEPS=$(OBJECTS:.o=.d)
-include $(DEPS)

clean:
	rm -f $(EXECUTABLE) $(OBJECTS) $(DEPS)
