# Makfile for improve UDP
#
WORKPATH = $(shell pwd)

export HEAD = $(WORKPATH)/head
export BIN  = $(WORKPATH)/bin
export COM  = $(WORKPATH)/com
export SRC  = $(WORKPATH)/src

export RM = `which rm` -fr
export CC = $(shell which gcc)

export OBJ = $(COM)/log.o        \
	         $(COM)/initSpace.o  \
	         $(COM)/mutex.o      \
	         $(COM)/rbtree.o     \
	         $(COM)/size.o       \

TARGET1 = $(BIN)/dealSize

all : $(TARGET1)

.PHONY : all

$(TARGET1) : 
	@cd $(COM) && make
	@$(CC) -o $(TARGET1) $(OBJ)  -g -lpthread
	@printf "\033[4m%70s\033[0m\n" ""
	@echo -e "    \033[34mMakefile.\033[0m"
	@printf "    \033[34m%-65s\033[0m[ \033[1;32mOK \033[0m]\n" "Complete."
	@echo -e "    \033[1;30;5m`date`\033[0m"

clean:
	-@$(RM) $(BIN)/dealSize
	@cd $(COM) && make clean
	@printf "    \033[34m%-65s\033[0m[ \033[1;32mOK \033[0m]\n" "Clean up."
	@echo -e "    \033[1;30;5m`date`\033[0m"    
