BUILD_TYPE ?= debug
BUILD_TYPE_LOWER := $(shell echo $(BUILD_TYPE) | tr '[:upper:]' '[:lower:]')
CC = clang++
# CXX := clang++
CXXFLAGS := -Wall --std=c++17

CXXFLAGS += $(shell pkg-config --cflags icu-uc icu-io)
CXXFLAGS += -I./pindf
CXXFLAGS += $(shell pkg-config --cflags nlohmann_json)

LD_FLAGS += -L./pindf -lpindf
LD_FLAGS += $(shell pkg-config --libs icu-uc icu-io)

SRC := destination.cpp name_tree.cpp outline.cpp page_tree.cpp utils.cpp
OBJS := destination.o name_tree.o outline.o page_tree.o utils.o

ifeq ($(BUILD_TYPE_LOWER),debug)
CXXFLAGS += -g
endif

all: main

_pindf:
	make -C pindf BUILD_TYPE=$(BUILD_TYPE)

_build_o: $(SRC) _pindf
	$(CC) $(CXXFLAGS) -c ${SRC}

test_extract: _pindf test_extract.cpp $(OBJS)
	$(CC) $(CXXFLAGS) -c test_extract.cpp
	$(CC) $(CXXFLAGS) $(LD_FLAGS) $(OBJS) test_extract.o -o test_extract

test_modif: _pindf test_modif.cpp $(OBJS)
	$(CC) $(CXXFLAGS) -c test_modif.cpp
	$(CC) $(CXXFLAGS) $(LD_FLAGS) $(OBJS) test_modif.o -o test_modif

test: test_extract test_modif

main: _pindf main.cpp $(OBJS)
	$(CC) $(CXXFLAGS) -c main.cpp -o pdf-outline.o
	$(CC) $(CXXFLAGS) $(LD_FLAGS) $(OBJS) pdf-outline.o -o pdf-outline

clean:
	make -C pindf BUILD_TYPE=$(BUILD_TYPE) clean
	rm -f test_extract
	rm -f test_modif
	rm -f pdf-outline
	rm -f *.o
	rm -f *.pch
	rm -rf *.dSYM

.PHONY: all test clean main