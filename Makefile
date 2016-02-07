
CC=g++
LD=ld
OS:=$(shell uname -s)
ROOT_PATH:=$(shell pwd)
THIRD_LIB_PATH=$(ROOT_PATH)/3rdparty/lib
INCLUDE_PATH:=-I$(ROOT_PATH)/3rdparty/include/ -I$(ROOT_PATH)/common/ -I$(ROOT_PATH)/utility/ -I$(ROOT_PATH)/core_src
LOAD_LIB:= -lpthread
STRICT_CCFLAGS:=-Wall -Wextra -Werror -Wconversion -Wno-unused-parameter -Wold-style-cast -Woverloaded-virtual -Wpointer-arith -Wshadow -Wwrite-strings
CCFlags:=--std=c++11 -g -march=native -O2 -finline-limit=1000 -DNDEBUG -DUNIT_TEST

prerun:

ifeq ($(OS),Linux)
LOAD_LIB += -lrt
LINK_DYNAMIC ?= -rdynamic
LD_LIBRARY_PATH?= /usr/lib/:/usr/lib64/:/usr/local/lib/:/usr/local/lib64/:/usr/lib/x86_64-linux-gnu/
else ifeq ($(OS),Darwin)
CCFlags+= -D__MACH__
else
$(warning $(OS))
endif

LIB_FLAGS:=-L$(THIRD_LIB_PATH) $(LOAD_LIB) -Wl,-rpath,$(ROOT_PATH)/3rdparty/lib/
MUDUO_CCFLAGS:= -DMUDUO_STD_STRING -DCHECK_PTHREAD_RETURN_VALUE -D_FILE_OFFSET_BITS=64
MUDUO_LDFLAGS:= $(LINK_DYNAMIC) $(THIRD_LIB_PATH)/libmuduo_net.a $(THIRD_LIB_PATH)/libmuduo_base.a
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
	$(CC) $(CCFlags) $(MUDUO_CCFLAGS) $(ALL_OBJS) -o adservice $(MUDUO_LDFLAGS) $(LIB_FLAGS)
all:prerun init unit_test.o
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
