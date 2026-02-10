
CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -g
LDFLAGS = -lraylib -lm -lpthread

# OS Specific adjustments
ifeq ($(OS),Windows_NT)
    LDFLAGS += -lopengl32 -lgdi32 -lwinmm
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
	$(CC) $(OBJ) -o $(TARGET) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET) $(TARGET).exe

.PHONY: all clean
