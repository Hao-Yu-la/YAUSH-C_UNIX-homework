CFILES = $(wildcard *.c)
TARGETS = $(CFILES:.c=)
CC ?= clang 
CFLAGS = -Wall -DDEBUG -g3 

all: $(TARGETS)

$(TARGETS): $(CFILES) $(wildcard *.h)
	$(CC) $(CFLAGS) -o $@ $(@:=.c) -lreadline


clean: $(TARGETS)
	rm $(TARGETS)