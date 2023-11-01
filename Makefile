CC=gcc
VERBOSE=0
GENERAL-FLAGS=-Wall -Wextra -Werror -Wswitch-enum -std=c11 -I.
DEBUG-FLAGS=-ggdb -fsanitize=address
RELEASE-FLAGS=-O3
CFLAGS:=$(GENERAL-FLAGS) $(DEBUG-FLAGS) -D VERBOSE=$(VERBOSE)
LIBS=
DIST=build
TERM_YELLOW:=$(shell echo -e "\e[0;33m")
TERM_GREEN:=$(shell echo -e "\e[0;32m")
TERM_RESET:=$(shell echo -e "\e[0;0m")

# Setup variables for source code, output, etc
## Lib setup
LIB_DIST=$(DIST)/lib
LIB_SRC=lib
LIB_CODE:=$(addprefix $(LIB_SRC)/, base.c darr.c inst.c)
LIB_OBJECTS:=$(LIB_CODE:$(LIB_SRC)/%.c=$(LIB_DIST)/%.o)
LIB_DEPS:=$(LIB_OBJECTS:%.o=%.d)
LIB_CFLAGS=$(CFLAGS)

## VM setup
VM_DIST=$(DIST)/vm
VM_SRC=vm
VM_CODE:=$(addprefix $(VM_SRC)/, runtime.c)
VM_OBJECTS:=$(VM_CODE:$(VM_SRC)/%.c=$(VM_DIST)/%.o)
VM_DEPS:=$(VM_OBJECTS:%.o=%.d) $(VM_DIST)/main.d
VM_CFLAGS:=$(CFLAGS)
VM_OUT=$(DIST)/ovm.out

## ASSEMBLY setup
ASM_DIST=$(DIST)/asm
ASM_SRC=asm
ASM_CODE:=$(addprefix $(ASM_SRC)/, lexer.c parser.c)
ASM_OBJECTS:=$(ASM_CODE:$(ASM_SRC)/%.c=$(ASM_DIST)/%.o)
ASM_DEPS:=$(ASM_OBJECTS:%.o=%.d) $(ASM_DIST)/main.d
ASM_CFLAGS=$(CFLAGS)
ASM_OUT=$(DIST)/asm.out

## EXAMPLES setup
EXAMPLES_DIST=$(DIST)/examples
EXAMPLES_SRC=examples
EXAMPLES=$(EXAMPLES_DIST)/fib.out $(EXAMPLES_DIST)/factorial.out $(EXAMPLES_DIST)/instruction-test.out

# Things you want to build on `make`
all: $(DIST) lib vm asm examples

lib: $(LIB_DIST) $(LIB_OBJECTS)
vm: $(VM_DIST) $(VM_OUT)
asm: $(ASM_DIST) $(ASM_OUT)
examples: $(EXAMPLES_DIST) $(EXAMPLES)

# Recipes
## LIB Recipes
-include $(LIB_DEPS)

$(LIB_DIST)/%.o: $(LIB_SRC)/%.c
	@$(CC) $(LIB_CFLAGS) -MMD -c $< -o $@ $(LIBS)
	@echo -e "$(TERM_YELLOW)$@$(TERM_RESET): $<"

## VM Recipes
$(VM_OUT): $(LIB_OBJECTS) $(VM_OBJECTS) $(VM_DIST)/main.o
	@$(CC) $(VM_CFLAGS) $^ -o $@ $(LIBS)
	@echo -e "$(TERM_GREEN)$@$(TERM_RESET): $^"

-include $(VM_DEPS)

$(VM_DIST)/%.o: $(VM_SRC)/%.c
	@$(CC) $(VM_CFLAGS) -MMD -c $< -o $@ $(LIBS)
	@echo -e "$(TERM_YELLOW)$@$(TERM_RESET): $<"

## ASSEMBLY Recipes
$(ASM_OUT): $(LIB_OBJECTS) $(ASM_OBJECTS) $(ASM_DIST)/main.o
	@$(CC) $(ASM_CFLAGS) $^ -o $@ $(LIBS)
	@echo -e "$(TERM_GREEN)$@$(TERM_RESET): $^"

-include $(ASM_DEPS)

$(ASM_DIST)/%.o: $(ASM_SRC)/%.c
	@$(CC) $(ASM_CFLAGS) -MMD -c $< -o $@ $(LIBS)
	@echo -e "$(TERM_YELLOW)$@$(TERM_RESET): $<"

## EXAMPLES recipes
$(EXAMPLES_DIST)/%.out: $(EXAMPLES_SRC)/%.asm $(ASM_OUT)
	@$(ASM_OUT) $< $@
	@echo -e "$(TERM_GREEN)$@$(TERM_RESET): $<"

OUT=
ARGS=

.PHONY: run
run: $(DIST)/$(OUT)
	./$^ $(ARGS)

.PHONY: clean
clean:
	rm -rfv $(DIST)/*

SOURCE=
BYTECODE=
.PHONY: exec
exec: $(ASM_OUT) $(VM_OUT)
	@$(ASM_OUT) $(SOURCE) $(BYTECODE)
	@$(VM_OUT) $(BYTECODE)

# Directories
$(DIST):
	mkdir -p $(DIST)

$(LIB_DIST):
	mkdir -p $(LIB_DIST)

$(VM_DIST):
	mkdir -p $(VM_DIST)

$(ASM_DIST):
	mkdir -p $(ASM_DIST)

$(EXAMPLES_DIST):
	mkdir -p $(EXAMPLES_DIST)
