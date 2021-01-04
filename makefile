CC=gcc
CFLAGS=-I. -masm=intel -O2 
LFLAGS=-pthread
SRCS:=$(wildcard *.c)
OBJ_:=$(SRCS:%.c=obj/%.o)
GPERF_=./gperf

all: output headerhash.h

$(OBJ_) : obj/%.o : %.c
	$(CC) -c $< -o $@ $(CFLAGS)

output: $(OBJ_) headerhash.h 
	$(CC) -I. -o ./bin/output $(OBJ_) $(LFLAGS)


run:
	@cd bin && ./output && cd ..	
       
headerhash.h: 
	rm headerhash.h
	cd $(GPERF_) && ./gperf -S 1 -R i header_fields --output-file=../headerhash.h -N hdr_ind -I -J -f && cd ..

clean:
	rm -f obj/*.o
	rm headerhash.h

.PHONY: all 
