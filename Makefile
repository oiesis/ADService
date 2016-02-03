
CC=g++-5
ROOT_PATH:=$(shell pwd)
INCLUDE_PATH:=-I$(ROOT_PATH)/3rdparty/include/ -I$(ROOT_PATH)/common/ -I$(ROOT_PATH)/utility/
LIB_PATH:=-lmuduo_net -lmuduo_base -Wl,-rpath,./3rdparty/lib/
CCFlags:=--std=c++11 $(INCLUDE_PATH) $(LIB_PATH)
#-DVERBOSE_DEBUG
SRC_FOLDER:=$(shell pwd)

BUILD_PATH:= $(SRC_FOLDER)/build

ALL_OBJS:= unit_test.o \
	cypher.o \
	hash.o  \
	time.o \
	json.o \
	core.o \

init:
	mkdir -p $(BUILD_PATH)
all:init unit_test.o
	cd $(BUILD_PATH) && \
	$(CC) $(CCFlags) $(ALL_OBJS) -o adservice

UNIT_TEST_FOLDER = $(SRC_FOLDER)/unit_test/

UNIT_TEST_SRC:= unit_test.cpp

unit_test.o:utility.o core.o
	cd $(UNIT_TEST_FOLDER) && \
	$(CC) -c $(CCFlags) $(UNIT_TEST_SRC) -o $(BUILD_PATH)/unit_test.o

UTILITY_FOLDER:=$(SRC_FOLDER)/utility/

UTILITY_BUILD_SOURCE:= cypher.cpp \
			hash.cpp \
			time.cpp \
			json.cpp
utility.o:
	cd $(UTILITY_FOLDER) && \
	$(CC) $(CCFlags) -c $(UTILITY_BUILD_SOURCE) && \
	mv $(UTILITY_FOLDER)/*.o $(BUILD_PATH)/

CORE_FOLDER:=$(SRC_FOLDER)/core_src
CORE_BUILD_SOURCE:= $(wildcard $(CORE_FOLDER)/*/*.cpp)
core.o:
    cd $(CORE_FOLDER) && \
    $(CC) $(CCFlags) -c $(CORE_BUILD_SOURCE) -o $(BUILD_PATH)/core.o

clean:
	rm -rf $(BUILD_PATH)
