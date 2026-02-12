CC = gcc
CFLAGS = -std=c99 -Wall -Wno-missing-braces -I. -Isrc -O3
LDFLAGS = -L. -lraylib -lopengl32 -lgdi32 -lwinmm

SRC = src/main.c src/utils.c src/logic.c src/render.c src/levels.c src/menus.c src/persistence.c src/atmosphere.c src/quantum.c
OBJ = $(SRC:.c=.o)
EXEC = eepers_v2.exe

all: $(EXEC)

$(EXEC): $(OBJ)
	$(CC) $(OBJ) -o $@ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(EXEC)
