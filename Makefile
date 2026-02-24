BUILD_TYPE ?= debug
BUILD_TYPE_LOWER := $(shell echo $(BUILD_TYPE) | tr '[:upper:]' '[:lower:]')
CC = llvm-g++ -stdlib=libc++ -std=c++17
# CXX := clang++
CXXFLAGS := -Wall --std=c++17

CXXFLAGS += $(shell pkg-config --cflags icu-uc icu-io)
CXXFLAGS += -I./pindf
CXXFLAGS += $(shell pkg-config --cflags nlohmann_json)

LD_FLAGS += -L./pindf -lpindf
LD_FLAGS += $(shell pkg-config --libs icu-uc icu-io)

SRC := destination.cpp name_tree.cpp outline.cpp page_tree.cpp utils.cpp

ifeq ($(BUILD_TYPE_LOWER),debug)
CXXFLAGS += -g
endif

all:
	make -C pindf BUILD_TYPE=$(BUILD_TYPE)
	$(CC) $(CXXFLAGS) $(LD_FLAGS) $(SRC) test_extract.cpp -o test_extract
	$(CC) $(CXXFLAGS) $(LD_FLAGS) $(SRC) test_modif.cpp -o test_modif
# 	$(CC) $(CXXFLAGS) $(LD_FLAGS) -o pdf-outline *.o

clean:
	make -C pindf BUILD_TYPE=$(BUILD_TYPE) clean
	rm -f test_extract
	rm -f test_modif
	rm -f *.o
	rm -f *.pch
	rm -rf *.dSYM