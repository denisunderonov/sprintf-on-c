CC = gcc
AR = ar
ARFLAGS = rcs
CFLAGS = -std=c11 -Wall -Wextra -Werror -pedantic
LIB = s21_string.a
SRC = s21_mem.c s21_str.c s21_sprintf.c
OBJ = $(SRC:.c=.o)
TEST_SRC = tests/s21_sprintf_test.c tests/s21_string_test.c
TEST_BIN = tests/s21_sprintf_test
UNAME_S := $(shell uname -s)
TEST_CFLAGS = $(CFLAGS)
ifeq ($(UNAME_S),Darwin)
TEST_CFLAGS += -Wno-gnu-zero-variadic-macro-arguments
endif
CHECK_CFLAGS := $(shell pkg-config --cflags check 2>/dev/null)
CHECK_LIBS := $(shell pkg-config --libs check 2>/dev/null)
ifeq ($(strip $(CHECK_LIBS)),)
CHECK_LIBS = -lcheck
ifeq ($(UNAME_S),Linux)
CHECK_LIBS += -lsubunit -lpthread -lrt
endif
endif
CHECK_LIBS += -lm
GCOV_FLAGS = --coverage
GCOV_OBJ = $(SRC:.c=.gcov.o)
GCOV_BIN = tests/s21_sprintf_gcov_test

.PHONY: all clean test gcov_report clean_coverage

all: $(LIB)

$(LIB): $(OBJ)
	$(AR) $(ARFLAGS) $@ $^

%.o: %.c s21_string.h
	$(CC) $(CFLAGS) -c $< -o $@

$(TEST_BIN): $(TEST_SRC) $(LIB)
	$(CC) $(TEST_CFLAGS) $(CHECK_CFLAGS) $(TEST_SRC) $(LIB) $(CHECK_LIBS) -o $@

test: $(TEST_BIN)
	./$(TEST_BIN)

%.gcov.o: %.c s21_string.h
	$(CC) $(CFLAGS) $(GCOV_FLAGS) -c $< -o $@

$(GCOV_BIN): $(TEST_SRC) $(GCOV_OBJ)
	$(CC) $(TEST_CFLAGS) $(GCOV_FLAGS) $(CHECK_CFLAGS) $^ $(CHECK_LIBS) -o $@

gcov_report: clean_coverage
	$(MAKE) $(GCOV_BIN)
	./$(GCOV_BIN)
	lcov --capture --directory . --output-file coverage.info --no-external
	lcov --remove coverage.info '*/tests/*' --output-file coverage.info
	genhtml coverage.info --output-directory report

clean_coverage:
	rm -f $(GCOV_OBJ) $(GCOV_OBJ:.o=.gcno) $(GCOV_OBJ:.o=.gcda) $(GCOV_BIN)
	rm -f *.gcno *.gcda tests/*.gcno tests/*.gcda
	rm -f coverage.info
	rm -rf report

clean: clean_coverage
	rm -f $(OBJ) $(LIB) $(TEST_BIN)
