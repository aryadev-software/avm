CC=gcc
VERBOSE=0
RELEASE=1

GENERAL-FLAGS:=-Wall -Wextra -Wswitch-enum -I$(shell pwd) -std=c11
DEBUG-FLAGS=-ggdb
RELEASE-FLAGS=-O3
FSAN-FLAGS=-fsanitize=address -fsanitize=undefined

ifeq ($(RELEASE),1)
TFLAGS:=$(GENERAL-FLAGS) $(FSAN-FLAGS) $(RELEASE-FLAGS) -DVERBOSE=$(VERBOSE)
CFLAGS:=$(GENERAL-FLAGS) -pedantic $(RELEASE-FLAGS) -DVERBOSE=$(VERBOSE)
else
TFLAGS:=$(GENERAL-FLAGS) $(FSAN-FLAGS) $(DEBUG-FLAGS) -DVERBOSE=$(VERBOSE)
CFLAGS:=$(GENERAL-FLAGS) -pedantic $(DEBUG-FLAGS) -DVERBOSE=$(VERBOSE)
endif

LIBS=
DIST=build

# Setup variables for source code, output, etc
## Lib setup
LIB_DIST=$(DIST)/lib
LIB_SRC=lib
LIB_CODE:=$(addprefix $(LIB_SRC)/, base.c darr.c heap.c inst.c bytecode.c)
LIB_OBJECTS:=$(LIB_CODE:$(LIB_SRC)/%.c=$(LIB_DIST)/%.o)
LIB_OUT=$(DIST)/libavm.so

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

lib: $(LIB_OBJECTS) $(LIB_OUT)
vm: $(VM_OUT)
tests: $(TEST_LIB_OUT)

# Recipes
$(LIB_DIST)/base.o: $(LIB_SRC)/base.c | $(LIB_DIST) $(DEPDIR)/lib
	$(CC) $(CFLAGS) -fPIC $(DEPFLAGS) $(DEPDIR)/lib/base.d -c $< -o $@ $(LIBS)

$(LIB_DIST)/inst.o: $(LIB_SRC)/inst.c | $(LIB_DIST) $(DEPDIR)/lib
	$(CC) $(CFLAGS) -fPIC $(DEPFLAGS) $(DEPDIR)/lib/inst.d -c $< -o $@ $(LIBS)

$(LIB_DIST)/bytecode.o: $(LIB_SRC)/bytecode.c | $(LIB_DIST) $(DEPDIR)/lib
	$(CC) $(CFLAGS) -fPIC $(DEPFLAGS) $(DEPDIR)/lib/bytecode.d -c $< -o $@ $(LIBS)

$(LIB_DIST)/%.o: $(LIB_SRC)/%.c | $(LIB_DIST) $(DEPDIR)/lib
	$(CC) $(CFLAGS) $(DEPFLAGS) $(DEPDIR)/lib/$*.d -c $< -o $@ $(LIBS)

$(LIB_OUT): $(LIB_DIST)/base.o $(LIB_DIST)/inst.o $(LIB_DIST)/bytecode.o
	$(CC) $(CFLAGS) -shared $^ -o $@ $(LIBS)

$(VM_OUT): $(LIB_OBJECTS) $(VM_OBJECTS) $(VM_DIST)/main.o
ifeq ($(RELEASE), 1)
	$(CC) $(CFLAGS) $^ -o $@ $(LIBS)
else
	$(CC) $(CFLAGS) $(FSAN-FLAGS) $^ -o $@ $(LIBS)
endif

$(VM_DIST)/%.o: $(VM_SRC)/%.c | $(VM_DIST) $(DEPDIR)/vm
ifeq ($(RELEASE), 1)
	$(CC) $(CFLAGS) $(DEPFLAGS) $(DEPDIR)/vm/$*.d -c $< -o $@ $(LIBS)
else
	$(CC) $(CFLAGS) $(FSAN-FLAGS) $(DEPFLAGS) $(DEPDIR)/vm/$*.d -c $< -o $@ $(LIBS)
endif

$(TEST_LIB_OUT): $(LIB_OBJECTS) $(TEST_LIB_DIST)/main.o
	$(CC) $(TFLAGS) $^ -o $@ $(LIBS)

$(TEST_LIB_DIST)/main.o: $(TEST_LIB_SRC)/main.c | $(TEST_LIB_DIST) $(DEPDIR)/test/lib
	$(CC) $(TFLAGS) $(DEPFLAGS) $(DEPDIR)/test/lib/main.d -c $< -o $@ $(LIBS)

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
	$(VM_OUT) $(BYTECODE)

TEST-LIB-ARGS=
.PHONY: run-test-lib
.ONESHELL:
run-test-lib: $(TEST_LIB_OUT)
	@echo "$(TERM_YELLOW)test/lib$(TERM_RESET): Starting tests"
	./$^ $(TEST-LIB-ARGS);
	if [ $$? -ne 0 ];
	then
		echo "$(TERM_RED)test/lib$(TERM_RESET): Tests failed";
	else
		echo "$(TERM_GREEN)test/lib$(TERM_RESET): Tests passed";
	fi

# Directories
$(DIST):
	@mkdir -p $@

$(LIB_DIST):
	@mkdir -p $@

$(VM_DIST):
	@mkdir -p $@

$(TEST_LIB_DIST):
	@mkdir -p $@

$(DEPDIR)/lib:
	@mkdir -p $@

$(DEPDIR)/vm:
	@mkdir -p $@

$(DEPDIR)/test/lib:
	@mkdir -p $@

-include $(wildcard $(DEPS))
