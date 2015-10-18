# The makefile for .o files.

all : $(OBJ)

$(COM)/%.o : $(SRC)/%.c
	$(CC) -c $< -o $@ -I$(HEAD)

.PHONY : clean
clean:
	-@$(RM) log.o initSpace.o mutex.o rbtree.o size.o 
