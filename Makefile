GXX = gcc
FLAGS = -std=gnu99 -Werror -O2
INCLUDES = -pthread -lncurses

tetris: tetris.c
	$(GXX) $(FLAGS) tetris.c -o tetris $(INCLUDES)
