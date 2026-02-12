CC = gcc
CFLAGS = -std=c99 -Wall -Wno-missing-braces -I. -Isrc -O3
LDFLAGS = -L. -lraylib -lopengl32 -lgdi32 -lwinmm

SRC = src/main.c src/utils.c src/logic.c src/render.c src/levels.c src/menus.c src/persistence.c src/atmosphere.c src/quantum.c
OBJ = $(SRC:.c=.o)
EXEC = phase_shift.exe

# Default target (debug mode)
all: CFLAGS += -DDEBUG_MODE
all: $(EXEC)

# Release target (no debug, copy files, no console)
release: LDFLAGS += -mwindows
release: clean
	windres phase_shift.rc -o phase_shift.o
	$(CC) $(CFLAGS) $(SRC) phase_shift.o -o $(EXEC) $(LDFLAGS)
	cmd //C "if not exist release mkdir release"
	cmd //C "if not exist release\assets mkdir release\assets"
	cmd //C "copy /Y $(EXEC) release"
	cmd //C "copy /Y *.dll release"
	cmd //C "copy /Y icon.png release\assets"
	cmd //C "xcopy /E /I /Y assets release\assets"

$(EXEC): $(OBJ)
	windres phase_shift.rc -o phase_shift.o
	$(CC) $(OBJ) phase_shift.o -o $@ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	-cmd //C "del /Q $(subst /,\,$(OBJ)) phase_shift.o $(EXEC)"

