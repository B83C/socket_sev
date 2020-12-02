CC=gcc
CFLAGS=-I. -pthread
SRCS:=$(wildcard *.c)
OBJ_:=$(SRCS:%.c=obj/%.o)

all: output

$(OBJ_) : obj/%.o : %.c
	$(CC) -c $< -o $@ $(CFLAGS)

output: $(OBJ_)
	$(CC) -I. -o ./bin/output $(OBJ_)

run:
	@cd bin && ./output && cd ..	
       
clean:
	rm -f obj/*.o

.PHONY: all 
