CC=gcc
VERBOSE=0
GENERAL-FLAGS:=-Wall -Wextra -Werror -Wswitch-enum -I$(shell pwd)
DEBUG-FLAGS=-ggdb -fsanitize=address
RELEASE-FLAGS=-O3

CFLAGS:=$(GENERAL-FLAGS) -std=c11 $(DEBUG-FLAGS) -D VERBOSE=$(VERBOSE)

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

## VM setup
VM_DIST=$(DIST)/vm
VM_SRC=vm
VM_CODE:=$(addprefix $(VM_SRC)/, struct.c runtime.c)
VM_OBJECTS:=$(VM_CODE:$(VM_SRC)/%.c=$(VM_DIST)/%.o)
VM_OUT=$(DIST)/avm.out

## Dependencies
DEPDIR:=$(DIST)/dependencies
DEPFLAGS = -MT $@ -MMD -MP -MF
DEPS:=$(LIB_CODE:$(LIB_SRC)/%.c=$(DEPDIR)/lib/%.d) $(VM_CODE:$(VM_SRC)/%.c=$(DEPDIR)/vm/%.d) $(DEPDIR)/vm/main.d

# Things you want to build on `make`
all: $(DIST) lib vm

lib: $(LIB_OBJECTS)
vm: $(VM_OUT)

# Recipes
$(LIB_DIST)/%.o: $(LIB_SRC)/%.c | $(LIB_DIST) $(DEPDIR)/lib
	@$(CC) $(CFLAGS) $(DEPFLAGS) $(DEPDIR)/lib/$*.d -c $< -o $@ $(LIBS)
	@echo "$(TERM_YELLOW)$@$(TERM_RESET): $<"

$(VM_OUT): $(LIB_OBJECTS) $(VM_OBJECTS) $(VM_DIST)/main.o
	@$(CC) $(CFLAGS) $^ -o $@ $(LIBS)
	@echo "$(TERM_GREEN)$@$(TERM_RESET): $^"

$(VM_DIST)/%.o: $(VM_SRC)/%.c | $(VM_DIST) $(DEPDIR)/vm
	@$(CC) $(CFLAGS) $(DEPFLAGS) $(DEPDIR)/vm/$*.d -c $< -o $@ $(LIBS)
	@echo "$(TERM_YELLOW)$@$(TERM_RESET): $<"

.PHONY: run
run: $(DIST)/$(VM_OUT)
	./$^ $(ARGS)

.PHONY: clean
clean:
	rm -rfv $(DIST)/*

.PHONY: interpret
interpret: $(VM_OUT)
	@$(VM_OUT) $(BYTECODE)

# Directories
$(DIST):
	mkdir -p $@

$(LIB_DIST):
	mkdir -p $@

$(VM_DIST):
	mkdir -p $@

$(DEPDIR)/lib:
	mkdir -p $@

$(DEPDIR)/vm:
	mkdir -p $@

-include $(wildcard $(DEPS))
