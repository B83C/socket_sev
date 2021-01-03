CC=gcc
CFLAGS=-I. -pthread -masm=intel -O2 
SRCS:=$(wildcard *.c)
OBJ_:=$(SRCS:%.c=obj/%.o)
GPERF_=./gperf

all: output headerhash.h

$(OBJ_) : obj/%.o : %.c
	$(CC) -c $< -o $@ $(CFLAGS)

output: $(OBJ_) headerhash.h 
	$(CC) -I. -o ./bin/output $(OBJ_)

run:
	@cd bin && ./output && cd ..	
       
headerhash.h: 
	rm headerhash.h
	cd $(GPERF_) && ./gperf -S 1 -R i header_fields --output-file=../headerhash.h -N hdr_ind -I -J -f && cd ..

clean:
	rm -f obj/*.o
	rm headerhash.h

.PHONY: all 
