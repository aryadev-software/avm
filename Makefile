CC=gcc
GENERAL-FLAGS=-Wall -Wextra -Werror -Wswitch-enum -std=c11
DEBUG-FLAGS=-ggdb -fsanitize=address
RELEASE-FLAGS=-O3
CFLAGS:=$(GENERAL-FLAGS) $(DEBUG-FLAGS)
LIBS=
DIST=build
TERM_YELLOW:=$(shell echo -e "\e[0;33m")
TERM_GREEN:=$(shell echo -e "\e[0;32m")
TERM_RESET:=$(shell echo -e "\e[0;0m")

# Setup variables for source code, output, etc
## Lib setup
LIB_DIST=$(DIST)/lib
LIB_SRC=lib
LIB_CODE:=$(addprefix $(LIB_SRC)/, darr.c)
LIB_OBJECTS:=$(LIB_CODE:$(LIB_SRC)/%.c=$(LIB_DIST)/%.o)
LIB_DEPS:=$(LIB_OBJECTS:%.o=%.d)
LIB_CFLAGS=$(CFLAGS)

## VM setup
VM_DIST=$(DIST)/vm
VM_SRC=vm
VM_CODE:=$(addprefix $(VM_SRC)/, inst.c runtime.c)
VM_OBJECTS:=$(VM_CODE:$(VM_SRC)/%.c=$(VM_DIST)/%.o)
VM_DEPS:=$(VM_OBJECTS:%.o=%.d) $(VM_DIST)/main.d
VM_VERBOSE=0
VM_CFLAGS:=$(CFLAGS) -D VERBOSE=$(VM_VERBOSE)
VM_OUT=$(DIST)/ovm.out


## EXAMPLES setup
EXAMPLES_DIST=$(DIST)/examples
EXAMPLES_SRC=examples
EXAMPLES_CFLAGS=$(CFLAGS)
EXAMPLES=$(DIST)/fib.out

# Things you want to build on `make`
all: $(DIST) lib vm examples

lib: $(LIB_DIST) $(LIB_OBJECTS)
vm: $(VM_DIST) $(VM_OUT)
examples: $(EXAMPLES_DIST) $(EXAMPLES)

# Recipes
## LIB Recipes
-include $(LIB_DEPS)

$(LIB_DIST)/%.o: $(LIB_SRC)/%.c
	@$(CC) $(LIB_CFLAGS) -MMD -c $< -o $@ $(LIBS)
	@echo -e "$(TERM_GREEN)$@$(TERM_RESET)"

## VM Recipes
$(VM_OUT): $(LIB_OBJECTS) $(VM_OBJECTS) $(VM_DIST)/main.o
	@$(CC) $(VM_CFLAGS) $^ -o $@ $(LIBS)
	@echo -e "$(TERM_GREEN)$@$(TERM_RESET)"

-include $(VM_DEPS)

$(VM_DIST)/%.o: $(VM_SRC)/%.c
	@$(CC) $(VM_CFLAGS) -MMD -c $< -o $@ $(LIBS)
	@echo -e "$(TERM_GREEN)$@$(TERM_RESET)"

## EXAMPLES recipes
$(DIST)/fib.out: $(LIB_OBJECTS) $(VM_OBJECTS) $(ASM_OBJECTS) $(EXAMPLES_DIST)/fib.o
	@$(CC) $(EXAMPLES_CFLAGS) $^ -o $@ $(LIBS)
	@echo -e "$(TERM_GREEN)$@$(TERM_RESET)"

$(EXAMPLES_DIST)/%.o: $(EXAMPLES_SRC)/%.c
	@$(CC) $(EXAMPLES_CFLAGS) -MMD -c $< -o $@ $(LIBS)
	@echo -e "$(TERM_GREEN)$@$(TERM_RESET)"

OUT=
ARGS=
.PHONY: run
run: $(DIST)/$(OUT)
	./$^ $(ARGS)

.PHONY:
clean:
	rm -rfv $(DIST)/*

# Directories
$(DIST):
	mkdir -p $(DIST)

$(LIB_DIST):
	mkdir -p $(LIB_DIST)

$(VM_DIST):
	mkdir -p $(VM_DIST)

$(EXAMPLES_DIST):
	mkdir -p $(EXAMPLES_DIST)
