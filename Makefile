
CC=g++
LD=ld
OS:=$(shell uname -s)
ROOT_PATH:=$(shell pwd)
THIRD_LIB_PATH=$(ROOT_PATH)/3rdparty/lib
INCLUDE_PATH:=-I$(ROOT_PATH)/3rdparty/include/ -I$(ROOT_PATH)/common/ -I$(ROOT_PATH)/utility/ -I$(ROOT_PATH)/core_src -I$(ROOT_PATH)/
LOAD_LIB:= -lpthread -lavrocpp -lonsclient4cpp -lssl -lcrypto
STRICT_CCFLAGS:=-Wall -Wextra -Werror -Wconversion -Wno-unused-parameter -Wold-style-cast -Woverloaded-virtual -Wpointer-arith -Wshadow -Wwrite-strings
CCFlags:=--std=c++11 -g -march=native -O2 -finline-limit=1000 -DNDEBUG -DUNIT_TEST -DMUDUO_STD_STRING
CCFlags+= -DUSE_KAFKA_LOG
CCFlags+= -DUSE_SHORT_CONN
#CCFlags+= -DUSE_ENCODING_GZIP
#CCFlags+= -DUSE_ALIYUN_LOG
LOAD_LIB+= -lrdkafka++ -lz
prerun:

ifeq ($(OS),Linux)
CCFlags+= -Dlinux
LOAD_LIB += -lrt -ldl
LINK_DYNAMIC ?= -rdynamic
LD_LIBRARY_PATH?= /usr/lib/:/usr/lib64/:/usr/local/lib/:/usr/local/lib64/:/usr/lib/x86_64-linux-gnu/:/usr/local/ssl/lib/
else ifeq ($(OS),Darwin)
CCFlags+= -D__MACH__
else
$(warning $(OS))
endif

LIB_FLAGS:=-L$(THIRD_LIB_PATH) $(LOAD_LIB) -Wl,-rpath,$(ROOT_PATH)/3rdparty/lib/
MUDUO_CCFLAGS:= -DMUDUO_STD_STRING -DCHECK_PTHREAD_RETURN_VALUE -D_FILE_OFFSET_BITS=64
MUDUO_LDFLAGS:= $(LINK_DYNAMIC) $(THIRD_LIB_PATH)/libmuduo_http.a $(THIRD_LIB_PATH)/libmuduo_net_cpp11.a $(THIRD_LIB_PATH)/libmuduo_base_cpp11.a
#-DVERBOSE_DEBUG
SRC_FOLDER:=$(shell pwd)

BUILD_PATH:= $(SRC_FOLDER)/build

ALL_OBJS:= unit_test.o \
	cypher.o \
	hash.o  \
	mttytime.o \
	json.o \
	url.o \
	file.o \
	escape.o \
	core.o \
	elasticsearch.o \
	platform.o \
	random.o \
	
init:
	mkdir -p $(BUILD_PATH)
final:
	cd $(BUILD_PATH) && \
	$(CC) $(CCFlags) $(MUDUO_CCFLAGS) $(ALL_OBJS) -o ../adservice $(MUDUO_LDFLAGS) $(LIB_FLAGS)
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
			mttytime.cpp \
			json.cpp \
			url.cpp \
			escape.cpp \
			file.cpp \
			random.cpp
utility.o:
	cd $(UTILITY_FOLDER) && \
	$(CC) $(CCFlags) $(INCLUDE_PATH) -c $(UTILITY_BUILD_SOURCE) && \
	mv $(UTILITY_FOLDER)/*.o $(BUILD_PATH)/

CORE_FOLDER:=$(SRC_FOLDER)/core_src
CORE_BUILD_SOURCE:= $(wildcard $(CORE_FOLDER)/*.cpp)
CORE_BUILD_SOURCE+= $(wildcard $(CORE_FOLDER)/net/*.cpp)
core.o:elasticsearch.o platform.o
	cd $(CORE_FOLDER) && \
	$(CC) -c $(CCFlags) $(INCLUDE_PATH) $(CORE_BUILD_SOURCE) && \
	$(LD) -r *.o -o $(BUILD_PATH)/core.o && \
	rm *.o

ES_FOLDER:=$(SRC_FOLDER)/elasticsearch
ES_SOURCE:= $(wildcard $(ES_FOLDER)/*.cpp)
elasticsearch.o:
	cd $(ES_FOLDER) && \
	$(CC) -c $(CCFlags) $(INCLUDE_PATH) $(ES_SOURCE) && \
	$(LD) -r *.o -o $(BUILD_PATH)/elasticsearch.o && \
	rm *.o

PLATFORM_FOLDER:=$(SRC_FOLDER)/protocol
PLATFORM_SOURCE:= $(wildcard $(PLATFORM_FOLDER)/baidu/*.cpp)
PLATFORM_SOURCE+= $(wildcard $(PLATFORM_FOLDER)/tanx/*.cpp)
platform.o:
	cd $(PLATFORM_FOLDER) && \
	$(CC) -c $(CCFlags) -Wno-narrowing  $(INCLUDE_PATH) $(PLATFORM_SOURCE) && \
	$(LD) -r *.o -o $(BUILD_PATH)/platform.o && \
	rm *.o
clean:
	rm -rf $(BUILD_PATH)
	rm adservice

JNI_FOLDER= jni/

export:
	cd $(JNI_FOLDER) && make jni

clean_jni:
	cd $(JNI_FOLDER) && make clean
