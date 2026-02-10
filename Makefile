CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -g
# Nota: Movemos las librerías comunes aquí, pero el orden final importa en $(CC)
LDFLAGS = -lraylib -lm -lpthread

# OS Specific adjustments
ifeq ($(OS),Windows_NT)
    # CORRECCIÓN AQUÍ: Añadido -lglfw3 y -static
    # El orden es importante: raylib primero, luego sus dependencias (glfw3, opengl, etc.)
	# usar -mwindows para compilar sin terminal
    LDFLAGS += -lglfw3 -lopengl32 -lgdi32 -lwinmm
else
    UNAME_S := $(shell uname -s)
    ifeq ($(UNAME_S),Darwin)
        LDFLAGS += -framework CoreVideo -framework IOKit -framework Cocoa -framework GLUT -framework OpenGL
    endif
endif

SRC = eepers.c
OBJ = $(SRC:.c=.o)
TARGET = eepers_c

all: $(TARGET)

$(TARGET): $(OBJ)
    # IMPORTANTE: $(LDFLAGS) debe ir AL FINAL para que el enlazador resuelva los símbolos en orden
	$(CC) $(OBJ) -o $(TARGET) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET) $(TARGET).exe

.PHONY: all clean
