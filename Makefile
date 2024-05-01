CC=gcc
VERBOSE=0
GENERAL-FLAGS:=-Wall -Wextra -Werror -Wswitch-enum -I$(shell pwd) -std=c11
DEBUG-FLAGS=-ggdb -fsanitize=address -fsanitize=undefined
RELEASE-FLAGS=-O3

CFLAGS:=$(GENERAL-FLAGS) $(DEBUG-FLAGS) -D VERBOSE=$(VERBOSE)

LIBS=-lm
DIST=build
TERM_YELLOW:=$(shell echo -e "\e[0;33m")
TERM_RED:=$(shell echo -e "\e[0;31m")
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

## Test setup
TEST_DIST=$(DIST)/test
TEST_SRC=test

TEST_LIB_SRC=$(TEST_SRC)/lib
TEST_LIB_DIST=$(TEST_DIST)/lib
TEST_LIB_OUT=$(DIST)/test-lib.out

## Dependencies
DEPDIR:=$(DIST)/dependencies
DEPFLAGS = -MT $@ -MMD -MP -MF
DEPS:=$(LIB_CODE:$(LIB_SRC)/%.c=$(DEPDIR)/lib/%.d) $(VM_CODE:$(VM_SRC)/%.c=$(DEPDIR)/vm/%.d) $(DEPDIR)/vm/main.d $(DEPDIR)/test/lib/main.d

# Things you want to build on `make`
all: $(DIST) lib vm tests

lib: $(LIB_OBJECTS)
vm: $(VM_OUT)
tests: $(TEST_LIB_OUT)

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

$(TEST_LIB_OUT): $(LIB_OBJECTS) $(TEST_LIB_DIST)/main.o
	@$(CC) $(CFLAGS) $^ -o $@ $(LIBS)
	@echo "$(TERM_GREEN)$@$(TERM_RESET): $^"

$(TEST_LIB_DIST)/main.o: $(TEST_LIB_SRC)/main.c | $(TEST_LIB_DIST) $(DEPDIR)/test/lib
	@$(CC) $(CFLAGS) $(DEPFLAGS) $(DEPDIR)/test/lib/main.d -c $< -o $@ $(LIBS)
	@echo "$(TERM_YELLOW)$@$(TERM_RESET): $<"

.PHONY: test
test: run-test-lib

.PHONY: run
run: $(DIST)/$(VM_OUT)
	./$^ $(ARGS)

.PHONY: clean
clean:
	rm -rfv $(DIST)/*

.PHONY: interpret
interpret: $(VM_OUT)
	@$(VM_OUT) $(BYTECODE)

.PHONY: run-test-lib
.ONESHELL:
run-test-lib: $(TEST_LIB_OUT)
	@echo "$(TERM_YELLOW)test/lib$(TERM_RESET): Starting tests"
	@./$^;
	if [ $$? -ne 0 ];
	then
		echo "$(TERM_RED)test/lib$(TERM_RESET): Tests failed";
	else
		echo "$(TERM_GREEN)test/lib$(TERM_RESET): Tests passed";
	fi

# Directories
$(DIST):
	mkdir -p $@

$(LIB_DIST):
	mkdir -p $@

$(VM_DIST):
	mkdir -p $@

$(TEST_LIB_DIST):
	mkdir -p $@

$(DEPDIR)/lib:
	mkdir -p $@

$(DEPDIR)/vm:
	mkdir -p $@

$(DEPDIR)/test/lib:
	mkdir -p $@

-include $(wildcard $(DEPS))
