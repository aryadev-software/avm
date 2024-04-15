CC=gcc
CPP=g++

VERBOSE=0
GENERAL-FLAGS=-Wall -Wextra -Werror -Wswitch-enum -I$(shell pwd)
DEBUG-FLAGS=-ggdb -fsanitize=address
RELEASE-FLAGS=-O3
CFLAGS:=$(GENERAL-FLAGS) -std=c11 $(DEBUG-FLAGS) -D VERBOSE=$(VERBOSE)
CPPFLAGS:=$(GENERAL-FLAGS) -std=c++17 $(DEBUG-FLAGS) -D VERBOSE=$(VERBOSE)

LIBS=-lm
DIST=build
TERM_YELLOW:=$(shell echo -e "\e[0;33m")
TERM_GREEN:=$(shell echo -e "\e[0;32m")
TERM_RESET:=$(shell echo -e "\e[0;0m")

# Setup variables for source code, output, etc
## Lib setup
LIB_DIST=$(DIST)/lib
LIB_SRC=lib
LIB_CODE:=$(addprefix $(LIB_SRC)/, base.c darr.c heap.c inst.c)
LIB_OBJECTS:=$(LIB_CODE:$(LIB_SRC)/%.c=$(LIB_DIST)/%.o)
LIB_CFLAGS=$(CFLAGS)

## VM setup
VM_DIST=$(DIST)/vm
VM_SRC=vm
VM_CODE:=$(addprefix $(VM_SRC)/, runtime.c)
VM_OBJECTS:=$(VM_CODE:$(VM_SRC)/%.c=$(VM_DIST)/%.o)
VM_CFLAGS:=$(CFLAGS)
VM_OUT=$(DIST)/ovm.out

## ASSEMBLY setup
ASM_DIST=$(DIST)/asm
ASM_SRC=asm
ASM_CODE:=$(addprefix $(ASM_SRC)/, base.cpp lexer.cpp preprocesser.cpp)
ASM_OBJECTS:=$(ASM_CODE:$(ASM_SRC)/%.cpp=$(ASM_DIST)/%.o)
ASM_CFLAGS=$(CPPFLAGS)
ASM_OUT=$(DIST)/asm.out

## EXAMPLES setup
EXAMPLES_DIST=$(DIST)/examples
EXAMPLES_SRC=examples
EXAMPLES=$(EXAMPLES_DIST)/instruction-test.out $(EXAMPLES_DIST)/fib.out $(EXAMPLES_DIST)/factorial.out $(EXAMPLES_DIST)/memory-print.out

## Dependencies
DEPDIR:=$(DIST)/dependencies
DEPFLAGS = -MT $@ -MMD -MP -MF
DEPS:=$($(LIB_SRC):%.c=$(DEPDIR):%.o) $($(ASM_SRC):%.c=$(DEPDIR):%.o) $($(VM_SRC):%.c=$(DEPDIR):%.o)

# Things you want to build on `make`
all: $(DIST) lib vm asm examples

lib: $(LIB_OBJECTS)
vm: $(VM_OUT)
asm: $(ASM_OUT)
examples: $(EXAMPLES)

# Recipes
## LIB Recipes

$(LIB_DIST)/%.o: $(LIB_SRC)/%.c | $(LIB_DIST) $(DEPDIR)/lib
	@$(CC) $(LIB_CFLAGS) $(DEPFLAGS) $(DEPDIR)/lib/$*.d -c $< -o $@ $(LIBS)
	@echo "$(TERM_YELLOW)$@$(TERM_RESET): $<"

## VM Recipes
$(VM_OUT): $(LIB_OBJECTS) $(VM_OBJECTS) $(VM_DIST)/main.o
	@$(CC) $(VM_CFLAGS) $^ -o $@ $(LIBS)
	@echo "$(TERM_GREEN)$@$(TERM_RESET): $^"

$(VM_DIST)/%.o: $(VM_SRC)/%.c | $(VM_DIST) $(DEPDIR)/vm
	@$(CC) $(VM_CFLAGS) $(DEPFLAGS) $(DEPDIR)/vm/$*.d -c $< -o $@ $(LIBS)
	@echo "$(TERM_YELLOW)$@$(TERM_RESET): $<"

## ASSEMBLY Recipes
$(ASM_OUT): $(LIB_OBJECTS) $(ASM_OBJECTS) $(ASM_DIST)/main.o
	@$(CPP) $(ASM_CFLAGS) $^ -o $@ $(LIBS)
	@echo "$(TERM_GREEN)$@$(TERM_RESET): $^"

$(ASM_DIST)/%.o: $(ASM_SRC)/%.cpp | $(ASM_DIST) $(DEPDIR)/asm
	@$(CPP) $(ASM_CFLAGS) $(DEPFLAGS) $(DEPDIR)/asm/$*.d -c $< -o $@ $(LIBS)
	@echo "$(TERM_YELLOW)$@$(TERM_RESET): $<"

## EXAMPLES recipes
$(EXAMPLES_DIST)/%.out: $(EXAMPLES_SRC)/%.asm $(ASM_OUT) | $(EXAMPLES_DIST)
	@$(ASM_OUT) $< $@
	@echo "$(TERM_GREEN)$@$(TERM_RESET): $<"

.PHONY: run-examples
run-examples: $(EXAMPLES)
	@$(foreach example,$(EXAMPLES), echo "$(TERM_YELLOW)$(example)$(TERM_RESET)"; $(MAKE) -s interpret BYTECODE=$(example);)

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
.PHONY: assemble
assemble: $(ASM_OUT)
	@$(ASM_OUT) $(SOURCE) $(BYTECODE)

.PHONY: interpret
interpret: $(VM_OUT)
	@$(VM_OUT) $(BYTECODE)

.PHONY: exec
exec: $(ASM_OUT) $(VM_OUT)
	@$(ASM_OUT) $(SOURCE) $(BYTECODE)
	@$(VM_OUT) $(BYTECODE)

# Directories
$(DIST):
	mkdir -p $@

$(LIB_DIST):
	mkdir -p $@

$(VM_DIST):
	mkdir -p $@

$(ASM_DIST):
	mkdir -p $@

$(EXAMPLES_DIST):
	mkdir -p $@

$(DEPDIR)/lib:
	mkdir -p $@

$(DEPDIR)/asm:
	mkdir -p $@

$(DEPDIR)/vm:
	mkdir -p $@

-include $(wildcard $(DEPS))
