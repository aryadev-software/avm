CC=gcc

FVERBOSE=0
GENERAL-FLAGS=-Wall -Wextra -Werror -Wswitch-enum -std=c11 -D VERBOSE=$(FVERBOSE)
DEBUG-FLAGS=-ggdb -fsanitize=address
RELEASE-FLAGS=-O3
CFLAGS=$(GENERAL-FLAGS) $(DEBUG-FLAGS)

LIBS=
ARGS=
OUT=ovm.out

SRC=src
DIST=build

CODE=$(addprefix $(SRC)/, darr.c inst.c runtime.c)
OBJECTS=$(CODE:$(SRC)/%.c=$(DIST)/%.o)
DEPS=$(OBJECTS:%.o=%.d) $(DIST)/fib.d $(DIST)/main.d

.PHONY: all
all: $(OUT) $(DIST)

$(DIST):
	mkdir -p $(DIST)

$(OUT): $(DIST)/$(OUT)

$(DIST)/$(OUT): $(DIST) $(OBJECTS) $(DIST)/main.o
	$(CC) $(CFLAGS) $(OBJECTS) $(DIST)/main.o -o $@ $(LIBS)

-include $(DEPS)

$(DIST)/%.o: $(SRC)/%.c
	$(CC) $(CFLAGS) -MMD -c $< -o $@ $(LIBS)

examples: $(DIST)/fib.out

$(DIST)/fib.out: $(DIST) $(OBJECTS) $(SRC)/fib.c
	$(CC) $(CFLAGS) $(OBJECTS) $(SRC)/fib.c -o $@ $(LIBS)

.PHONY: run
run: $(DIST)/$(OUT)
	./$^ $(ARGS)

.PHONY:
clean:
	rm -rfv $(DIST)/*
