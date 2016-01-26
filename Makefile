
CC=g++
CCFlags=--std=c++11
SRC_FOLDER=$(shell pwd)

BUILD_PATH = $(SRC_FOLDER)/build

ALL_OBJS = unit_test.o \
	cypher.o \
	hash.o  \
	time.o \

init:
	mkdir -p $(BUILD_PATH)
all:init unit_test.o
	cd $(BUILD_PATH) && \
	$(CC) $(CCFlags) $(ALL_OBJS) -o adservice

UNIT_TEST_FOLDER = $(SRC_FOLDER)/unit_test/

UNIT_TEST_SRC = unit_test.cpp

unit_test.o:utility.o
	cd $(UNIT_TEST_FOLDER) && \
	$(CC) -c $(CCFlags) $(UNIT_TEST_SRC) -o $(BUILD_PATH)/unit_test.o

UTILITY_FOLDER=$(SRC_FOLDER)/utility/

UTILITY_BUILD_SOURCE = cypher.cpp \
			hash.cpp \
			time.cpp
utility.o:
	cd $(UTILITY_FOLDER) && \
	$(CC) $(CCFlags) -c $(UTILITY_BUILD_SOURCE) && \
	mv $(UTILITY_FOLDER)/*.o $(BUILD_PATH)/
clean:
	rm -rf $(BUILD_PATH)
