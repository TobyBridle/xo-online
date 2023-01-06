CC=gcc
INCDIRS=-I./src/lib -I./src/ -I./bin
OPT=-O0
CFLAGS=-Wall -Wextra -g -DDEBUG $(INCDIRS) $(OPT)
TESTS_DIR=./tests

# We want to compile all with CFLAGS and -g and -DDEBUG
# Release builds should be compiled with -O3 and just the CFLAGS
all: server client tests

server: bin/server.o bin/utils.o
	$(CC) $(CFLAGS) bin/utils.o bin/server.o -o bin/server
	@echo "\033[32;1mDone Compiling Server\033[0m"

client: bin/client.o bin/utils.o
	$(CC) $(CFLAGS) bin/utils.o bin/client.o -o bin/client
	@echo "\033[32;1mDone Compiling Client\033[0m"

# TODO: Don't explicitly include utils.o - manage dependencies automatically
tests: $(TESTS_DIR)/bin/generics.o bin/utils.o $(wildcard $(TESTS_DIR)/bin/*.o)
	$(foreach test,$(filter-out $(TESTS_DIR)/generics.c, $(wildcard $(TESTS_DIR)/*.c)),$(CC) $(CFLAGS) $(TESTS_DIR)/bin/generics.o bin/utils.o $(test) -o $(patsubst $(TESTS_DIR)/%.c,$(TESTS_DIR)/bin/%,$(test)) &&) true
	@echo "\033[32;1mDone Compiling Tests\033[0m"

test: tests
	@echo "\033[32;1mRunning Tests\033[0m"
	@$(foreach test,$(filter-out $(TESTS_DIR)/generics.c, $(wildcard $(TESTS_DIR)/*.c)),$(TESTS_DIR)/bin/$(patsubst $(TESTS_DIR)/%.c,%,$(test)) &&) true

$(TESTS_DIR)/bin/generics.o: $(TESTS_DIR)/generics.c
	$(CC) $(CFLAGS) -c $(TESTS_DIR)/generics.c -o $(TESTS_DIR)/bin/generics.o

$(TESTS_DIR)/bin/%.o: $(TESTS_DIR)/%.c
	$(CC) $(CFLAGS) -c -o $(TESTS_DIR)/bin/$@ $<

bin/%.o: src/%.c
	$(CC) $(CFLAGS) -c -o $@ $^

clean: bin tests/bin
	@rm -rf bin/*
	@rm -rf tests/bin/*
