BUILD_TYPE ?= debug
BUILD_TYPE_LOWER := $(shell echo $(BUILD_TYPE) | tr '[:upper:]' '[:lower:]')
CXX := clang++
CXXFLAGS := -std=c++17 -Wall

CXXFLAGS += $(shell pkg-config --cflags icu-uc icu-io)
CXXFLAGS += -I./pindf

LD_FLAGS += -L./pindf -lpindf
LD_FLAGS += $(shell pkg-config --libs icu-uc icu-io)

SRC := *.cpp

ifeq ($(BUILD_TYPE_LOWER),debug)
CXXFLAGS += -g
endif

all:
	make -C pindf BUILD_TYPE=$(BUILD_TYPE)
	$(CXX) $(CXXFLAGS) -std=c++17 -c $(SRC)
	$(CXX) $(CXXFLAGS) $(LD_FLAGS) -o pdf-outline *.o

clean:
	make -C pindf BUILD_TYPE=$(BUILD_TYPE) clean
	rm -f pdf-outline
	rm -f *.o
	rm -f *.pch
	rm -rf *.dSYM