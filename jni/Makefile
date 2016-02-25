
CC=g++
CCFlags:= -fPIC -D_REENTRANT --std=c++11
#JAVA_VERSION:=$(shell java --version)
JAVA_VERSION?=1.8.0
ifeq ($(JAVA_HOME),)
JAVA_HOME:=/usr/lib/jvm/java-$(JAVA_VERSION)
endif

INCLUDE_PATH= -I$(JAVA_HOME)/include -I$(JAVA_HOME)/include/linux

JNI_SRC_FOLDER:= ./src
JNI_SRC:=$(wildcard $(JNI_SRC_FOLDER)/*.cpp)

SHARE_FLAGS:= -shared
TARGET_FOLDER:= lib/

share:
	$(CC) $(SHARE_FLAGS) $(TARGET_FOLDER)/*.o -o $(TARGET_FOLDER)/libmttyutil.so

jni:AliBinaryEscaper.cpp
	cd $(JNI_SRC_FOLDER) && \
	$(CC) $(CCFlags) $(INCLUDE_PATH) -c $< 
	mv $(JNI_SRC_FOLDER)/*.o $(TARGET_FOLDER)/
	make share

AliBinaryEscaper.cpp:../utility/escape.cpp
	$(CC) $(CCFlags) -I../ -I../common/ -c $< -o $(TARGET_FOLDER)/escape.o

clean:
	cd $(TARGET_FOLDER) && \
	rm *.o && \
	rm *.so
