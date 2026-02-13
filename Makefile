CC = gcc
CFLAGS = -std=c99 -Wall -Wno-missing-braces -I. -Isrc -O3 -fno-stack-protector -U_FORTIFY_SOURCE
LDFLAGS = -L. -lraylib -lopengl32 -lgdi32 -lwinmm -lole32 -lwininet

SRC = src/main.c src/utils.c src/logic.c src/render.c src/levels.c src/menus.c src/persistence.c src/atmosphere.c src/quantum.c src/audio.c src/qiskit.c
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
ifeq ($(OS),Windows_NT)
	-cmd //C "del /Q $(subst /,\,$(OBJ)) phase_shift.o $(EXEC)"
else
	rm -f $(OBJ) phase_shift.o $(EXEC) phase_shift
endif


# Linux Release Target
release-linux: LDFLAGS = -lraylib -lGL -lm -lpthread -ldl -lrt -lX11
release-linux: EXEC = phase_shift
release-linux: clean $(OBJ)
	$(CC) $(OBJ) -o $(EXEC) $(LDFLAGS)
	mkdir -p release-linux/assets
	cp $(EXEC) release-linux/
	cp /usr/local/lib/libraylib.so* release-linux/ || echo "Warning: libraylib.so not found in /usr/local/lib"
	cp icon.png release-linux/assets/
	cp -r assets/* release-linux/assets/
