CC=gcc
CC_OPTION= -lraylib -lm
EXEC_NAME= rayfun
CFLAGS= -Wall
SRC_DIRS = $(shell find . -type f \( -name '*.c' -o -name '*.h' \) -exec dirname {} \; | sort | uniq)

# Trouver tous les fichiers .c dans les répertoires trouvés
SRC = $(shell find $(SRC_DIRS) -name '*.c')

OBJ= $(SRC:.c=.o)


$(EXEC_NAME) : $(OBJ)
	$(CC) -o $@ $^ $(CC_OPTION) 

%.o : %.c
	$(CC) $(CFLAGS) -o $@  -c $< $(CC_OPTION)

clean:
	rm *.o $(EXEC_NAME) $(SRC_DIRS)/*.o
