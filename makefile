CC=gcc
CFLAGS=-I. -masm=intel -D_GNU_SOURCE -I./cmph
LFLAGS=-pthread -Os -fdata-sections -ffunction-sections -Wl,--gc-sections -lm -L. -ldeflate
SRCS:=$(wildcard *.c) $(wildcard cmph/*.c)
OBJ_:=$(SRCS:%.c=obj/%.o)
GPERF_=./gperf

all: output headerhash.h

$(OBJ_) : obj/%.o :%.c headerhash.h 
	@mkdir -p obj/cmph
	$(CC) -c $< -o $@ $(CFLAGS)

output: $(OBJ_) 
	$(CC) -I. -o ./bin/output $^ $(LFLAGS)

run:
	@cd bin && ./output && cd ..
       
headerhash.h: 
	cd $(GPERF_) && ./gperf -S 1 -R i header_fields --output-file=../headerhash.h -N hdr_ind -I -J -f && cd ..

clean:
	rm -rf obj/*
	rm headerhash.h

.PHONY: all 
