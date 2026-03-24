# genql_c/Makefile
# Build the GenQL C implementation: static library, demo, and test suite.
#
# Targets
# -------
#   make all          Build everything (default)
#   make test         Build and run the test suite
#   make clean        Remove all build artefacts

CC      = gcc
CFLAGS  = -Wall -Wextra -std=c11 -g -I.

SOURCES = strbuf.c \
          kv.c \
          intent.c \
          roles.c \
          absorber.c \
          compiler.c \
          sql_compiler.c \
          python_compiler.c \
          schema_compiler.c \
          policy_compiler.c \
          workflow_compiler.c \
          coordinator.c

OBJECTS  = $(SOURCES:.c=.o)
LIB      = libgenql.a
DEMO     = genql_demo
TESTS    = test_genql

.PHONY: all test clean

all: $(LIB) $(DEMO) $(TESTS)

# Static library
$(LIB): $(OBJECTS)
	ar rcs $@ $^

# Demo executable
$(DEMO): main.o $(LIB)
	$(CC) $(CFLAGS) -o $@ $^

# Test executable
$(TESTS): test_genql.o $(LIB)
	$(CC) $(CFLAGS) -o $@ $^

# Generic object-file rule
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Build and run tests
test: $(TESTS)
	./$(TESTS)

clean:
	rm -f $(OBJECTS) main.o test_genql.o $(LIB) $(DEMO) $(TESTS)
