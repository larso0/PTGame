CFLAGS=-Iinclude
LDFLAGS=-lGL -lGLU -lm -ldl -lSDL2main -lSDL2
SRC=$(wildcard src/*.c) $(wildcard src/*/*.c)
OBJ=$(patsubst src/%.c, build/obj/%.o, $(SRC))

all: build/game
build/game: $(OBJ)
	gcc $(OBJ) $(LDFLAGS) -o $@
build/obj/%.o: src/%.c
	mkdir -p $(dir $@) && gcc $(CFLAGS) -c $< -o $@
clean:
	rm -rf build
