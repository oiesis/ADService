
CC=g++-5
LD=ld
ROOT_PATH:=$(shell pwd)
INCLUDE_PATH:=-I$(ROOT_PATH)/3rdparty/include/ -I$(ROOT_PATH)/common/ -I$(ROOT_PATH)/utility/ -I$(ROOT_PATH)/core_src
LIB_PATH:=-L$(ROOT_PATH)/3rdparty/lib/ -lmuduo_net -lmuduo_base -lmuduo_net_cpp11 -lmuduo_base_cpp11 -lpthread -Wl,-rpath,$(ROOT_PATH)/3rdparty/lib/
EXTRA_CCFLAGS:=-g -DMUDUO_STD_STRING -D_FILE_OFFSET_BITS=64
CCFlags:=--std=c++11
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
final:
	cd $(BUILD_PATH) && \
	$(CC) $(CCFlags) $(EXTRA_CCFLAGS)  $(LIB_PATH) $(ALL_OBJS) -o adservice
all:init unit_test.o
	$(MAKE) final

UNIT_TEST_FOLDER = $(SRC_FOLDER)/unit_test/

UNIT_TEST_SRC:= unit_test.cpp

unit_test.o:utility.o core.o
	cd $(UNIT_TEST_FOLDER) && \
	$(CC) -c $(CCFlags) $(INCLUDE_PATH) $(UNIT_TEST_SRC) -o $(BUILD_PATH)/unit_test.o

UTILITY_FOLDER:=$(SRC_FOLDER)/utility/

UTILITY_BUILD_SOURCE:= cypher.cpp \
			hash.cpp \
			time.cpp \
			json.cpp
utility.o:
	cd $(UTILITY_FOLDER) && \
	$(CC) $(CCFlags) $(INCLUDE_PATH) -c $(UTILITY_BUILD_SOURCE) && \
	mv $(UTILITY_FOLDER)/*.o $(BUILD_PATH)/

CORE_FOLDER:=$(SRC_FOLDER)/core_src
CORE_BUILD_SOURCE:= $(wildcard $(CORE_FOLDER)/*.cpp)
CORE_BUILD_SOURCE+= $(wildcard $(CORE_FOLDER)/net/*.cpp)
core.o:
	cd $(CORE_FOLDER) && \
	$(CC) -c $(CCFlags) $(INCLUDE_PATH) $(CORE_BUILD_SOURCE) && \
	$(LD) -r *.o -o $(BUILD_PATH)/core.o && \
	rm *.o

clean:
	rm -rf $(BUILD_PATH)
