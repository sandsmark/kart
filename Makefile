OBJECTS=main.o
CFLAGS+=-std=c99 -Wall -Wextra -pedantic -O2 -g
LDFLAGS+=-lSDL2 -lm
ALL = kart

kart: $(OBJECTS)
	gcc -o $@ $^ $(LDFLAGS)

clean:
	rm -f kart $(OBJECTS)
