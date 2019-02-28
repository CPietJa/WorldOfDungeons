CC = gcc
CFLAGS = -Wall -g
PROG = gen_map
OBJ = gen_map.o seed.o liste.o procedural.o

SDL_DIR=$(HOME)/SDL2
SDLLIB_DIR=$(SDL_DIR)/lib
SDLINC_DIR=$(SDL_DIR)/include

LIBS=-L$(SDLLIB_DIR) -lSDL2 -lSDL2_ttf -lSDL2_image
INCLUDES=-I$(SDLINC_DIR) 

OBJ_DIR=obj
SRC_DIR=src
BIN_DIR=bin
INC_DIR=include

all: $(PROG)

$(PROG): $(OBJ_DIR)/$(OBJ)
	$(CC) $(CFLAGS) $^ -o $(BIN_DIR)/$@  $(LIBS) $(INCLUDES) -I./$(INC_DIR)

$(OBJ_DIR)/$(OBJ) : $(OBJ_DIR)/%.o : $(SRC_DIR)/%.c $(INC_DIR)/%.h
	$(CC) -o $@ -c $< $(CFLAGS) $(LIBS) $(INCLUDES) -I./$(INC_DIR)
	
clean:
	rm $(OBJ_DIR)/*.o
	rm *.gch

mrproper: clean
	rm -i $(BIN_DIR)/$(PROG)
